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
 * File:   Scan.cpp
 * Author: mschartn
 *
 * Created on June 29, 2017, 3:27 PM
 */

#include "Scan.h"


using namespace std;
using namespace VieVS;

unsigned int Scan::nScanSelections{ 0 };

bool Scan::scanSequence_flag = false;                     ///< true if you have a custom scan sequence
thread_local unsigned int Scan::scanSequence_modulo = 0;  ///< modulo of scan selection scanSequence_cadence
std::map<unsigned int, std::vector<unsigned long>>
    Scan::scanSequence_target;  ///< map with modulo number as key and list of target source ids as value

unsigned long Scan::nextId = 0;


Scan::Scan( vector<PointingVector> &pointingVectors, vector<unsigned int> &endOfLastScan, ScanType type )
    : VieVS_Object( nextId++ ),
      times_{ ScanTimes( static_cast<unsigned int>( pointingVectors.size() ) ) },
      pointingVectorsStart_{ move( pointingVectors ) },
      type_{ type },
      constellation_{ ScanConstellation::single },
      score_{ 0 } {
    nsta_ = Scan::pointingVectorsStart_.size();
    srcid_ = Scan::pointingVectorsStart_.at( 0 ).getSrcid();
    times_.setEndOfLastScan( endOfLastScan );
    observations_.reserve( ( nsta_ * ( nsta_ - 1 ) ) / 2 );
}


Scan::Scan( vector<PointingVector> pv, ScanTimes times, vector<Observation> obs, ScanType type )
    : VieVS_Object( nextId++ ),
      srcid_{ pv[0].getSrcid() },
      nsta_{ static_cast<unsigned long>(pv.size()) },
      pointingVectorsStart_{ move( pv ) },
      score_{ 0 },
      times_{ move( times ) },
      observations_{ move( obs ) },
      constellation_{ ScanConstellation::subnetting },
      type_{ type } {
    times_.giveNewId();
}

Scan::Scan( const boost::property_tree::ptree &ptree, Network &network, const SourceList &sourceList,
            Scan::ScanType type )
    : VieVS_Object( nextId++ ),
      times_{ ScanTimes( static_cast<unsigned int>( 0 ) ) },
      score_{ 0 },
      srcid_{ 0 },
      nsta_{ 0 },
      constellation_{ ScanConstellation::single },
      type_{ type } {
    vector<unsigned int> system_delays;
    vector<unsigned int> preobs;
    vector<unsigned int> observation_start;
    vector<unsigned int> observation_end;
    vector<unsigned int> eols;

    string source_name = ptree.get<string>( "source" );
    shared_ptr<const AbstractSource> source;
    for ( const auto &this_source : sourceList.getSources() ) {
        if ( this_source->hasName( source_name ) ) {
            srcid_ = this_source->getId();
            source = this_source;
            break;
        }
    }

    for ( const auto &any : ptree ) {
        if ( any.first == "station" ) {
            ++nsta_;

            string name = any.second.get<string>( "<xmlattr>.name" );
            Station &station = network.refStation( name );
            unsigned long staid = station.getId();

            unsigned int obs_start =
                TimeSystem::posixTime2InternalTime( TimeSystem::string2ptime( any.second.get<string>( "obs_start" ) ) );
            observation_start.push_back( obs_start );
            unsigned int obs_end =
                TimeSystem::posixTime2InternalTime( TimeSystem::string2ptime( any.second.get<string>( "obs_end" ) ) );
            observation_end.push_back( obs_end );

            auto preob = any.second.get<unsigned int>( "preob" );
            preobs.push_back( preob );
            auto system_delay = any.second.get<unsigned int>( "system_delay" );
            system_delays.push_back( system_delay );
            eols.push_back( obs_start - preob - system_delay );


            PointingVector pv_start( staid, srcid_ );
            pv_start.setTime( obs_start );
            station.calcAzEl_rigorous( source, pv_start );
            pointingVectorsStart_.push_back( pv_start );

            PointingVector pv_end( staid, srcid_ );
            pv_end.setTime( obs_end );
            station.calcAzEl_rigorous( source, pv_end );
            pointingVectorsEnd_.push_back( pv_end );
        }
    }


    times_ = ScanTimes( static_cast<unsigned int>( nsta_ ) );
    setScanTimes( eols, system_delays, vector<unsigned int>( nsta_, 0 ), preobs, observation_start, observation_end );

    observations_.reserve( ( nsta_ * ( nsta_ - 1 ) ) / 2 );
    constructObservations( network, source );

    for ( auto &obs : observations_ ) {
        auto idx1 = findIdxOfStationId( obs.getStaid1() );
        auto idx2 = findIdxOfStationId( obs.getStaid2() );
        obs.setObservingTime( times_.getObservingDuration( *idx1, *idx2 ) );
    }
}

bool Scan::constructObservations( const Network &network,
                                  const std::shared_ptr<const AbstractSource> &source ) noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " create observations";
#endif
    observations_.clear();
    bool valid = false;
    unsigned long srcid = source->getId();

    // loop over all pointingVectors
    for ( int i = 0; i < pointingVectorsStart_.size(); ++i ) {
        for ( int j = i + 1; j < pointingVectorsStart_.size(); ++j ) {
            unsigned long staid1 = pointingVectorsStart_[i].getStaid();
            unsigned long staid2 = pointingVectorsStart_[j].getStaid();
            const Baseline &bl = network.getBaseline( staid1, staid2 );
            unsigned long blid = bl.getId();

            // check if Baseline is ignored
            if ( bl.getParameters().ignore ) {
#ifdef VIESCHEDPP_LOG
                if ( Flags::logTrace )
                    BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " ignore baseline " << bl.getName();
#endif
                continue;
            }
            // check if this source has to ignore this baseline
            if ( !source->getPARA().ignoreBaselines.empty() ) {
                const auto &PARA = source->getPARA();
                unsigned long blid = bl.getId();
                if ( find( PARA.ignoreBaselines.begin(), PARA.ignoreBaselines.end(), blid ) !=
                     PARA.ignoreBaselines.end() ) {
#ifdef VIESCHEDPP_LOG
                    if ( Flags::logTrace )
                        BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " ignore baseline " << bl.getName();
#endif
                    continue;
                }
            }

            // add new baseline
            unsigned int startTime = max(
                { times_.getObservingTime( i, Timestamp::start ), times_.getObservingTime( j, Timestamp::start ) } );

            if ( source->checkJetAngle() ) {
                double date1 = 2400000.5;
                double date2 = TimeSystem::mjdStart + static_cast<double>( startTime ) / 86400.0;
                double gmst = iauGmst82( date1, date2 );
                if ( !source->jet_angle_valid(startTime, gmst, network.getDxyz( staid1, staid2 ) ) ){
                    continue;
                }
            }

#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace )
                BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " ignore baseline " << bl.getName();
#endif
            observations_.emplace_back( blid, staid1, staid2, srcid, startTime );
            valid = true;
        }
    }
    return valid;
}


void Scan::addTimes( int idx, unsigned int fieldSystem, unsigned int slew, unsigned int preob ) noexcept {
    times_.addTimes( idx, fieldSystem, slew, preob );
}


bool Scan::removeStation( int idx, const shared_ptr<const AbstractSource> &source ) noexcept {
    unsigned long staid = pointingVectorsStart_[idx].getStaid();
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "scan " << this->printId() << " remove station " << staid;
#endif
    bool flag = true;

    --nsta_;
    vector<unsigned long> staids;
    for ( const auto &pv : pointingVectorsStart_ ) {
        staids.push_back( pv.getStaid() );
    }

    // check if you want to remove a required station
    if ( !source->getPARA().requiredStations.empty() ) {
        const vector<unsigned long> &rsta = source->getPARA().requiredStations;
        if ( find( rsta.begin(), rsta.end(), staid ) != rsta.end() ) {
#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace )
                BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " this was a required station";
#endif
            flag = false;
        }
    }

    // remove element from time
    times_.removeElement( idx );

    // erase the pointing vector
    pointingVectorsStart_.erase( next( pointingVectorsStart_.begin(), idx ) );
    if ( !pointingVectorsEnd_.empty() ) {
        pointingVectorsEnd_.erase( next( pointingVectorsEnd_.begin(), idx ) );
    }

    // check if you still have enough stations
    staids = {};
    for ( const auto &pv : pointingVectorsStart_ ) {
        staids.push_back( pv.getStaid() );
    }
    if ( Network::stationIdsToNSites( staids ) < source->getPARA().minNumberOfSites ) {
#ifdef VIESCHEDPP_LOG
        if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " not enough stations left";
#endif
        flag = false;
    }


    // remove all observations with this station
    unsigned long nbl_before = observations_.size();
    int i = 0;
    while ( i < observations_.size() ) {
        if ( observations_[i].containsStation( staid ) ) {
#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace )
                BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId()
                                           << " remove observation between stations: " << observations_[i].getStaid1()
                                           << " and " << observations_[i].getStaid2();
#endif
            observations_.erase( next( observations_.begin(), i ) );
        } else {
            ++i;
        }
    }
    if ( nbl_before != 0 && observations_.empty() ) {
        flag = false;
    }
    // check if there are any observations left
    return flag;
}


bool Scan::removeObservation( int iobs, const std::shared_ptr<const AbstractSource> &source ) noexcept {
    // remove observation
    unsigned long staid1 = observations_[iobs].getStaid1();
    unsigned long staid2 = observations_[iobs].getStaid2();
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace )
        BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " remove observation between stations: " << staid1
                                   << " and " << staid2;
#endif
    observations_.erase( next( observations_.begin(), iobs ) );
    if ( observations_.empty() ) {
#ifdef VIESCHEDPP_LOG
        if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " no observation left";
#endif
        return false;
    }

    // check if it necessary to remove a pointing vector (a station is no longer part of any baseline)
    bool scanValid = true;
    int counterStaid1 = 0;
    int counterStaid2 = 0;
    for ( const auto &any : observations_ ) {
        if ( any.containsStation( staid1 ) ) {
            ++counterStaid1;
        }
        if ( any.containsStation( staid2 ) ) {
            ++counterStaid2;
        }
    }

    // remove station if necessary
    if ( counterStaid1 == 0 ) {
        boost::optional<unsigned long> idx_pv = findIdxOfStationId( staid1 );
#ifdef VIESCHEDPP_LOG
        if ( Flags::logTrace )
            BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " no observation with station " << staid1
                                       << "left";
#endif
        scanValid = removeStation( static_cast<int>( *idx_pv ), source );
    }
    if ( scanValid && counterStaid2 == 0 ) {
        boost::optional<unsigned long> idx_pv = findIdxOfStationId( staid2 );
#ifdef VIESCHEDPP_LOG
        if ( Flags::logTrace )
            BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " no observation with station " << staid2
                                       << "left";
#endif
        scanValid = removeStation( static_cast<int>( *idx_pv ), source );
    }
    return scanValid;
}


bool Scan::checkIdleTimes( std::vector<unsigned int> &maxIdle,
                           const std::shared_ptr<const AbstractSource> &source ) noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " check idle times";
#endif

    bool scan_valid = true;
    bool idleTimeValid;
    unsigned int latestSlewTime;
    // check if idle time is in valid range
    do {
        idleTimeValid = true;

        // get end of slew times
        const vector<unsigned int> &eosl = times_.getEndOfSlewTimes();
        auto it = max_element( eosl.begin(), eosl.end() );
        latestSlewTime = *it;
        long idx = distance( eosl.begin(), it );

        // check idle time for each station
        for ( int i = 0; i < nsta_; ++i ) {
            unsigned int dt = latestSlewTime - eosl[i];
            // if idle time is too long remove station which has latest slew arrival and restart
            if ( dt > maxIdle[i] ) {
                scan_valid = removeStation( static_cast<int>( idx ), source );
                if ( scan_valid ) {
                    maxIdle.erase( next( maxIdle.begin(), idx ) );
                    idleTimeValid = false;
                    break;
                } else {
                    break;
                }
            }
        }

    } while ( !idleTimeValid && !scan_valid );

    // align start times at the end
    if ( scan_valid ) {
        times_.alignStartTimes();
    }

    return scan_valid;
}

double Scan::getAverageSNR( const Network &network, const std::shared_ptr<const AbstractSource> &source,
                            const std::shared_ptr<const Mode> &mode ) {
    double meanSNR = 0.;
    for ( auto &thisObservation : observations_ ) {
        unordered_map<string, double> band2snr = calcSNR( network, source, mode, thisObservation );
        double sum = 0;
        for ( const auto &any : band2snr ) {
            sum += any.second;
        }
        double average = sum / band2snr.size();
        meanSNR += average / getNObs();
    }
    return meanSNR;
}


vector<double> Scan::getSNRs( const Network &network, const std::shared_ptr<const AbstractSource> &source,
                              const std::shared_ptr<const Mode> &mode ) {
    vector<double> snrs;
    for ( auto &thisObservation : observations_ ) {
        unordered_map<string, double> band2snr = calcSNR( network, source, mode, thisObservation );
        double sum = 0;
        for ( const auto &any : band2snr ) {
            sum += any.second;
        }
        double average = sum / band2snr.size();
        snrs.push_back( average );
    }
    return snrs;
}


unordered_map<string, double> Scan::calcSNR( const Network &network,
                                             const std::shared_ptr<const AbstractSource> &source,
                                             const std::shared_ptr<const Mode> &mode,
                                             const Observation &thisObservation ) {
    unordered_map<string, double> band2snr;

    // get station ids from this baseline
    unsigned long staid1 = thisObservation.getStaid1();
    const Station &sta1 = network.getStation( staid1 );
    unsigned long staid2 = thisObservation.getStaid2();
    const Station &sta2 = network.getStation( staid2 );
    const Baseline &bl = network.getBaseline( staid1, staid2 );
    unsigned int duration = thisObservation.getObservingTime();

    // get baseline scan start time
    unsigned int startTime = thisObservation.getStartTime();

    // calculate greenwhich meridian sedirial time
    double date1 = 2400000.5;
    double date2 = TimeSystem::mjdStart + static_cast<double>( startTime ) / 86400.0;
    double gmst = iauGmst82( date1, date2 );

    // loop over each band
    bool flag_observationRemoved = false;
    for ( auto &band : mode->getAllBands() ) {
        double el1 = pointingVectorsStart_[*findIdxOfStationId( staid1 )].getEl();
        double el2 = pointingVectorsStart_[*findIdxOfStationId( staid2 )].getEl();
        double SEFD_src;
        if ( source->needsElDistFlux() ) {
            SEFD_src = source->observedFluxElDist( band, startTime, sta1.getPosition(), sta2.getPosition(), el1, el2 );
        } else if ( source->hasFluxInformation( band ) ) {
            // calculate observed flux density for each band
            SEFD_src = source->observedFlux( band, startTime, gmst, network.getDxyz( staid1, staid2 ) );
        } else if ( ObservingMode::sourceBackup[band] == ObservingMode::Backup::internalModel ) {
            // calculate observed flux density based on model
            double wavelength = ObservingMode::wavelengths[band];
            SEFD_src = source->observedFlux_model( wavelength, startTime, gmst, network.getDxyz( staid1, staid2 ) );
        } else {
            SEFD_src = 1e-3;
        }


        if ( SEFD_src == 0 ) {
            SEFD_src = 1e-3;
        }

        // calculate system equivalent flux density for each station
        double SEFD_sta1 = sta1.getEquip().getSEFD( band, el1 );
        double SEFD_sta2 = sta2.getEquip().getSEFD( band, el2 );

        double efficiency = mode->efficiency( sta1.getId(), sta2.getId() );
        double rec = mode->recordingRate( staid1, staid2, band );
        double SNR = efficiency * SEFD_src / sqrt( SEFD_sta1 * SEFD_sta2 ) * sqrt( rec * duration );
        band2snr[band] = SNR;
    }
    return band2snr;
}


bool Scan::calcObservationDuration( const Network &network, const std::shared_ptr<const AbstractSource> &source,
                                    const std::shared_ptr<const Mode> &mode ) noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace )
        BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " calc required observing time per observation";
#endif

    // check if it is a calibrator scan and there is a fixed scan duration for calibrator scans
    if ( type_ == ScanType::astroCalibrator ) {
        if ( AstrometricCalibratorBlock::targetScanLengthType ==
             AstrometricCalibratorBlock::TargetScanLengthType::seconds ) {
            unsigned int maxObservingTime = AstrometricCalibratorBlock::scanLength;
            // set baseline scan duration to fixed scan duration
            for ( auto &thisObservation : observations_ ) {
                thisObservation.setObservingTime( maxObservingTime );
            }
            return true;
        }
    }

    // check if there is a fixed scan duration for this source
    boost::optional<unsigned int> fixedScanDuration = source->getPARA().fixedScanDuration;
    if ( fixedScanDuration.is_initialized() ) {
        unsigned int maxObservingTime = *fixedScanDuration;
        // set baseline scan duration to fixed scan duration
        for ( auto &thisBaseline : observations_ ) {
            thisBaseline.setObservingTime( maxObservingTime );
        }
        return true;
    }

    // loop over all observed baselines
    int idxObs = 0;
    while ( idxObs < observations_.size() ) {
        auto &thisObservation = observations_[idxObs];

        // get station ids from this baseline
        unsigned long staid1 = thisObservation.getStaid1();
        const Station &sta1 = network.getStation( staid1 );
        unsigned long staid2 = thisObservation.getStaid2();
        const Station &sta2 = network.getStation( staid2 );
        const Baseline &bl = network.getBaseline( staid1, staid2 );

        // get baseline scan start time
        unsigned int startTime = thisObservation.getStartTime();

        // calculate greenwhich meridian sedirial time
        double date1 = 2400000.5;
        double date2 = TimeSystem::mjdStart + static_cast<double>( startTime ) / 86400.0;
        double gmst = iauGmst82( date1, date2 );

        unsigned int maxDuration = 0;

        // loop over each band
        bool flag_observationRemoved = false;
        for ( auto &band : mode->getAllBands() ) {
            double SEFD_src;
            double el1 = pointingVectorsStart_[*findIdxOfStationId( staid1 )].getEl();
            double el2 = pointingVectorsStart_[*findIdxOfStationId( staid2 )].getEl();
            if ( source->needsElDistFlux() ) {
                SEFD_src =
                    source->observedFluxElDist( band, startTime, sta1.getPosition(), sta2.getPosition(), el1, el2 );
            } else if ( source->hasFluxInformation( band ) ) {
                // calculate observed flux density for each band
                SEFD_src = source->observedFlux( band, startTime, gmst, network.getDxyz( staid1, staid2 ) );
            } else if ( ObservingMode::sourceBackup[band] == ObservingMode::Backup::internalModel ) {
                // calculate observed flux density based on model
                double wavelength = ObservingMode::wavelengths[band];
                SEFD_src = source->observedFlux_model( wavelength, startTime, gmst, network.getDxyz( staid1, staid2 ) );
            } else {
                SEFD_src = 1e-3;
            }


            if ( SEFD_src == 0 ) {
                SEFD_src = 1e-3;
            }

            // calculate system equivalent flux density for each station
            double SEFD_sta1 = sta1.getEquip().getSEFD( band, el1 );
            double SEFD_sta2 = sta2.getEquip().getSEFD( band, el2 );

            // get minimum required SNR for each station, baseline and source
            double minSNR_sta1 = sta1.getPARA().minSNR.at( band );
            double minSNR_sta2 = sta2.getPARA().minSNR.at( band );
            double minSNR_bl = bl.getParameters().minSNR.at( band );
            double minSNR_src = source->getPARA().minSNR.at( band );

            // maximum required minSNR
            double maxminSNR = max( { minSNR_src, minSNR_bl, minSNR_sta1, minSNR_sta2 } );

            // get maximum correlator synchronization time for
            double maxCorSynch1 = sta1.getPARA().midob;
            double maxCorSynch2 = sta2.getPARA().midob;
            double maxCorSynch = max( { maxCorSynch1, maxCorSynch2 } );

            // calc required baseline scan duration
            double efficiency = mode->efficiency( sta1.getId(), sta2.getId() );
            double anum = ( maxminSNR / ( SEFD_src * efficiency ) );
            double anu1 = SEFD_sta1 * SEFD_sta2;
            double anu2 = mode->recordingRate( staid1, staid2, band );
            if ( anu2 == 0 ) {
                return false;
            }
            double new_duration = anum * anum * anu1 / anu2 + maxCorSynch;
            new_duration = ceil( new_duration );
            auto new_duration_uint = static_cast<unsigned int>( new_duration );

            // check if required baseline scan duration is within min and max scan times of baselines
            unsigned int minScanBl = bl.getParameters().minScan;
            if ( new_duration_uint < minScanBl ) {
                new_duration_uint = minScanBl;
            }
            unsigned int maxScanBl = bl.getParameters().maxScan;
            if ( new_duration_uint > maxScanBl ) {
                bool scanValid = removeObservation( idxObs, source );
                if ( !scanValid ) {
                    return false;
                }
                flag_observationRemoved = true;
                break;
            }

            // change maxDuration if it is higher for this band
            if ( new_duration_uint > maxDuration ) {
                maxDuration = new_duration_uint;
            }
        }

        // if you have not removed the baseline increment counter and set the baseline scan duration
        if ( !flag_observationRemoved ) {
            ++idxObs;
            thisObservation.setObservingTime( maxDuration );
        }
    }

    if ( source->getPARA().forceSameObservingDuration ) {
        unsigned int maxDurationInScan = 0;
        for ( const auto &obs : observations_ ) {
            unsigned int thisDuration = obs.getObservingTime();
            maxDurationInScan = max( maxDurationInScan, thisDuration );
        }
        for ( auto &obs : observations_ ) {
            obs.setObservingTime( maxDurationInScan );
        }
    }

    return true;
}


bool Scan::scanDuration( const Network &network, const shared_ptr<const AbstractSource> &source ) noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace )
        BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " calc required observing time per station";
#endif

    // check if it is a calibrator scan with a fixed scan duration
    if ( type_ == ScanType::astroCalibrator ) {
        if ( AstrometricCalibratorBlock::targetScanLengthType ==
             AstrometricCalibratorBlock::TargetScanLengthType::seconds ) {
            times_.setObservingTimes( AstrometricCalibratorBlock::scanLength );
            return true;
        }
    }

    // check if there is a fixed scan duration for this source
    boost::optional<unsigned int> fixedScanDuration = source->getPARA().fixedScanDuration;
    if ( fixedScanDuration.is_initialized() ) {
        times_.setObservingTimes( *fixedScanDuration );
        return true;
    }

    // save minimum and maximum scan time
    vector<unsigned int> minscanTimes( nsta_, source->getPARA().minScan );
    vector<unsigned int> maxScanTimes( nsta_, source->getPARA().maxScan );
    for ( int i = 0; i < nsta_; ++i ) {
        unsigned long staid = pointingVectorsStart_[i].getStaid();
        unsigned int stationMinScanTime = network.getStation( staid ).getPARA().minScan;
        unsigned int stationMaxScanTime = network.getStation( staid ).getPARA().maxScan;

        if ( minscanTimes[i] < stationMinScanTime ) {
            minscanTimes[i] = stationMinScanTime;
        }
        if ( maxScanTimes[i] > stationMaxScanTime ) {
            maxScanTimes[i] = stationMaxScanTime;
        }
    }

    // iteratively calculate scan times
    bool scanDurationsValid;
    vector<unsigned int> scanTimes;
    do {
        scanDurationsValid = true;

        // counter which counts how often a station should be removed due to too long observation time
        vector<int> counter( nsta_ );

        // set scanTimes to minimum required scan times
        scanTimes = minscanTimes;

        // loop through all baselines
        for ( auto &thisObservation : observations_ ) {
            // get index of stations in this baseline
            unsigned long staid1 = thisObservation.getStaid1();
            boost::optional<unsigned long> staidx1o = findIdxOfStationId( staid1 );
            unsigned long staidx1 = *staidx1o;
            unsigned long staid2 = thisObservation.getStaid2();
            boost::optional<unsigned long> staidx2o = findIdxOfStationId( staid2 );
            unsigned long staidx2 = *staidx2o;

            // get scan duration of baseline
            unsigned int duration = thisObservation.getObservingTime();

            // if there is a higher required scan time update scanTimes
            if ( scanTimes[staidx1] < duration ) {
                scanTimes[staidx1] = duration;
            }
            if ( scanTimes[staidx2] < duration ) {
                scanTimes[staidx2] = duration;
            }

            // check if required duration is higher then maximum allowed scan time.
            // Increase counter for station which are observed too long
            if ( duration > maxScanTimes[staidx1] || duration > maxScanTimes[staidx2] ) {
                counter[staidx1]++;
                counter[staidx2]++;
                scanDurationsValid = false;
            }
        }

        // if the scan duration is not valid
        if ( !scanDurationsValid ) {
            int eraseThis;

            // get index of all station which have the maximum amount of too long observations
            int max = 0;
            vector<int> maxIdx;
            for ( int i = 0; i < nsta_; ++i ) {
                if ( counter[i] == max ) {
                    maxIdx.push_back( i );
                }
                if ( counter[i] > max ) {
                    max = counter[i];
                    maxIdx.clear();
                    maxIdx.push_back( i );
                }
            }

            // if there is only one maximum remove this station
            if ( maxIdx.size() == 1 ) {
                eraseThis = maxIdx[0];
            } else {
                // if more stations have the same maximum amount of too long observations look at the max SEFD and
                // remove station with highest SEFD
                double maxSEFD = 0;
                vector<int> maxSEFDId( maxIdx.size() );
                for ( int i = 0; i < maxIdx.size(); ++i ) {
                    int thisIdx = maxIdx[i];
                    unsigned long staid = pointingVectorsStart_[thisIdx].getStaid();
                    double thisMaxSEFD = network.getStation( staid ).getEquip().getMaxSEFD();
                    if ( thisMaxSEFD == maxSEFD ) {
                        maxSEFDId.push_back( thisIdx );
                    }
                    if ( thisMaxSEFD > maxSEFD ) {
                        maxSEFD = thisMaxSEFD;
                        maxSEFDId.clear();
                        maxSEFDId.push_back( i );
                    }
                }

                // if there is only one maximum remove this station
                if ( maxSEFDId.size() == 1 ) {
                    eraseThis = maxSEFDId[0];
                } else {
                    // if more stations have the same maximum amount of too long observations and highest SEFD
                    // look at the earliest possible scan start time and remove station with latest scan start time
                    vector<unsigned int> thisScanStartTimes( maxSEFDId.size() );
                    for ( int i : maxSEFDId ) {
                        thisScanStartTimes[( times_.getSlewTime( i, Timestamp::end ) )];
                    }

                    // remove station with latest slew end time. If multiple have the same value simply pick one
                    long maxSlewEnd = distance( thisScanStartTimes.begin(),
                                                max_element( thisScanStartTimes.begin(), thisScanStartTimes.end() ) );
                    eraseThis = maxSEFDId[maxSlewEnd];
                }
            }

            bool scanValid = removeStation( eraseThis, source );
            if ( !scanValid ) {
                return false;
            }
            minscanTimes.erase( next( minscanTimes.begin(), eraseThis ) );
        }

    } while ( !scanDurationsValid );

    times_.setObservingTimes( scanTimes );
    return true;
}


boost::optional<unsigned long> Scan::findIdxOfStationId( unsigned long id ) const noexcept {
    for ( unsigned long idx = 0; idx < nsta_; ++idx ) {
        if ( pointingVectorsStart_[idx].getStaid() == id ) {
            return idx;
        }
    }
    return boost::none;
}


vector<unsigned long> Scan::getStationIds() const noexcept {
    vector<unsigned long> ids;
    ids.reserve( nsta_ );
    for ( int i = 0; i < nsta_; ++i ) {
        ids.push_back( pointingVectorsStart_[i].getStaid() );
    }

    return std::move( ids );
}


double Scan::calcScore_numberOfObservations( unsigned long maxObs ) const noexcept {
    unsigned long nbl = observations_.size();
    double thisScore = static_cast<double>( nbl ) / static_cast<double>( maxObs );
    return thisScore;
}


double Scan::calcScore_idleTime( const std::vector<double> &idleScore ) const noexcept {
    double score = 0;
    for ( int idx = 0; idx < nsta_; ++idx ) {
        unsigned long thisId = pointingVectorsStart_[idx].getStaid();
        score += idleScore[thisId];
    }

    return score;
}

double Scan::calcScore_closures( unsigned long nclosures_max, const std::shared_ptr<const AbstractSource> &source) const noexcept{
    double score = 0;
    unsigned long nClosures = source->getNClosures();
    if (nClosures >= WeightFactors::maxClosures){
        return score;
    }

    unsigned long closures;
    if (nsta_ <= 2){
        closures = 0;
    }else{
        closures = (nsta_-1)*(nsta_-2)/2 + nsta_ * (nsta_ -3) / 2;
    }

    score = static_cast<double>(closures) / static_cast<double>(nclosures_max);
    return score;
}


double Scan::calcScore_averageStations( const vector<double> &astas, unsigned long nMaxSta ) const noexcept {
    double finalScore = 0;
    unsigned long nMaxStaPossible = nMaxSta - 1;
    double max = accumulate( astas.begin(), astas.end(), 0.0 );
    if ( max < 1e-4 ) {
        return 0;
    }

    for ( const auto &pv : pointingVectorsStart_ ) {
        unsigned long staid = pv.getStaid();
        finalScore += astas[staid];
    }
    finalScore /= max;

    return finalScore;
}


double Scan::calcScore_averageBaselines( const std::vector<double> &abls ) const noexcept {
    double finalScore = 0;
    for ( const auto &obs : observations_ ) {
        unsigned long blid = obs.getBlid();
        finalScore += abls[blid];
    }

    return finalScore;
}


double Scan::calcScore_averageSources( const vector<double> &asrcs, unsigned long nMaxBls ) const noexcept {
    unsigned long nbl = observations_.size();
    return asrcs[srcid_] * static_cast<double>( nbl ) / static_cast<double>( nMaxBls );
}


double Scan::calcScore_duration( unsigned long nMaxSta, unsigned int minTime, unsigned int maxTime ) const noexcept {
    unsigned int thisScanDuration = times_.getScanDuration();
    double score;
    if ( maxTime - minTime == 0 ) {
        score = 1;
    } else {
        score = 1 - static_cast<double>( thisScanDuration - minTime ) / static_cast<double>( maxTime - minTime );
    }
    score *= static_cast<double>( nsta_ ) / static_cast<double>( nMaxSta );
    return score;
}


double Scan::calcScore_lowElevation( unsigned long nmaxsta ) {
    double score = 0;
    for ( const auto &pv : pointingVectorsStart_ ) {
        double el = pv.getEl();
        double f = 0;
        if ( el > WeightFactors::lowElevationStartWeight ) {
            f = 0;
        } else if ( el < WeightFactors::lowElevationFullWeight ) {
            f = 1;
        } else {
            f = ( el - WeightFactors::lowElevationStartWeight ) /
                ( WeightFactors::lowElevationFullWeight - WeightFactors::lowElevationStartWeight );
        }
        score += f;
    }
    return score / nmaxsta;
}


double Scan::calcScore_lowDeclination( unsigned long nMaxObs ) {
    double dec = pointingVectorsStart_[0].getDc();
    double score = 0;
    if ( dec > WeightFactors::declinationStartWeight ) {
        score = 0;
    } else if ( dec < WeightFactors::declinationFullWeight ) {
        score = 1;
    } else {
        score = ( dec - WeightFactors::declinationStartWeight ) /
                ( WeightFactors::declinationFullWeight - WeightFactors::declinationStartWeight );
    }
    return score * static_cast<double>( observations_.size() ) / static_cast<double>( nMaxObs );
}


bool Scan::rigorousUpdate( Network &network, const std::shared_ptr<const AbstractSource> &source,
                           const std::shared_ptr<const Mode> &mode,
                           const boost::optional<StationEndposition> &endposition ) noexcept {
    bool scanValid;
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " rigorous update";
#endif

    // check if source is available during whole scan and iteratively check everything
    bool stationRemoved;
    do {
        stationRemoved = false;

        // calc earliest possible slew end times for each station:
        scanValid = rigorousSlewtime( network, source );
        if ( !scanValid ) {
#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " no longer valid";
#endif
            return scanValid;
        }

        scanValid = rigorousTotalObservingDuration( network, source );
        if ( !scanValid ) {
#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " no longer valid";
#endif
            return scanValid;
        }

        // check if sun distance criteria is fulfilled
        scanValid = rigorousSunDistance( network, source );
        if ( !scanValid ) {
#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " no longer valid";
#endif
            return scanValid;
        }

        // check if source is not moving faster than antenna slew speed
        scanValid = rigorousSourceVelocity( network, source );
        if ( !scanValid ) {
#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " no longer valid";
#endif
            return scanValid;
        }

        // align start times to earliest possible one:
        scanValid = rigorousScanStartTimeAlignment( network, source, mode );
        if ( !scanValid ) {
#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " no longer valid";
#endif
            return scanValid;
        }

        // check if source is available during whole scan
        scanValid = rigorousScanVisibility( network, source, stationRemoved );
        if ( !scanValid ) {
#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " no longer valid";
#endif
            return scanValid;
        }
        if ( stationRemoved ) {
            continue;
        }

        // check if source is available during whole scan
        scanValid = rigorousSatelliteAvoidance( network, source, stationRemoved );
        if ( !scanValid ) {
#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " no longer valid";
#endif
            return scanValid;
        }
        if ( stationRemoved ) {
            continue;
        }

        // check if end position can be reached
        scanValid = rigorousScanCanReachEndposition( network, source, endposition, stationRemoved );
        if ( !scanValid ) {
#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " no longer valid";
#endif
            return scanValid;
        }

    } while ( stationRemoved );

    return true;
}


bool Scan::rigorousSlewtime( Network &network, const std::shared_ptr<const AbstractSource> &source ) noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " rigorous update slewtime";
#endif

    bool scanValid = true;

    // loop through all stations
    int ista = 0;
    while ( ista < nsta_ ) {
        PointingVector &pv = pointingVectorsStart_[ista];
        unsigned int slewStart = times_.getSlewTime( ista, Timestamp::start );
        Station &thisStation = network.refStation( pv.getStaid() );

        // old slew end time and new slew end time, required for iteration
        unsigned int twoOldSlewEnd = 0;
        unsigned int oldSlewEnd = 0;
        unsigned int newSlewEnd = times_.getSlewTime( ista, Timestamp::end );

        // big slew indicates if the slew distance is > 180 degrees
        bool bigSlew = false;

        // timeDiff is difference between two estimated slew rates in iteration.
        unsigned int timeDiff = numeric_limits<unsigned int>::max();

        bool stationRemoved = false;
        // iteratively calculate slew time
        unsigned int counter = 0;
        while ( timeDiff > 1 ) {
            // break condition to avoid infinite loops - necessary for some satellite scans although I do not understand
            // why...
            ++counter;
            if(counter>11){
                scanValid = false;
                return scanValid;
            }
            // change slew times for iteration
            oldSlewEnd = newSlewEnd;
            double oldAz = pv.getAz();

            // calculate az, el for pointing vector for previouse time
            pv.setTime( oldSlewEnd );
            thisStation.calcAzEl_rigorous( source, pv );
            if ( !thisStation.isVisible( pv, source->getPARA().minElevation ) ) {
                scanValid = removeStation( ista, source );
                if ( !scanValid ) {
                    return scanValid;
                }
                stationRemoved = true;
                break;
            }
            thisStation.getCableWrap().calcUnwrappedAz( thisStation.getCurrentPointingVector(), pv );

            // if you have a "big slew" unwrap the azimuth near the old azimuth
            if ( bigSlew ) {
                thisStation.getCableWrap().unwrapAzNearAz( pv, oldAz );
            } else {
                thisStation.getCableWrap().calcUnwrappedAz( thisStation.getCurrentPointingVector(), pv );
            }
            double newAz = pv.getAz();

            // check if you are near the cable wrap limits and the software decides to slew the other direction
            if ( std::abs( oldAz - newAz ) > .5 * pi ) {
                // big slew detected, this means you are somewhere close to the cable wrap limit.
                // from this point on you calc your unwrapped Az near the azimuth, which is further away from this limit

                // if there was already a bigSlew flag you know that both azimuth are unsafe... remove station
                if ( bigSlew ) {
                    scanValid = removeStation( ista, source );
                    if ( !scanValid ) {
                        return scanValid;
                    }
                    stationRemoved = true;
                    break;
                }
                // otherwise set the big slew flag
                bigSlew = true;
            }

            // calculate new slewtime
            auto thisSlewtime = thisStation.slewTime( pv );
            if ( !thisSlewtime.is_initialized() ) {
                scanValid = removeStation( ista, source );
                if ( !scanValid ) {
                    return scanValid;
                }
                stationRemoved = true;
                break;
            }

            // calculate new slew end time and time difference
            unsigned int newSlewTime = *thisSlewtime;
            newSlewEnd = slewStart + newSlewTime;
            if ( newSlewEnd > oldSlewEnd ) {
                timeDiff = newSlewEnd - oldSlewEnd;
            } else {
                timeDiff = oldSlewEnd - newSlewEnd;
            }
        }
        // if no station was removed update slewtimes and increase counter... otherwise restart with same staid
        if ( !stationRemoved ) {
            // update the slewtime
            times_.setSlewTime( ista, max( { newSlewEnd, oldSlewEnd } ) - slewStart );
            ++ista;
        }
    }

    return scanValid;
}

bool Scan::rigorousTotalObservingDuration( Network &network,
                                           const std::shared_ptr<const AbstractSource> &source ) noexcept {
    bool scanValid = true;

    int idx = 0;
    while ( idx < nsta_ ) {
        PointingVector &pv = pointingVectorsStart_[idx];
        const Station &thisStation = network.getStation( pv.getStaid() );
        unsigned int dur = times_.getObservingDuration( idx );
        if ( thisStation.getTotalObservingTime() + dur > thisStation.getPARA().maxTotalObsTime ) {
            scanValid = removeStation( idx, source );
            if ( !scanValid ) {
                return scanValid;
            }
        } else {
            ++idx;
        }
    }

    return scanValid;
}


bool Scan::rigorousScanStartTimeAlignment( Network &network, const std::shared_ptr<const AbstractSource> &source,
                                           const std::shared_ptr<const Mode> &mode ) noexcept {
    bool scanValid;
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace )
        BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " rigorous update scan start time";
#endif

    // iteratively align start times
    unsigned long nsta_beginning;
    do {
        nsta_beginning = nsta_;

        // align start times
        times_.alignStartTimes();
        scanValid = constructObservations( network, source );
        if ( !scanValid ) {
            return scanValid;
        }
        if ( nsta_ != nsta_beginning ) {
            continue;
        }

        // calc baseline scan durations
        scanValid = calcObservationDuration( network, source, mode );
        if ( !scanValid ) {
            return scanValid;
        }
        if ( nsta_ != nsta_beginning ) {
            continue;
        }

        // calc scan durations
        scanValid = scanDuration( network, source );
        if ( !scanValid ) {
            return scanValid;
        }

    } while ( nsta_ != nsta_beginning );

    return scanValid;
}


bool Scan::rigorousScanVisibility( Network &network, const std::shared_ptr<const AbstractSource> &source,
                                   bool &stationRemoved ) noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " rigorous update visibility";
#endif

    pointingVectorsEnd_.clear();

    // loop over all stations
    int ista = 0;
    while ( ista < nsta_ ) {
        // get all required members
        unsigned int scanStart = times_.getObservingTime( ista, Timestamp::start );
        unsigned int scanEnd = times_.getObservingTime( ista, Timestamp::end );
        const PointingVector &pv = pointingVectorsStart_[ista];
        Station &thisStation = network.refStation( pv.getStaid() );

        // create moving pointing vector which is used to check visibility during scan
        PointingVector moving_pv( pv.getStaid(), pv.getSrcid() );
        moving_pv.setAz( pv.getAz() );
        moving_pv.setEl( pv.getEl() );

        // loop over whole scan time in 30 second steps.
        // Ignore last 30seconds because it is in a next step checked at end time
        for ( unsigned int time = scanStart; time < scanEnd; time += 30 ) {
            // check if there is no change in slew direction (no change in azimuth ambigurity)
            double oldAz = moving_pv.getAz();
            moving_pv.setTime( time );
            thisStation.calcAzEl_rigorous( source, moving_pv );
            thisStation.getCableWrap().unwrapAzNearAz( moving_pv, oldAz );
            double newAz = moving_pv.getAz();

            // check if there is a change in azimuth ambiguity during scan
            if ( std::abs( oldAz - newAz ) > .5 * pi ) {
                stationRemoved = true;
                return removeStation( ista, source );
            }

            // check if source is visible during scan
            bool flag = thisStation.isVisible( moving_pv, source->getPARA().minElevation );
            if ( !flag ) {
                stationRemoved = true;
                return removeStation( ista, source );
            }

            if ( time == scanStart ) {
                pointingVectorsStart_[ista].copyValuesFromOtherPv( moving_pv );
            }
        }

        // check if source is visible at scan end time
        double oldAz = moving_pv.getAz();
        moving_pv.setTime( scanEnd );
        thisStation.calcAzEl_rigorous( source, moving_pv );
        thisStation.getCableWrap().unwrapAzNearAz( moving_pv, oldAz );
        double newAz = moving_pv.getAz();

        // check if there is a change in azimuth ambigurity during scan
        if ( std::abs( oldAz - newAz ) > .5 * pi ) {
            stationRemoved = true;
            return removeStation( ista, source );
        }

        // check if source is visible during scan
        bool flag = thisStation.isVisible( moving_pv, source->getPARA().minElevation );
        if ( !flag ) {
            stationRemoved = true;
            return removeStation( ista, source );
        }

        // save pointing vector for end time
        pointingVectorsEnd_.push_back( moving_pv );
        ++ista;
    }

    return true;
}

bool Scan::rigorousSatelliteAvoidance( Network &network, const std::shared_ptr<const AbstractSource> &source,
                                       bool &stationRemoved ) noexcept {
    if ( AvoidSatellites::satellitesToAvoid.empty() ) {
        return true;
    }

    for ( int idx = 0; idx < pointingVectorsStart_.size(); ++idx ) {
        if ( pointingVectorsStart_[idx].getEl() < AvoidSatellites::minElevation ) {
            continue;
        }
        vector<unsigned int> times;
        unsigned int t = getTimes().getObservingTime( idx );
        while ( t < getTimes().getObservingTime( idx, Timestamp::end ) ) {
            times.push_back( t );
            t += AvoidSatellites::frequency;
        }
        times.push_back( getTimes().getObservingTime( idx, Timestamp::end ) );
        unsigned long staid = getStationId( idx );

        Station &sta = network.refStation( staid );
        for ( auto tt : times ) {
            pair<double, double> rade_src = source->getRaDe( tt, sta.getPosition() );
            for ( const auto &sat : AvoidSatellites::satellitesToAvoid ) {
                const vector<pair<int, int>> &precalc = AvoidSatellites::visible_[staid][sat->getId()];
                bool calc = false;
                for ( const auto &any : precalc ) {
                    unsigned int start = any.first;
                    unsigned int end = any.second;
                    if ( tt >= start && tt <= end ) {
                        calc = true;
                        break;
                    }
                }
                if ( !calc ) {
                    continue;
                }

                auto rade_sat = sat->calcRaDeDistTime( tt, sta.getPosition() );
                double tmp =
                    sin( rade_src.second ) * sin( get<1>( rade_sat ) ) +
                    cos( rade_src.second ) * cos( get<1>( rade_sat ) ) * cos( rade_src.first - get<0>( rade_sat ) );
                double accurate_delta = acos( tmp );

                double dt = get<3>( rade_sat );
                double orbitError = AvoidSatellites::orbitError + AvoidSatellites::orbitErrorPerDay * dt;
                double threshold = atan( orbitError / get<2>( rade_sat ) ) + AvoidSatellites::extraMargin;
                if ( accurate_delta < threshold ) {
                    stationRemoved = true;
                    return removeStation( idx, source );
                }
            }
        }
    }
    return true;
}

void Scan::checkSatelliteDistance( Station &sta, const shared_ptr<const AbstractSource> &source, ofstream &of ) {
    boost::optional<unsigned long> oidx = findIdxOfStationId( sta.getId() );
    if ( !oidx.is_initialized() ) {
        return;
    }

    unsigned long idx = oidx.get();
    if ( pointingVectorsStart_[idx].getEl() < AvoidSatellites::minElevation ) {
        return;
    }


    unsigned int startTime = getTimes().getObservingTime();
    vector<unsigned int> times;
    unsigned int t = getTimes().getObservingTime( idx );
    while ( t < getTimes().getObservingTime( idx, Timestamp::end ) ) {
        times.push_back( t );
        t += AvoidSatellites::frequency;
    }
    times.push_back( getTimes().getObservingTime( idx, Timestamp::end ) );

    for ( auto tt : times ) {
        pair<double, double> rade_src = source->getRaDe( tt, sta.getPosition() );
        for ( const auto &sat : AvoidSatellites::satellitesToAvoid ) {
            auto rade_sat = sat->calcRaDeDistTime( tt, sta.getPosition() );
            double tmp =
                sin( rade_src.second ) * sin( get<1>( rade_sat ) ) +
                cos( rade_src.second ) * cos( get<1>( rade_sat ) ) * cos( rade_src.first - get<0>( rade_sat ) );
            double accurate_delta = acos( tmp );

            double dt = get<3>( rade_sat );
            double orbitError = AvoidSatellites::orbitError + AvoidSatellites::orbitErrorPerDay * dt;
            double threshold = atan( orbitError / get<2>( rade_sat ) ) + AvoidSatellites::extraMargin;
            if ( accurate_delta < threshold ) {
                of << boost::format( "%8s %8s %2s %s %s\n" ) % source->getName() % sta.getName() %
                          sta.getAlternativeName() % TimeSystem::time2string_doy( startTime ) %
                          TimeSystem::time2string( startTime );
                return;
            }
        }
    }
}


bool Scan::rigorousSunDistance( const Network &network, const std::shared_ptr<const AbstractSource> &thisSource ) {
    int ista = 0;
    bool valid = true;
    while ( ista < nsta_ ) {
        // get all required members
        unsigned int scanStart = times_.getObservingTime( ista, Timestamp::start );
        unsigned int scanEnd = times_.getObservingTime( ista, Timestamp::end );
        const PointingVector &pv = pointingVectorsStart_[ista];
        const Station &thisStation = network.getStation( pv.getStaid() );

        // loop over whole scan time in 30 second steps.
        // Ignore last 30seconds because it is in a next step checked at end time
        bool removed = false;
        for ( unsigned int time = scanStart; time < scanEnd; time += 30 ) {
            double dist = thisSource->getSunDistance( time, thisStation.getPosition() );
            if ( dist < thisSource->getPARA().minSunDistance ) {
                valid = removeStation( ista, thisSource );
                if ( !valid ) {
                    return valid;
                }
                removed = true;
                break;
            }
        }
        if (removed){
            continue;
        }

        // check distance at scan end time
        double dist = thisSource->getSunDistance( scanEnd, thisStation.getPosition() );
        if ( dist < thisSource->getPARA().minSunDistance ) {
            valid = removeStation( ista, thisSource );
            if ( !valid ) {
                return valid;
            }
            removed = true;
            break;
        }

        if ( !removed ) {
            ++ista;
        }
    }
    return valid;
}

bool Scan::rigorousSourceVelocity( Network &network, const shared_ptr<const AbstractSource> &source ) {
    int ista = 0;
    bool valid = true;

    while ( ista < nsta_ ) {
        bool stationRemoved = false;

        // get all required members
        unsigned int scanStart = times_.getObservingTime( ista, Timestamp::start );
        unsigned int scanEnd = times_.getObservingTime( ista, Timestamp::end );
        const PointingVector &pv = pointingVectorsStart_[ista];
        Station &thisStation = network.refStation( pv.getStaid() );

        // create moving pointing vectors which are used to check slew time during tracking
        PointingVector moving_pv_before( pv.getStaid(), pv.getSrcid() );
        moving_pv_before.setAz( pv.getAz() );
        moving_pv_before.setEl( pv.getEl() );
        moving_pv_before.setHa( pv.getHa() );
        moving_pv_before.setDc( pv.getDc() );
        moving_pv_before.setTime( pv.getTime() );

        PointingVector moving_after( pv.getStaid(), pv.getSrcid() );

        // loop over whole scan time in 30 second steps.
        // Ignore last 30seconds because it is in a next step checked at end time
        unsigned int dt = 30;
        unsigned int scanEndLoop = scanEnd;
        if ( scanEndLoop > 10 ) {
            scanEndLoop -= 10;
        }
        for ( unsigned int time = scanStart + dt; time < scanEndLoop; time += dt ) {
            double oldAz = moving_pv_before.getAz();
            moving_after.setTime( time );
            thisStation.calcAzEl_rigorous( source, moving_after );
            thisStation.getCableWrap().unwrapAzNearAz( moving_after, oldAz );

            unsigned int slewTimeTracking = thisStation.getAntenna().slewTimeTracking( moving_pv_before, moving_after );
            if ( slewTimeTracking > dt ) {
                valid = removeStation( ista, source );
                if ( !valid ) {
                    return valid;
                }
                stationRemoved = true;
                break;
            }

            moving_pv_before = moving_after;
        }
        if(stationRemoved){
            continue;
        }

        dt = scanEnd - moving_pv_before.getTime();
        unsigned int time = scanEnd;
        double oldAz = moving_pv_before.getAz();
        moving_after.setTime( time );
        thisStation.calcAzEl_rigorous( source, moving_after );
        thisStation.getCableWrap().unwrapAzNearAz( moving_after, oldAz );

        unsigned int slewTimeTracking = thisStation.getAntenna().slewTimeTracking( moving_pv_before, moving_after );
        if ( slewTimeTracking > dt ) {
            valid = removeStation( ista, source );
            if ( !valid ) {
                return valid;
            }
            stationRemoved = true;
            break;
        }

        if ( !stationRemoved ) {
            ++ista;
        }
    }
    return valid;
}


bool Scan::rigorousScanCanReachEndposition( const Network &network,
                                            const std::shared_ptr<const AbstractSource> &thisSource,
                                            const boost::optional<StationEndposition> &endposition,
                                            bool &stationRemoved ) {
    if ( !endposition.is_initialized() ) {
        return true;
    }
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace )
        BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " rigorous update to reach endposition";
#endif

    for ( int idxSta = 0; idxSta < nsta_; ++idxSta ) {
        // start position for slewing
        const PointingVector &slewStart = pointingVectorsEnd_[idxSta];

        // get station
        unsigned long staid = slewStart.getStaid();
        const Station &thisSta = network.getStation( staid );

        // check if there is a required endposition
        int possibleEndpositionTime;
        if ( endposition->hasEndposition( staid ) ) {
            // required endposition for slewing
            const PointingVector &thisEndposition = endposition->getFinalPosition( staid ).get();

            // calculate slew time between pointing vectors
            unsigned int duration = times_.getObservingDuration( idxSta );
            auto oslewtime = thisSta.slewTime( slewStart, thisEndposition, duration, true );
            unsigned int slewtime;
            if ( oslewtime.is_initialized() ) {
                slewtime = *oslewtime;
            } else {
                slewtime = TimeSystem::duration;
            }

            // check if there is enough time
            possibleEndpositionTime = times_.getObservingTime( idxSta, Timestamp::end ) +
                                      thisSta.getPARA().systemDelay + slewtime + thisSta.getPARA().preob;
        } else {
            possibleEndpositionTime = times_.getObservingTime( idxSta, Timestamp::end );
        }

        // get minimum required endpositon time
        int requiredEndpositionTime = endposition->requiredEndpositionTime( staid );

        if ( possibleEndpositionTime > requiredEndpositionTime ) {
            stationRemoved = true;
            return removeStation( idxSta, thisSource );
        }
    }
    return true;
}


void Scan::addTagalongStation( const PointingVector &pv_start, const PointingVector &pv_end,
                               const std::vector<Observation> &observations, unsigned int slewtime,
                               const Station &station ) {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace )
        BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " add tagalong station " << station.getName();
#endif
    pointingVectorsStart_.push_back( pv_start );
    pointingVectorsEnd_.push_back( pv_end );
    ++nsta_;
    for ( const auto &any : observations ) {
#ifdef VIESCHEDPP_LOG
        if ( Flags::logTrace )
            BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " add tagalong observation between stations "
                                       << any.getStaid1() << " and " << any.getStaid2();
#endif
        observations_.push_back( any );
    }
    if ( station.getPARA().firstScan ) {
        times_.addTagalongStationTime( pv_start, pv_end, 0, 0, 0, 0 );
    } else {
        times_.addTagalongStationTime( pv_start, pv_end, slewtime, station.getCurrentTime(),
                                       station.getPARA().systemDelay, station.getPARA().preob );
    }
}


double Scan::calcScore_firstPart( const std::vector<double> &astas, const std::vector<double> &asrcs,
                                  const std::vector<double> &abls, unsigned int minTime, unsigned int maxTime,
                                  const Network &network, const std::shared_ptr<const AbstractSource> &source,
                                  bool subnetting, const std::vector<double> &idleScore ) {
    unsigned long nmaxsta = network.getNSta();
    unsigned long nmaxbl = network.getNBls();
    double this_score = 0;

    double weight_numberOfObservations = WeightFactors::weightNumberOfObservations;
    if ( weight_numberOfObservations != 0 ) {
        this_score += calcScore_numberOfObservations( nmaxbl ) * weight_numberOfObservations;
    }
    double weight_averageSources = WeightFactors::weightAverageSources;
    if ( weight_averageSources != 0 && !asrcs.empty() ) {
        this_score += calcScore_averageSources( asrcs, nmaxbl ) * weight_averageSources;
    }
    double weight_averageStations = WeightFactors::weightAverageStations;
    if ( weight_averageStations != 0 && !astas.empty() ) {
        this_score += calcScore_averageStations( astas, nmaxsta ) * weight_averageStations;
    }
    double weight_averageBaselines = WeightFactors::weightAverageBaselines;
    if ( weight_averageBaselines != 0 && !abls.empty() ) {
        this_score += calcScore_averageBaselines( abls ) * weight_averageBaselines;
    }
    double weight_duration = WeightFactors::weightDuration;
    if ( weight_duration != 0 ) {
        this_score += calcScore_duration( nmaxsta, minTime, maxTime ) * weight_duration;
    }
    double weight_idle = WeightFactors::weightIdleTime;
    if ( weight_idle != 0 ) {
        this_score += calcScore_idleTime( idleScore ) * weight_idle;
    }
    double weight_closures = WeightFactors::weightClosures;
    if ( weight_closures != 0 ) {
        this_score += calcScore_closures( network.getNClosures_max(), source ) * weight_closures;
    }

    double weightDeclination = WeightFactors::weightDeclination;
    if ( weightDeclination != 0 ) {
        this_score += calcScore_lowDeclination( nmaxbl ) * weightDeclination;
    }

    double weightLowElevation = WeightFactors::weightLowElevation;
    if ( weightLowElevation != 0 ) {
        this_score += calcScore_lowElevation( nmaxsta ) * weightLowElevation;
    }

    return this_score;
}


double Scan::calcScore_secondPart( double this_score, const Network &network,
                                   const std::shared_ptr<const AbstractSource> &source, bool calib ) {
    if ( source->getPARA().tryToFocusIfObservedOnce ) {
        unsigned int nscans = source->getNscans();
        if ( nscans > 0 ) {
            if ( *source->getPARA().tryToFocusOccurrency == AbstractSource::TryToFocusOccurrency::once ) {
                if ( *source->getPARA().tryToFocusType == AbstractSource::TryToFocusType::additive ) {
                    this_score += *source->getPARA().tryToFocusFactor;
                } else {
                    this_score *= *source->getPARA().tryToFocusFactor;
                }
            } else {
                if ( *source->getPARA().tryToFocusType == AbstractSource::TryToFocusType::additive ) {
                    this_score += ( nscans * *source->getPARA().tryToFocusFactor );
                } else {
                    this_score *= ( nscans * *source->getPARA().tryToFocusFactor );
                }
            }
        }
    }
//    auto includes = [](const vector<unsigned long> &v, unsigned long x){
//        return find(v.begin(), v.end(), x) != v.end();
//    };
//    const auto &staids = getStationIds();
//    if ( staids.size() == 3 && includes(staids, 3) && includes(staids, 4) && includes(staids,6)){
//        this_score *= .1;
//    }

    if ( !calib ) {
        if ( scanSequence_flag && type_ == ScanType::standard ) {
            if ( scanSequence_target.find( scanSequence_modulo ) != scanSequence_target.end() ) {
                const vector<unsigned long> &target = scanSequence_target[scanSequence_modulo];
                if ( find( target.begin(), target.end(), source->getId() ) != target.end() ) {
                    this_score *= 1e3;
                } else {
                    if ( target.size() == 1 ) {
                        this_score /= 1e3;
                    } else {
                        this_score /= 1e3;
                    }
                }
            }
        }
    }


    if ( source->getNTotalScans() > 0 && util::absDiff( times_.getScanTime( Timestamp::start ),
                                                        source->lastScanTime() ) < source->getPARA().minRepeat ) {
        double deltaTime = util::absDiff( times_.getScanTime( Timestamp::start ), source->lastScanTime() );
        double d = deltaTime / source->getPARA().minRepeat * 1e-3;

        this_score *= d;
    }
    double srcweight = source->getPARA().weight;
    if ( calib ) {
        srcweight = 1;
    }

    this_score *= srcweight * weight_stations( network.getStations() ) * weight_baselines( network.getBaselines() );

    return this_score;
}


void Scan::calcScore( const std::vector<double> &astas, const std::vector<double> &asrcs,
                      const std::vector<double> &abls, unsigned int minTime, unsigned int maxTime,
                      const Network &network, const std::shared_ptr<const AbstractSource> &source, bool subnetting,
                      const std::vector<double> &idleScore ) noexcept {
    double this_score =
        calcScore_firstPart( astas, asrcs, abls, minTime, maxTime, network, source, subnetting, idleScore );

    double weight_skyCoverage = WeightFactors::weightSkyCoverage;
    if ( weight_skyCoverage != 0 ) {
        this_score += network.calcScore_skyCoverage( pointingVectorsStart_ ) * weight_skyCoverage;
    }

    score_ = calcScore_secondPart( this_score, network, source );
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " score " << score_;
#endif
}


void Scan::calcScore( const std::vector<double> &astas, const std::vector<double> &asrcs,
                      const std::vector<double> &abls, unsigned int minTime, unsigned int maxTime,
                      const Network &network, const std::shared_ptr<const AbstractSource> &source,
                      unordered_map<unsigned long, double> &staids2skyCoverageScore,
                      const std::vector<double> &idleScore ) noexcept {
    double this_score = calcScore_firstPart( astas, asrcs, abls, minTime, maxTime, network, source, false, idleScore );

    double weight_skyCoverage = WeightFactors::weightSkyCoverage;
    if ( weight_skyCoverage != 0 ) {
        this_score +=
            network.calcScore_skyCoverage( pointingVectorsStart_, staids2skyCoverageScore ) * weight_skyCoverage;
    }

    score_ = calcScore_secondPart( this_score, network, source );
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " score " << score_;
#endif
}


void Scan::calcScore_subnetting( const std::vector<double> &astas, const std::vector<double> &asrcs,
                                 const std::vector<double> &abls, unsigned int minTime, unsigned int maxTime,
                                 const Network &network, const std::shared_ptr<const AbstractSource> &source,
                                 const unordered_map<unsigned long, double> &staids2skyCoverageScore,
                                 const std::vector<double> &idleScore ) noexcept {
    double this_score = calcScore_firstPart( astas, asrcs, abls, minTime, maxTime, network, source, true, idleScore );

    double weight_skyCoverage = WeightFactors::weightSkyCoverage;
    if ( weight_skyCoverage != 0 ) {
        this_score += network.calcScore_skyCoverage_subnetting( pointingVectorsStart_, staids2skyCoverageScore ) *
                      weight_skyCoverage;
    }

    score_ = calcScore_secondPart( this_score, network, source );
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " score " << score_;
#endif
}


void Scan::calcScore( unsigned int minTime, unsigned int maxTime, const Network &network,
                      const std::shared_ptr<const AbstractSource> &source, double hiscore, bool subnetting ) {
    double this_score = calcScore_firstPart( vector<double>(), vector<double>(), vector<double>(), minTime, maxTime,
                                             network, source, subnetting, vector<double>( network.getNSta(), 0 ) );

    score_ = calcScore_secondPart( this_score, network, source ) * hiscore;
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " score " << score_;
#endif
}


bool Scan::calcScore( const std::vector<double> &prevLowElevationScores,
                      const std::vector<double> &prevHighElevationScores, const Network &network,
                      unsigned int minRequiredTime, unsigned int maxRequiredTime,
                      const std::shared_ptr<const AbstractSource> &source, bool subnetting ) {
    double lowElevationSlopeStart = AstrometricCalibratorBlock::lowElevationStartWeight;
    double lowElevationSlopeEnd = AstrometricCalibratorBlock::lowElevationFullWeight;

    double highElevationSlopeStart = AstrometricCalibratorBlock::highElevationStartWeight;
    double highElevationSlopeEnd = AstrometricCalibratorBlock::highElevationFullWeight;

    double improvementLowElevation = 0;
    double improvementHighElevation = 0;

    unsigned long nMaxSta = network.getNSta();

    int i = 0;
    while ( i < nsta_ ) {
        const PointingVector &pv = pointingVectorsStart_[i];
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
        double deltaLowElScore = lowElScore - prevLowElevationScores[staid];
        if ( deltaLowElScore > 0 ) {
            improvementLowElevation += deltaLowElScore;
        }

        double highElScore;
        if ( el < highElevationSlopeStart ) {
            highElScore = 0;
        } else if ( el > highElevationSlopeEnd ) {
            highElScore = 1;
        } else {
            highElScore = ( el - highElevationSlopeStart ) / ( highElevationSlopeEnd - lowElevationSlopeStart );
        }
        double deltaHighElScore = highElScore - prevHighElevationScores[staid];
        if ( deltaHighElScore > 0 ) {
            improvementHighElevation += deltaHighElScore;
        }

        ++i;
    }

    double scoreDuration = calcScore_duration( nMaxSta, minRequiredTime, maxRequiredTime );

    double scoreBaselines = calcScore_numberOfObservations( network.getNBls() );

    double this_score = 0;
    if ( improvementHighElevation + improvementLowElevation > 0 ) {
        this_score =
            improvementLowElevation / nMaxSta + improvementHighElevation / nMaxSta + scoreDuration + scoreBaselines;

        score_ = calcScore_secondPart( this_score, network, source );

#ifdef VIESCHEDPP_LOG
        if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " score " << score_;
#endif
        return true;

    } else {
#ifdef VIESCHEDPP_LOG
        if ( Flags::logTrace )
            BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId()
                                       << " removed because no improvement to calibration";
#endif
        return false;
    }
}

void Scan::calcScoreCalibrator( const Network &network, const std::shared_ptr<const AbstractSource> &source,
                                const std::vector<double> &astas, const std::vector<double> &abls, double meanSNR,
                                unsigned int minRequiredTime, unsigned int maxRequiredTime ) {
    double scoreBaselines =
        calcScore_numberOfObservations( network.getNBls() ) * CalibratorBlock::numberOfObservations_factor;
    //                                + CalibratorBlock::numberOfObservations_offset;
    double asta = calcScore_averageStations( astas, network.getNBls() ) * CalibratorBlock::averageStations_factor;
    //                   + CalibratorBlock::averageStations_offset;
    double dur =
        calcScore_duration( network.getNSta(), minRequiredTime, maxRequiredTime ) * CalibratorBlock::duration_factor;
    //                 + CalibratorBlock::duration_offset;
    double abl = calcScore_averageBaselines( abls ) * CalibratorBlock::averageBaseline_factor;
    //                 + CalibratorBlock::averageBaseline_offset;

    double this_score = meanSNR * ( scoreBaselines + asta + dur + abl );
    score_ = calcScore_secondPart( this_score, network, source, true );

    if ( !CalibratorBlock::tryToIncludeAllStationFlag && nsta_ < network.getNSta() * 0.7 ) {
        score_ *= 0.1;
    }
}

void Scan::calcScoreDPar( const Network &network, const std::shared_ptr<const AbstractSource> &source,
                          vector<double> meanSNRs ) {
    auto parallacticAngle = []( const Station &sta, const shared_ptr<const AbstractSource> &src,
                                const PointingVector &pv, unsigned int startTime ) {
        double ha = pv.getHa();
        double dec = src->getRaDe( startTime, sta.getPosition() ).second;
        double lat = sta.getPosition()->getLat();
        double p = atan2( sin( ha ), cos( dec ) * tan( lat ) - sin( dec ) * cos( ha ) );
        return p;
    };

    double angle = DifferentialParallacticAngleBlock::angles[DifferentialParallacticAngleBlock::iScan];
    double thisScore = 0;
    int n = 0;
    for ( int iobs = 0; iobs < observations_.size(); ++iobs ) {
        const Observation &obs = observations_[iobs];
        if ( !DifferentialParallacticAngleBlock::isAllowedBaseline( obs.getBlid() ) ) {
            continue;
        }

        double snr = meanSNRs[iobs];
        const Station &sta1 = network.getStation( obs.getStaid1() );
        const Station &sta2 = network.getStation( obs.getStaid2() );
        const PointingVector &pv1 = getPointingVector( findIdxOfStationId( obs.getStaid1() ).get() );
        const PointingVector &pv2 = getPointingVector( findIdxOfStationId( obs.getStaid2() ).get() );
        unsigned int time = obs.getStartTime();

        double par1 = parallacticAngle( sta1, source, pv1, time );
        double par1_deg = par1 * rad2deg;
        double par2 = parallacticAngle( sta2, source, pv2, time );
        double par2_deg = par2 * rad2deg;
        double dpar = util::wrap2twoPi( par1 - par2 );
        double dpar_deg = dpar * rad2deg;

        //        cout << source->getName() << " dPar:" << dpar_deg << "[deg] SNR:" << snr << "\n";

        // score equals sine wave with peak at requested angle +/- n*90 degrees (n beeing an integer)
        double score =
            pow( 0.5 + 0.5 * cos( 4 * ( dpar - angle ) ), DifferentialParallacticAngleBlock::distanceScaling );
        thisScore += snr * score;
        ++n;
    }
    thisScore /= n;

    score_ = calcScore_secondPart( thisScore, network, source, true );
}


void Scan::calcScorePar( const Network &network, const std::shared_ptr<const AbstractSource> &source, double meanSNR ) {
    //    auto parallacticAngle = [](const Station &sta,
    //                               const shared_ptr<const AbstractSource> &src,
    //                               const PointingVector &pv,
    //                               unsigned int startTime){
    //
    //      double ha = pv.getHa();
    //      double dec = src->getRaDe(startTime, sta.getPosition()).second;
    //      double lat = sta.getPosition()->getLat();
    //      double p = atan2(sin(ha), cos(dec)*tan(lat)-sin(dec)*cos(ha));
    //      return p;
    //    };

    double maxEl = 0 * deg2rad;
    for ( const auto &pv : pointingVectorsStart_ ) {
        unsigned long staid = pv.getStaid();
        if ( !ParallacticAngleBlock::isAllowedStation( staid ) ) {
            continue;
        }
        double el = pv.getEl();
        //        const Station &sta = network.getStation(staid);
        //        unsigned int time = pv.getTime();
        //        double par = parallacticAngle(sta, source, pv, time);
        //        par = abs(par);
        //        if ( par < parMin ){
        //            parMin = par;
        //        }
        if ( el > maxEl ) {
            maxEl = el;
        }
    }
    //    double parMin_deg = parMin * rad2deg;
    //    double factor = pow(0.5 + 0.5 * cos(2*parMin), ParallacticAngleBlock::distanceScaling);
    double maxEl_deg = maxEl * rad2deg;
    double factor = pow( 0.5 + 0.5 * cos( pi + 2 * maxEl ), ParallacticAngleBlock::distanceScaling );

    double this_score = meanSNR * factor;
    score_ = calcScore_secondPart( this_score, network, source, true );
}


void Scan::output( unsigned long observed_scan_nr, const Network &network,
                   const std::shared_ptr<const AbstractSource> &source, ofstream &of ) const noexcept {
    string type = toString( type_ );
    string type2 = toString( constellation_ );

    string line1Right = ( boost::format( " duration: %8s - %8s" ) %
                          TimeSystem::time2timeOfDay( times_.getObservingTime( Timestamp::start ) ) %
                          TimeSystem::time2timeOfDay( times_.getObservingTime( Timestamp::end ) ) )
                            .str();

    if ( scanSequence_flag && type_ == ScanType::standard ) {
        if ( scanSequence_target.find( scanSequence_modulo ) != scanSequence_target.end() ) {
            const vector<unsigned long> &target = scanSequence_target[scanSequence_modulo];
            int n = target.size();
            of << boost::format( "| custom scan sequence %d with %d target scans\n" ) % scanSequence_modulo % n;
        }
    }

    if ( observed_scan_nr == numeric_limits<unsigned long>::max() ) {
        of << boost::format( "| a priori scan    %-25s                        %74s |\n" ) % printId() % line1Right;
    } else {
        of << boost::format( "| scan:   no%04d   %-25s                        %74s |\n" ) % observed_scan_nr %
                  printId() % line1Right;
    }

    string line2Right = ( boost::format( " type: %s %s (%.2f)" ) % type % type2 % score_ ).str();
    string line2Left = ( boost::format( "source: %8s %-15s" ) % source->getName() % source->printId() ).str();
    of << boost::format( "| %-69s %70s |\n" ) % line2Left % line2Right;

    of << "|-----------------------------------------------------------------------------------------------------------"
          "-----------------------------------|\n";
    if ( observed_scan_nr % 5 == 0 ) {
        of << "|     station  | delay |  slew |  idle | preob |  obs  |       duration      |        az [deg]     |    "
              "   unaz [deg]      |       el [deg]    |\n"
              "|              |  [s]  |  [s]  |  [s]  |  [s]  |  [s]  |    start - end      |    start - end      |    "
              " start - end       |   start - end     |\n"
              "|--------------|-------|-------|-------|-------|-------|---------------------|---------------------|----"
              "-------------------|-------------------|\n";
    }

    for ( int i = 0; i < nsta_; ++i ) {
        const PointingVector &pv = pointingVectorsStart_[i];
        const PointingVector &pve = pointingVectorsEnd_[i];
        const Station &thisSta = network.getStation( pv.getStaid() );
        double az_s = util::wrap2twoPi( pv.getAz() ) * rad2deg;
        double az_e = util::wrap2twoPi( pve.getAz() ) * rad2deg;

        of << boost::format(
                  "|     %-8s | %5d | %5d | %5d | %5d | %5d | %8s - %8s | %8.4f - %8.4f | %9.4f - %9.4f | %7.4f - "
                  "%7.4f | (id: %d and %d) \n" ) %
                  thisSta.getName() % times_.getFieldSystemDuration( i ) % times_.getSlewDuration( i ) %
                  times_.getIdleDuration( i ) % times_.getPreobDuration( i ) % times_.getObservingDuration( i ) %
                  TimeSystem::time2timeOfDay( times_.getObservingTime( i, Timestamp::start ) ) %
                  TimeSystem::time2timeOfDay( times_.getObservingTime( i, Timestamp::end ) ) % az_s % az_e %
                  ( pv.getAz() * rad2deg ) % ( pve.getAz() * rad2deg ) % ( pv.getEl() * rad2deg ) %
                  ( pve.getEl() * rad2deg ) % pv.getId() % pve.getId();
    }
    vector<string> ignoreBaseline;
    for ( int i = 0; i < nsta_; ++i ) {
        for ( int j = i + 1; j < nsta_; ++j ) {
            unsigned long staid1 = pointingVectorsStart_[i].getStaid();
            unsigned long staid2 = pointingVectorsStart_[j].getStaid();
            bool found = false;
            for ( const auto &any : observations_ ) {
                if ( any.containsStation( staid1 ) && any.containsStation( staid2 ) ) {
                    found = true;
                    break;
                }
            }
            if ( !found ) {
                ignoreBaseline.push_back( network.getBaseline( staid1, staid2 ).getName() );
            }
        }
    }
    if ( !ignoreBaseline.empty() ) {
        of << "| ignore observations: ";
        int i = 0;
        for ( ; i < ignoreBaseline.size(); ++i ) {
            if ( i % 20 == 0 && i > 0 ) {
                of << "|\n|                      ";
            }
            of << ignoreBaseline[i] << " ";
        }
        i = i % 15;
        if ( i > 0 ) {
            for ( ; i < 20; ++i ) {
                of << "      ";
            }
        }
        of << "|\n";
    }
    of << "|-----------------------------------------------------------------------------------------------------------"
          "-----------------------------------|\n";
}


boost::optional<Scan> Scan::copyScan( const std::vector<unsigned long> &ids,
                                      const std::shared_ptr<const AbstractSource> &source ) const noexcept {
    vector<PointingVector> pv;
    pv.reserve( ids.size() );
    ScanTimes t = times_;
    vector<Observation> obs;

    int counter = 0;
    // add all found pointing vectors to new pointing vector vector
    for ( auto &any : pointingVectorsStart_ ) {
        unsigned long id = any.getStaid();
        if ( find( ids.begin(), ids.end(), id ) != ids.end() ) {
            pv.push_back( pointingVectorsStart_[counter] );
        }
        ++counter;
    }
    // check if there are enough pointing vectors
    vector<unsigned long> staids;
    for ( const auto &p : pointingVectorsStart_ ) {
        staids.push_back( p.getStaid() );
    }
    if ( Network::stationIdsToNSites( staids ) < source->getPARA().minNumberOfSites ) {
        return boost::none;
    }

    // check if there are some required stations for this source
    if ( !source->getPARA().requiredStations.empty() ) {
        const vector<unsigned long> &rsta = source->getPARA().requiredStations;
        for ( auto &thisRequiredStationId : rsta ) {
            if ( find( ids.begin(), ids.end(), thisRequiredStationId ) == ids.end() ) {
                return boost::none;
            }
        }
    }

    // remove times of pointingVectorsStart_(original scan) which are not found (not part of pv)
    for ( auto i = static_cast<int>( nsta_ - 1 ); i >= 0; --i ) {
        unsigned long thisId = pointingVectorsStart_[i].getStaid();
        if ( find( ids.begin(), ids.end(), thisId ) == ids.end() ) {
            t.removeElement( i );
        }
    }

    // move all observations whose stations were found to bl
    for ( const auto &iobs : observations_ ) {
        const Observation &thisBl = iobs;
        unsigned long staid1 = thisBl.getStaid1();
        unsigned long staid2 = thisBl.getStaid2();

        if ( find( ids.begin(), ids.end(), staid1 ) != ids.end() &&
             find( ids.begin(), ids.end(), staid2 ) != ids.end() ) {
            obs.push_back( iobs );
        }
    }
    if ( obs.empty() ) {
        return boost::none;
    }

    return Scan( move( pv ), move( t ), move( obs ), type_ );
}


double Scan::weight_stations( const std::vector<Station> &stations ) {
    double weight = 1;
    for ( const auto &any : pointingVectorsStart_ ) {
        weight *= stations[any.getStaid()].getPARA().weight;
    }

    return weight;
}


double Scan::weight_baselines( const std::vector<Baseline> &baselines ) {
    double weight = 1;
    for ( const auto &any : observations_ ) {
        weight *= baselines[any.getBlid()].getParameters().weight;
    }

    return weight;
}


void Scan::setFixedScanDuration( unsigned int scanDuration ) noexcept { times_.setObservingTimes( scanDuration ); }


bool Scan::setScanTimes( const std::vector<unsigned int> &eols, const std::vector<unsigned int> &fieldSystemTime,
                         const std::vector<unsigned int> &slewTime, const std::vector<unsigned int> &preob,
                         unsigned int scanStart, const std::vector<unsigned int> &observingTimes ) {
    times_.setEndOfLastScan( eols );
    for ( int i = 0; i < slewTime.size(); ++i ) {
        times_.addTimes( i, fieldSystemTime[i], slewTime.at( static_cast<unsigned long>( i ) ), 0 );
    }
    times_.setObservingStarts( scanStart );
    bool valid = times_.setPreobTime( preob );
    times_.setObservingTimes( observingTimes, false );
    return valid;
}

bool Scan::setScanTimes( const vector<unsigned int> &eols, const vector<unsigned int> &fieldSystemTime,
                         const vector<unsigned int> &slewTime, const vector<unsigned int> &preob,
                         const vector<unsigned int> &scanStart, const vector<unsigned int> &observingTimes ) {
    times_.setEndOfLastScan( eols );
    for ( int i = 0; i < slewTime.size(); ++i ) {
        times_.addTimes( i, fieldSystemTime[i], slewTime.at( static_cast<unsigned long>( i ) ), 0 );
        times_.setObservingTime( i, observingTimes[i], Timestamp::end );
    }
    times_.setObservingStarts( scanStart );
    bool valid = times_.setPreobTime( preob );
    // times_.setObservingTimes(observingTimes);
    return valid;
}


void Scan::setPointingVectorsEndtime( vector<PointingVector> pv_end ) { pointingVectorsEnd_ = std::move( pv_end ); }


void Scan::createDummyObservations( const Network &network ) {
    for ( int i = 0; i < nsta_; ++i ) {
        unsigned long staid1 = pointingVectorsStart_[i].getStaid();
        unsigned int dur1 = times_.getObservingDuration( i );
        for ( int j = i + 1; j < nsta_; ++j ) {
            unsigned long staid2 = pointingVectorsStart_[j].getStaid();
            unsigned int dur2 = times_.getObservingDuration( j );

            unsigned int dur = std::min( dur1, dur2 );
            unsigned long blid = network.getBlid( staid1, staid2 );
#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace )
                BOOST_LOG_TRIVIAL( trace ) << "scan " << this->printId() << " create observation between stations "
                                           << staid1 << " and " << staid2;
#endif

            Observation obs( blid, staid1, staid2, srcid_, times_.getObservingTime( Timestamp::start ), dur );
            observations_.push_back( std::move( obs ) );
        }
    }
}


unsigned long Scan::getNObs( unsigned long staid ) const noexcept {
    unsigned long n = 0;
    for ( const auto &any : observations_ ) {
        if ( any.containsStation( staid ) ) {
            ++n;
        }
    }
    return n;
}


void Scan::setPointingVector( int idx, PointingVector pv, Timestamp ts ) {
    times_.setObservingTime( idx, pv.getTime(), ts );
    switch ( ts ) {
        case Timestamp::start: {
            pointingVectorsStart_[idx] = move( pv );
            break;
        }
        case Timestamp::end: {
            pointingVectorsEnd_[idx] = move( pv );
            break;
        }
        default:
            break;
    }
}


void Scan::removeUnnecessaryObservingTime( Network &network, const std::shared_ptr<const AbstractSource> &thisSource,
                                           std::ofstream &of, Timestamp ts ) {
    int idx = times_.removeUnnecessaryObservingTime( ts );
    unsigned int t = times_.getObservingTime( idx, ts );
    PointingVector &pv = referencePointingVector( idx, ts );
    double az = pv.getAz();
    pv.setTime( t );
    unsigned long staid = pv.getStaid();
    Station &thisSta = network.refStation( staid );
    thisSta.calcAzEl_rigorous( thisSource, pv );
    thisSta.getCableWrap().unwrapAzNearAz( pv, az );
    bool visible = thisSta.isVisible( pv, thisSource->getPARA().minElevation );
    if ( !visible ) {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( error ) << "scan " << this->printId() << " source " << thisSource->getName()
                                   << " might not be visible from " << thisSta.getName();
#else
        cout << "[error] scan " << this->printId() << " source " << thisSource->getName()
             << " might not be visible from " << thisSta.getName();
#endif

        of << ( boost::format( "ERROR while extending observing time to idle time:\n    "
                               "source %s might not be visible from %s during %s. " ) %
                thisSource->getName() % thisSta.getName() % TimeSystem::time2timeOfDay( t ) )
                  .str();
    }
}


void Scan::removeAdditionalObservingTime( unsigned int time, const Station &thisSta,
                                          const std::shared_ptr<const AbstractSource> &thisSource, std::ofstream &of,
                                          Timestamp ts ) {
    unsigned long staid = thisSta.getId();
    auto oidx = findIdxOfStationId( staid );

    if ( oidx.is_initialized() ) {
        auto idx = static_cast<int>( oidx.get() );

        bool reduced = times_.reduceObservingTime( idx, time, ts );

        if ( reduced ) {
            unsigned int t = times_.getObservingTime( idx, ts );
            PointingVector &pv = referencePointingVector( idx, ts );
            double az = pv.getAz();
            pv.setTime( t );
            thisSta.calcAzEl_simple( thisSource, pv );
            thisSta.getCableWrap().unwrapAzNearAz( pv, az );

            bool visible = thisSta.isVisible( pv, thisSource->getPARA().minElevation );

            if ( !visible ) {
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( error ) << "scan " << this->printId() << " source " << thisSource->getName()
                                           << " might not be visible from " << thisSta.getName();
#else
                cout << "[error] scan " << this->printId() << " source " << thisSource->getName()
                     << " might not be visible from " << thisSta.getName();
#endif

                of << ( boost::format( "ERROR while extending observing time to idle time:\n    "
                                       "source %s might not be visible from %s during %s. " ) %
                        thisSource->getName() % thisSta.getName() % TimeSystem::time2timeOfDay( t ) )
                          .str();
            }
        }
    }
}


void Scan::updateObservingTime() {
    for ( auto &obs : observations_ ) {
        int idx1 = *findIdxOfStationId( obs.getStaid1() );
        int idx2 = *findIdxOfStationId( obs.getStaid2() );
        unsigned int dur = times_.getObservingDuration( idx1, idx2 );
        obs.setObservingTime( dur );
        unsigned int start = times_.getObservingTime( idx1, idx2, VieVS::Timestamp::start );
        obs.setStartTime( start );
    }
}


bool Scan::prepareForScanEnd( Network &network, const shared_ptr<const AbstractSource> &source,
                              const std::shared_ptr<const Mode> &mode, unsigned int endTime ) {
    int ista = 0;
    while ( ista < nsta_ ) {
        if ( times_.getObservingTime( ista, Timestamp::end ) > endTime ) {
            bool valid = removeStation( ista, source );
            if ( !valid ) {
                return false;
            }
        } else {
            ++ista;
        }
    }

    if ( !rigorousUpdate( network, source, mode ) ) {
        return false;
    }

    while ( ista < nsta_ ) {
        if ( times_.getObservingTime( ista, Timestamp::end ) > endTime ) {
            if ( !removeStation( ista, source ) ) {
                return false;
            }
        } else {
            ++ista;
        }
    }

    return true;
}


string Scan::getName( unsigned long indexOfThisScanInList, const std::vector<Scan> &otherScans ) const {
    unsigned int start = times_.getObservingTime( Timestamp::start );
    boost::posix_time::ptime scanStart = TimeSystem::internalTime2PosixTime( start );
    int doy = scanStart.date().day_of_year();
    auto hour = static_cast<int>( scanStart.time_of_day().hours() );
    auto min = static_cast<int>( scanStart.time_of_day().minutes() );
    string scanId = ( boost::format( "%03d-%02d%02d" ) % doy % hour % min ).str();

    int suffix = 0;
    if ( indexOfThisScanInList > 0 ) {
        for ( long i = indexOfThisScanInList - 1; i >= 0; --i ) {
            unsigned int ostart = otherScans[i].getTimes().getObservingTime( Timestamp::start );
            boost::posix_time::ptime oScanStart = TimeSystem::internalTime2PosixTime( ostart );
            int odoy = oScanStart.date().day_of_year();
            auto ohour = static_cast<int>( oScanStart.time_of_day().hours() );
            auto omin = static_cast<int>( oScanStart.time_of_day().minutes() );

            if ( odoy == doy && ohour == hour && omin == min ) {
                ++suffix;
            } else {
                break;
            }
        }
        if ( suffix > 0 ) {
            char csuffix = static_cast<char>( 'a' + suffix );
            scanId += csuffix;
        }
    }

    if ( suffix == 0 && indexOfThisScanInList + 1 < otherScans.size() ) {
        unsigned int ostart = otherScans[indexOfThisScanInList + 1].getTimes().getObservingTime( Timestamp::start );
        boost::posix_time::ptime oScanStart = TimeSystem::internalTime2PosixTime( ostart );
        int odoy = oScanStart.date().day_of_year();
        auto ohour = static_cast<int>( oScanStart.time_of_day().hours() );
        auto omin = static_cast<int>( oScanStart.time_of_day().minutes() );

        if ( odoy == doy && ohour == hour && omin == min ) {
            scanId += 'a';
        }
    }

    return scanId;
}


unsigned long Scan::indexOfObservation( unsigned long staid1, unsigned long staid2 ) const {
    for ( unsigned long idx = 0; idx < observations_.size(); ++idx ) {
        const auto &any = observations_[idx];
        if ( any.containsStation( staid1 ) && any.containsStation( staid2 ) ) {
            return idx;
        }
    }

    return -1;
}

Observation &Scan::refObservation( unsigned long staid1, unsigned long staid2 ) {
    for ( auto &any : observations_ ) {
        if ( any.containsStation( staid1 ) && any.containsStation( staid2 ) ) {
            return any;
        }
    }
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(fatal)
        << "This code should never be reached! Scan::refObservation( unsigned long staid1, unsigned long staid2 )";
#else
    cout << "This code should never be reached! Scan::refObservation( unsigned long staid1, unsigned long staid2 )";
#endif
    terminate();
}


std::string Scan::toSkedOutputTimes( const std::shared_ptr<const AbstractSource> &source,
                                     unsigned long nMaxSta ) const {
    unsigned int time = times_.getObservingTime( Timestamp::start );
    string out = ( boost::format( " %-8s %s|" ) % source->getName() % TimeSystem::time2string_doy_minus( time ) ).str();

    for ( int staid = 0; staid < nMaxSta; ++staid ) {
        const auto &idx = findIdxOfStationId( staid );
        if ( idx.is_initialized() ) {
            out.append( ( boost::format( "%4d" ) % times_.getObservingDuration( *idx ) ).str() );
        } else {
            out.append( "    " );
        }
    }
    out.append( "\n" );
    return out;
}


void Scan::includesStations( std::vector<char> &flag ) const {
    for ( const auto &pv : pointingVectorsStart_ ) {
        unsigned long staid = pv.getStaid();
        flag[staid] = true;
    }
}


boost::property_tree::ptree Scan::toPropertyTree( const Network &network, const string &sourceName ) const {
    boost::property_tree::ptree ptree;
    ptree.add( "scan.source", sourceName );
    for ( int i = 0; i < nsta_; ++i ) {
        boost::property_tree::ptree sta_tree;
        string staName = network.getStation( getStationId( i ) ).getName();
        sta_tree.add( "station.<xmlattr>.name", staName );
        sta_tree.add( "station.obs_start", TimeSystem::time2string( times_.getObservingTime( i ) ) );
        sta_tree.add( "station.obs_end", TimeSystem::time2string( times_.getObservingTime( i, Timestamp::end ) ) );
        sta_tree.add( "station.system_delay", times_.getFieldSystemDuration( i ) );
        sta_tree.add( "station.preob", times_.getPreobDuration( i ) );
        ptree.add_child( "scan.station", sta_tree.get_child( "station" ) );
    }

    return ptree;
}

bool Scan::noInterception( const vector<Scan> &scans, const Network &network ) {
    for ( int idx = 0; idx < nsta_; ++idx ) {
        unsigned long staid = pointingVectorsStart_[idx].getStaid();
        const Station &thisStation = network.getStation( staid );
        unsigned int constantTimes = thisStation.getPARA().preob + thisStation.getPARA().systemDelay;

        // store current pointing vector and time
        const PointingVector &start = pointingVectorsStart_[idx];
        const PointingVector &end = pointingVectorsEnd_[idx];
        unsigned int startTime = times_.getObservingTime( idx );
        unsigned int endTime = times_.getObservingTime( idx, Timestamp::end );
        unsigned int observingDuration = times_.getObservingDuration( idx );

        // store previous and following pointing vector and time
        PointingVector before = PointingVector( staid, -1 );
        PointingVector after = PointingVector( staid, -1 );
        unsigned int observingDurationBefore = 0;
        unsigned int deltaTime_before = TimeSystem::duration;
        unsigned int deltaTime_after = TimeSystem::duration;

        // loop over every scan to find previous and following pointing vector (and time)
        for ( const auto &scan : scans ) {
            auto oidx = scan.findIdxOfStationId( staid );
            if ( !oidx.is_initialized() ) {
                continue;
            }

            // extract relevant times and pointing vectors
            const PointingVector &thisStart = scan.getPointingVector( *oidx, Timestamp::start );
            const PointingVector &thisEnd = scan.getPointingVector( *oidx, Timestamp::end );
            unsigned int thisStartTime = scan.getTimes().getObservingTime( *oidx, Timestamp::start );
            unsigned int thisEndTime = scan.getTimes().getObservingTime( *oidx, Timestamp::end );

            // check if it is closer to current pointing vector and time
            if ( thisStartTime >= endTime && thisStartTime - endTime < deltaTime_after ) {
                deltaTime_after = thisStartTime - endTime;
                after = thisStart;
            }
            if ( thisEndTime <= startTime && startTime - thisEndTime < deltaTime_before ) {
                deltaTime_before = startTime - thisEndTime;
                before = thisEnd;
                observingDurationBefore = scan.getTimes().getObservingDuration( *oidx );
            }
        }

        if ( deltaTime_before != TimeSystem::duration ) {
            // check slew time
            auto oslewtime = thisStation.slewTime( before, start, observingDurationBefore, true );
            unsigned int slewtime;
            if ( oslewtime.is_initialized() ) {
                slewtime = *oslewtime;
            } else {
                slewtime = TimeSystem::duration;
            }
            unsigned int min_neededTime = slewtime + constantTimes;
            if ( min_neededTime > deltaTime_before ) {
                return false;
            }
        }
        if ( deltaTime_after != TimeSystem::duration ) {
            // check slew time
            auto oslewtime = thisStation.slewTime( end, after, observingDuration, true );
            unsigned int slewtime;
            if ( oslewtime.is_initialized() ) {
                slewtime = *oslewtime;
            } else {
                slewtime = TimeSystem::duration;
            }
            unsigned int min_neededTime = slewtime + constantTimes;
            if ( min_neededTime > deltaTime_after ) {
                return false;
            }
        }
    }
    return true;
}
