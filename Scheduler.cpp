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
      path_{std::move( path )},
      network_{std::move( init.network_ )},
      sources_{std::move( init.sources_ )},
      himp_{std::move( init.himp_ )},
      multiSchedulingParameters_{std::move( init.multiSchedulingParameters_ )},
      xml_{init.xml_},
      obsModes_{init.obsModes_},
      currentObservingMode_{obsModes_->getMode( 0 )} {
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

    parameters_.idleToObservingTime = init.parameters_.idleToObservingTime;

    parameters_.andAsConditionCombination = init.parameters_.andAsConditionCombination;
    parameters_.minNumberOfSourcesToReduce = init.parameters_.minNumberOfSourcesToReduce;
    parameters_.maxNumberOfIterations = init.parameters_.maxNumberOfIterations;
    parameters_.numberOfGentleSourceReductions = init.parameters_.numberOfGentleSourceReductions;
    parameters_.reduceFactor = init.parameters_.reduceFactor;

    parameters_.writeSkyCoverageData = false;
}


Scheduler::Scheduler( std::string name, Network network, std::vector<Source> sources, std::vector<Scan> scans,
                      boost::property_tree::ptree xml, std::shared_ptr<ObservingMode> obsModes_ )
    : VieVS_NamedObject( move( name ), nextId++ ),
      network_{std::move( network )},
      sources_{std::move( sources )},
      scans_{std::move( scans )},
      obsModes_{std::move( obsModes_ )},
      currentObservingMode_{nullptr},
      xml_{xml} {}


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

    if ( type == Scan::ScanType::calibrator ) {
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
            subcon.checkIfEnoughTimeToReachEndposition( network_, sources_, opt_endposition );
            subcon.clearSubnettingScans();
            if ( parameters_.subnetting != nullptr ) {
                subcon.createSubnettingScans( parameters_.subnetting, network_, sources_ );
            }
        } else {
            // otherwise calculate new subcon
            subcon = createSubcon( parameters_.subnetting, type, opt_endposition );
        }

        // check algorithm focus corners for intensive sessions
        if ( FocusCorners::startFocusCorner && depth == 0 ) {
            of << boost::format( "| %=140s |\n" ) % "reweight sources to focus observation at corner";
            FocusCorners::reweight( subcon, sources_, of );
            FocusCorners::nextStart += FocusCorners::interval;
        }

        if ( type != Scan::ScanType::calibrator ) {
            // standard case
            subcon.generateScore( network_, sources_ );
        } else {
            // special case for calibrator scans
            subcon.generateScore( prevLowElevationScores, prevHighElevationScores, network_, sources_ );
        }

        unsigned long nSingleScans = subcon.getNumberSingleScans();
        unsigned long nSubnettingScans = subcon.getNumberSubnettingScans();

        // select the best possible next scan(s) and save them under 'bestScans'
        vector<Scan> bestScans;
        if ( type != Scan::ScanType::calibrator ) {
            // standard case
            bestScans = subcon.selectBest( network_, sources_, currentObservingMode_, opt_endposition );
        } else {
            // special calibrator case
            bestScans = subcon.selectBest( network_, sources_, currentObservingMode_, prevLowElevationScores,
                                           prevHighElevationScores, opt_endposition );
        }

        if ( FocusCorners::startFocusCorner ) {
            FocusCorners::reset( bestScans, sources_ );
        }

        // check if you have possible next scan
        if ( bestScans.empty() ) {
            if ( depth == 0 && type != Scan::ScanType::fillin ) {
                if ( type == Scan::ScanType::calibrator ) {
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
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( warning ) << "no valid scan found, checking one minute later";
#else
                cout << "[warning] no valid scan found, checking one minute later\n";
#endif

                of << ( boost::format( "[warning] no valid scan found, checking one minute later: %s\n" ) %
                        TimeSystem::time2string( maxScanEnd ) )
                          .str();
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

        // if end time of best possible next scans is greater than end time of scan selection stop
        if ( maxScanEnd > TimeSystem::duration ) {
            int i = 0;
            while ( i < bestScans.size() ) {
                Scan &any = bestScans[i];
                bool valid =
                    any.prepareForScanEnd( network_, sources_[any.getSourceId()], currentObservingMode_, endTime );
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
            }

            newEndposition->setStationAvailable( network_.getStations() );
            newEndposition->checkStationPossibility( network_.getStations() );

            boost::optional<Subcon> new_opt_subcon( std::move( subcon ) );
            // start recursion for fillin mode scans
            unsigned long scansBefore = scans_.size();
            startScanSelection( newEndposition->getEarliestScanStart(), of, Scan::ScanType::fillin, newEndposition,
                                new_opt_subcon, depth + 1 );

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
                                            idx, thisSta.getCurrentTime(), thisSta.getWaittimes().fieldSystem, slewTime,
                                            thisSta.getWaittimes().preob );
                                    }
                                }
                            } else {
                                valid = false;
                            }

                            if ( !valid ) {
#ifdef VIESCHEDPP_LOG
                                BOOST_LOG_TRIVIAL( warning ) << "check fillin mode scan for station "
                                                             << thisSta.getName() << " prior to this scan";
#else
                                cout << "[warning] check fillin mode scan for station " << thisSta.getName()
                                     << " prior to this scan\n";
#endif

                                of << "[warning] check fillin mode scan for station " << thisSta.getName()
                                   << " prior to this scan!\n";
                            }

                            break;
                        }
                    }
                }
            }
        }

        // update highestElevation and lowestElevation in case of calibrator block
        bool stopScanSelection = false;
        if ( type == Scan::ScanType::calibrator ) {
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
        if ( type == Scan::ScanType::calibrator ) {
            if ( stopScanSelection || countCalibratorScans >= CalibratorBlock::nmaxScans ) {
                break;
            }
        }

        // update number of scan selections if it is a standard scan
        if ( type == Scan::ScanType::standard ) {
            ++Scan::nScanSelections;
            if ( Scan::scanSequence.customScanSequence ) {
                Scan::scanSequence.newScan();
            }
        }

        // check if you need to schedule a calibration block
        if ( type == Scan::ScanType::standard && CalibratorBlock::scheduleCalibrationBlocks ) {
            switch ( CalibratorBlock::cadenceUnit ) {
                case CalibratorBlock::CadenceUnit::scans: {
                    if ( Scan::nScanSelections == CalibratorBlock::nextBlock ) {
                        boost::optional<Subcon> empty_subcon = boost::none;
                        startScanSelection( endTime, of, Scan::ScanType::calibrator, opt_endposition, empty_subcon,
                                            depth + 1 );
                        CalibratorBlock::nextBlock += CalibratorBlock::cadence;
                    }
                    break;
                }
                case CalibratorBlock::CadenceUnit::seconds: {
                    if ( maxScanEnd >= CalibratorBlock::nextBlock ) {
                        boost::optional<Subcon> empty_subcon = boost::none;
                        startScanSelection( endTime, of, Scan::ScanType::calibrator, opt_endposition, empty_subcon,
                                            depth + 1 );
                        CalibratorBlock::nextBlock += CalibratorBlock::cadence;
                    }
                    break;
                }
            }
        }
    }

    // write clibrator statistics
    if ( type == Scan::ScanType::calibrator ) {
        writeCalibratorStatistics( of, highestElevations, lowestElevations );
    }

    // scan selection block is over. Change station availability back to start value
    if ( opt_endposition.is_initialized() ) {
        changeStationAvailability( opt_endposition, StationEndposition::change::end );
    }
}


void Scheduler::start() noexcept {
    string fileName = getName() + "_iteration_" + to_string( parameters_.currentIteration ) + ".txt";
    ofstream of;
    if ( xml_.get( "VieSchedpp.output.iteration_log", true ) ) {
        of.open( path_ + fileName );
    }
    if ( FocusCorners::flag ) {
        FocusCorners::initialize( network_, of );
    }

    if ( network_.getNSta() == 0 || sources_.empty() || network_.getNBls() == 0 ) {
        string e = ( boost::format( "ERROR: number of stations: %d number of baselines: %d number of sources: %d;\n" ) %
                     network_.getNSta() % network_.getNBls() % sources_.size() )
                       .str();
        of << e;
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
    checkForNewEvents( 0, false, of, false );
    listSourceOverview( of );

    boost::optional<StationEndposition> endposition = boost::none;
    boost::optional<Subcon> subcon = boost::none;

    if ( himp_.is_initialized() ) {
        highImpactScans( himp_.get(), of );
    }

    of << ".-----------------------------------------------------------------------------------------------------------"
          "-----------------------------------.\n";
    // check if you have some fixed high impact scans
    if ( scans_.empty() ) {
        // no fixed scans: start creating a schedule
        startScanSelection( TimeSystem::duration, of, Scan::ScanType::standard, endposition, subcon, 0 );

        // sort scans
        sortSchedule( Timestamp::start );

    } else {
        startScanSelectionBetweenScans( TimeSystem::duration, of, Scan::ScanType::standard, true, false );
    }

    // start fillinmode a posterior
    if ( parameters_.fillinmodeAPosteriori ) {
        of << "|-------------------------------------------------------------------------------------------------------"
              "---------------------------------------|\n";
        of << "|                                                                                                       "
              "                                       |\n";
        of << "|                                                        start fillin mode a posteriori                 "
              "                                       |\n";
        of << "|                                                                                                       "
              "                                       |\n";
        of << "|-------------------------------------------------------------------------------------------------------"
              "---------------------------------------|\n";
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
    of << "\n";
    of << "\n";
    of << "summary:\n";
    of << "number of scheduled scans         " << scans_.size() << "\n";
    of << "considered single source scans:   " << nSingleScansConsidered << "\n";
    of << "considered subnetting combiation: " << nSubnettingScansConsidered << "\n";
    of << "total scans considered:           " << nSingleScansConsidered + 2 * nSubnettingScansConsidered << "\n";
    int nobs = std::accumulate( scans_.begin(), scans_.end(), 0,
                                []( int sum, const Scan &any ) { return sum + any.getNObs(); } );
    of << "number of observations:           " << nobs << "\n";
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "created schedule with " << scans_.size() << " scans and " << nobs << " observations";
#else
    cout << "[info] created schedule with " << scans_.size() << " scans and " << nobs << " observations";
#endif
}


Subcon Scheduler::createSubcon( const shared_ptr<Subnetting> &subnetting, Scan::ScanType type,
                                const boost::optional<StationEndposition> &endposition ) noexcept {
    Subcon subcon = allVisibleScans( type, endposition );
    subcon.calcStartTimes( network_, sources_, endposition );
    subcon.updateAzEl( network_, sources_ );
    subcon.constructAllBaselines( network_, sources_ );
    subcon.calcAllBaselineDurations( network_, sources_, currentObservingMode_ );
    subcon.calcAllScanDurations( network_, sources_, endposition );
    subcon.checkIfEnoughTimeToReachEndposition( network_, sources_, endposition );

    if ( subnetting != nullptr ) {
        subcon.createSubnettingScans( subnetting, network_, sources_ );
    }
    return subcon;
}


Subcon Scheduler::allVisibleScans( Scan::ScanType type,
                                   const boost::optional<StationEndposition> &endposition ) noexcept {
    // get latest start time of new scan
    unsigned int currentTime = 0;
    for ( auto &station : network_.getStations() ) {
        if ( station.getCurrentTime() > currentTime ) {
            currentTime = station.getCurrentTime();
        }
    }

    // save all ids of the next observed sources (if there is a required endposition)
    set<unsigned long> observedSources;
    if ( endposition.is_initialized() ) {
        observedSources = endposition->getObservedSources();
    }

    // create subcon with all visible scans
    Subcon subcon;
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "creating new subcon " << subcon.printId();
#endif

    for ( const auto &thisSource : sources_ ) {
        subcon.visibleScan( currentTime, type, network_, thisSource, observedSources );
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
    Source &thisSource = sources_[srcid];
    thisSource.update( nbl, latestTime, influence );

    // update minimum slew time in case of custom data write speed to disk
    for ( int i = 0; i < scan.getNSta(); ++i ) {
        unsigned long staid = scan.getPointingVector( i ).getStaid();

        Station &sta = network_.refStation( staid );
        if ( sta.getPARA().dataWriteSpeed.is_initialized() ) {
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

    for ( auto &thisStation : network_.refStations() ) {
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "checking station " << thisStation.getName();
#endif

        of << "    checking station " << thisStation.getName() << ":\n";
        unsigned long staid = thisStation.getId();
        unsigned int constTimes = thisStation.getWaittimes().fieldSystem + thisStation.getWaittimes().preob;

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
                    unsigned int slewtime = thisStation.getAntenna().slewTime( thisEnd, nextStart );
                    if ( slewtime < thisStation.getPARA().minSlewtimeDataWriteSpeed ) {
                        slewtime = thisStation.getPARA().minSlewtimeDataWriteSpeed;
                    }
                    if ( slewtime < thisStation.getPARA().minSlewtime ) {
                        slewtime = thisStation.getPARA().minSlewtime;
                    }

                    unsigned int min_neededTime = slewtime + constTimes;
                    unsigned int availableTime = nextStartTime - thisEndTime;
                    unsigned int idleTime;

                    // update staStatistics
                    staStatistics.totalSlewTime += slewtime;
                    if ( availableTime >= min_neededTime ) {
                        idleTime = availableTime - min_neededTime;
                    } else {
                        idleTime = 0;
                    }

                    staStatistics.totalIdleTime += idleTime;

                    if ( availableTime + 1 < min_neededTime ) {
                        ++countErrors;
                        of << "    ERROR #" << countErrors
                           << ": not enough available time for slewing! scans: " << scan_thisEnd.printId() << " and "
                           << scan_nextStart.printId() << "\n";
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
#ifdef VIESCHEDPP_LOG
                        BOOST_LOG_TRIVIAL( error )
                            << boost::format( "%s iteration %d:" ) % getName() % ( parameters_.currentIteration )
                            << "not enough available time for slewing! station: " << thisStation.getName() << " ("
                            << ( (long)availableTime - (long)min_neededTime ) << " [sec])";
#endif

                    } else {
                        if ( idleTime > 1200 ) {
                            unsigned int midpoint = ( thisEndTime + nextStartTime ) / 2;
                            bool dummy;
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
    std::vector<Source::Statistics> srcStatistics( sources_.size() );
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
    for ( unsigned long isrc = 0; isrc < sources_.size(); ++isrc ) {
        Source &source = sources_[isrc];
        source.setStatistics( srcStatistics[isrc] );
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
            of << "TAGALONG for station " << any.getName() << " required!\n";
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
    }

    // check if a source has to be changed
    vector<string> sourcesChanged;
    for ( auto &any : sources_ ) {
        bool changed = any.checkForNewEvent( time, hard_break );
        if ( changed ) {
            sourcesChanged.push_back( any.getName() );
#ifdef VIESCHEDPP_LOG
            if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "changed parameters for source " << any.getName();
#endif
        }
    }
    if ( !sourcesChanged.empty() && output && time < TimeSystem::duration ) {
        util::outputObjectList( "source parameter changed", sourcesChanged, of );
        listSourceOverview( of );
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

    for ( const auto &any : sources_ ) {
        if ( any.getPARA().available && any.getPARA().globalAvailable ) {
            available.push_back( any.getName() );

        } else if ( !any.getPARA().globalAvailable ) {
            notAvailable_optimization.push_back( any.getName() );

        } else if ( any.getMaxFlux() < any.getPARA().minFlux ) {
            string message =
                ( boost::format( "%8s (%4.2f/%4.2f)" ) % any.getName() % any.getMaxFlux() % any.getPARA().minFlux )
                    .str();
            notAvailable_tooWeak.push_back( message );

        } else if ( any.getSunDistance() < any.getPARA().minSunDistance ) {
            string message = ( boost::format( "%8s (%4.2f/%4.2f)" ) % any.getName() %
                               ( any.getSunDistance() * rad2deg ) % ( any.getPARA().minSunDistance * rad2deg ) )
                                 .str();
            notAvailable_tooCloseToSun.push_back( message );

        } else {
            notAvailable.push_back( any.getName() );
        }
    }

    of << "Total number of sources: " << sources_.size() << "\n";
    util::outputObjectList( "available source", available, of );
    util::outputObjectList( "not available", notAvailable, of );
    util::outputObjectList( "not available because of optimization", notAvailable_optimization, of );
    util::outputObjectList( "not available because too weak", notAvailable_tooWeak, of );
    util::outputObjectList( "not available because of sun distance", notAvailable_tooCloseToSun, of );
}


void Scheduler::startTagelongMode( Station &station, SkyCoverage &skyCoverage, std::ofstream &of ) {
    unsigned long staid = station.getId();
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "start tagalong mode for station " << station.getName();
#endif

    of << "Start tagalong mode for station " << station.getName() << ": \n";

    // get wait times
    unsigned int stationConstTimes = station.getWaittimes().fieldSystem + station.getWaittimes().preob;

    // loop through all scans
    unsigned long counter = 0;
    for ( auto &scan : scans_ ) {
        bool hardBreak = false;
        station.checkForNewEvent( scan.getTimes().getScanTime( Timestamp::start ), hardBreak );
        if ( !station.getPARA().available ) {
            continue;
        }

        ++counter;
        unsigned int scanStartTime = scan.getTimes().getObservingTime( Timestamp::start );
        unsigned int currentStationTime = station.getCurrentTime();

        if ( scan.getType() == Scan::ScanType::fillin ) {
            continue;
        }

        // look if this scan is possible for tagalong mode
        if ( scanStartTime > currentStationTime ) {
            unsigned long srcid = scan.getSourceId();
            Source &source = sources_[scan.getSourceId()];

            PointingVector pv_new_start( staid, srcid );

            pv_new_start.setTime( scanStartTime );

            station.calcAzEl_rigorous( source, pv_new_start );

            // check if source is up from station
            bool flag = station.isVisible( pv_new_start, source.getPARA().minElevation );
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
                if ( !source.getPARA().ignoreBaselines.empty() ) {
                    const auto &PARA = source.getPARA();
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

                unsigned int maxScanDuration = 0;
                if ( source.getPARA().fixedScanDuration.is_initialized() ) {
                    maxScanDuration = *source.getPARA().fixedScanDuration;
                } else {
                    for ( auto &band : currentObservingMode_->getAllBands() ) {
                        double SEFD_src;
                        if ( source.hasFluxInformation( band ) ) {
                            // calculate observed flux density for each band
                            SEFD_src =
                                source.observedFlux( band, gmst, network_.getDxyz( sta1.getId(), sta2.getId() ) );
                        } else if ( ObservingMode::sourceBackup[band] == ObservingMode::Backup::internalModel ) {
                            // calculate observed flux density based on model
                            double wavelength = ObservingMode::getWavelength( band );
                            SEFD_src = source.observedFlux_model( wavelength, gmst,
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

                        double minSNR_src = source.getPARA().minSNR.at( band );

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

                        double maxCorSynch1 = sta1.getWaittimes().midob;
                        double maxCorSynch = maxCorSynch1;
                        double maxCorSynch2 = sta2.getWaittimes().midob;
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

                        unsigned int minScanBl = std::max( {bl.getParameters().minScan, sta1.getPARA().minScan,
                                                            sta2.getPARA().minScan, source.getPARA().minScan} );

                        if ( new_duration_uint < minScanBl ) {
                            new_duration_uint = minScanBl;
                        }
                        unsigned int maxScanBl = std::min( {bl.getParameters().maxScan, sta1.getPARA().maxScan,
                                                            sta2.getPARA().maxScan, source.getPARA().maxScan} );

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

                        unsigned int maxScanTime = scan.getPointingVector( i, Timestamp::end ).getTime() -
                                                   scan.getPointingVector( i ).getTime();

                        if ( maxScanDuration > maxScanTime ) {
                            maxScanDuration = maxScanTime;
                        }
                    }
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

            unsigned int maxBl = 0;
            for ( const auto &any : newObs ) {
                if ( any.getObservingTime() > maxBl ) {
                    maxBl = any.getObservingTime();
                }
            }

            // check if source is still visible at end of scan... with same cable wrap
            PointingVector pv_new_end( staid, srcid );

            pv_new_end.setTime( scanStartTime + maxBl );

            station.calcAzEl_rigorous( source, pv_new_end );

            // check if source is up from station
            flag = station.isVisible( pv_new_end, source.getPARA().minElevation );
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

            scan.addTagalongStation( pv_new_start, pv_new_end, newObs, *slewtime, station );
            auto txt =
                boost::format( "    possible to observe source: %-8s (scan: %4d) scan start: %s scan end: %s \n" ) %
                source.getName() % counter % TimeSystem::time2timeOfDay( pv_new_start.getTime() ) %
                TimeSystem::time2timeOfDay( pv_new_end.getTime() );

#ifdef VIESCHEDPP_LOG
            if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << txt;
#endif

            of << txt;
            if ( station.referencePARA().firstScan ) {
                station.referencePARA().firstScan = false;
            }
            station.update( newObs.size(), pv_new_end, true );
            skyCoverage.update( pv_new_end );
        }
    }

    //    station.applyNextEvent(of);
}


bool Scheduler::checkOptimizationConditions( ofstream &of ) {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "checking optimization condition";
#endif
    bool newScheduleNecessary = false;
    vector<string> excludedSources;
    int excludedScans = 0;
    int excludedBaselines = 0;
    string message = "checking optimization conditions... ";
    int consideredSources = 0;
    bool lastExcluded = false;

    vector<pair<unsigned long, unsigned long>> possibleExcludeIds;

    for ( auto &thisSource : sources_ ) {
        if ( !thisSource.getPARA().globalAvailable ) {
            continue;
        }
        ++consideredSources;
        bool scansValid = true;
        if ( thisSource.getNTotalScans() < thisSource.getOptimization().minNumScans ) {
            scansValid = false;
        }

        bool baselinesValid = true;
        if ( thisSource.getNObs() < thisSource.getOptimization().minNumObs ) {
            baselinesValid = false;
        }

        bool exclude;
        if ( parameters_.andAsConditionCombination ) {
            exclude = !( scansValid && baselinesValid );
        } else {
            exclude = !( scansValid || baselinesValid );
        }

        if ( exclude ) {
            possibleExcludeIds.emplace_back( thisSource.getId(), thisSource.getNTotalScans() );
        }
    }

    // exclude only half the sources
    if ( parameters_.currentIteration < parameters_.numberOfGentleSourceReductions ) {
        std::sort( possibleExcludeIds.begin(), possibleExcludeIds.end(),
                   []( auto &left, auto &right ) { return left.second < right.second; } );

        int counter = 0;

        // exclude all sources with zero scans
        for ( const auto &any : possibleExcludeIds ) {
            auto &thisSource = sources_[any.first];

            if ( any.second == 0 ) {
                excludedScans += thisSource.getNTotalScans();
                excludedBaselines += thisSource.getNObs();
                excludedSources.push_back( thisSource.getName() );
                newScheduleNecessary = true;
                thisSource.referencePARA().setGlobalAvailable( false );
                continue;
            }
        }

        auto diff = possibleExcludeIds.size() - excludedSources.size();
        // exclude half of the remaining sources
        for ( const auto &any : possibleExcludeIds ) {
            auto &thisSource = sources_[any.first];

            if ( any.second == 0 ) {
                continue;
            }

            if ( counter < diff * parameters_.reduceFactor ) {
                excludedScans += thisSource.getNTotalScans();
                excludedBaselines += thisSource.getNObs();
                excludedSources.push_back( thisSource.getName() );
                newScheduleNecessary = true;
                thisSource.referencePARA().setGlobalAvailable( false );
                ++counter;
            }
        }

        // exclude all sources
    } else {
        for ( const auto &any : possibleExcludeIds ) {
            auto &thisSource = sources_[any.first];

            excludedScans += thisSource.getNTotalScans();
            excludedBaselines += thisSource.getNObs();
            excludedSources.push_back( thisSource.getName() );
            newScheduleNecessary = true;
            thisSource.referencePARA().setGlobalAvailable( false );
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
        CalibratorBlock::nextBlock = 0;
        unsigned long sourcesLeft = consideredSources - excludedSources.size();
        of << "|                                                                                                       "
              "                                       |\n";
        if ( sourcesLeft < 10 ) {
            of << boost::format( "| %=140s |\n" ) % message;
            string message2 = ( boost::format( "Abortion: only %d sources left" ) % sourcesLeft ).str();
            of << boost::format( "| %=140s |\n" ) % message2;
            of << "|                                                                                                   "
                  "                                           |\n";
            of << "|---------------------------------------------------------------------------------------------------"
                  "-------------------------------------------|\n";
            return false;
        }
        string message2 = ( boost::format( "creating new schedule with %d sources" ) % sourcesLeft ).str();
        of << boost::format( "| %=140s |\n" ) % message;
        of << boost::format( "| %=140s |\n" ) % message2;
        of << "|                                                                                                       "
              "                                       |\n";
        of << "'-------------------------------------------------------------------------------------------------------"
              "---------------------------------------'\n\n";

        util::outputObjectList( "List of removed sources", excludedSources, of );

        scans_.clear();
        for ( auto &any : network_.refStations() ) {
            any.clearObservations();
        }
        for ( auto &any : sources_ ) {
            any.clearObservations();
        }
        for ( auto &any : network_.refSkyCoverages() ) {
            any.clearObservations();
        }
        for ( auto &any : network_.refBaselines() ) {
            any.setNextEvent( 0 );
        }

    } else {
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "no new iteration needed";
#endif
        message.append( "no new iteration needed!" );
        of << "|                                                                                                       "
              "                                       |\n";
        of << boost::format( "| %=140s |\n" ) % message;
        of << "|                                                                                                       "
              "                                       |\n";
        of << "|-------------------------------------------------------------------------------------------------------"
              "---------------------------------------|\n";
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
                sta.referencePARA().available = endposition->getStationAvailable( sta.getId() );
            }
            break;
        }
    }
}


void Scheduler::startScanSelectionBetweenScans( unsigned int duration, std::ofstream &of, Scan::ScanType type,
                                                bool output, bool ignoreTagalong ) {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "start scan selection between scans";
#endif

    // save number of predefined scans (new scans will be added at end of those)
    auto nMainScans = static_cast<int>( scans_.size() );
    if ( nMainScans == 0 ) {
        return;
    }

    // reset all events
    resetAllEvents( of );

    // loop through all predefined scans
    for ( int i = 0; i < nMainScans - 1; ++i ) {
        if ( output ) {
            of << "|---------------------------------------------------------------------------------------------------"
                  "-------------------------------------------|\n";
            of << "|                                                           start new scan selection                "
                  "                                           |\n";
            of << "|---------------------------------------------------------------------------------------------------"
                  "-------------------------------------------|\n";
        }
        // look through all stations of last scan and set current pointing vector to last scan
        Scan &lastScan = scans_[i];
        for ( int k = 0; k < lastScan.getNSta(); ++k ) {
            const auto &pv = lastScan.getPointingVector( k, Timestamp::end );
            unsigned long staid = pv.getStaid();
            unsigned int time = pv.getTime();
            Station &thisSta = network_.refStation( staid );
            if ( time >= thisSta.getCurrentTime() ) {
                thisSta.setCurrentPointingVector( pv );
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

        // check if there was an new upcoming event in the meantime
        unsigned int startTime = lastScan.getTimes().getScanTime( Timestamp::end );
        checkForNewEvents( startTime, true, of, false );
        if ( ignoreTagalong ) {
            ignoreTagalongParameter();
        }

        // recursively start scan selection
        boost::optional<Subcon> subcon = boost::none;
        startScanSelection( scans_[i + 1].getTimes().getScanTime( Timestamp::end ), of, type, endposition, subcon, 0 );
    }

    // do the same between time at from last scan until duration with no endposition
    if ( output ) {
        of << "|-------------------------------------------------------------------------------------------------------"
              "---------------------------------------|\n";
        of << "|                                                          start final scan selection                   "
              "                                       |\n";
        of << "|-------------------------------------------------------------------------------------------------------"
              "---------------------------------------|\n";
    }

    // get last predefined scan and set current position of station
    Scan &lastScan = scans_[nMainScans - 1];
    for ( int k = 0; k < lastScan.getNSta(); ++k ) {
        const auto &pv = lastScan.getPointingVector( k, Timestamp::end );
        unsigned long staid = pv.getStaid();
        unsigned int time = pv.getTime();
        Station &thisSta = network_.refStation( staid );
        if ( time >= thisSta.getCurrentTime() ) {
            thisSta.setCurrentPointingVector( pv );
        }
    }
    // check if there was an new upcoming event in the meantime
    unsigned int startTime = lastScan.getTimes().getScanTime( Timestamp::end );
    checkForNewEvents( startTime, true, of, false );
    if ( ignoreTagalong ) {
        ignoreTagalongParameter();
    }

    // recursively start scan selection
    boost::optional<Subcon> subcon = boost::none;
    boost::optional<StationEndposition> endposition = boost::none;
    startScanSelection( duration, of, type, endposition, subcon, 0 );

    // sort scans at the end
    sortSchedule( Timestamp::start );
}


void Scheduler::highImpactScans( HighImpactScanDescriptor &himp, ofstream &of ) {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "fix high impact scans";
#endif

    of << "|-----------------------------------------------------------------------------------------------------------"
          "-----------------------------------|\n";
    of << "|                                                                                                           "
          "                                   |\n";
    of << "|                                                           fixing high impact scans                        "
          "                                   |\n";
    of << "|                                                                                                           "
          "                                   |\n";
    of << "|-----------------------------------------------------------------------------------------------------------"
          "-----------------------------------|\n";

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
            thisStation.setCurrentPointingVector( pv );
            thisStation.referencePARA().firstScan = true;
        }

        // create all possible high impact scan pointing vectors for this time
        himp.possibleHighImpactScans( iTime, network_, sources_ );
    }

    // create the actual scans
    himp.updateHighImpactScans( network_, sources_, currentObservingMode_, parameters_.subnetting );

    himp.updateLogfile( of );

    // select bestScans
    vector<Scan> bestScans;
    do {
        bestScans = himp.highestImpactScans( network_, sources_, currentObservingMode_ );
        for ( auto &scan : bestScans ) {
            const Source &source = sources_[scan.getSourceId()];
            if ( himp.isCorrectHighImpactScan( scan, scans_, source ) ) {
                update( scan, of );

                for ( auto &thisStation : network_.refStations() ) {
                    thisStation.referencePARA().firstScan = true;
                }
            }
        }
    } while ( himp.hasMoreScans() );

    sortSchedule( Timestamp::start );

    of << "|-----------------------------------------------------------------------------------------------------------"
          "-----------------------------------|\n";
    of << "|                                                                                                           "
          "                                   |\n";
    of << "|                                                       start with normal scan selection                    "
          "                                   |\n";
    of << "|                                                                                                           "
          "                                   |\n";
    of << "|-----------------------------------------------------------------------------------------------------------"
          "-----------------------------------|\n";

    // reset all events
    resetAllEvents( of );

    for ( auto &thisStation : network_.refStations() ) {
        PointingVector pv( thisStation.getId(), numeric_limits<unsigned long>::max() );
        pv.setTime( 0 );
        thisStation.setCurrentPointingVector( pv );
        thisStation.referencePARA().firstScan = true;
    }
}


void Scheduler::resetAllEvents( std::ofstream &of ) {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "reset all events";
#endif

    // reset all events
    for ( auto &any : network_.refStations() ) {
        PointingVector pv( any.getId(), numeric_limits<unsigned long>::max() );
        pv.setTime( 0 );
        any.setCurrentPointingVector( pv );
        any.setNextEvent( 0 );
    }
    for ( auto &any : sources_ ) {
        any.setNextEvent( 0 );
    }
    for ( auto &any : network_.refBaselines() ) {
        any.setNextEvent( 0 );
    }
    checkForNewEvents( 0, false, of, false );
}


void Scheduler::idleToScanTime( Timestamp ts, std::ofstream &of ) {
    switch ( ts ) {
        case Timestamp::start:
#ifdef VIESCHEDPP_LOG
            if ( Flags::logDebug )
                BOOST_LOG_TRIVIAL( debug ) << "start changing idle to observing time at start of scan";
#endif
            of << "|                                                                                                   "
                  "                                           |\n"
                  "|                                                  increasing observing time at start of scan       "
                  "                                           |\n"
                  "|                                                                                                   "
                  "                                           |\n"
                  "|---------------------------------------------------------------------------------------------------"
                  "-------------------------------------------|\n";
            break;
        case Timestamp::end:
#ifdef VIESCHEDPP_LOG
            if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "start changing idle to observing time at end of scan";
#endif
            of << "|                                                                                                   "
                  "                                           |\n"
                  "|                                                   increasing observing time at end of scan        "
                  "                                           |\n"
                  "|                                                                                                   "
                  "                                           |\n"
                  "|---------------------------------------------------------------------------------------------------"
                  "-------------------------------------------|\n";
            break;
    }

    // hard copy previous observing times
    map<unsigned long, ScanTimes> oldScanTimes;

    for ( const auto &scan : scans_ ) {
        oldScanTimes.insert( {scan.getId(), scan.getTimes()} );
    }

    for ( auto &thisSta : network_.refStations() ) {
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
                const Source &thisSource = sources_[scan1.getSourceId()];
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
                    if ( !thisSta.isVisible( variable, thisSource.getPARA().minElevation ) ) {
                        valid = false;
                    }
                    if ( valid ) {
                        scan1.setPointingVector( staidx1, move( variable ), Timestamp::start );
                    }
                }
            }

            // check for new events
            checkForNewEvents( scan1.getTimes().getScanTime( Timestamp::start ), false, of, false );

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

            // get times
            unsigned int availableTime = pv2.getTime() - pv1.getTime();
            unsigned int prevSlewTime = thisSta.getAntenna().slewTime( pv1, pv2 );
            if ( prevSlewTime < thisSta.getPARA().minSlewtime ) {
                prevSlewTime = thisSta.getPARA().minSlewtime;
            }

            unsigned int fsTime = thisSta.getWaittimes().fieldSystem;
            unsigned int preobTime = thisSta.getWaittimes().preob;

            // avoid rounding errors
            if ( availableTime < prevSlewTime + fsTime + preobTime ) {
                continue;
            }

            // calc idle time
            unsigned int idleTime = availableTime - prevSlewTime - fsTime - preobTime;
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

            const auto &thisSource = sources_[ref.getSrcid()];

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
            if ( !thisSta.isVisible( variable, thisSource.getPARA().minElevation ) ) {
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
                    break;
                }
            }

            // iteratively adjust new idle time and new slew time until it is equal to previous slew time
            // we also allow 1 second offset (1 sec additional idle time) as a live saver
            int offset = 0;
            bool valid = true;
            while ( slewTime + offset != prevSlewTime && slewTime + offset != prevSlewTime - 1 ) {
                offset = prevSlewTime - slewTime;

                switch ( ts ) {
                    case Timestamp::start: {
                        variable.setTime( variableStartTime - offset );
                        break;
                    }
                    case Timestamp::end: {
                        variable.setTime( variableStartTime + offset );
                        break;
                    }
                }

                thisSta.calcAzEl_rigorous( thisSource, variable );
                thisSta.getCableWrap().calcUnwrappedAz( ref, variable );

                // check if cable wrap changes
                if ( abs( ref.getAz() - variable.getAz() ) > pi / 2 ) {
                    valid = false;
                    break;
                }

                // check if azimuth and elevation is visible
                if ( !thisSta.isVisible( variable, thisSource.getPARA().minElevation ) ) {
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
                }
            }

            // continue if source is still visible
            if ( !valid ) {
                continue;
            }

            // if scan time would be reduced skip station!
            if ( offset + static_cast<int>( idleTime ) < 0 ) {
                continue;
            }

            // adjust observing times
            switch ( ts ) {
                case Timestamp::start: {
                    scan2.setPointingVector( staidx2, move( variable ), Timestamp::start );
                    break;
                }
                case Timestamp::end: {
                    scan1.setPointingVector( staidx1, move( variable ), Timestamp::end );
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

                    const Source &thisSource = sources_[lastScan.getSourceId()];
                    const PointingVector &pv1 = lastScan.getPointingVector( staidx1, Timestamp::end );

                    if ( pv1.getTime() != 0 ) {
                        PointingVector variable( pv1 );
                        variable.setId( pv1.getId() );
                        variable.setTime( TimeSystem::duration );
                        thisSta.calcAzEl_rigorous( thisSource, variable );
                        thisSta.getCableWrap().calcUnwrappedAz( pv1, variable );
                        bool valid = true;
                        // check if cable wrap changes
                        if ( abs( pv1.getAz() - variable.getAz() ) > pi / 2 ) {
                            valid = false;
                        }
                        // check if azimuth and elevation is visible
                        if ( !thisSta.isVisible( variable, thisSource.getPARA().minElevation ) ) {
                            valid = false;
                        }
                        if ( valid ) {
                            lastScan.setPointingVector( staidx1, move( variable ), Timestamp::end );
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
        Source &thisSource = sources_[thisScan.getSourceId()];
        bool dummy;
        switch ( ts ) {
            case Timestamp::start: {
                thisSource.checkForNewEvent( thisScan.getTimes().getScanTime( Timestamp::end ), dummy );
                break;
            }
            case Timestamp::end: {
                thisSource.checkForNewEvent( thisScan.getTimes().getScanTime( Timestamp::start ), dummy );
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
            unsigned int maxExtendedObservingTime = min( {thisSource.getPARA().maxScan, thisSta.getPARA().maxScan} );

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
                if ( !thisSta.isVisible( variable, thisSource.getPARA().minElevation ) ) {
                    valid = false;
                }
                thisScan.setPointingVector( staidx, move( variable ), ts );
                if ( !valid ) {
                    auto txt = boost::format(
                                   "extending observing time to idle time: source %s might not be visible from %s "
                                   "during %s. " ) %
                               thisSource.getName() % thisSta.getName() % TimeSystem::time2timeOfDay( maximum );
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
        const Source &source = sources_[scan.getSourceId()];

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
            string right = ( boost::format( "source:  %s %s" ) % source.getName() % source.printId() ).str();
            of << boost::format( "| scan: %-15s                                  %85s |\n" ) % scan.printId() % right;

            of << "|---------------------------------------------------------------------------------------------------"
                  "-------------------------------------------|\n";
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
            of << "|---------------------------------------------------------------------------------------------------"
                  "-------------------------------------------|\n";
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
    of << "|-----------------------------------------------------------------------------------------------------------"
          "-----------------------------------|\n";

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
    of << "'-----------------------------------------------------------------------------------------------------------"
          "-----------------------------------'\n";
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
    of << "|                                                                                                           "
          "                                   |\n";
    of << "|                                                           start calibration block                         "
          "                                   |\n";
    of << "|                                                                                                           "
          "                                   |\n";
    of << "|-----------------------------------------------------------------------------------------------------------"
          "-----------------------------------|\n";
}


void Scheduler::writeCalibratorStatistics( std::ofstream &of, std::vector<double> &highestElevations,
                                           std::vector<double> &lowestElevations ) {
    of << "|                                                                                                           "
          "                                   |\n";
    of << "|                                                          calibration block summary                        "
          "                                   |\n";
    of << "|                                                                                                           "
          "                                   |\n";
    of << "|-----------------------------------------------------------------------------------------------------------"
          "-----------------------------------|\n";
    of << "|     station  | highest elevation | lowest elevation  |                                                    "
          "                                   |\n";
    of << "|              |       [deg]       |       [deg]       |                                                    "
          "                                   |\n";
    of << "|--------------|-------------------|-------------------|                                                    "
          "                                   |\n";
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
        of << boost::format(
                  "|     %-8s |       %5s       |       %5s       |                                                    "
                  "                                   |\n" ) %
                  sta.getName() % highstr % lowstr;
    }
    of << "|-----------------------------------------------------------------------------------------------------------"
          "-----------------------------------|\n";
    of << "|                                                                                                           "
          "                                   |\n";
    of << "|                                                          finished calibration block                       "
          "                                   |\n";
    of << "|                                                                                                           "
          "                                   |\n";
    of << "|-----------------------------------------------------------------------------------------------------------"
          "-----------------------------------|\n";
}


bool Scheduler::calibratorUpdate( const std::vector<Scan> &bestScans, std::vector<double> &prevHighElevationScores,
                                  std::vector<double> &prevLowElevationScores, std::vector<double> &highestElevations,
                                  std::vector<double> &lowestElevations ) {
    double lowElevationSlopeStart = CalibratorBlock::lowElevationStartWeight;
    double lowElevationSlopeEnd = CalibratorBlock::lowElevationFullWeight;

    double highElevationSlopeStart = CalibratorBlock::highElevationStartWeight;
    double highElevationSlopeEnd = CalibratorBlock::highElevationFullWeight;

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
