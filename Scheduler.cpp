/*
 *  VieSched++ Very Long Baseline Interferometry (VLBI) Scheduling Software
 *  Copyright (C) 2018  Matthias Schartner
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * File:   Scheduler.cpp
 * Author: mschartn
 *
 * Created on June 29, 2017, 2:29 PM
 */

#include "Scheduler.h"


using namespace std;
using namespace VieVS;
unsigned long Scheduler::nextId = 0;


Scheduler::Scheduler( Initializer &init, string path, string fname )
    : VieVS_NamedObject( move( fname ), nextId++ ),
      version_{init.version_},
      path_{ std::move( path ) },
      network_{ std::move( init.network_ ) },
      sourceList_{ std::move( init.sourceList_ ) },
      himp_{ std::move( init.himp_ ) },
      calib_{ std::move( init.calib_ ) },
      multiSchedulingParameters_{ std::move( init.multiSchedulingParameters_ ) },
      xml_{ init.xml_ },
      obsModes_{ init.obsModes_ },
      currentObservingMode_{ obsModes_->getMode( 0 ) } {
    if ( init.parameters_.subnetting ) {
        if ( init.parameters_.subnettingMinNStaPercent_otherwiseAllBut ) {
            parameters_.subnetting = make_unique<Subnetting_percent>( init.preCalculated_.subnettingSrcIds,
                                                                      init.parameters_.subnettingMinNStaPercent );
        } else {
            parameters_.subnetting = make_unique<Subnetting_minIdle>( init.preCalculated_.subnettingSrcIds,
                                                                      init.parameters_.subnettingMinNStaAllBut );
        }
    }

    parameters_.fillinmodeDuringScanSelection = init.parameters_.fillinmodeDuringScanSelection;
    parameters_.fillinmodeInfluenceOnSchedule = init.parameters_.fillinmodeInfluenceOnSchedule;
    parameters_.fillinmodeAPosteriori = init.parameters_.fillinmodeAPosteriori;
    parameters_.fillinmodeAPosteriori_minSta = init.parameters_.fillinmodeAPosteriori_minSta;
    parameters_.fillinmodeAPosteriori_minRepeat = init.parameters_.fillinmodeAPosteriori_minRepeat;

    parameters_.idleToObservingTime = init.parameters_.idleToObservingTime;
    if ( parameters_.idleToObservingTime ) {
        parameters_.idleToObservingTime_staids =
            init.getMembers( init.parameters_.idleToObservingTimeGroup, network_.getStations() );
    }

    parameters_.andAsConditionCombination = init.parameters_.andAsConditionCombination;
    parameters_.minNumberOfSourcesToReduce = init.parameters_.minNumberOfSourcesToReduce;
    parameters_.maxNumberOfIterations = init.parameters_.maxNumberOfIterations;
    parameters_.numberOfGentleSourceReductions = init.parameters_.numberOfGentleSourceReductions;
    parameters_.reduceFactor = init.parameters_.reduceFactor;

    parameters_.writeSkyCoverageData = false;
    parameters_.doNotObserveSourcesWithinMinRepeat = init.parameters_.doNotObserveSourcesWithinMinRepeat;
    parameters_.ignoreSuccessiveScansSameSrc = init.parameters_.ignoreSuccessiveScansSameSrc;
}


Scheduler::Scheduler(std::string name, std::string path, Network network, SourceList sourceList,
                     std::vector<Scan> scans,
                     boost::property_tree::ptree xml, std::shared_ptr<ObservingMode> obsModes_ )
    : VieVS_NamedObject( move( name ), nextId++ ),
      version_{0},
      path_{std::move(path)},
      network_{ std::move( network ) },
      sourceList_{ std::move( sourceList ) },
      scans_{ std::move( scans ) },
      obsModes_{ std::move( obsModes_ ) },
      currentObservingMode_{ nullptr },
      xml_{ xml } {}


void Scheduler::startScanSelection( unsigned int endTime, std::ofstream &of, Scan::ScanType type,
                                    boost::optional<StationEndposition> &opt_endposition,
                                    boost::optional<Subcon> &opt_subcon, int depth ) {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "start scan selection (depth " << depth << ")";
#endif

    // necessary vectors for calibrator blocks
    vector<double> prevLowElevationScores( network_.getNSta(), 0 );
    vector<double> prevHighElevationScores( network_.getNSta(), 0 );
    vector<double> highestElevations( network_.getNSta(), numeric_limits<double>::min() );
    vector<double> lowestElevations( network_.getNSta(), numeric_limits<double>::max() );
    int countCalibratorScans = 0;

    if ( type == Scan::ScanType::astroCalibrator ) {
        writeCalibratorHeader( of );
    }

    // Check if there is a required opt_endposition. If yes change station availability with respect to opt_endposition
    if ( opt_endposition.is_initialized() ) {
        changeStationAvailability( opt_endposition, StationEndposition::change::start );
    }

    while ( true ) {
        // look if station is possible with respect to opt_endposition
        if ( opt_endposition.is_initialized() ) {
            if ( !opt_endposition->checkStationPossibility( network_.getStations() ) ) {
                break;
            }
        }

        // create a subcon with all possible next scans
        Subcon subcon;
        if ( opt_subcon.is_initialized() ) {
            // if you already have a subcon, check the endposition of each scan and generate subnetting scans and new
            // score.
            subcon = std::move( *opt_subcon );
            opt_subcon = boost::none;
            subcon.changeType( Scan::ScanType::fillin );
            subcon.checkIfEnoughTimeToReachEndposition( network_, sourceList_, opt_endposition );
            subcon.clearSubnettingScans();
            if ( parameters_.subnetting != nullptr ) {
                subcon.createSubnettingScans( parameters_.subnetting, network_, sourceList_ );
            }
        } else {
            // otherwise calculate new subcon
            subcon = createSubcon( parameters_.subnetting, type, opt_endposition );

            //            string fileName = path_ + getName();
            //            for ( const auto &sky : network_.getSkyCoverages() ) {
            //                string stations;
            //                unsigned int i = 0;
            //                for ( const auto &any : network_.getStaid2skyCoverageId() ) {
            //                    if ( any.second == sky.getId() ) {
            //                        i = network_.getStation( any.first ).getCurrentTime();
            //                        stations.append( network_.getStation( any.first ).getName() ).append( " " );
            //                    }
            //                }
            //                sky.generateDebuggingFiles( i, fileName, stations );
            //            }
        }

        // check algorithm focus corners for intensive sessions
        if ( FocusCorners::startFocusCorner && depth == 0 ) {
            of << boost::format( "| %=140s |\n" ) % "reweight sources to focus observation at corner";
            FocusCorners::reweight( subcon, sourceList_, of );
            ++FocusCorners::iscan;
            of << boost::format( "| %=140s |\n" ) %
                      ( boost::format( "Focus corner scan %d of %d" ) % FocusCorners::iscan % FocusCorners::nscans )
                          .str();
        }

        if ( type != Scan::ScanType::astroCalibrator ) {
            // standard case
            subcon.generateScore( network_, sourceList_ );
        } else {
            // special case for calibrator scans
            subcon.generateScore( prevLowElevationScores, prevHighElevationScores, network_, sourceList_ );
        }

        unsigned long nSingleScans = subcon.getNumberSingleScans();
        unsigned long nSubnettingScans = subcon.getNumberSubnettingScans();

        // select the best possible next scan(s) and save them under 'bestScans'
        vector<Scan> bestScans;
        if ( type != Scan::ScanType::astroCalibrator ) {
            // standard case
            bestScans = subcon.selectBest( network_, sourceList_, currentObservingMode_, opt_endposition );
        } else {
            // special calibrator case
            bestScans = subcon.selectBest( network_, sourceList_, currentObservingMode_, prevLowElevationScores,
                                           prevHighElevationScores, opt_endposition );
        }

        if ( FocusCorners::startFocusCorner && depth == 0 && FocusCorners::iscan >= FocusCorners::nscans ) {
            FocusCorners::reset( bestScans, sourceList_ );
        }

        // check if you have possible next scan
        if ( bestScans.empty() ) {
            if ( depth == 0 && type != Scan::ScanType::fillin ) {
                if ( type == Scan::ScanType::astroCalibrator ) {
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( warning )
                        << "no valid scan found in calibrator block -> finished calibration block";
#else
                    cout << "[warning] no valid scan found in calibrator block -> finished calibration block\n";
#endif
                    break;
                }

                // if there is no more possible scan at the outer most iteration, check 1minute later
                unsigned int maxScanEnd = 0;
                for ( auto &any : network_.refStations() ) {
                    PointingVector pv = any.getCurrentPointingVector();
                    pv.setTime( pv.getTime() + 60 );
                    if ( !any.getPARA().available || !any.getPARA().tagalong ) {
                        any.setCurrentPointingVector( pv );
                        if ( pv.getTime() > maxScanEnd ) {
                            maxScanEnd = pv.getTime();
                        }
                    }
                }

                if ( util::absDiff( endTime, maxScanEnd ) > 1800 ) {
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( warning ) << "no valid scan found, checking one minute later";
#else
                    cout << "[warning] no valid scan found, checking one minute later\n";
#endif

                    of << boost::format( "| [warning] no valid scan found, checking one minute later: %s %143t|\n" ) %
                              TimeSystem::time2string( maxScanEnd );
                }
                checkForNewEvents( maxScanEnd, true, of, true );
                if ( maxScanEnd > endTime ) {
                    break;
                }

                continue;
            } else {
                // if there is no more possible scan in an deep recursion, break this recursion
                break;
            }
        }

        // check end time of best possible next scan
        unsigned int maxScanEnd = 0;
        for ( const auto &any : bestScans ) {
            if ( any.getTimes().getScanTime( Timestamp::end ) > maxScanEnd ) {
                maxScanEnd = any.getTimes().getScanTime( Timestamp::end );
            }
        }
        if ( maxScanEnd > FocusCorners::nextStart ) {
            FocusCorners::startFocusCorner = true;
        }

        // check if end time triggers a new event
        bool hardBreak = checkForNewEvents( maxScanEnd, true, of, true );
        if ( hardBreak ) {
            continue;
        }
        if ( maxScanEnd > endTime ) {
            break;
        }

        // if end time of best possible next scans is greater than end time of scan selection stop
        if ( maxScanEnd > TimeSystem::duration ) {
            int i = 0;
            while ( i < bestScans.size() ) {
                Scan &any = bestScans[i];
                bool valid = any.prepareForScanEnd( network_, sourceList_.getSource( any.getSourceId() ),
                                                    currentObservingMode_, endTime );
                if ( !valid ) {
                    bestScans.erase( bestScans.begin() + i );
                } else {
                    ++i;
                }
            }

            if ( bestScans.empty() ) {
                break;
            }
        }
        // the best scans are now already fixed. add observing duration to total observing duration to avoid fillin mode
        // scans which extend the allowed total observing time.
        for ( const auto &scan : bestScans ) {
            for ( int i = 0; i < scan.getNSta(); ++i ) {
                unsigned long staid = scan.getStationId( i );
                unsigned int obsDur = scan.getTimes().getObservingDuration( i );
                network_.refStation( staid ).addObservingTime( obsDur );
            }
        }

        // check if it is possible to start a fillin mode block, otherwise put best scans to schedule
        if ( parameters_.fillinmodeDuringScanSelection && !scans_.empty() ) {
            boost::optional<StationEndposition> newEndposition( network_.getNSta() );
            if ( opt_endposition.is_initialized() ) {
                for ( unsigned long i = 0; i < network_.getNSta(); ++i ) {
                    if ( opt_endposition->hasEndposition( i ) ) {
                        newEndposition->addPointingVectorAsEndposition( opt_endposition->getFinalPosition( i ).get() );
                    }
                }
            }

            for ( const auto &any : bestScans ) {
                for ( int i = 0; i < any.getNSta(); ++i ) {
                    newEndposition->addPointingVectorAsEndposition( any.getPointingVector( i ) );
                }
                //                newEndposition->setBackupEarliestScanStart(any.getTimes().getObservingTime());
            }

            newEndposition->setStationAvailable( network_.getStations() );
            newEndposition->checkStationPossibility( network_.getStations() );

            boost::optional<Subcon> new_opt_subcon( std::move( subcon ) );
            // start recursion for fillin mode scans
            unsigned long scansBefore = scans_.size();
            startScanSelection( min( maxScanEnd, TimeSystem::duration ), of, Scan::ScanType::fillin,
                                newEndposition, new_opt_subcon, depth + 1 );

            // check if a fillin mode scan was created and update times if necessary
            unsigned long scansAfter = scans_.size();
            if ( scansAfter != scansBefore ) {
                // loop over all stations
                for ( const Station &thisSta : network_.getStations() ) {
                    unsigned long staid = thisSta.getId();

                    // search for station in all best scans
                    for ( auto &thisScan : bestScans ) {
                        boost::optional<unsigned long> oidx = thisScan.findIdxOfStationId( staid );
                        if ( oidx.is_initialized() ) {
                            bool valid = true;
                            auto idx = static_cast<int>( oidx.get() );
                            const PointingVector &slewEnd = thisScan.getPointingVector( idx, Timestamp::start );
                            boost::optional<unsigned int> oSlewTime = thisSta.slewTime( slewEnd );
                            if ( oSlewTime.is_initialized() ) {
                                unsigned int slewTime = oSlewTime.get();
                                auto oidx2 = thisScan.findIdxOfStationId( staid );
                                if ( oidx2.is_initialized() ) {
                                    if ( thisSta.getPARA().firstScan ) {
                                        valid = thisScan.referenceTime().updateAfterFillinmode(
                                            idx, thisSta.getCurrentTime(), 0, slewTime, 0 );
                                    } else {
                                        valid = thisScan.referenceTime().updateAfterFillinmode(
                                            idx, thisSta.getCurrentTime(), thisSta.getPARA().systemDelay, slewTime,
                                            thisSta.getPARA().preob );
                                    }
                                }
                            } else {
                                valid = false;
                            }

                            if ( !valid ) {
                                string msg = (boost::format("check fillin mode scan for station %s "
                                                              "prior to scan %d") % thisSta.getName()
                                               % thisScan.getId()).str();
#ifdef VIESCHEDPP_LOG
                                BOOST_LOG_TRIVIAL( warning ) << msg;
#else
                                cout << "[warning] "<< msg << "\n";
#endif

                                of << "[warning] "<< msg << "\n";
                            }

                            break;
                        }
                    }
                }
            }
        }

        // update highestElevation and lowestElevation in case of calibrator block
        bool stopScanSelection = false;
        if ( type == Scan::ScanType::astroCalibrator ) {
            ++countCalibratorScans;
            stopScanSelection = calibratorUpdate( bestScans, prevHighElevationScores, prevLowElevationScores,
                                                  highestElevations, lowestElevations );
        }

        // update best possible scans
        consideredUpdate( nSingleScans, nSubnettingScans, depth, of );
        for ( auto &bestScan : bestScans ) {
            update( bestScan, of );
        }

        // stop if calibration block is finished of number of maximum scans reached
        if ( type == Scan::ScanType::astroCalibrator ) {
            if ( stopScanSelection || countCalibratorScans >= AstrometricCalibratorBlock::nmaxScans ) {
                break;
            }
        }

        // update number of scan selections if it is a standard scan
        if ( type == Scan::ScanType::standard ) {
            ++Scan::nScanSelections;
            if ( Scan::scanSequence_flag ) {
                Scan::newScan();
            }
        }

        // check if you need to schedule a calibration block
        if ( type == Scan::ScanType::standard && AstrometricCalibratorBlock::scheduleCalibrationBlocks && depth == 0 ) {
            switch ( AstrometricCalibratorBlock::cadenceUnit ) {
                case AstrometricCalibratorBlock::CadenceUnit::scans: {
                    if ( Scan::nScanSelections == AstrometricCalibratorBlock::nextBlock ) {
                        boost::optional<Subcon> empty_subcon = boost::none;
                        startScanSelection( endTime, of, Scan::ScanType::astroCalibrator, opt_endposition, empty_subcon,
                                            depth + 1 );
                        AstrometricCalibratorBlock::nextBlock += AstrometricCalibratorBlock::cadence;
                    }
                    break;
                }
                case AstrometricCalibratorBlock::CadenceUnit::seconds: {
                    if ( maxScanEnd >= AstrometricCalibratorBlock::nextBlock ) {
                        boost::optional<Subcon> empty_subcon = boost::none;
                        startScanSelection( endTime, of, Scan::ScanType::astroCalibrator, opt_endposition, empty_subcon,
                                            depth + 1 );
                        AstrometricCalibratorBlock::nextBlock += AstrometricCalibratorBlock::cadence;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    // write clibrator statistics
    if ( type == Scan::ScanType::astroCalibrator ) {
        writeCalibratorStatistics( of, highestElevations, lowestElevations );
    }

    // scan selection block is over. Change station availability back to start value
    if ( opt_endposition.is_initialized() ) {
        changeStationAvailability( opt_endposition, StationEndposition::change::end );
    }
}


void Scheduler::start() noexcept {
    string prefix = util::version2prefix(version_);
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(info) << prefix << "start scheduling";
#else
    cout << "[info] " + prefix + "start scheduling\n";
#endif

    string fileName = getName() + "_iteration_" + to_string( parameters_.currentIteration ) + ".txt";
    ofstream of;
    if ( xml_.get( "VieSchedpp.output.iteration_log", true ) ) {
        of.open( path_ + fileName );
    }
    if ( FocusCorners::flag ) {
        FocusCorners::initialize( network_, of );
    }
    CalibratorBlock::stationFlag = vector<char>( network_.getNSta(), false );

    if ( network_.getNSta() == 0 || sourceList_.empty() || network_.getNBls() == 0 ) {
        string e = ( boost::format( "number of stations: %d number of baselines: %d number of sources: %d;\n" ) %
                     network_.getNSta() % network_.getNBls() % sourceList_.getNSrc() )
                       .str();
        of << "ERROR: " << e;
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( error ) << e;
#else
        cout << "ERROR: " << e;
#endif
        return;
    }

#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing scheduling file to: " << fileName;
#else
    cout << "[info] writing scheduling file to: " << fileName;
#endif

    if ( parameters_.currentIteration > 0 ) {
        of << "Iteration number: " << parameters_.currentIteration << "\n";
    }
    if ( multiSchedulingParameters_.is_initialized() ) {
        of << "multi scheduling parameters:\n";
        multiSchedulingParameters_->output( of );
    }
    resetAllEvents( of );
    listSourceOverview( of );

    boost::optional<StationEndposition> endposition = boost::none;
    boost::optional<Subcon> subcon = boost::none;

    of << boost::format( ".%|143T-|.\n" );

    const auto &o_a_priori_scans = xml_.get_child_optional( "VieSchedpp.a_priori_satellite_scans" );
    //    for(auto &sta : network_.refStations()){
    //        for(const auto &sat : sourceList_.getSatellites()){
    //            ofstream of;
    //            of.open(sta.getName()+"_"+sat->getName()+".txt");
    //            PointingVector pv(sta.getId(), sat->getId());
    //            for(unsigned int time = 0; time < TimeSystem::duration; ++time){
    //                pv.setTime(time);
    //                sta.calcAzEl_rigorous(sat, pv);
    //                of << boost::format("%s %18.10f %18.10f \n") % TimeSystem::time2string(pv.getTime()) %
    //                (pv.getAz()*rad2deg) % (pv.getEl()*rad2deg);
    //            }
    //        }
    //    }

    if ( o_a_priori_scans.is_initialized() ) {
        scheduleAPrioriScans( *o_a_priori_scans, of );
    }

    if ( !calib_.empty() ) {
        calibratorBlocks( of );
    }
    if ( ParallacticAngleBlock::nscans > 0 ) {
        parallacticAngleBlocks( of );
    }
    if ( DifferentialParallacticAngleBlock::nscans > 0 ) {
        differentialParallacticAngleBlocks( of );
    }

    if ( himp_.is_initialized() ) {
        highImpactScans( himp_.get(), of );
    }

    Scan::scanSequence_modulo = 0;

    // check if you have some fixed high impact scans
    if ( scans_.empty() ) {
        // no fixed scans: start creating a schedule
        startScanSelection( TimeSystem::duration, of, Scan::ScanType::standard, endposition, subcon, 0 );

        // sort scans
        sortSchedule( Timestamp::start );

    } else {
        startScanSelectionBetweenScans( TimeSystem::duration, of, Scan::ScanType::standard, true, false );
    }
    checkForNewEvents( TimeSystem::duration, true, of, true );

    // start fillinmode a posterior
    if ( parameters_.fillinmodeAPosteriori ) {
        of << boost::format( "|%|143t||\n" );
        of << boost::format( "|%=142s|\n" ) % "start fillin mode a posteriori";
        of << boost::format( "|%|143t||\n" );
        of << boost::format( "|%|143T-||\n" );
        startScanSelectionBetweenScans( TimeSystem::duration, of, Scan::ScanType::fillin, false, true );
    }

    // output some statistics
    statistics( of );
    bool newScheduleNecessary = checkOptimizationConditions( of );

    // check if new iteration is necessary
    if ( newScheduleNecessary ) {
         of.close();
        ++parameters_.currentIteration;

#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( info )
            << "source optimization conditions not met -> restarting schedule with reduced number of sources";
#else
        cout << "[info] source optimization conditions not met -> restarting schedule with reduced number of sources";
#endif

        // restart schedule
        start();
    } else {
        if ( parameters_.idleToObservingTime ) {
            switch ( ScanTimes::getAlignmentAnchor() ) {
                case ScanTimes::AlignmentAnchor::start: {
                    idleToScanTime( Timestamp::end, of );
                    break;
                }
                case ScanTimes::AlignmentAnchor::end: {
                    idleToScanTime( Timestamp::start, of );
                    break;
                }
                case ScanTimes::AlignmentAnchor::individual: {
                    idleToScanTime( Timestamp::end, of );
                    idleToScanTime( Timestamp::start, of );
                    break;
                }
                default:
                    break;
            }
        }

        updateObservingTimes();

        // check if there was an error during the session
        if ( !checkAndStatistics( of ) ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( error ) << boost::format( "%s iteration %d error while checking the schedule" ) %
                                              getName() % ( parameters_.currentIteration );
#else
            cout << boost::format( "[error] %s iteration %d error while checking the schedule" ) % getName() %
                        ( parameters_.currentIteration );
#endif
        }
        of.close();
    }

    sortSchedule( Timestamp::start );
}


void Scheduler::statistics( ofstream &of ) {
    int nobs = std::accumulate( scans_.begin(), scans_.end(), 0,
                                []( int sum, const Scan &any ) { return sum + any.getNObs(); } );

    of << boost::format( "|%|143t||\n" );
    of << boost::format( "|%=142s|\n" ) % "SUMMARY";
    of << boost::format( "|%|143t||\n" );
    of << boost::format( "| %-35s %-30d %143t|\n" ) % "number of scans" % scans_.size();
    of << boost::format( "| %-35s %-30d %143t|\n" ) % "number of observations" % nobs;
    of << boost::format( "| %-35s %d (single source scans %d, subnetting scans %d) %143t|\n" ) %
              "total scans considered" % ( nSingleScansConsidered + 2 * nSubnettingScansConsidered ) %
              nSingleScansConsidered % ( 2 * nSubnettingScansConsidered );

#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "created schedule with " << scans_.size() << " scans and " << nobs << " observations";
#else
    cout << "[info] created schedule with " << scans_.size() << " scans and " << nobs << " observations";
#endif
}


Subcon Scheduler::createSubcon( const shared_ptr<Subnetting> &subnetting, Scan::ScanType type,
                                const boost::optional<StationEndposition> &endposition ) noexcept {
    Subcon subcon = allVisibleScans( type, endposition, parameters_.doNotObserveSourcesWithinMinRepeat );
    subcon.calcStartTimes( network_, sourceList_, endposition );
    subcon.updateAzEl( network_, sourceList_ );
    subcon.constructAllBaselines( network_, sourceList_ );
    subcon.calcAllBaselineDurations( network_, sourceList_, currentObservingMode_ );
    subcon.calcAllScanDurations( network_, sourceList_, endposition );
    subcon.checkTotalObservingTime( network_, sourceList_ );
    subcon.checkIfEnoughTimeToReachEndposition( network_, sourceList_, endposition );

    if ( subnetting != nullptr ) {
        subcon.createSubnettingScans( subnetting, network_, sourceList_ );
    }
    return subcon;
}


Subcon Scheduler::allVisibleScans( Scan::ScanType type, const boost::optional<StationEndposition> &endposition,
                                   bool doNotObserveSourcesWithinMinRepeat ) noexcept {
    // get latest start time of new scan
    unsigned int currentTime = 0;
    for ( auto &station : network_.getStations() ) {
        if ( station.getCurrentTime() > currentTime ) {
            currentTime = station.getCurrentTime();
        }
    }

    // save all ids of the next observed sources (if there is a required endposition)
    set<unsigned long> observedSources;
    if ( parameters_.ignoreSuccessiveScansSameSrc ){
        if ( endposition.is_initialized() ) {
            observedSources = endposition->getObservedSources( currentTime, sourceList_ );
        }
        for ( const auto &sta : network_.getStations() ) {
            observedSources.insert( sta.getCurrentPointingVector().getSrcid() );
        }
    }

    // create subcon with all visible scans
    Subcon subcon;
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "creating new subcon " << subcon.printId();
#endif

    for ( const auto &thisSource : sourceList_.getSources() ) {
        subcon.visibleScan( currentTime, type, network_, thisSource, observedSources,
                            doNotObserveSourcesWithinMinRepeat );
    }

    return subcon;
}


void Scheduler::update( Scan &scan, ofstream &of ) noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "adding scan " << scan.printId() << " to schedule";
#endif

    // check if scan has influence (only required for fillin mode scans)
    bool influence;
    influence = !( scan.getType() == Scan::ScanType::fillin && !parameters_.fillinmodeInfluenceOnSchedule );

    unsigned long srcid = scan.getSourceId();

    for ( int i = 0; i < scan.getNSta(); ++i ) {
        const PointingVector &pv = scan.getPointingVector( i );
        unsigned long staid = pv.getStaid();
        const PointingVector &pv_end = scan.getPointingVector( i, Timestamp::end );
        unsigned long nObs = scan.getNObs( staid );

        network_.update( nObs, pv_end, influence );
    }
    for ( int i = 0; i < scan.getNObs(); ++i ) {
        const Observation &obs = scan.getObservation( i );
        network_.update( obs.getBlid(), influence );
    }

    unsigned long nbl = scan.getNObs();
    unsigned int latestTime = scan.getTimes().getObservingTime( Timestamp::start );
    const auto &thisSource = sourceList_.refSource( srcid );
    thisSource->update( scan.getNSta(), nbl, latestTime, influence );

    // update minimum slew time in case of custom data write speed to disk
    for ( int i = 0; i < scan.getNSta(); ++i ) {
        unsigned long staid = scan.getPointingVector( i ).getStaid();

        Station &sta = network_.refStation( staid );
        if ( sta.getPARA().dataWriteRate.is_initialized() ) {
            //            double recRate = currentObservingMode_->recordingRate( staid );
            unsigned int duration = scan.getTimes().getObservingDuration( i );
            sta.referencePARA().overheadTimeDueToDataWriteSpeed( duration );
        }
    }

    scan.output( scans_.size(), network_, thisSource, of );
    scans_.push_back( std::move( scan ) );
}


void Scheduler::consideredUpdate( unsigned long n1scans, unsigned long n2scans, int depth, ofstream &of ) noexcept {
    if ( n1scans + n2scans > 0 ) {
        string right;
        if ( n2scans == 0 ) {
            right = ( boost::format( "considered single scans %d" ) % n1scans ).str();
        } else {
            right = ( boost::format( "considered single scans %d, subnetting scans %d" ) % n1scans % n2scans ).str();
        }
        of << boost::format( "| depth:  %d %130s |\n" ) % depth % right;
        nSingleScansConsidered += n1scans;
        nSubnettingScansConsidered += n2scans;
    }
}


bool Scheduler::checkAndStatistics( ofstream &of ) noexcept {
    resetAllEvents( of );

    bool everythingOk = true;
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "checking schedule";
#endif

    of << "starting check routine!\n";

    int countErrors = 0;
    int countWarnings = 0;

    bool debug = xml_.get("VieSchedpp.output.createSlewFile", false);
//    debug = true;

    for ( auto &thisStation : network_.refStations() ) {
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "checking station " << thisStation.getName();
#endif
        ofstream of_slew;
        if (debug){
            string name = path_ + (boost::format("%s_slew_%s.txt") %this->getName() %thisStation.getName()).str();
            of_slew.open(name);
        }
        of << "    checking station " << thisStation.getName() << ":\n";
        unsigned long staid = thisStation.getId();
        unsigned int constTimes = thisStation.getPARA().systemDelay + thisStation.getPARA().preob;
        thisStation.referencePARA().firstScan = false;

        // sort scans based on observation start of this station (can be different if you align scans individual or at
        // end)
        sortSchedule( staid );

        // get first scan with this station and initialize idx
        int i_thisEnd = 0;
        int idx_thisEnd = -1;
        while ( i_thisEnd < scans_.size() ) {
            const Scan &scan_thisEnd = scans_[i_thisEnd];
            // look for index of station in scan
            boost::optional<unsigned long> oidx_thisEnd = scan_thisEnd.findIdxOfStationId( staid );
            if ( !oidx_thisEnd.is_initialized() ) {
                ++i_thisEnd;
                continue;  // if you do not find one continue
            }
            idx_thisEnd = static_cast<int>( *oidx_thisEnd );
            break;
        }

        // save staStatistics
        Station::Statistics staStatistics;

        while ( i_thisEnd < scans_.size() ) {
            // get scan and pointing vector at start
            const Scan &scan_thisEnd = scans_[i_thisEnd];
            const PointingVector &thisEnd = scan_thisEnd.getPointingVector( idx_thisEnd, Timestamp::end );

            // update staStatistics
            staStatistics.scanStartTimes.push_back( scan_thisEnd.getPointingVector( idx_thisEnd ).getTime() );
            staStatistics.totalObservingTime += scan_thisEnd.getTimes().getObservingDuration( idx_thisEnd );
            staStatistics.totalFieldSystemTime += scan_thisEnd.getTimes().getFieldSystemDuration( idx_thisEnd );
            staStatistics.totalPreobTime += scan_thisEnd.getTimes().getPreobDuration( idx_thisEnd );

            int i_nextStart = i_thisEnd + 1;
            while ( i_nextStart < scans_.size() ) {
                // get scan and pointing vector at end
                const Scan &scan_nextStart = scans_[i_nextStart];
                // look for index of station in scan
                boost::optional<unsigned long> oidx_nextStart = scan_nextStart.findIdxOfStationId( staid );
                if ( !oidx_nextStart.is_initialized() ) {
                    ++i_nextStart;
                    continue;  // if you do not find one continue
                }
                auto idx_nextStart = static_cast<int>( *oidx_nextStart );
                const PointingVector &nextStart = scan_nextStart.getPointingVector( idx_nextStart );

                // check if scan times are in correct order
                unsigned int thisEndTime = thisEnd.getTime();
                unsigned int nextStartTime = nextStart.getTime();

                if ( nextStartTime < thisEndTime ) {
                    ++countErrors;
                    of << "    ERROR #" << countErrors
                       << ": start time of next scan is before end time of previouse scan! scans: "
                       << scan_thisEnd.printId() << " and " << scan_nextStart.printId() << "\n";
                    boost::posix_time::ptime thisEndTime_ = TimeSystem::internalTime2PosixTime( thisEndTime );
                    boost::posix_time::ptime nextStartTime_ = TimeSystem::internalTime2PosixTime( nextStartTime );

                    of << "        end time of previouse scan: " << thisEndTime_.time_of_day() << " "
                       << thisEnd.printId() << "\n";
                    of << "        start time of next scan:    " << nextStartTime_.time_of_day() << " "
                       << nextStart.printId() << "\n";
                    of << "*\n";
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( error )
                        << boost::format( "%s iteration %d:" ) % getName() % ( parameters_.currentIteration )
                        << " start time of next scan is before end time of previouse scan! station: "
                        << thisStation.getName();
#endif
                    everythingOk = false;
                } else {
                    // check slew time
                    auto oslewtime = thisStation.slewTime(
                        thisEnd, nextStart, scan_thisEnd.getTimes().getObservingDuration( idx_thisEnd ) );
                    unsigned int slewtime;
                    if ( oslewtime.is_initialized() ) {
                        slewtime = *oslewtime;
                    } else {
                        slewtime = TimeSystem::duration;
                    }


                    unsigned int min_neededTime = slewtime + constTimes;
                    unsigned int availableTime = nextStartTime - thisEndTime;
                    unsigned int idleTime;

                    if ( debug ) {
                        double abs_unaz = abs( thisEnd.getAz() - nextStart.getAz() );
                        double abs_el = abs( thisEnd.getEl() - nextStart.getEl() );
                        of_slew << boost::format(
                            "%s - %s dt %4d unaz from %9.4f to %9.4f total %9.4f el from %9.4f to %9.4f total %9.4f "
                                        "slew %3d fixed %3d good by %4d src %s - %s\n")
                                % TimeSystem::internalTime2PosixTime( thisEndTime ).time_of_day()
                                % TimeSystem::internalTime2PosixTime( nextStartTime ).time_of_day()
                                % (static_cast<long>(nextStartTime) - static_cast<long>(thisEndTime))
                                % (thisEnd.getAz() *rad2deg)
                                % (nextStart.getAz() *rad2deg)
                                % (abs_unaz *rad2deg)
                                % (thisEnd.getEl() *rad2deg)
                                % (nextStart.getEl() *rad2deg)
                                % (abs_el *rad2deg)
                                % slewtime
                                % constTimes
                                % (static_cast<long>(availableTime) - static_cast<long>(slewtime)
                                            - static_cast<long>(constTimes))
                                % sourceList_.getSource(thisEnd.getSrcid())->getName()
                                % sourceList_.getSource(nextStart.getSrcid())->getName();

                    }


                    // update staStatistics
                    staStatistics.totalSlewTime += slewtime;
                    if ( availableTime >= min_neededTime ) {
                        idleTime = availableTime - min_neededTime;
                    } else {
                        idleTime = 0;
                    }

                    staStatistics.totalIdleTime += idleTime;
                    int buffer = 1;
                    if ( this->sourceList_.isSatellite(scan_nextStart.getSourceId()) ){
                        buffer = 2;
                    }
                    if ( availableTime + buffer < min_neededTime ) {
                        if ( this->sourceList_.isSatellite(scan_nextStart.getSourceId()) ){
                            ++countWarnings;
                            of << "    WARNING #" << countErrors
                               << ": maybe not enough available time for slewing! scans: " << scan_thisEnd.printId() << " and "
                               << scan_nextStart.printId() << "\n";
                            of << "it might not be a problem if there is enough preob/fs time to compensate for it";
#ifdef VIESCHEDPP_LOG
                            BOOST_LOG_TRIVIAL( warning )
                                << boost::format( "%s iteration %d:" ) % getName() % ( parameters_.currentIteration )
                                << "maybe not enough available time for slewing to satellite? station: " << thisStation.getName() << " ("
                                << ( (long)availableTime - (long)min_neededTime ) << " [sec])";
#endif
                        }else{
                            ++countErrors;
                            of << "    ERROR #" << countErrors
                               << ": not enough available time for slewing! scans: " << scan_thisEnd.printId() << " and "
                               << scan_nextStart.printId() << "\n";
#ifdef VIESCHEDPP_LOG
                            BOOST_LOG_TRIVIAL( error )
                                << boost::format( "%s iteration %d:" ) % getName() % ( parameters_.currentIteration )
                                << "not enough available time for slewing! station: " << thisStation.getName() << " ("
                                << ( (long)availableTime - (long)min_neededTime ) << " [sec])";
#endif
                        }
                        boost::posix_time::ptime thisEndTime_ = TimeSystem::internalTime2PosixTime( thisEndTime );
                        boost::posix_time::ptime nextStartTime_ = TimeSystem::internalTime2PosixTime( nextStartTime );
                        of << "        end time of previouse scan: " << thisEndTime_.time_of_day() << " "
                           << thisEnd.printId() << "\n";
                        of << "        start time of next scan:    " << nextStartTime_.time_of_day() << " "
                           << nextStart.printId() << "\n";
                        of << "        available time: " << availableTime << "\n";
                        of << "            needed slew time:           " << slewtime << "\n";
                        of << "            needed constant times:      " << constTimes << "\n";
                        of << "        needed time:    " << min_neededTime << "\n";
                        of << "        difference:     " << (long)availableTime - (long)min_neededTime << "\n";
                        of << "*\n";
                        everythingOk = false;

                    } else {
                        if ( idleTime > 1200 ) {
                            unsigned int midpoint = ( thisEndTime + nextStartTime ) / 2;
                            bool dummy = false;
                            thisStation.checkForNewEvent( midpoint, dummy );

                            if ( thisStation.getPARA().available ) {
                                ++countWarnings;
                                of << "    WARNING #" << countWarnings
                                   << ": long idle time! scans: " << scan_thisEnd.printId() << " and "
                                   << scan_nextStart.printId() << "\n";
                                of << "        idle time: " << idleTime << "[s]\n";
                                boost::posix_time::ptime thisEndTime_ =
                                    TimeSystem::internalTime2PosixTime( thisEndTime );
                                boost::posix_time::ptime nextStartTime_ =
                                    TimeSystem::internalTime2PosixTime( nextStartTime );
                                of << "            end time of previouse scan: " << thisEndTime_.time_of_day() << " "
                                   << thisEnd.printId() << "\n";
                                of << "            start time of next scan:    " << nextStartTime_.time_of_day() << " "
                                   << nextStart.printId() << "\n";
                                of << "*\n";
#ifdef VIESCHEDPP_LOG
                                BOOST_LOG_TRIVIAL( warning )
                                    << boost::format( "%s iteration %d:" ) % getName() %
                                           ( parameters_.currentIteration )
                                    << "long idle time! (" << idleTime << " [s]) station: " << thisStation.getName();
#endif
                            }
                        }
                    }
                }
                // change index of pointing vector and scan
                idx_thisEnd = idx_nextStart;
                break;
            }
            // change index of pointing vector and scan
            i_thisEnd = i_nextStart;
        }
        of << "    finished!\n";
        thisStation.setStatistics( staStatistics );
    }
    of << "Total: " << countErrors << " errors and " << countWarnings << " warnings\n";

    // sort scans again based on their observation start
    sortSchedule( Timestamp::start );

    // add source srcStatistics
    std::vector<AbstractSource::Statistics> srcStatistics( sourceList_.getNSrc() );
    std::vector<Baseline::Statistics> blsStatistics( network_.getNBls() );
    for ( const auto &any : scans_ ) {
        unsigned long srcid = any.getSourceId();
        auto &thisSrcStatistics = srcStatistics[srcid];
        thisSrcStatistics.scanStartTimes.push_back( any.getTimes().getObservingTime( Timestamp::start ) );
        thisSrcStatistics.totalObservingTime += any.getTimes().getObservingDuration();

        for ( const auto &obs : any.getObservations() ) {
            unsigned long blid = obs.getBlid();
            auto &thisBlsStatistics = blsStatistics[blid];
            thisBlsStatistics.scanStartTimes.push_back( any.getTimes().getObservingTime( Timestamp::start ) );
            thisBlsStatistics.totalObservingTime += any.getTimes().getObservingDuration();
        }
    }
    for ( unsigned long isrc = 0; isrc < sourceList_.getNSrc(); ++isrc ) {
        const auto &source = sourceList_.refSource( isrc );
        source->setStatistics( srcStatistics[isrc] );
    }
    for ( unsigned long ibl = 0; ibl < network_.getNBls(); ++ibl ) {
        Baseline &baseline = network_.refBaseline( ibl );
        baseline.setStatistics( blsStatistics[ibl] );
    }
    return everythingOk;
}


bool Scheduler::checkForNewEvents( unsigned int time, bool output, ofstream &of, bool scheduleTagalong ) noexcept {
    bool hard_break = false;
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "check for parameter changes";
#endif

    // check if it is required to tagalong a station
    for ( auto &any : network_.refStations() ) {
        bool tagalong = any.checkForTagalongMode( time );
        if ( tagalong && scheduleTagalong ) {
#ifdef VIESCHEDPP_LOG
            if ( Flags::logDebug )
                BOOST_LOG_TRIVIAL( debug ) << "tagalong for station " << any.getName() << " required";
#endif
            auto &skyCoverage = network_.refSkyCoverage( network_.getStaid2skyCoverageId().at( any.getId() ) );
            startTagelongMode( any, skyCoverage, of );
        }
    }

    // check if a station has to be changed
    vector<string> stationChanged;
    for ( auto &any : network_.refStations() ) {
        bool changed = any.checkForNewEvent( time, hard_break );
        if ( changed ) {
            stationChanged.push_back( any.getName() );
#ifdef VIESCHEDPP_LOG
            if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "changed parameters for station " << any.getName();
#endif
        }
    }
    if ( !stationChanged.empty() && output && time < TimeSystem::duration ) {
        util::outputObjectList( "station parameter changed", stationChanged, of );
        of << boost::format( "|%|143T-||\n" );
    }

    // check if a source has to be changed
    vector<string> sourcesChanged;
    for ( const auto &any : sourceList_.refSources() ) {
        bool changed = any->checkForNewEvent( time, hard_break );
        if ( changed ) {
            sourcesChanged.push_back( any->getName() );
#ifdef VIESCHEDPP_LOG
            if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "changed parameters for source " << any->getName();
#endif
        }
    }
    if ( !sourcesChanged.empty() && output && time < TimeSystem::duration ) {
        util::outputObjectList( "source parameter changed", sourcesChanged, of );
        listSourceOverview( of );
        of << boost::format( "|%|143T-||\n" );
    }

    // check if a baseline has to be changed
    vector<string> baselineChanged;
    for ( auto &any : network_.refBaselines() ) {
        bool changed = any.checkForNewEvent( time, hard_break );
        if ( changed ) {
            baselineChanged.push_back( any.getName() );
#ifdef VIESCHEDPP_LOG
            if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "changed parameters for baseline " << any.getName();
#endif
        }
    }
    if ( !baselineChanged.empty() && output && time < TimeSystem::duration ) {
        util::outputObjectList( "baseline parameter changed", baselineChanged, of );
        of << boost::format( "|%|143T-||\n" );
    }
    return hard_break;
}


void Scheduler::ignoreTagalongParameter() {
    // ignore the tagalong mode for each station
    for ( auto &any : network_.refStations() ) {
        any.referencePARA().tagalong = false;
    }
}


void Scheduler::listSourceOverview( ofstream &of ) noexcept {
    //    unsigned int counter = 0;
    vector<string> available;
    vector<string> notAvailable;
    vector<string> notAvailable_optimization;
    vector<string> notAvailable_tooWeak;
    vector<string> notAvailable_tooCloseToSun;

    vector<string> available_sat;
    vector<string> notAvailable_sat;
    vector<string> notAvailable_optimization_sat;
    vector<string> notAvailable_tooWeak_sat;

    for ( const auto &any : sourceList_.getQuasars() ) {
        if ( any->getPARA().available && any->getPARA().globalAvailable ) {
            available.push_back( any->getName() );

        } else if ( !any->getPARA().globalAvailable ) {
            notAvailable_optimization.push_back( any->getName() );

        } else if ( any->getMaxFlux() < any->getPARA().minFlux ) {
            string message =
                ( boost::format( "-%8s (%4.2f/%4.2f)" ) % any->getName() % any->getMaxFlux() % any->getPARA().minFlux )
                    .str();
            notAvailable_tooWeak.push_back( message );

        } else {
            notAvailable.push_back( any->getName() );
        }
        if ( any->getSunDistance( 0, nullptr ) < any->getPARA().minSunDistance ) {
            string message =
                ( boost::format( "%-8s (%4.2f/%4.2f)" ) % any->getName() %
                  ( any->getSunDistance( 0, nullptr ) * rad2deg ) % ( any->getPARA().minSunDistance * rad2deg ) )
                    .str();

            notAvailable_tooCloseToSun.push_back( message );
        }
    }


    for ( const auto &any : sourceList_.getSatellites() ) {
        if ( any->getPARA().available && any->getPARA().globalAvailable ) {
            available_sat.push_back( any->getName() );

        } else if ( !any->getPARA().globalAvailable ) {
            notAvailable_optimization_sat.push_back( any->getName() );

        } else if ( any->getMaxFlux() < any->getPARA().minFlux ) {
            string message =
                ( boost::format( "%8s (%4.2f/%4.2f)" ) % any->getName() % any->getMaxFlux() % any->getPARA().minFlux )
                    .str();
            notAvailable_tooWeak_sat.push_back( message );

        } else {
            notAvailable_sat.push_back( any->getName() );
        }
    }


    of << boost::format( "Total number of sources: %d (quasars %d, satellites %d)\n" ) % sourceList_.getNSrc() %
              sourceList_.getNQuasars() % sourceList_.getNSatellites();

    util::outputObjectList( "    available quasars", available, of, 6 );
    util::outputObjectList( "    not available", notAvailable, of, 6 );
    util::outputObjectList( "    not available because of optimization", notAvailable_optimization, of, 6 );
    util::outputObjectList( "    not available because too weak", notAvailable_tooWeak, of, 6 );
    util::outputObjectList( "    not available because of sun distance", notAvailable_tooCloseToSun, of, 6 );
    of << "\n";
    util::outputObjectList( "    available satellites", available_sat, of, 6 );
    util::outputObjectList( "    not available", notAvailable_sat, of, 6 );
    util::outputObjectList( "    not available because of optimization", notAvailable_optimization_sat, of, 6 );
    util::outputObjectList( "    not available because too weak", notAvailable_tooWeak_sat, of, 6 );
}


void Scheduler::startTagelongMode( Station &station, SkyCoverage &skyCoverage, std::ofstream &of,
                                   bool ignoreFillinMode ) {
    unsigned long staid = station.getId();
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "start tagalong mode for station " << station.getName();
#endif

    of << boost::format( "| Start tagalong mode for station %s %|143t||\n" ) % station.getName();

    // get wait times
    unsigned int stationConstTimes = station.getPARA().systemDelay + station.getPARA().preob;

    // tagalong end time
    unsigned int tagalongEndTime = scans_.back().getTimes().getScanTime( Timestamp::end );

    // sort and keep indices
    unsigned long n_scans = scans_.size();
    vector<pair<Scan, unsigned long>> vp;
    vp.reserve( n_scans );
    for ( unsigned long i = 0; i < n_scans; ++i ) {
        vp.emplace_back( move( scans_[i] ), i );
    }

    Timestamp ts = Timestamp::start;
    stable_sort(
        vp.begin(), vp.end(), [ts]( const pair<Scan, unsigned long> &scan1, const pair<Scan, unsigned long> &scan2 ) {
            return scan1.first.getTimes().getObservingTime( ts ) < scan2.first.getTimes().getObservingTime( ts );
        } );

    // split sorted
    vector<Scan> newScans;
    newScans.reserve( n_scans );
    vector<unsigned long> indices = vector<unsigned long>( n_scans );
    for ( int i = 0; i < n_scans; ++i ) {
        newScans.push_back( move( vp[i].first ) );
        indices[i] = vp[i].second;
    }


    // loop through all scans
    unsigned long counter = 0;
    for ( auto &scan : newScans ) {
        if ( scan.getTimes().getScanTime( Timestamp::end ) > tagalongEndTime ) {
            continue;
        }

        bool hardBreak = false;
        station.checkForNewEvent( scan.getTimes().getScanTime( Timestamp::start ), hardBreak );
        if ( !station.getPARA().available ) {
            continue;
        }

        ++counter;
        unsigned int scanStartTime = scan.getTimes().getObservingTime( Timestamp::start );
        unsigned int currentStationTime = station.getCurrentTime();

        if ( ignoreFillinMode && scan.getType() == Scan::ScanType::fillin ) {
            continue;
        }

        // look if this scan is possible for tagalong mode
        if ( scanStartTime > currentStationTime ) {
            unsigned long srcid = scan.getSourceId();
            const auto &source = sourceList_.refSource( scan.getSourceId() );

            PointingVector pv_new_start( staid, srcid );

            pv_new_start.setTime( scanStartTime );

            station.calcAzEl_rigorous( source, pv_new_start );

            // check if source is up from station
            bool flag = station.isVisible( pv_new_start, source->getPARA().minElevation );
            if ( !flag ) {
#ifdef VIESCHEDPP_LOG
                if ( Flags::logDebug )
                    BOOST_LOG_TRIVIAL( debug ) << "scan " << scan.printId() << " not possible (source not visible)";
#endif
                continue;
            }

            station.getCableWrap().calcUnwrappedAz( station.getCurrentPointingVector(), pv_new_start );

            auto slewtime = station.slewTime( pv_new_start );
            if ( !slewtime.is_initialized() ) {
#ifdef VIESCHEDPP_LOG
                if ( Flags::logDebug )
                    BOOST_LOG_TRIVIAL( debug ) << "scan " << scan.printId() << " not possible (unallowed slew time)";
#endif
                continue;
            }

            // check if there is enough time to slew to source before scan starts
            if ( scanStartTime < currentStationTime + *slewtime + stationConstTimes ) {
#ifdef VIESCHEDPP_LOG
                if ( Flags::logDebug )
                    BOOST_LOG_TRIVIAL( debug )
                        << "scan " << scan.printId() << " not possible (cannot reach source in time)";
#endif
                continue;
            }

            // loop through all other participating stations and prepare baselines
            vector<Observation> newObs;
            for ( int i = 0; i < scan.getNSta(); ++i ) {
                // create baseline
                const PointingVector &otherPv = scan.getPointingVector( i );

                const Station &sta1 = network_.getStation( staid );
                const Station &sta2 = network_.getStation( otherPv.getStaid() );
                if ( sta1.getId() == sta2.getId() ) {
                    continue;
                }
                const Baseline &bl = network_.getBaseline( sta1.getId(), sta2.getId() );

                if ( bl.getParameters().ignore ) {
#ifdef VIESCHEDPP_LOG
                    if ( Flags::logTrace )
                        BOOST_LOG_TRIVIAL( trace )
                            << "scan " << scan.printId() << " ignore observation on baseline " << bl.getName();
#endif
                    continue;
                }
                if ( !source->getPARA().ignoreBaselines.empty() ) {
                    const auto &PARA = source->getPARA();
                    if ( find( PARA.ignoreBaselines.begin(), PARA.ignoreBaselines.end(), bl.getId() ) !=
                         PARA.ignoreBaselines.end() ) {
#ifdef VIESCHEDPP_LOG
                        if ( Flags::logTrace )
                            BOOST_LOG_TRIVIAL( trace )
                                << "scan " << scan.printId() << " ignore observation on baseline " << bl.getName();
#endif
                        continue;
                    }
                }
                Observation obs( bl.getId(), sta1.getId(), sta2.getId(), srcid, scanStartTime );

                // calc baseline scan length
                double date1 = 2400000.5;
                double date2 = TimeSystem::mjdStart + static_cast<double>( scanStartTime ) / 86400.0;
                double gmst = iauGmst82( date1, date2 );

                if ( source->checkJetAngle() ) {
                    if ( !source->jet_angle_valid(scanStartTime, gmst, network_.getDxyz( sta1.getId(), sta2.getId() ) ) ){
                        continue;
                    }
                }


                unsigned int maxScanDuration = 0;
                if ( source->getPARA().fixedScanDuration.is_initialized() ) {
                    maxScanDuration = *source->getPARA().fixedScanDuration;
                } else if ( source->getPARA().forceSameObservingDuration ) {
                    maxScanDuration = scan.getTimes().getObservingDuration();
                } else {
                    for ( auto &band : currentObservingMode_->getAllBands() ) {
                        double SEFD_src;
                        if ( source->hasFluxInformation( band ) ) {
                            // calculate observed flux density for each band
                            SEFD_src = source->observedFlux( band, scanStartTime, gmst,
                                                             network_.getDxyz( sta1.getId(), sta2.getId() ) );
                        } else if ( ObservingMode::sourceBackup[band] == ObservingMode::Backup::internalModel ) {
                            // calculate observed flux density based on model
                            double wavelength = ObservingMode::wavelengths[band];
                            SEFD_src = source->observedFlux_model( wavelength, scanStartTime, gmst,
                                                                   network_.getDxyz( sta1.getId(), sta2.getId() ) );
                        } else {
                            SEFD_src = 1e-3;
                        }

                        double el1 = pv_new_start.getEl();
                        double SEFD_sta1 = sta1.getEquip().getSEFD( band, el1 );

                        double el2 = otherPv.getEl();
                        double SEFD_sta2 = sta2.getEquip().getSEFD( band, el2 );

                        double minSNR_sta1 = sta1.getPARA().minSNR.at( band );
                        double minSNR_sta2 = sta2.getPARA().minSNR.at( band );

                        double minSNR_bl = bl.getParameters().minSNR.at( band );

                        double minSNR_src = source->getPARA().minSNR.at( band );

                        double maxminSNR = minSNR_src;
                        if ( minSNR_sta1 > maxminSNR ) {
                            maxminSNR = minSNR_sta1;
                        }
                        if ( minSNR_sta2 > maxminSNR ) {
                            maxminSNR = minSNR_sta2;
                        }
                        if ( minSNR_bl > maxminSNR ) {
                            maxminSNR = minSNR_bl;
                        }

                        double maxCorSynch1 = sta1.getPARA().midob;
                        double maxCorSynch = maxCorSynch1;
                        double maxCorSynch2 = sta2.getPARA().midob;
                        if ( maxCorSynch2 > maxCorSynch ) {
                            maxCorSynch = maxCorSynch2;
                        }

                        double efficiency = currentObservingMode_->efficiency( sta1.getId(), sta2.getId() );
                        double anum = ( maxminSNR / ( SEFD_src * efficiency ) );
                        double anu1 = SEFD_sta1 * SEFD_sta2;
                        double anu2 = currentObservingMode_->recordingRate( sta1.getId(), sta2.getId(), band );

                        double new_duration = anum * anum * anu1 / anu2 + maxCorSynch;
                        new_duration = ceil( new_duration );
                        auto new_duration_uint = static_cast<unsigned int>( new_duration );

                        unsigned int minScanBl = std::max( { bl.getParameters().minScan, sta1.getPARA().minScan,
                                                             sta2.getPARA().minScan, source->getPARA().minScan } );

                        if ( new_duration_uint < minScanBl ) {
                            new_duration_uint = minScanBl;
                        }
                        unsigned int maxScanBl = std::min( { bl.getParameters().maxScan, sta1.getPARA().maxScan,
                                                             sta2.getPARA().maxScan, source->getPARA().maxScan } );

                        if ( new_duration_uint > maxScanBl ) {
#ifdef VIESCHEDPP_LOG
                            if ( Flags::logTrace )
                                BOOST_LOG_TRIVIAL( trace )
                                    << "scan " << scan.printId() << " ignore observation on baseline " << bl.getName()
                                    << " (too long observing time)";
#endif
                            continue;
                        }

                        if ( new_duration_uint > maxScanDuration ) {
                            maxScanDuration = new_duration_uint;
                        }
                    }
                }
                if ( maxScanDuration == 0 ) {
                    continue;
                }
                unsigned int maxStart = max( obs.getStartTime(), scan.getTimes().getObservingTime( i ) );
                unsigned int minEnd =
                    min( obs.getStartTime() + maxScanDuration, scan.getTimes().getObservingTime( i, Timestamp::end ) );
                if ( maxStart > minEnd ) {
                    continue;
                }
                unsigned int delta_t = minEnd - maxStart;
                if ( delta_t < maxScanDuration ) {
                    continue;
                }
                obs.setObservingTime( maxScanDuration );
#ifdef VIESCHEDPP_LOG
                if ( Flags::logTrace )
                    BOOST_LOG_TRIVIAL( trace )
                        << "scan " << scan.printId() << " add observation on baseline " << bl.getName();
#endif
                newObs.push_back( obs );
            }
            if ( newObs.empty() ) {
#ifdef VIESCHEDPP_LOG
                if ( Flags::logDebug )
                    BOOST_LOG_TRIVIAL( debug )
                        << "scan " << scan.printId() << " not possible (no valid observation possible)";
#endif
                continue;
            }

            unsigned int obsDur = 0;
            for ( const auto &any : newObs ) {
                if ( any.getObservingTime() > obsDur ) {
                    obsDur = any.getObservingTime();
                }
            }

            // check if source is still visible at end of scan... with same cable wrap
            PointingVector pv_new_end( staid, srcid );

            pv_new_end.setTime( scanStartTime + obsDur );

            station.calcAzEl_rigorous( source, pv_new_end );

            // check if source is up from station
            flag = station.isVisible( pv_new_end, source->getPARA().minElevation );
            if ( !flag ) {
#ifdef VIESCHEDPP_LOG
                if ( Flags::logDebug )
                    BOOST_LOG_TRIVIAL( debug ) << "scan " << scan.printId() << " not possible (source not visible)";
#endif
                continue;
            }

            station.getCableWrap().calcUnwrappedAz( pv_new_start, pv_new_end );
            if ( abs( pv_new_end.getAz() - pv_new_start.getAz() ) > halfpi ) {
#ifdef VIESCHEDPP_LOG
                if ( Flags::logDebug )
                    BOOST_LOG_TRIVIAL( debug )
                        << "scan " << scan.printId() << " not possible (change of cable wrap required)";
#endif
                continue;
            }

            // check if there is enough time to reach potential endposition
            PointingVector slew_end( staid, -1 );
            slew_end.setTime( TimeSystem::duration );
            for ( const auto &tmp : newScans ) {
                const auto &oidx = tmp.findIdxOfStationId( staid );
                if ( oidx.is_initialized() ) {
                    unsigned long idx = *oidx;
                    unsigned int time = tmp.getTimes().getObservingTime( idx );
                    if ( time > station.getCurrentTime() && time < slew_end.getTime() ) {
                        slew_end = tmp.getPointingVector( idx );
                        break;
                    }
                }
            }
            if ( slew_end.getTime() != TimeSystem::duration ) {
                auto endpos_slewtime = station.slewTime( pv_new_end, slew_end );
                if ( !endpos_slewtime.is_initialized() ) {
#ifdef VIESCHEDPP_LOG
                    if ( Flags::logDebug )
                        BOOST_LOG_TRIVIAL( debug )
                            << "scan " << scan.printId() << " not possible (unallowed slew time to endposition)";
#endif
                    continue;
                }

                // check if there is enough time to slew to source before scan starts
                unsigned int endpos_newEnd = pv_new_end.getTime() + *endpos_slewtime + stationConstTimes;
                unsigned int endpos_required = slew_end.getTime();
                if ( endpos_required < endpos_newEnd ) {
#ifdef VIESCHEDPP_LOG
                    if ( Flags::logDebug )
                        BOOST_LOG_TRIVIAL( debug )
                            << "scan " << scan.printId() << " not possible (cannot reach endposition in time)";
#endif
                    continue;
                }
            }


            scan.addTagalongStation( pv_new_start, pv_new_end, newObs, *slewtime, station );
            for ( const auto &o : newObs ) {
                unsigned long staid2 = o.getStaid2();
                Station &sta2 = network_.refStation( staid2 );
                sta2.increaseNObs();
                source->increaseNObs();
            }
            auto txt = boost::format(
                    "|    possible to observe source: %-8s scan start: %s scan end: %s +%d obs  (scan: %d) %|143t||\n" ) %
                source->getName() % TimeSystem::time2timeOfDay( pv_new_start.getTime() ) %
                TimeSystem::time2timeOfDay( pv_new_end.getTime() ) % newObs.size() % scan.getId();

#ifdef VIESCHEDPP_LOG
            if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << txt;
#endif

            of << txt;
            if ( station.referencePARA().firstScan ) {
                station.referencePARA().firstScan = false;
            }

            station.addObservingTime( obsDur );
            station.update( newObs.size(), pv_new_end, true );
            skyCoverage.update( pv_new_end );
        }
    }
    of << boost::format( "|%|143T-||\n" );


    // sort back to original order
    scans_ = vector<Scan>();
    scans_.reserve( n_scans );
    for ( int i = 0; i < n_scans; ++i ) {
        unsigned long idx = distance( indices.begin(), find( indices.begin(), indices.end(), i ) );
        scans_.push_back( move( newScans[idx] ) );
    }

    //    station.applyNextEvent(of);
}


bool Scheduler::checkOptimizationConditions( ofstream &of ) {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "checking optimization condition";
#endif
    bool newScheduleNecessary = false;
    vector<string> excludedSources;
    unsigned long excludedScans = 0;
    unsigned long excludedBaselines = 0;
    string message = "checking optimization conditions... ";
    unsigned long consideredSources = 0;
    bool lastExcluded = false;

    vector<pair<unsigned long, unsigned long>> possibleExcludeIds;

    for ( auto &thisSource : sourceList_.getSources() ) {
        if ( !thisSource->getPARA().globalAvailable ) {
            continue;
        }
        ++consideredSources;
        bool scansValid = true;
        if ( thisSource->getNTotalScans() < thisSource->getOptimization().minNumScans ) {
            scansValid = false;
        }

        bool baselinesValid = true;
        if ( thisSource->getNObs() < thisSource->getOptimization().minNumObs ) {
            baselinesValid = false;
        }

        bool exclude;
        if ( parameters_.andAsConditionCombination ) {
            exclude = !( scansValid && baselinesValid );
        } else {
            exclude = !( scansValid || baselinesValid );
        }

        if ( exclude ) {
            possibleExcludeIds.emplace_back( thisSource->getId(), thisSource->getNTotalScans() );
        }
    }

    // exclude only half the sources
    if ( parameters_.currentIteration < parameters_.numberOfGentleSourceReductions ) {
        std::sort( possibleExcludeIds.begin(), possibleExcludeIds.end(),
                   []( auto &left, auto &right ) { return left.second < right.second; } );

        int counter = 0;

        // exclude all sources with zero scans
        for ( const auto &any : possibleExcludeIds ) {
            const auto &thisSource = sourceList_.refSource( any.first );

            if ( any.second == 0 ) {
                excludedScans += thisSource->getNTotalScans();
                excludedBaselines += thisSource->getNObs();
                excludedSources.push_back( thisSource->getName() );
                newScheduleNecessary = true;
                thisSource->referencePARA().setGlobalAvailable( false );
                continue;
            }
        }

        auto diff = possibleExcludeIds.size() - excludedSources.size();
        // exclude half of the remaining sources
        for ( const auto &any : possibleExcludeIds ) {
            const auto &thisSource = sourceList_.refSource( any.first );

            if ( any.second == 0 ) {
                continue;
            }

            if ( counter < diff * parameters_.reduceFactor ) {
                excludedScans += thisSource->getNTotalScans();
                excludedBaselines += thisSource->getNObs();
                excludedSources.push_back( thisSource->getName() );
                newScheduleNecessary = true;
                thisSource->referencePARA().setGlobalAvailable( false );
                ++counter;
            }
        }

        // exclude all sources
    } else {
        for ( const auto &any : possibleExcludeIds ) {
            const auto &thisSource = sourceList_.refSource( any.first );

            excludedScans += thisSource->getNTotalScans();
            excludedBaselines += thisSource->getNObs();
            excludedSources.push_back( thisSource->getName() );
            newScheduleNecessary = true;
            thisSource->referencePARA().setGlobalAvailable( false );
        }
    }

    if ( parameters_.currentIteration > parameters_.maxNumberOfIterations ) {
        newScheduleNecessary = false;
        message.append( "max number of iterations reached " );
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "max number of iterations reached";
#endif
    }
    if ( possibleExcludeIds.size() < parameters_.minNumberOfSourcesToReduce ) {
        newScheduleNecessary = false;
        message.append( "only " )
            .append( to_string( excludedSources.size() ) )
            .append( " sources have to be excluded (minimum = " )
            .append( to_string( parameters_.minNumberOfSourcesToReduce ) )
            .append( ") " );
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "not enough sources left for new iteration";
#endif
    }

    if ( newScheduleNecessary && excludedScans > 0 ) {
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "new schedule with reduced source list necessary";
#endif
        message.append( "new schedule with reduced source list necessary" );
        AstrometricCalibratorBlock::nextBlock = 0;
        unsigned long sourcesLeft = consideredSources - excludedSources.size();
        of << boost::format( "|%|143t||\n" );
        if ( sourcesLeft < 10 ) {
            of << boost::format( "| %=140s |\n" ) % message;
            string message2 = ( boost::format( "Abortion: only %d sources left" ) % sourcesLeft ).str();
            of << boost::format( "| %=140s |\n" ) % message2;
            of << boost::format( "|%|143t||\n" );
            of << boost::format( "|%|143T-||\n" );
            return false;
        }
        string message2 = ( boost::format( "creating new schedule with %d sources" ) % sourcesLeft ).str();
        of << boost::format( "| %=140s |\n" ) % message;
        of << boost::format( "| %=140s |\n" ) % message2;
        of << boost::format( "|%|143t||\n" );
        of << boost::format( "|%|143T-||\n" );

        util::outputObjectList( "List of removed sources", excludedSources, of );

        scans_.clear();
        for ( auto &any : network_.refStations() ) {
            any.clearObservations();
        }
        for ( const auto &any : sourceList_.refSources() ) {
            any->clearObservations();
        }
        for ( auto &any : network_.refSkyCoverages() ) {
            any.clearObservations();
        }
        for ( auto &any : network_.refBaselines() ) {
            any.clearObservations();
            any.setNextEvent( 0 );
        }

    } else {
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "no new iteration needed";
#endif
        message.append( "no new iteration needed!" );
        of << boost::format( "|%|143t||\n" );
        of << boost::format( "| %=140s |\n" ) % message;
        of << boost::format( "|%|143t||\n" );
        of << boost::format( "|%|143T-||\n" );
        newScheduleNecessary = false;
    }
    return newScheduleNecessary;
}


void Scheduler::changeStationAvailability( const boost::optional<StationEndposition> &endposition,
                                           StationEndposition::change change ) {
    switch ( change ) {
        case StationEndposition::change::start: {
            for ( auto &sta : network_.refStations() ) {
                sta.referencePARA().available = endposition->getStationPossible( sta.getId() );
            }
            break;
        }
        case StationEndposition::change::end: {
            for ( auto &sta : network_.refStations() ) {
                sta.referencePARA().available =
                    endposition->getStationAvailable( sta.getId() ) || sta.getPARA().available;
            }
            break;
        }
        default:
            break;
    }
}


void Scheduler::startScanSelectionBetweenScans( unsigned int duration, std::ofstream &of, Scan::ScanType type,
                                                bool output, bool ignoreTagalong ) {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "start scan selection between scans";
#endif
    bool changeSourcePara = false;
    if ( type == Scan::ScanType::fillin && ( parameters_.fillinmodeAPosteriori_minSta.is_initialized() ||
                                             parameters_.fillinmodeAPosteriori_minRepeat.is_initialized() ) ) {
        changeSourcePara = true;
    }

    auto changeSourcePara_function = [this]() {
        for ( const auto &src : sourceList_.refSources() ) {
            auto &para = src->referencePARA();
            if ( parameters_.fillinmodeAPosteriori_minSta.is_initialized() ) {
                para.minNumberOfStations = *parameters_.fillinmodeAPosteriori_minSta;
            }
            if ( parameters_.fillinmodeAPosteriori_minRepeat.is_initialized() ) {
                para.minRepeat = *parameters_.fillinmodeAPosteriori_minRepeat;
            }
        }
    };

    // save number of predefined scans (new scans will be added at end of those)
    auto nMainScans = static_cast<int>( scans_.size() );
    if ( nMainScans == 0 ) {
        return;
    }

    // reset all events
    resetAllEvents( of );
    if ( changeSourcePara ) {
        changeSourcePara_function();
    }

    // ####### FIRST SCAN #######
    // look through all stations and set first scan to true
    for ( auto &thisSta : network_.refStations() ) {
        thisSta.referencePARA().firstScan = true;
    }
    // loop through all upcoming scans and set endposition
    boost::optional<StationEndposition> endposition( network_.getNSta() );
    for ( int j = 0; j < nMainScans; ++j ) {
        const Scan &nextScan = scans_[j];
        bool nextRequired = true;
        for ( int k = 0; k < nextScan.getNSta(); ++k ) {
            endposition->addPointingVectorAsEndposition( nextScan.getPointingVector( k ) );
            if ( endposition->everyStationInitialized() ) {
                nextRequired = false;
                break;
            }
        }
        if ( !nextRequired ) {
            break;
        }
    }
    endposition->setStationAvailable( network_.getStations() );
    endposition->checkStationPossibility( network_.getStations() );

    // recursively start scan selection
    boost::optional<Subcon> subcon = boost::none;
    unsigned int endTime = scans_[0].getTimes().getScanTime( Timestamp::start );
    if ( output ) {
        string s1 = TimeSystem::time2string( 0 );
        string s2 = TimeSystem::time2string( endTime );
        double dur = ( endTime - 0 ) / 3600.;
        of << boost::format( "|%|143t||\n" );
        string tmp = ( boost::format( "start scan selection:   %s - %s (%5.2f h)" ) % s1 % s2 % dur ).str();
        of << boost::format( "|%=142s|\n" ) % tmp;
        of << boost::format( "|%|143t||\n" );
        of << boost::format( "|%|143T-||\n" );
    }
    startScanSelection( endTime, of, type, endposition, subcon, 0 );

    // update slew times
    for ( int idx = 0; idx < scans_[0].getNSta(); ++idx ) {
        const PointingVector &pv_slew_end = scans_[0].getPointingVector( idx );
        unsigned long staid = pv_slew_end.getStaid();
        const auto &sta = network_.getStation( staid );
        for ( int j = scans_.size() - 1; j >= nMainScans; --j ) {
            const Scan &tmp = scans_[j];
            if ( tmp.findIdxOfStationId( staid ).is_initialized() ) {
                int idx2 = *tmp.findIdxOfStationId( staid );
                const PointingVector &pv_slew_start = tmp.getPointingVector( idx2, Timestamp::end );
                boost::optional<unsigned int> oSlewTime = sta.slewTime( pv_slew_start, pv_slew_end );
                unsigned int slewTime = oSlewTime.get();
                unsigned int endOfLastScan = tmp.getTimes().getObservingTime( idx2, Timestamp::end );
                scans_[0].referenceTime().updateAfterFillinmode( idx, endOfLastScan, sta.getPARA().systemDelay,
                                                                 slewTime, sta.getPARA().preob );
                break;
            }
        }
    }

    // add tagalong
    for ( auto &sta : network_.refStations() ) {
        unsigned int time = scans_[0].getTimes().getScanTime();
        bool dummy = false;
        sta.checkForNewEvent( time, dummy );
        bool tagalong = sta.checkForTagalongMode( TimeSystem::duration );
        if ( tagalong ) {
            auto &skyCoverage = network_.refSkyCoverage( network_.getStaid2skyCoverageId().at( sta.getId() ) );
            startTagelongMode( sta, skyCoverage, of, false );
        }
    }

    //    updateTimes(scans_[0]);

    // reset all events
    resetAllEvents( of, false );
    if ( changeSourcePara ) {
        changeSourcePara_function();
    }

    // loop through all predefined scans
    for ( int i = 0; i < nMainScans; ++i ) {
        // look through all stations of last scan and set current pointing vector to last scan
        Scan &lastScan = scans_[i];
        if ( type != Scan::ScanType::fillin ) {
            of << boost::format( "|%|143t||\n" );
            of << boost::format( "|%=142s|\n" ) % "a priori scan";
            of << boost::format( "|%|143t||\n" );
            of << boost::format( "|%|143T-||\n" );
        }
        if ( output ) {
            lastScan.output( numeric_limits<unsigned long>::max(), network_,
                             sourceList_.getSource( lastScan.getSourceId() ), of );
        }


        // check if there was an new upcoming event in the meantime
        resetAllEvents(of, false);
        unsigned int startTime = lastScan.getTimes().getScanTime(Timestamp::end);
        checkForNewEvents( startTime, false, of, false );
        if (ignoreTagalong) {
            ignoreTagalongParameter();
        }
        if (changeSourcePara) {
            changeSourcePara_function();
        }

        // set minimum required slew time
        for ( int k = 0; k < lastScan.getNSta(); ++k ) {
            const auto &pv = lastScan.getPointingVector( k, Timestamp::end );
            unsigned long staid = pv.getStaid();
            unsigned int time = pv.getTime();
            Station &thisSta = network_.refStation( staid );
            if ( time >= thisSta.getCurrentTime() ) {
                thisSta.setCurrentPointingVector( pv );
                thisSta.referencePARA().firstScan = false;
                thisSta.referencePARA().overheadTimeDueToDataWriteSpeed(
                        lastScan.getTimes().getObservingDuration(k));
            }
        }


        // loop through all upcoming scans and set endposition
        boost::optional<StationEndposition> endposition( network_.getNSta() );
        for ( int j = i + 1; j < nMainScans; ++j ) {
            const Scan &nextScan = scans_[j];
            bool nextRequired = true;
            for ( int k = 0; k < nextScan.getNSta(); ++k ) {
                endposition->addPointingVectorAsEndposition( nextScan.getPointingVector( k ) );
                if ( endposition->everyStationInitialized() ) {
                    nextRequired = false;
                    break;
                }
            }
            if ( !nextRequired ) {
                break;
            }
        }
        endposition->setStationAvailable( network_.getStations() );
        endposition->checkStationPossibility( network_.getStations() );


        // recursively start scan selection
        boost::optional<Subcon> subcon = boost::none;
        unsigned int endTime;
        if ( i + 1 != nMainScans ) {
            endTime = scans_[i + 1].getTimes().getScanTime( Timestamp::start );
        } else {
            endTime = duration;
        }
        bool consecutive = false;
        if ( output ) {
            string s1 = TimeSystem::time2string( startTime );
            string s2 = TimeSystem::time2string( endTime );
            if ( endTime > startTime ) {
                double dur = ( endTime - startTime ) / 3600.;
                of << boost::format( "|%|143t||\n" );
                string tmp = ( boost::format( "start scan selection:   %s - %s (%5.2f h)" ) % s1 % s2 % dur ).str();
                of << boost::format( "|%=142s|\n" ) % tmp;
                of << boost::format( "|%|143t||\n" );
                of << boost::format( "|%|143T-||\n" );
            } else {
                consecutive = true;
            }
        }
        if ( !consecutive ) {
            startScanSelection( endTime, of, type, endposition, subcon, 0 );
        }

        // update slew times
        if ( i < nMainScans - 1 ) {
            vector<char> flagStationUpdated( network_.getNSta(), false );
            Scan &scanToUpdate = scans_[i + 1];
            for ( int idx = 0; idx < scanToUpdate.getNSta(); ++idx ) {
                const PointingVector &pv_slew_end = scans_[i + 1].getPointingVector( idx );
                unsigned long staid = pv_slew_end.getStaid();
                const auto &sta = network_.getStation( staid );

                unsigned int endOfLastScan = 0;
                PointingVector pv_slew_start( numeric_limits<unsigned long>::max(),
                                              numeric_limits<unsigned long>::max() );
                pv_slew_start.setTime( 0 );
                for ( int j = i; j >= 0; --j ) {
                    const Scan &tmp = scans_[j];
                    if ( tmp.findIdxOfStationId( staid ).is_initialized() ) {
                        int idx2 = *tmp.findIdxOfStationId( staid );
                        const PointingVector &tmp_pv = tmp.getPointingVector( idx2, Timestamp::end );
                        if ( tmp_pv.getTime() < pv_slew_end.getTime() ) {
                            pv_slew_start = tmp.getPointingVector( idx2, Timestamp::end );
                            endOfLastScan = tmp.getTimes().getObservingTime( idx2, Timestamp::end );
                            break;
                        }
                    }
                }
                for ( int j = scans_.size() - 1; j >= nMainScans; --j ) {
                    const Scan &tmp = scans_[j];
                    if ( tmp.findIdxOfStationId( staid ).is_initialized() ) {
                        int idx2 = *tmp.findIdxOfStationId( staid );
                        const PointingVector &pv_slew_start_2 = tmp.getPointingVector( idx2, Timestamp::end );
                        if ( pv_slew_start.getTime() < pv_slew_start_2.getTime() ) {
                            pv_slew_start = pv_slew_start_2;
                            endOfLastScan = tmp.getTimes().getObservingTime( idx2, Timestamp::end );
                            break;
                        }
                    }
                }
                if ( pv_slew_start.getStaid() != numeric_limits<unsigned long>::max() ) {
                    boost::optional<unsigned int> oSlewTime = sta.slewTime( pv_slew_start, pv_slew_end );
                    unsigned int slewTime = oSlewTime.get();
                    scans_[i + 1].referenceTime().updateAfterFillinmode( idx, endOfLastScan, sta.getPARA().systemDelay,
                                                                         slewTime, sta.getPARA().preob );
                }
            }
        }


        // add tagalong
        if ( !consecutive && i + 1 < scans_.size() ) {
            for ( auto &sta : network_.refStations() ) {
                unsigned int time = scans_[i + 1].getTimes().getScanTime();
                bool dummy = false;
                sta.checkForNewEvent( time, dummy );
                bool tagalong = sta.checkForTagalongMode( TimeSystem::duration );
                if ( tagalong ) {
                    auto &skyCoverage = network_.refSkyCoverage( network_.getStaid2skyCoverageId().at( sta.getId() ) );
                    startTagelongMode( sta, skyCoverage, of, false );
                }
            }
        }

        //        updateTime(scans_[i+1]);
    }
    // sort scans at the end
    sortSchedule( Timestamp::start );
}

void Scheduler::calibratorBlocks( std::ofstream &of ) {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "fix fringeFinder block scans";
#endif
    auto tmp = parameters_.subnetting;
    if ( !CalibratorBlock::subnetting ) {
        parameters_.subnetting = nullptr;
    }

    for ( const auto &block : calib_ ) {
        std::fill( CalibratorBlock::stationFlag.begin(), CalibratorBlock::stationFlag.end(), false );
        of << boost::format( "|%|143t||\n" );
        of << boost::format( "|%=142s|\n" ) % "start calibration block";
        of << boost::format( "|%|143t||\n" );
        of << boost::format( "|%|143T-||\n" );

        unsigned int time = block.getStartTime();
        if ( time < TimeSystem::duration ) {
            checkForNewEvents( time, false, of, false );
            for ( const auto &src : sourceList_.refSources() ) {
                src->referencePARA().fixedScanDuration = block.getDuration();
                if ( !block.isAllowedSource( src->getName() ) ) {
                    src->referencePARA().available = false;
                } else {
                    src->referencePARA().available = true;
                }
            }

            for ( auto &thisStation : network_.refStations() ) {
                PointingVector pv( thisStation.getCurrentPointingVector() );
                pv.setTime( time );
                thisStation.setCurrentPointingVector( pv );
                thisStation.referencePARA().firstScan = true;
            }

            // start scheduling
            int i_scan = 0;
            while ( i_scan < block.getNScans() ) {
                Subcon subcon = createSubcon( parameters_.subnetting, Scan::ScanType::fringeFinder );
                subcon.generateCalibratorScore( network_, sourceList_, currentObservingMode_ );
                vector<Scan> bestScans = subcon.selectBest( network_, sourceList_, currentObservingMode_ );

                // check end time of best possible next scan
                unsigned int maxScanEnd = 0;
                for ( const auto &any : bestScans ) {
                    if ( any.getTimes().getScanTime( Timestamp::end ) > maxScanEnd ) {
                        maxScanEnd = any.getTimes().getScanTime( Timestamp::end );
                    }
                }
                if ( maxScanEnd > TimeSystem::duration ) {
                    break;
                }

                // check if end time triggers a new event
                bool hardBreak = checkForNewEvents( maxScanEnd, true, of, true );
                if ( hardBreak ) {
                    continue;
                }
                for ( const auto &src : sourceList_.refSources() ) {
                    src->referencePARA().fixedScanDuration = block.getDuration();
                    if ( !block.isAllowedSource( src->getName() ) ) {
                        src->referencePARA().available = false;
                    } else {
                        src->referencePARA().available = true;
                    }
                }

                // the best scans are now already fixed. add observing duration to total observing duration to avoid
                // fillin mode scans which extend the allowed total observing time.
                for ( const auto &scan : bestScans ) {
                    for ( int i = 0; i < scan.getNSta(); ++i ) {
                        unsigned long staid = scan.getStationId( i );
                        unsigned int obsDur = scan.getTimes().getObservingDuration( i );
                        network_.refStation( staid ).addObservingTime( obsDur );
                        CalibratorBlock::stationFlag[staid] = true;
                    }
                }

                // update best possible scans
                nSingleScansConsidered += subcon.getNumberSingleScans();
                nSubnettingScansConsidered += subcon.getNumberSubnettingScans();
                for ( auto &bestScan : bestScans ) {
                    update( bestScan, of );
                }
                ++i_scan;

                if ( CalibratorBlock::tryToIncludeAllStationFlag &&
                     std::all_of( CalibratorBlock::stationFlag.begin(), CalibratorBlock::stationFlag.end(),
                                  []( bool i ) { return i; } ) ) {
                    break;
                }
            }
        }
    }
    if ( !CalibratorBlock::subnetting ) {
        parameters_.subnetting = tmp;
    }
    resetAllEvents( of );
}

void Scheduler::parallacticAngleBlocks( ofstream &of ) {
    if ( ParallacticAngleBlock::nscans == 0 ) {
        return;
    }
    of << boost::format( "|%|143t||\n" );
    of << boost::format( "|%=142s|\n" ) % "schedule calibrator scan (rapid parallactic angle change)";
    of << boost::format( "|%|143t||\n" );
    of << boost::format( "|%|143T-||\n" );

    int cadence = ParallacticAngleBlock::cadence;
    sortSchedule();

    vector<Scan> allBestScans;
    vector<unsigned long> alreadyScheduled;

    for ( int time = cadence; time <= TimeSystem::duration - cadence; time += cadence ) {
        bool flagOverlap = false;
        for ( const auto &scan : scans_ ) {
            unsigned int scan_start = scan.getTimes().getObservingTime();
            unsigned int scan_end = scan.getTimes().getObservingTime( Timestamp::end );
            if ( ( scan_start <= time && scan_end >= time ) ||
                 ( time <= scan_start && time + DifferentialParallacticAngleBlock::duration >= scan_start ) ) {
                flagOverlap = true;
                break;
            }
        }
        if ( flagOverlap ) {
            continue;
        }

        checkForNewEvents( time, false, of, false );
        for ( const auto &src : sourceList_.refSources() ) {
            src->referencePARA().fixedScanDuration = ParallacticAngleBlock::duration;
            if ( !ParallacticAngleBlock::isAllowedSource( src->getId() ) ) {
                src->referencePARA().available = false;
            } else {
                src->referencePARA().available = true;
            }
        }

        for ( auto &thisStation : network_.refStations() ) {
            PointingVector pv( thisStation.getCurrentPointingVector() );
            pv.setTime( time );
            thisStation.setCurrentPointingVector( pv );
            thisStation.referencePARA().firstScan = true;
        }

        Subcon subcon = createSubcon( nullptr, Scan::ScanType::parallacticAngle );
        subcon.generateCalibratorScore( network_, sourceList_, currentObservingMode_ );
        vector<Scan> bestScans = subcon.selectBest( network_, sourceList_, currentObservingMode_ );
        if ( bestScans.size() == 1 ) {
            allBestScans.push_back( bestScans[0] );
        }
    }
    resetAllEvents( of );

    // push data into queue
    std::priority_queue<std::pair<double, unsigned long>> q;
    for ( unsigned long i = 0; i < allBestScans.size(); ++i ) {
        q.push( std::pair<double, unsigned long>( allBestScans[i].getScore(), i ) );
    }

    // find n highest scoring scans
    // NOTE: do not increase "i" here but only later (might be that the highest value scan is not valid)
    for ( int i = 0; i < ParallacticAngleBlock::nscans; ) {
        if ( q.empty() ) {
            break;
        }

        // get index of scan(s) with highest score and remove it from list
        unsigned long idx = q.top().second;
        q.pop();

        // get scan with highest score
        Scan &thisScan = allBestScans[idx];
        if ( find( alreadyScheduled.begin(), alreadyScheduled.end(), thisScan.getSourceId() ) !=
             alreadyScheduled.end() ) {
            continue;
        }
        resetAllEvents( of );
        checkForNewEvents( thisScan.getTimes().getObservingTime(), false, of, false );
        if ( thisScan.noInterception( scans_, network_ ) ) {
            alreadyScheduled.push_back( thisScan.getSourceId() );
            update( thisScan, of );
            sortSchedule();
            ++i;
        }
    }
}


void Scheduler::differentialParallacticAngleBlocks( ofstream &of ) {
    if ( DifferentialParallacticAngleBlock::nscans == 0 ) {
        return;
    }
    of << boost::format( "|%|143t||\n" );
    of << boost::format( "|%=142s|\n" ) % "schedule calibrator scan (differential parallactic angle)";
    of << boost::format( "|%|143t||\n" );
    of << boost::format( "|%|143T-||\n" );

    int cadence = DifferentialParallacticAngleBlock::cadence;
    sortSchedule();

    vector<Scan> allBestScans;
    vector<unsigned long> alreadyScheduled;
    for ( int time = cadence; time <= TimeSystem::duration - cadence; time += cadence ) {
        bool flagOverlap = false;
        for ( const auto &scan : scans_ ) {
            unsigned int scan_start = scan.getTimes().getObservingTime();
            unsigned int scan_end = scan.getTimes().getObservingTime( Timestamp::end );
            if ( ( scan_start <= time && scan_end >= time ) ||
                 ( time <= scan_start && time + DifferentialParallacticAngleBlock::duration >= scan_start ) ) {
                flagOverlap = true;
                break;
            }
        }
        if ( flagOverlap ) {
            continue;
        }

        checkForNewEvents( time, false, of, false );
        for ( const auto &src : sourceList_.refSources() ) {
            src->referencePARA().fixedScanDuration = DifferentialParallacticAngleBlock::duration;
            if ( !DifferentialParallacticAngleBlock::isAllowedSource( src->getId() ) ) {
                src->referencePARA().available = false;
            } else {
                src->referencePARA().available = true;
            }
        }

        for ( auto &thisStation : network_.refStations() ) {
            PointingVector pv( thisStation.getCurrentPointingVector() );
            pv.setTime( time );
            thisStation.setCurrentPointingVector( pv );
            thisStation.referencePARA().firstScan = true;
        }

        Subcon subcon = createSubcon( nullptr, Scan::ScanType::diffParallacticAngle );
        subcon.generateCalibratorScore( network_, sourceList_, currentObservingMode_ );
        vector<Scan> bestScans = subcon.selectBest( network_, sourceList_, currentObservingMode_ );
        if ( bestScans.size() == 1 ) {
            allBestScans.push_back( bestScans[0] );
        }
    }
    resetAllEvents( of );

    // push data into queue
    std::priority_queue<std::pair<double, unsigned long>> q;
    for ( unsigned long i = 0; i < allBestScans.size(); ++i ) {
        q.push( std::pair<double, unsigned long>( allBestScans[i].getScore(), i ) );
    }

    // find n highest scoring scans
    // NOTE: do not increase "i" here but only later (might be that the highest value scan is not valid)
    for ( int i = 0; i < DifferentialParallacticAngleBlock::nscans; ) {
        if ( q.empty() ) {
            break;
        }

        // get index of scan(s) with highest score and remove it from list
        unsigned long idx = q.top().second;
        q.pop();

        // get scan with highest score
        Scan &thisScan = allBestScans[idx];
        if ( find( alreadyScheduled.begin(), alreadyScheduled.end(), thisScan.getSourceId() ) !=
             alreadyScheduled.end() ) {
            continue;
        }

        resetAllEvents( of );
        checkForNewEvents( thisScan.getTimes().getObservingTime(), false, of, false );
        if ( thisScan.noInterception( scans_, network_ ) ) {
            alreadyScheduled.push_back( thisScan.getSourceId() );
            update( thisScan, of );
            sortSchedule();
            ++i;
        }
    }
}


void Scheduler::highImpactScans( HighImpactScanDescriptor &himp, ofstream &of ) {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "fix high impact scans";
#endif

    of << boost::format( "|%|143T-||\n" );
    of << boost::format( "|%|143t||\n" );

    of << boost::format( "|%=142s|\n" ) % "fixing high impact scans";
    of << boost::format( "|%|143t||\n" );
    of << boost::format( "|%|143T-||\n" );

    unsigned int interval = himp.getInterval();
    int n = TimeSystem::duration / interval;

    // look for possible high impact scans
    for ( unsigned int iTime = 0; iTime < n; ++iTime ) {
        // check for new event and changes current pointing vector as well as first scan
        unsigned int time = iTime * interval;
        checkForNewEvents( time, false, of, false );
        for ( auto &thisStation : network_.refStations() ) {
            PointingVector pv( thisStation.getId(), numeric_limits<unsigned long>::max() );
            pv.setTime( time );
            pv.setAz( ( thisStation.getCableWrap().getNLow() + thisStation.getCableWrap().getNUp() ) / 2 );
            pv.setEl( 0 );
            thisStation.setCurrentPointingVector( pv );
            thisStation.referencePARA().firstScan = true;
        }

        // create all possible high impact scan pointing vectors for this time
        himp.possibleHighImpactScans( iTime, network_, sourceList_ );
    }

    // create the actual scans
    himp.updateHighImpactScans( network_, sourceList_, currentObservingMode_, parameters_.subnetting );

    himp.updateLogfile( of );

    // select bestScans
    vector<Scan> bestScans;
    do {
        bestScans = himp.highestImpactScans( network_, sourceList_, currentObservingMode_ );
        for ( auto &scan : bestScans ) {
            const auto &source = sourceList_.getSource( scan.getSourceId() );
            if ( himp.isCorrectHighImpactScan( scan, scans_, source ) ) {
                update( scan, of );

                for ( auto &thisStation : network_.refStations() ) {
                    thisStation.referencePARA().firstScan = true;
                }
            }
        }
    } while ( himp.hasMoreScans() );

    sortSchedule( Timestamp::start );

    of << boost::format( "|%|143T-||\n" );
    of << boost::format( "|%|143t||\n" );
    of << boost::format( "|%=142s|\n" ) % "start with normal scan selection";
    of << boost::format( "|%|143t||\n" );
    of << boost::format( "|%|143T-||\n" );

    // reset all events
    resetAllEvents( of );

    for ( auto &thisStation : network_.refStations() ) {
        PointingVector pv( thisStation.getId(), numeric_limits<unsigned long>::max() );
        pv.setTime( 0 );
        pv.setAz( ( thisStation.getCableWrap().getNLow() + thisStation.getCableWrap().getNUp() ) / 2 );
        pv.setEl( 0 );
        thisStation.setCurrentPointingVector( pv );
        thisStation.referencePARA().firstScan = true;
    }
}


void Scheduler::resetAllEvents( std::ofstream &of, bool resetCurrentPointingVector ) {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "reset all events";
#endif

    // reset all events
    for ( auto &any : network_.refStations() ) {
        if ( resetCurrentPointingVector ) {
            PointingVector pv( any.getId(), numeric_limits<unsigned long>::max() );
            pv.setTime( 0 );
            pv.setAz( ( any.getCableWrap().getNLow() + any.getCableWrap().getNUp() ) / 2 );
            pv.setEl( 0 );
            any.setCurrentPointingVector( pv );
        }
        any.setNextEvent( 0 );
    }
    for ( const auto &any : sourceList_.refSources() ) {
        any->setNextEvent( 0 );
    }
    for ( auto &any : network_.refBaselines() ) {
        any.setNextEvent( 0 );
    }
    checkForNewEvents( 0, false, of, false );
    if ( resetCurrentPointingVector ){
        for ( auto &any : network_.refStations() ) {
            any.referencePARA().firstScan = true;
        }
    }
}


void Scheduler::idleToScanTime( Timestamp ts, std::ofstream &of ) {
    switch ( ts ) {
        case Timestamp::start:
#ifdef VIESCHEDPP_LOG
            if ( Flags::logDebug )
                BOOST_LOG_TRIVIAL( debug ) << "start changing idle to observing time at start of scan";
#endif
            of << boost::format( "|%|143t||\n" );
            of << boost::format( "|%=142s|\n" ) % "increasing observing time at start of scan";
            of << boost::format( "|%|143t||\n" );
            of << boost::format( "|%|143T-||\n" );
            break;
        case Timestamp::end:
#ifdef VIESCHEDPP_LOG
            if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "start changing idle to observing time at end of scan";
#endif
            of << boost::format( "|%|143t||\n" );
            of << boost::format( "|%=142s|\n" ) % "increasing observing time at end of scan";
            of << boost::format( "|%|143t||\n" );
            of << boost::format( "|%|143T-||\n" );

            break;
        default:
            break;
    }

    // hard copy previous observing times
    map<unsigned long, ScanTimes> oldScanTimes;

    for ( const auto &scan : scans_ ) {
        oldScanTimes.insert( { scan.getId(), scan.getTimes() } );
    }

    for ( auto &thisSta : network_.refStations() ) {
        // check if observations from this station should be extended
        if ( find( parameters_.idleToObservingTime_staids.begin(), parameters_.idleToObservingTime_staids.end(),
                   thisSta.getId() ) == parameters_.idleToObservingTime_staids.end() ) {
            continue;
        }

        // reset all events
        resetAllEvents( of );
        unsigned long staid = thisSta.getId();

        // sort schedule based on this station
        sortSchedule( staid );

        bool first = true;

        // loop over all scans
        for ( int iscan1 = 0; iscan1 < scans_.size(); ++iscan1 ) {
            // get scan1
            auto &scan1 = scans_[iscan1];

            // skip scan if it does not contain this station
            if ( !scan1.findIdxOfStationId( staid ).is_initialized() ) {
                continue;
            }

            // increase observing time to session start in case of first scan
            if ( first && ts == Timestamp::start ) {
                first = false;

                int staidx1 = static_cast<int>( *scan1.findIdxOfStationId( staid ) );
                const auto &thisSource = sourceList_.getSource( scan1.getSourceId() );
                const PointingVector &pv1 = scan1.getPointingVector( staidx1, Timestamp::start );

                if ( pv1.getTime() != 0 ) {
                    PointingVector variable( pv1 );
                    variable.setId( pv1.getId() );
                    variable.setTime( 0 );
                    thisSta.calcAzEl_rigorous( thisSource, variable );
                    thisSta.getCableWrap().calcUnwrappedAz( pv1, variable );
                    bool valid = true;
                    // check if cable wrap changes
                    if ( abs( pv1.getAz() - variable.getAz() ) > pi / 2 ) {
                        valid = false;
                    }
                    // check if azimuth and elevation is visible
                    if ( !thisSta.isVisible( variable, thisSource->getPARA().minElevation ) ) {
                        valid = false;
                    }
                    int extraTime = pv1.getTime();
                    if ( thisSta.getTotalObservingTime() + extraTime > thisSta.getPARA().maxTotalObsTime ) {
                        valid = false;
                    }
                    if ( valid ) {
                        scan1.setPointingVector( staidx1, move( variable ), Timestamp::start );
                        thisSta.addObservingTime( extraTime );
                    }
                }
            }

            // check for new events
            checkForNewEvents( scan1.getTimes().getScanTime( Timestamp::start ), false, of, false );

            double write_rec_fraction = 1;
            if ( thisSta.getPARA().dataWriteRate.is_initialized() ) {
                write_rec_fraction = *thisSta.getPARA().dataWriteRate / thisSta.getPARA().totalRecordingRate;
            }
            if ( write_rec_fraction < 1 && ts == Timestamp::start ) {
                continue;
            }

            // look for index of next scan with this station
            int iscan2 = iscan1 + 1;
            while ( iscan2 < scans_.size() && !scans_[iscan2].findIdxOfStationId( staid ).is_initialized() ) {
                ++iscan2;
            }

            // continue with next station if end of schedule is reached
            if ( iscan2 == scans_.size() ) {
                continue;
            }

            // get scan2
            auto &scan2 = scans_[iscan2];

            // get indices
            auto staidx1 = static_cast<int>( *scan1.findIdxOfStationId( staid ) );
            auto staidx2 = static_cast<int>( *scan2.findIdxOfStationId( staid ) );

            // get pointing vectors
            const PointingVector &pv1 = scan1.getPointingVector( staidx1, Timestamp::end );
            const PointingVector &pv2 = scan2.getPointingVector( staidx2, Timestamp::start );

            unsigned int oldObservingTime;
            switch ( ts ) {
                case Timestamp::start: {
                    oldObservingTime = scan2.getTimes().getObservingDuration( staidx2 );
                    break;
                }
                case Timestamp::end: {
                    oldObservingTime = scan1.getTimes().getObservingDuration( staidx1 );
                    break;
                }
                default:
                    break;
            }


            // get times
            unsigned int availableTime = pv2.getTime() - pv1.getTime();
            unsigned int prevSlewTime = thisSta.getAntenna().slewTime( pv1, pv2 );
            if ( prevSlewTime < thisSta.getPARA().minSlewtime ) {
                prevSlewTime = thisSta.getPARA().minSlewtime;
            }

            unsigned int fsTime = thisSta.getPARA().systemDelay;
            unsigned int preobTime = thisSta.getPARA().preob;

            // avoid rounding errors
            if ( availableTime < prevSlewTime + fsTime + preobTime ) {
                continue;
            }

            // calc idle time
            unsigned int idleTime = availableTime - prevSlewTime - fsTime - preobTime;
            if ( write_rec_fraction < 1 ) {
                unsigned int duration = scan1.getTimes().getObservingDuration( staidx1 );
                prevSlewTime = max( { prevSlewTime, thisSta.getPARA().minSlewTimeDueToDataWriteSpeed( duration ) } );
                idleTime = write_rec_fraction * ( availableTime - prevSlewTime - fsTime - preobTime );
            }

            if ( idleTime == 0 ) {
                continue;
            }

            // if ts == end: time will be added after pv1
            PointingVector ref = scan1.getPointingVector( staidx1, Timestamp::end );
            ref.setId( pv1.getId() );
            unsigned int variableStartTime = pv1.getTime() + idleTime;
            // if ts == start: time will be added before pv2
            if ( ts == Timestamp::start ) {
                ref = scan2.getPointingVector( staidx2, Timestamp::start );
                ref.setId( pv2.getId() );
                variableStartTime = pv2.getTime() - idleTime;
            }

            const auto &thisSource = sourceList_.getSource( ref.getSrcid() );

            PointingVector variable( ref );
            variable.setId( ref.getId() );

            // set new time for variable
            variable.setTime( variableStartTime );

            // calc new azimuth and elevation
            thisSta.calcAzEl_rigorous( thisSource, variable );

            // unwrap cable wrap near reference pointing vector
            thisSta.getCableWrap().calcUnwrappedAz( ref, variable );

            // check if cable wrap changes
            if ( abs( ref.getAz() - variable.getAz() ) > pi / 2 ) {
                continue;
            }

            // check if azimuth and elevation is visible
            if ( !thisSta.isVisible( variable, thisSource->getPARA().minElevation ) ) {
                continue;
            }

            // calc new slew time
            unsigned int slewTime;
            switch ( ts ) {
                case Timestamp::start: {
                    slewTime = thisSta.getAntenna().slewTime( pv1, variable );
                    if ( slewTime < thisSta.getPARA().minSlewtime ) {
                        slewTime = thisSta.getPARA().minSlewtime;
                    }
                    break;
                }
                case Timestamp::end: {
                    slewTime = thisSta.getAntenna().slewTime( variable, pv2 );
                    if ( slewTime < thisSta.getPARA().minSlewtime ) {
                        slewTime = thisSta.getPARA().minSlewtime;
                    }
                    if ( write_rec_fraction < 1 ) {
                        unsigned int duration = scan1.getTimes().getObservingDuration( staidx1 );
                        slewTime = thisSta.getPARA().minSlewTimeDueToDataWriteSpeed( duration );
                    }
                    break;
                }
                default:
                    break;
            }

            // iteratively adjust new idle time and new slew time until it is equal to previous slew time
            // we also allow 1 second additionalTime (1 sec additional idle time) as a life saver
            // int extraTime = 0;
            int additionalTime = 0;
            bool valid = true;
            int counter = 0;
            while ( slewTime + additionalTime != prevSlewTime && slewTime + additionalTime != prevSlewTime - 1 ) {
                if ( counter > 100 ) {
                    break;
                }
                ++counter;
                additionalTime = prevSlewTime - slewTime;
                // extraTime += additionalTime;
                switch ( ts ) {
                    case Timestamp::start: {
                        variable.setTime( variableStartTime - additionalTime );
                        break;
                    }
                    case Timestamp::end: {
                        variable.setTime( variableStartTime + additionalTime );
                        break;
                    }
                    default:
                        break;
                }

                thisSta.calcAzEl_rigorous( thisSource, variable );
                thisSta.getCableWrap().calcUnwrappedAz( ref, variable );

                // check if cable wrap changes
                if ( abs( ref.getAz() - variable.getAz() ) > pi / 2 ) {
                    valid = false;
                    break;
                }

                // check if azimuth and elevation is visible
                if ( !thisSta.isVisible( variable, thisSource->getPARA().minElevation ) ) {
                    valid = false;
                    break;
                }

                // get new slewtime
                switch ( ts ) {
                    case Timestamp::start: {
                        slewTime = thisSta.getAntenna().slewTime( pv1, variable );
                        if ( slewTime < thisSta.getPARA().minSlewtime ) {
                            slewTime = thisSta.getPARA().minSlewtime;
                        }
                        break;
                    }
                    case Timestamp::end: {
                        slewTime = thisSta.getAntenna().slewTime( variable, pv2 );
                        if ( slewTime < thisSta.getPARA().minSlewtime ) {
                            slewTime = thisSta.getPARA().minSlewtime;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
            if ( counter > 100 ) {
                continue;
            }

            // continue if source is still visible
            if ( !valid ) {
                continue;
            }

            // if scan time would be reduced skip station!
            if ( additionalTime + static_cast<int>( idleTime ) < 0 ) {
                continue;
            }
            switch ( ts ) {
                case Timestamp::start: {
                    if ( ref.getTime() < variable.getTime() ) {
                        continue;
                        break;
                    }
                }
                case Timestamp::end: {
                    if ( ref.getTime() > variable.getTime() ) {
                        continue;
                        break;
                    }
                }
                default:
                    break;
            }

            // adjust observing times
            switch ( ts ) {
                case Timestamp::start: {
                    unsigned int newObservingTime =
                        scan2.getPointingVector( staidx2, Timestamp::end ).getTime() - variable.getTime();
                    if ( scan2.getType() == Scan::ScanType::fringeFinder ||
                         scan2.getType() == Scan::ScanType::diffParallacticAngle ||
                         scan2.getType() == Scan::ScanType::parallacticAngle ||
                         scan2.getType() == Scan::ScanType::astroCalibrator ) {
                        unsigned int maxExtendedObservingTime =
                            min( { thisSource->getPARA().maxScan, thisSta.getPARA().maxScan } );
                        maxExtendedObservingTime =
                            max( { maxExtendedObservingTime, scan2.getTimes().getObservingDuration() } );
                        if ( newObservingTime > maxExtendedObservingTime ) {
                            newObservingTime = maxExtendedObservingTime;
                            variable.setTime( scan2.getTimes().getObservingTime( staidx2, Timestamp::end ) -
                                              newObservingTime );
                            thisSta.calcAzEl_rigorous( thisSource, variable );
                            thisSta.getCableWrap().calcUnwrappedAz( ref, variable );
                        }
                    }
                    unsigned int extraTime = newObservingTime - oldObservingTime;
                    if ( thisSta.getTotalObservingTime() + extraTime < thisSta.getPARA().maxTotalObsTime ) {
                        scan2.setPointingVector( staidx2, move( variable ), Timestamp::start );
                        thisSta.addObservingTime( extraTime );
                    }

                    break;
                }
                case Timestamp::end: {
                    unsigned int newObservingTime =
                        variable.getTime() - scan1.getPointingVector( staidx1, Timestamp::start ).getTime();
                    if ( scan1.getType() == Scan::ScanType::fringeFinder ||
                         scan1.getType() == Scan::ScanType::diffParallacticAngle ||
                         scan1.getType() == Scan::ScanType::parallacticAngle ||
                         scan1.getType() == Scan::ScanType::astroCalibrator ) {
                        unsigned int maxExtendedObservingTime =
                            min( { thisSource->getPARA().maxScan, thisSta.getPARA().maxScan } );
                        maxExtendedObservingTime =
                            max( { maxExtendedObservingTime, scan1.getTimes().getObservingDuration() } );
                        if ( newObservingTime > maxExtendedObservingTime ) {
                            newObservingTime = maxExtendedObservingTime;
                            variable.setTime( scan1.getTimes().getObservingTime( staidx1 ) + newObservingTime );
                            thisSta.calcAzEl_rigorous( thisSource, variable );
                            thisSta.getCableWrap().calcUnwrappedAz( ref, variable );
                        }
                    }
                    unsigned int extraTime = newObservingTime - oldObservingTime;
                    if ( thisSta.getTotalObservingTime() + extraTime < thisSta.getPARA().maxTotalObsTime ) {
                        scan1.setPointingVector( staidx1, move( variable ), Timestamp::end );
                        thisSta.addObservingTime( extraTime );
                    }

                    break;
                }
            }
        }

        // extend observing time until session end in case of last scan
        if ( ts == Timestamp::end ) {
            for ( int iscan = scans_.size() - 1; iscan >= 0; --iscan ) {
                // get lastScan
                auto &lastScan = scans_[iscan];
                auto oidx = lastScan.findIdxOfStationId( staid );

                // skip scan if it does not contain this station
                if ( oidx.is_initialized() ) {
                    auto staidx1 = static_cast<int>( *oidx );

                    const auto &thisSource = sourceList_.getSource( lastScan.getSourceId() );
                    const PointingVector &pv1 = lastScan.getPointingVector( staidx1, Timestamp::end );


                    unsigned int sessionStop;
                    double write_rec_fraction = 1;
                    if ( thisSta.getPARA().dataWriteRate.is_initialized() ) {
                        write_rec_fraction = *thisSta.getPARA().dataWriteRate / thisSta.getPARA().totalRecordingRate;
                    }

                    if ( write_rec_fraction < 1 ) {
                        sessionStop = pv1.getTime() + ( TimeSystem::duration - pv1.getTime() ) * write_rec_fraction;
                    } else {
                        sessionStop = TimeSystem::duration;
                    }

                    if ( pv1.getTime() != 0 ) {
                        PointingVector variable( pv1 );
                        variable.setId( pv1.getId() );
                        variable.setTime( sessionStop );
                        thisSta.calcAzEl_rigorous( thisSource, variable );
                        thisSta.getCableWrap().calcUnwrappedAz( pv1, variable );
                        bool valid = true;
                        // check if cable wrap changes
                        if ( abs( pv1.getAz() - variable.getAz() ) > pi / 2 ) {
                            valid = false;
                        }
                        // check if azimuth and elevation is visible
                        if ( !thisSta.isVisible( variable, thisSource->getPARA().minElevation ) ) {
                            valid = false;
                        }
                        int extraTime = variable.getTime() - pv1.getTime();
                        if ( thisSta.getTotalObservingTime() + extraTime > thisSta.getPARA().maxTotalObsTime ) {
                            valid = false;
                        }

                        if ( valid ) {
                            lastScan.setPointingVector( staidx1, move( variable ), Timestamp::end );
                            thisSta.addObservingTime( extraTime );
                        }
                    }
                    break;
                }
            }
        }
    }

    sortSchedule( Timestamp::start );

    resetAllEvents( of );
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "remove unnecessary observing time (single antenna time)";
#endif
    // remove unnecessary observing times
    for ( auto &thisScan : scans_ ) {
        const auto &thisSource = sourceList_.refSource( thisScan.getSourceId() );
        bool dummy;
        switch ( ts ) {
            case Timestamp::start: {
                thisSource->checkForNewEvent( thisScan.getTimes().getScanTime( Timestamp::end ), dummy );
                break;
            }
            case Timestamp::end: {
                thisSource->checkForNewEvent( thisScan.getTimes().getScanTime( Timestamp::start ), dummy );
                break;
            }
        }

        // loop over all stations and check for down times
        for ( int staidx = 0; staidx < thisScan.getNSta(); ++staidx ) {
            unsigned long staid = thisScan.getStationId( staidx );
            Station &thisSta = network_.refStation( staid );

            // change event to time of scan
            switch ( ts ) {
                case Timestamp::start: {
                    thisSta.checkForNewEvent( thisScan.getPointingVector( staidx, Timestamp::end ).getTime(), dummy );
                    break;
                }
                case Timestamp::end: {
                    thisSta.checkForNewEvent( thisScan.getPointingVector( staidx, Timestamp::start ).getTime(), dummy );
                    break;
                }
            }

            // look for maximum allowed time before/after current event time
            unsigned int maximum = thisSta.maximumAllowedObservingTime( ts );
            unsigned int maxExtendedObservingTime = min( { thisSource->getPARA().maxScan, thisSta.getPARA().maxScan } );
            if ( thisScan.getType() == Scan::ScanType::fringeFinder ||
                 thisScan.getType() == Scan::ScanType::diffParallacticAngle ||
                 thisScan.getType() == Scan::ScanType::parallacticAngle ||
                 thisScan.getType() == Scan::ScanType::astroCalibrator ) {
                maxExtendedObservingTime =
                    max( { maxExtendedObservingTime, thisScan.getTimes().getObservingDuration() } );
            }

            switch ( ts ) {
                case Timestamp::start: {
                    unsigned int newMax = 0;
                    // check underflow
                    if ( thisScan.getTimes().getObservingTime( Timestamp::end ) > maxExtendedObservingTime ) {
                        newMax = thisScan.getTimes().getObservingTime( Timestamp::end ) - maxExtendedObservingTime;
                    }
                    if ( newMax > maximum ) {
                        maximum = newMax;
                    }
                    break;
                }
                case Timestamp::end: {
                    unsigned int newMax =
                        thisScan.getTimes().getObservingTime( Timestamp::start ) + maxExtendedObservingTime;
                    if ( newMax < maximum ) {
                        maximum = newMax;
                    }
                    break;
                }
            }

            // check if it is necessary to adjust observing time
            const PointingVector &pv1 = thisScan.getPointingVector( staidx, ts );
            bool changeToMaximum = false;
            switch ( ts ) {
                case Timestamp::start: {
                    if ( pv1.getTime() < maximum ) {
                        changeToMaximum = true;
                    }
                    break;
                }
                case Timestamp::end: {
                    if ( pv1.getTime() > maximum ) {
                        changeToMaximum = true;
                    }
                    break;
                }
            }

            // adjust observing time
            if ( changeToMaximum ) {
                PointingVector variable( pv1 );
                variable.setId( pv1.getId() );
                variable.setTime( maximum );
                thisSta.calcAzEl_rigorous( thisSource, variable );
                thisSta.getCableWrap().calcUnwrappedAz( pv1, variable );
                bool valid = true;
                // check if cable wrap changes
                if ( abs( pv1.getAz() - variable.getAz() ) > pi / 2 ) {
                    valid = false;
                }
                // check if azimuth and elevation is visible
                if ( !thisSta.isVisible( variable, thisSource->getPARA().minElevation ) ) {
                    valid = false;
                }
                thisScan.setPointingVector( staidx, move( variable ), ts );
                if ( !valid ) {
                    auto txt = boost::format(
                                   "extending observing time to idle time: source %s might not be visible from %s "
                                   "during %s. " ) %
                               thisSource->getName() % thisSta.getName() % TimeSystem::time2timeOfDay( maximum );
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( error ) << txt;
#else
                    cout << "[error] " << txt;
#endif
                    of << "[error] " << txt;
                }
            }
        }

        thisScan.removeUnnecessaryObservingTime( network_, thisSource, of, ts );
    }

    int sum = 0;
    int counter = 0;
    vector<int> sumPerSta( network_.getNSta(), 0 );
    // output
    for ( const auto &scan : scans_ ) {
        const ScanTimes &copyOfScanTimes = oldScanTimes.at( scan.getId() );
        const auto &source = sourceList_.getSource( scan.getSourceId() );

        // check if output is required
        bool change = false;
        for ( int i = 0; i < scan.getNSta(); ++i ) {
            unsigned int oldObservingTime = copyOfScanTimes.getObservingDuration( i );
            unsigned int newObservingTime = scan.getTimes().getObservingDuration( i );
            if ( oldObservingTime != newObservingTime ) {
                change = true;
                break;
            }
        }

        // output if there was a change
        if ( change ) {
            string right = ( boost::format( "source:  %s %s" ) % source->getName() % source->printId() ).str();
            of << boost::format( "| scan: %-15s                                  %85s |\n" ) % scan.printId() % right;

            of << boost::format( "|%|143T-||\n" );
            if ( counter % 5 == 0 ) {
                of << "|     station  | increase |     new duration    | new obs |                         |     old "
                      "duration    | old obs |                          |\n"
                      "|              |    [s]   |    start - end      |   [s]   |                         |    start "
                      "- end      |   [s]   |                          |\n"
                      "|--------------|----------|---------------------|---------|-------------------------|-----------"
                      "----------|---------|--------------------------|\n";
            }

            for ( int i = 0; i < scan.getNSta(); ++i ) {
                unsigned long staid = scan.getStationId( i );
                unsigned int oldObservingTime = copyOfScanTimes.getObservingDuration( i );
                unsigned int newObservingTime = scan.getTimes().getObservingDuration( i );
                if ( oldObservingTime == newObservingTime ) {
                    continue;
                }
                int diff = static_cast<int>( newObservingTime ) - static_cast<int>( oldObservingTime );
                sum += diff;
                sumPerSta[staid] += diff;
                of << boost::format(
                          "|     %-8s |  %+6d  | %8s - %8s |  %5d  |                         | %8s - %8s |  %5d  |     "
                          "                     | %s\n" ) %
                          network_.getStation( staid ).getName() % diff %
                          TimeSystem::time2timeOfDay( scan.getTimes().getObservingTime( i, Timestamp::start ) ) %
                          TimeSystem::time2timeOfDay( scan.getTimes().getObservingTime( i, Timestamp::end ) ) %
                          newObservingTime %
                          TimeSystem::time2timeOfDay( copyOfScanTimes.getObservingTime( i, Timestamp::start ) ) %
                          TimeSystem::time2timeOfDay( copyOfScanTimes.getObservingTime( i, Timestamp::end ) ) %
                          oldObservingTime % scan.getPointingVector( i, ts ).printId();
            }
            of << boost::format( "|%|143T-||\n" );
            ++counter;
        }
    }

    string tmp;
    if ( sum < 60 ) {
        tmp = ( boost::format( "%2d [s]" ) % sum ).str();
    } else if ( sum < 3600 ) {
        tmp = ( boost::format( "%2d [min] %02d [s]" ) % ( sum / 60 ) % ( sum % 60 ) ).str();
    } else {
        tmp = ( boost::format( "%2d [h] %02d [min] %02d [s]" ) % ( sum / 3600 ) % ( sum % 3600 / 60 ) % ( sum % 60 ) )
                  .str();
    }
    of << boost::format(
              "| sum of additional observing time: %-40s                                                               "
              "    |\n" ) %
              tmp;
    unsigned long sumAvg = sum / network_.getNSta();
    if ( sumAvg < 60 ) {
        tmp = ( boost::format( "%2d [s]" ) % sumAvg ).str();
    } else if ( sumAvg < 3600 ) {
        tmp = ( boost::format( "%2d [min] %02d [s]" ) % ( sumAvg / 60 ) % ( sumAvg % 60 ) ).str();
    } else {
        tmp = ( boost::format( "%2d [h] %02d [min] %02d [s]" ) % ( sumAvg / 3600 ) % ( sumAvg % 3600 / 60 ) %
                ( sumAvg % 60 ) )
                  .str();
    }
    of << boost::format(
              "|           on average per station: %-40s                                                               "
              "    |\n" ) %
              tmp;
    of << boost::format( "|%|143T-||\n" );

    bool h = false;
    bool m = false;
    for ( int t : sumPerSta ) {
        if ( t >= 3600 ) {
            h = true;
        }
        if ( t >= 60 ) {
            m = true;
        }
    }

    for ( int i = 0; i < network_.getNSta(); ++i ) {
        int thisSum = sumPerSta[i];
        const Station &thisSta = network_.getStation( i );

        if ( h ) {
            tmp = ( boost::format( "%2d [h] %02d [min] %02d [s]" ) % ( thisSum / 3600 ) % ( thisSum % 3600 / 60 ) %
                    ( thisSum % 60 ) )
                      .str();
        } else if ( m ) {
            tmp = ( boost::format( "%2d [min] %02d [s]" ) % ( thisSum / 60 ) % ( thisSum % 60 ) ).str();
        } else {
            tmp = ( boost::format( "%2d [s]" ) % thisSum ).str();
        }

        of << boost::format(
                  "|                         %-8s: %-40s                                                               "
                  "    |\n" ) %
                  thisSta.getName() % tmp;
    }
    of << boost::format( "|%|143T-||\n" );
}


void Scheduler::sortSchedule( Timestamp ts ) {
    stable_sort( scans_.begin(), scans_.end(), [ts]( const Scan &scan1, const Scan &scan2 ) {
        return scan1.getTimes().getObservingTime( ts ) < scan2.getTimes().getObservingTime( ts );
    } );
}


void Scheduler::sortSchedule( unsigned long staid, Timestamp ts ) {
    stable_sort( scans_.begin(), scans_.end(), [staid, ts]( const Scan &scan1, const Scan &scan2 ) {
        boost::optional<unsigned long> idx1 = scan1.findIdxOfStationId( staid );
        boost::optional<unsigned long> idx2 = scan2.findIdxOfStationId( staid );

        if ( !idx1.is_initialized() && !idx2.is_initialized() ) {
            return scan1.getTimes().getObservingTime( ts ) < scan2.getTimes().getObservingTime( ts );

        } else if ( idx1.is_initialized() && !idx2.is_initialized() ) {
            return scan1.getTimes().getObservingTime( static_cast<int>( *idx1 ), ts ) <
                   scan2.getTimes().getObservingTime( ts );

        } else if ( !idx1.is_initialized() && idx2.is_initialized() ) {
            return scan1.getTimes().getObservingTime( ts ) <
                   scan2.getTimes().getObservingTime( static_cast<int>( *idx2 ), ts );

        } else {
            return scan1.getTimes().getObservingTime( static_cast<int>( *idx1 ), ts ) <
                   scan2.getTimes().getObservingTime( static_cast<int>( *idx2 ), ts );
        }
    } );
}


void Scheduler::writeCalibratorHeader( std::ofstream &of ) {
    of << boost::format( "|%|143t||\n" );
    of << boost::format( "|%=142s|\n" ) % "start calibration block";
    of << boost::format( "|%|143t||\n" );
    of << boost::format( "|%|143T-||\n" );
}


void Scheduler::writeCalibratorStatistics( std::ofstream &of, std::vector<double> &highestElevations,
                                           std::vector<double> &lowestElevations ) {
    of << boost::format( "|%|143t||\n" );
    of << boost::format( "|%=142s|\n" ) % "calibration block summary";
    of << boost::format( "|%|143t||\n" );
    of << boost::format( "|%|143T-||\n" );
    of << boost::format( "|     station  | highest elevation | lowest elevation  |%143t|\n" );
    of << boost::format( "|              |       [deg]       |       [deg]       |%143t|\n" );
    of << boost::format( "|--------------|-------------------|-------------------|%143t|\n" );
    for ( unsigned long i = 0; i < network_.getNSta(); ++i ) {
        const Station &sta = network_.getStation( i );
        double high = highestElevations[i] * rad2deg;
        string highstr;
        if ( high <= .1 ) {
            highstr = "-----";
        } else {
            highstr = ( boost::format( "%5.2f" ) % high ).str();
        }
        double low = lowestElevations[i] * rad2deg;
        string lowstr;
        if ( low > 90 ) {
            lowstr = "-----";
        } else {
            lowstr = ( boost::format( "%5.2f" ) % low ).str();
        }
        of << boost::format( "|     %-8s |       %5s       |       %5s       |%143t|\n" ) % sta.getName() % highstr %
                  lowstr;
    }
    of << boost::format( "|%|143T-||\n" );
    of << boost::format( "|%|143t||\n" );
    of << boost::format( "|%=142s|\n" ) % "finished calibration block";
    of << boost::format( "|%|143t||\n" );
    of << boost::format( "|%|143T-||\n" );
}


bool Scheduler::calibratorUpdate( const std::vector<Scan> &bestScans, std::vector<double> &prevHighElevationScores,
                                  std::vector<double> &prevLowElevationScores, std::vector<double> &highestElevations,
                                  std::vector<double> &lowestElevations ) {
    double lowElevationSlopeStart = AstrometricCalibratorBlock::lowElevationStartWeight;
    double lowElevationSlopeEnd = AstrometricCalibratorBlock::lowElevationFullWeight;

    double highElevationSlopeStart = AstrometricCalibratorBlock::highElevationStartWeight;
    double highElevationSlopeEnd = AstrometricCalibratorBlock::highElevationFullWeight;

    // update prev low/high elevation scores
    for ( const auto &scan : bestScans ) {
        for ( int j = 0; j < scan.getNSta(); ++j ) {
            const PointingVector &pv = scan.getPointingVector( j );
            unsigned long staid = pv.getStaid();

            double el = pv.getEl();
            double lowElScore;
            if ( el > lowElevationSlopeStart ) {
                lowElScore = 0;
            } else if ( el < lowElevationSlopeEnd ) {
                lowElScore = 1;
            } else {
                lowElScore = ( lowElevationSlopeStart - el ) / ( lowElevationSlopeStart - lowElevationSlopeEnd );
            }
            if ( lowElScore > prevLowElevationScores[staid] ) {
                prevLowElevationScores[staid] = lowElScore;
            }
            if ( el < lowestElevations[staid] ) {
                lowestElevations[staid] = el;
            }

            double highElScore;
            if ( el < highElevationSlopeStart ) {
                highElScore = 0;
            } else if ( el > highElevationSlopeEnd ) {
                highElScore = 1;
            } else {
                highElScore = ( el - highElevationSlopeStart ) / ( highElevationSlopeEnd - lowElevationSlopeStart );
            }
            if ( highElScore > prevHighElevationScores[staid] ) {
                prevHighElevationScores[staid] = highElScore;
            }
            if ( el > highestElevations[staid] ) {
                highestElevations[staid] = el;
            }
        }
    }

    bool stopScanSelection = true;
    for ( double any : prevLowElevationScores ) {
        if ( any < .5 ) {
            stopScanSelection = false;
            break;
        }
    }
    if ( stopScanSelection ) {
        for ( double any : prevHighElevationScores ) {
            if ( any < .5 ) {
                stopScanSelection = false;
                break;
            }
        }
    }

    return stopScanSelection;
}


void Scheduler::updateObservingTimes() {
    for ( auto &scan : scans_ ) {
        scan.updateObservingTime();
    }
}


int Scheduler::getNumberOfObservations() const noexcept {
    int n = 0;
    for ( const auto &any : scans_ ) {
        n += any.getNObs();
    }
    return n;
}


void Scheduler::scheduleAPrioriScans( const boost::property_tree::ptree &ptree, ofstream &of ) {
    for ( const auto &any : ptree ) {
        if ( any.first == "scan" ) {
            Scan scan( any.second, network_, sourceList_ );
            const auto &src = sourceList_.getSource( scan.getSourceId() );
            scan.output( numeric_limits<unsigned long>::max(), network_, src, of );
            scans_.push_back( scan );
        }
    }
    sortSchedule();
    resetAllEvents( of );
}
