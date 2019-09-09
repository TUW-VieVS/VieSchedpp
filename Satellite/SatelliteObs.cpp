//
// Created by hwolf on 2/13/19.
//

#include "SatelliteObs.h"

using namespace std;
using namespace VieVS;
unsigned long SatelliteObs::nextId = 0;

SatelliteObs::SatelliteObs() : start( 0 ), end( 0 ), VieVS_NamedObject::VieVS_NamedObject( "SatObs", "", nextId++ ) {}

SatelliteObs::SatelliteObs( DateTime startTime, DateTime endTime )
    : start( startTime ), end( endTime ), VieVS_NamedObject::VieVS_NamedObject( "SatObs", "", nextId++ ) {}

SatelliteObs::~SatelliteObs() {}

const unsigned long SatelliteObs::getNumberofStations() const noexcept { return this->StationIDList.size(); }

const std::vector<unsigned long> SatelliteObs::getStationIDList() const noexcept { return this->StationIDList; }

const DateTime SatelliteObs::getStart() const noexcept { return this->start; }

const DateTime SatelliteObs::getEnd() const noexcept { return this->end; }

std::vector<SatelliteObs> SatelliteObs::passList2Overlap( std::vector<std::vector<Satellite::SatPass>> PassList ) {
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "start creating Overlaps";
#else
    cout << "[info] start creating Overlaps";
#endif

    std::vector<TimePoint> TimePoints_;
    TimePoints_ = createSortedTimePoints( PassList );

    std::vector<SatelliteObs> overlap_list;
    SatelliteObs overlap = SatelliteObs();
    bool signal = false;

    // find Overlaps (compare time -> check if timepoints are overlapping each other and check timestamp (if it is a
    // start or end of observation)
    // if it is a start the station will be added to the list, if it is an end the station will be removed from the list

    // for(unsigned long i=0; i<TimePoints_.size();i++)
    for ( const auto &any : TimePoints_ ) {
        if ( !signal ) {
            overlap.start = any.time;
            overlap.StationIDList.push_back( any.StationID );
            overlap.SatelliteID = any.SatelliteID;
            signal = true;
            continue;
        } else  // if there is already signal (at least one station is already observing)
        {
            switch ( any.ts ) {
                case Timestamp::start:  // another station starts to observe
                {
                    if ( overlap.start == any.time )  // the satellite start to be visible for 2 stations at the same
                    {
                        overlap.StationIDList.push_back( any.StationID );
                        continue;
                    } else  // another stations starts to observe -> end last overlap and start a new overlap (set
                            // start, add station to  IDList)
                    {
                        overlap.end = any.time;
                        overlap_list.push_back( overlap );
                        overlap.start = any.time;
                        overlap.StationIDList.push_back( any.StationID );
                        continue;
                    }
                }
                case Timestamp::end:  // a station stops to observe
                {
                    if ( overlap.end == any.time )  // two stations stop to observe at the same time
                    {
                        removeStationID( overlap.StationIDList, any.StationID );
                        continue;
                    } else {
                        overlap.end = any.time;
                        overlap_list.push_back( overlap );
                        removeStationID( overlap.StationIDList, any.StationID );
                        if ( overlap.StationIDList.empty() ) {
                            signal = false;  // if any station is no longer observing
                        } else {
                            overlap.start = any.time;
                        }
                        continue;
                    }
                }
            }
        }
    }
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "finish creating Overlaps";
#else
    cout << "[info] finish creating Overlaps";
#endif
    return overlap_list;
}


bool SatelliteObs::compareTimePoint( TimePoint i1, TimePoint i2 ) { return ( i1.time < i2.time ); }

void SatelliteObs::removeStationID( std::vector<unsigned long> &StationIDs, unsigned long reStation ) {
    auto position = std::find( StationIDs.begin(), StationIDs.end(), reStation );
    if ( position != StationIDs.end() )  // == StationIDs.end() means the element was not found
        StationIDs.erase( position );
}


std::vector<std::vector<std::vector<VieVS::PointingVector>>> SatelliteObs::PassList2pv(
    std::vector<std::vector<Satellite::SatPass>> PassList_, Satellite sat, VieVS::Network network,
    DateTime SessionStartTime ) {
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "start creating PointingVectors";
#else
    cout << "[info] start creating PointingVectors";
#endif
    std::vector<VieVS::PointingVector> pvVec;
    std::vector<std::vector<VieVS::PointingVector>> pvList;
    std::vector<std::vector<std::vector<VieVS::PointingVector>>> pvRes;
    std::vector<VieVS::Station> stations = network.getStations();
    SGP4 *sgp4 = sat.getSGP4Data();
    for ( unsigned long i = 0; i < PassList_.size(); i++ )  // i Station
    {
        pvList.clear();
        for ( int j = 0; j < PassList_.at( i ).size(); j++ ) {
            pvVec.clear();
            VieVS::PointingVector pv = VieVS::PointingVector( stations.at( i ).getId(), sat.getId() );
            Observer obs( stations.at( i ).getPosition().getLat() * 180 / pi,
                          stations.at( i ).getPosition().getLon() * 180 / pi,
                          stations.at( i ).getPosition().getHeight() / 1000 );

            DateTime start_time = ( ceil( ( PassList_.at( i ).at( j ).start.Ticks() ) / pow( 10, 6 ) ) * pow( 10, 6 ) );
            DateTime end_time = ( floor( ( PassList_.at( i ).at( j ).end.Ticks() ) / pow( 10, 6 ) ) * pow( 10, 6 ) );
            DateTime current_time( start_time );

            while ( current_time < end_time + TimeSpan( 0, 1, 0 ) ) {
                if ( current_time > end_time ) {
                    current_time = end_time;
                }
                Eci eci = sgp4->FindPosition( current_time );
                CoordTopocentric topo = obs.GetLookAngle( eci );
                pv.setAz( topo.azimuth );
                pv.setEl( topo.elevation );

                SatelliteObs::calcRaDeHa( current_time, SessionStartTime, eci, obs, &pv );

                pvVec.push_back( pv );
                current_time = current_time + TimeSpan( 0, 10, 0 );
            }
            pvList.push_back( pvVec );
        }
        pvRes.push_back( pvList );
    }
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "finish creating PointingVectors";
#else
    cout << "[info] finish creating PointingVectors";
#endif
    return pvRes;
}


void SatelliteObs::createOutput( std::vector<std::vector<Satellite::SatPass>> PassList_,
                                 std::vector<SatelliteObs> overlap, Network network ) {
    vector<VieVS::Station> all_stations = network.getStations();
    unsigned long nSta = all_stations.size();
    vector<vector<char>> StationBoolList(
        overlap.size(),
        vector<char>( all_stations.size(),
                      false ) );  // create 2D boolean vector with size of overlaps and number of stations
    vector<long> StationCount( overlap.size(), 0 );

    vector<TimePoint> TimePoints_;
    TimePoints_ = createSortedTimePoints( PassList_ );

    for ( unsigned long i = 0; i < overlap.size(); i++ )  // add value true at position of id of station
    {
        for ( const unsigned long &j : overlap.at( i ).StationIDList ) {
            StationBoolList.at( i ).at( j ) = true;
        }
        StationCount[i] = std::count( StationBoolList.at( i ).begin(), StationBoolList.at( i ).end(), true );
    }

    DateTime last;
    for ( unsigned long i = 0; i < TimePoints_.size(); i++ ) {
        if ( i == 0 ) {
            std::cout << boost::format( "  %2f:%2f:%2f " ) % TimePoints_.at( i ).time.Hour() %
                             TimePoints_.at( i ).time.Minute() % TimePoints_.at( i ).time.Second();
            last = TimePoints_.at( i ).time;
            continue;
        } else if ( TimePoints_.at( i ).time == last ) {
            last = TimePoints_.at( i ).time;
            continue;
        } else {
            std::cout << boost::format( "  %2f:%2f:%2f " ) % TimePoints_.at( i ).time.Hour() %
                             TimePoints_.at( i ).time.Minute() % TimePoints_.at( i ).time.Second();
            last = TimePoints_.at( i ).time;
            continue;
        }
    }
    std::cout << std::endl;
    for ( unsigned long j = 0; j < nSta; j++ ) {
        for ( const auto &StationBool : StationBoolList ) {
            if ( StationBool.at( j ) != 0 ) {
                std::cout << "     " << 1 << "     ";
            } else {
                std::cout << "     " << 0 << "     ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

void SatelliteObs::calcObservations( Scan *scan, Network &network, Satellite sat, vector<PointingVector> PV_start ) {
    vector<Observation> obs = scan->getObservations();
    obs.clear();
    // bool valid = false;
    unsigned long srcid = sat.getId();

    // loop over all pointingVectors
    for ( unsigned long i = 0; i < PV_start.size(); ++i ) {
        for ( unsigned long j = i + 1; j < PV_start.size(); ++j ) {
            unsigned long staid1 = PV_start[i].getStaid();
            unsigned long staid2 = PV_start[j].getStaid();
            const Baseline &bl = network.getBaseline( staid1, staid2 );
            unsigned long blid = bl.getId();

            // add new baseline
            unsigned int startTime = max( {scan->getTimes().getObservingTime( i, Timestamp::start ),
                                           scan->getTimes().getObservingTime( j, Timestamp::start )} );

            obs.emplace_back( blid, staid1, staid2, srcid, startTime );

            unsigned int observingTime = scan->referenceTime().getObservingDuration( i, j );
            obs[obs.size() - 1].setObservingTime( observingTime );
            // valid = true;
        }
    }
    scan->setObservations( obs );
}

VieVS::PointingVector SatelliteObs::createPV( VieVS::Station station, Satellite sat, DateTime sessionStartTime,
                                              TimePoint TimePoint ) {
    Observer obs( station.getPosition().getLat() * 180 / pi, station.getPosition().getLon() * 180 / pi,
                  station.getPosition().getHeight() / 1000 );
    PointingVector pv( station.getId(), sat.getId() );
    DateTime time;
    switch ( TimePoint.ts ) {
        case Timestamp::start: {
            time = ( ceil( ( TimePoint.time.Ticks() ) / pow( 10, 6 ) ) * pow( 10, 6 ) );
            break;
        }
        case Timestamp::end: {
            time = ( floor( ( TimePoint.time.Ticks() ) / pow( 10, 6 ) ) * pow( 10, 6 ) );
            break;
        }
    }
    Eci eci = sat.getSGP4Data()->FindPosition( time );
    CoordTopocentric topo = obs.GetLookAngle( eci );
    pv.setAz( topo.azimuth );
    pv.setEl( topo.elevation );

    SatelliteObs::calcRaDeHa( time, sessionStartTime, eci, obs, &pv );
    return pv;
}

/* This function calculates the ra and dec and ha for the satellite to the given current time and sets the variables in
 * the pointing vector
 * */
void SatelliteObs::calcRaDeHa( DateTime current_time, DateTime start_time, Eci eci, Observer obs, PointingVector *pv ) {
    Eci stat( current_time, obs.GetLocation() );

    // difference vector between station and satellite
    Vector x_sat = eci.Position();
    Vector x_stat = stat.Position();
    Vector xd = x_sat - x_stat;

    // calculation of right ascension and declination for satellite
    double r = sqrt( xd.x * xd.x + xd.y * xd.y + xd.z * xd.z );
    double dec = asin( xd.z / r );
    double ra;
    if ( eci.Position().y / r > 0 ) {
        ra = acos( xd.x / r * 1 / cos( dec ) );
    } else {
        ra = 2 * pi - acos( xd.x / r * 1 / cos( dec ) );
    }

    pv->setTime(
        round( ( ( current_time.ToJ2000() - start_time.ToJ2000() ) * 86400 ) ) );  // seconds since session start
    double mjd = current_time.ToJulian() - 2400000.5;
    double gmst = VieVS::TimeSystem::mjd2gmst( mjd );
    double ha = gmst + obs.GetLocation().longitude - ra;
    while ( ha > pi ) {
        ha = ha - twopi;
    }
    while ( ha < -pi ) {
        ha = ha + twopi;
    }
    pv->setHa( ha );
    pv->setDc( dec );
}

bool SatelliteObs::comparePV( PointingVector pv1, PointingVector pv2 ) { return ( pv1.getStaid() < pv2.getStaid() ); }

std::vector<SatelliteObs::TimePoint> SatelliteObs::createSortedTimePoints(
    std::vector<std::vector<Satellite::SatPass>> &PassList ) {
    std::vector<TimePoint> TimePoints_;

    for ( const auto &pass : PassList ) {
        for ( const auto &any : pass ) {
            struct TimePoint tStart;
            tStart.time = any.start;
            tStart.ts = Timestamp::start;
            tStart.StationID = any.StationID;
            tStart.SatelliteID = any.SatelliteID;
            TimePoints_.push_back( tStart );

            struct TimePoint tEnd;
            tEnd.time = any.end;
            tEnd.ts = Timestamp::end;
            tEnd.StationID = any.StationID;
            tEnd.SatelliteID = any.SatelliteID;
            TimePoints_.push_back( tEnd );
        }
    }
    // sort time points by time (start and end times)
    std::sort( TimePoints_.begin(), TimePoints_.end(), compareTimePoint );
    return TimePoints_;
}


/*This function creates the List of possible scans. it converts the passlist array into an array of the type timepoint.
 * Then it is checked if the observation windows of  the stations are overlapping.
 * there is an vector ignoreNow which has a size of number of the stations and includes for each stations how many
 * observations windows should be ignored by running through the timepoints. if a station stops observing and would
 * start again during a scan, a 1 will be written in the vector ignoreNext, in the end (if ignoreNext isn't empty)
 * ignoreNext and ignoreNow will be added, so the number of observation windows to be ignored will increase.*/

std::vector<Scan> SatelliteObs::createScanList( std::vector<std::vector<Satellite::SatPass>> PassList, Network network,
                                                Satellite sat, DateTime sessionStartTime ) {
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "start creating Scanlist";
#else
    cout << "[info] start creating Scanlist";
#endif
    std::vector<Scan> scan_list;
    std::vector<TimePoint> TimePoints_;
    unsigned long nSta = network.getNSta();
    std::vector<Station> stations = network.getStations();
    TimePoints_ = createSortedTimePoints( PassList );

    std::vector<bool> isobs( nSta, false );
    std::vector<int> ignoreNow( nSta, 0 );
    std::vector<int> ignoreNext( nSta, 0 );
    std::vector<int> isDone( nSta, 0 );
    std::vector<int> ignoreNowCOPY( nSta, 0 );
    vector<PointingVector> pointingVectorsStart;
    vector<PointingVector> pointingVectorsEnd;
    bool scan = false;

    unsigned long i = 0;
    while ( i < TimePoints_.size() ) {
        if ( ignoreNow.at( TimePoints_.at( i ).StationID ) == 0 ) {
            if ( isDone.at( TimePoints_.at( i ).StationID ) == 0 ) {
                if ( !scan ) {
                    switch ( TimePoints_.at( i ).ts ) {
                        case Timestamp::start: {
                            auto itr = std::find( isobs.begin(), isobs.end(), true );
                            long idx = std::distance( isobs.begin(), itr );
                            isobs[TimePoints_.at( i ).StationID] = true;
                            if ( std::count( isobs.begin(), isobs.end(), true ) > 1 )  // start scan
                            {
                                PointingVector pvStart( createPV( stations.at( TimePoints_.at( i ).StationID ), sat,
                                                                  sessionStartTime, TimePoints_.at( i ) ) );
                                PointingVector pvStartPast(
                                    createPV( stations.at( idx ), sat, sessionStartTime, TimePoints_.at( i ) ) );
                                pointingVectorsStart.push_back( pvStart );
                                pointingVectorsStart.push_back( pvStartPast );
                                scan = true;
                                i++;
                                continue;
                            } else {
                                i++;
                                continue;
                            }
                        }
                        case Timestamp::end:  // stations stops again to observe
                        {
                            isobs[TimePoints_.at( i ).StationID] = false;
                            i++;
                            continue;
                        }
                    }
                } else {
                    switch ( TimePoints_.at( i ).ts ) {
                        case Timestamp::start: {
                            isobs[TimePoints_.at( i ).StationID] = true;
                            PointingVector pvStart( createPV( stations.at( TimePoints_.at( i ).StationID ), sat,
                                                              sessionStartTime, TimePoints_.at( i ) ) );
                            pointingVectorsStart.push_back( pvStart );
                            i++;
                            continue;
                        }
                        case Timestamp::end: {
                            isobs[TimePoints_.at( i ).StationID] = false;
                            isDone.at( TimePoints_.at( i ).StationID ) = 1;
                            PointingVector pvEnd( createPV( stations.at( TimePoints_.at( i ).StationID ), sat,
                                                            sessionStartTime, TimePoints_.at( i ) ) );
                            pointingVectorsEnd.push_back( pvEnd );
                            if ( std::count( isobs.begin(), isobs.end(), true ) < 2 )  // finish this scan
                            {
                                auto itr = std::find( isobs.begin(), isobs.end(), true );
                                long idx = std::distance( isobs.begin(), itr );
                                PointingVector pvEnd(
                                    createPV( stations[idx], sat, sessionStartTime, TimePoints_.at( i ) ) );
                                pointingVectorsEnd.push_back( pvEnd );
                                std::sort( pointingVectorsStart.begin(), pointingVectorsStart.end(), comparePV );
                                std::sort( pointingVectorsEnd.begin(), pointingVectorsEnd.end(), comparePV );
                                unsigned long nsta = pointingVectorsStart.size();
                                vector<unsigned int> endOfObservingTime_( nsta, 0 );
                                vector<unsigned int> endOfLastScans( nsta, 0 );
                                vector<unsigned int> fieldSystemTimes( nsta, 0 );
                                vector<unsigned int> slewTimes( nsta, 0 );
                                vector<unsigned int> preobTimes( nsta, 0 );
                                vector<unsigned int> pvStartTimes( nsta, 0 );
                                vector<PointingVector> pvstartcopy( pointingVectorsStart );

                                for ( unsigned long k = 0; k < nsta; k++ ) {
                                    endOfObservingTime_.at( k ) = pointingVectorsEnd.at( k ).getTime();
                                    pvStartTimes.at( k ) = pointingVectorsStart.at( k ).getTime();
                                    fieldSystemTimes.at( k ) = network.getStation( k ).getWaittimes().fieldSystem;
                                    preobTimes.at( k ) = network.getStation( k ).getWaittimes().preob;
                                    if ( pvStartTimes.at( k ) < ( fieldSystemTimes.at( k ) + preobTimes.at( k ) ) ) {
                                        endOfLastScans.at( k ) = pvStartTimes.at( k );
                                        fieldSystemTimes.at( k ) = 0;
                                        preobTimes.at( k ) = 0;
                                    } else {
                                        endOfLastScans.at( k ) = pointingVectorsStart.at( k ).getTime() -
                                                                 fieldSystemTimes.at( k ) - preobTimes.at( k );
                                    }
                                }
                                Scan Scan( pointingVectorsStart, endOfLastScans, Scan::ScanType::standard );
                                Scan.setScanTimes( endOfLastScans, fieldSystemTimes, slewTimes, preobTimes,
                                                   pvStartTimes, endOfObservingTime_ );
                                Scan.setPointingVectorsEndtime( pointingVectorsEnd );
                                calcObservations( &Scan, network, sat, pvstartcopy );
                                scan_list.push_back( Scan );
                                scan = false;
                                std::vector<int> addThis;
                                if ( std::count( ignoreNext.begin(), ignoreNext.end(), true ) == 0 ) {
                                    if ( i == TimePoints_.size() - 1 ) {
                                        break;
                                    } else {
                                        addThis = isDone;
                                    }
                                } else {
                                    addThis = ignoreNext;
                                }
                                for ( unsigned long j = 0; j < ignoreNow.size(); j++ ) {
                                    ignoreNow.at( j ) = ignoreNowCOPY.at( j ) + addThis.at( j );
                                }
                                i = 0;
                                isobs.assign( nSta, false );
                                isDone.assign( nSta, 0 );
                                ignoreNext.assign( nSta, 0 );
                                ignoreNowCOPY = ignoreNow;
                                pointingVectorsStart.clear();
                                pointingVectorsEnd.clear();
                                i++;
                                continue;
                            } else {
                                i++;
                                continue;
                            }
                        }
                    }
                }
            } else {
                ignoreNext.at( TimePoints_.at( i ).StationID ) =
                    1;  // one more observation window from this station will be ignored in the next run
                i++;
            }
        } else {
            switch ( TimePoints_.at( i ).ts ) {
                case Timestamp::start: {
                    i++;
                    continue;
                }
                case Timestamp::end: {
                    ignoreNow.at(
                        TimePoints_.at( i ).StationID )--;  // one observation window was ignored, so reduce number by 1
                    i++;
                }
            }
        }
    }
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "finish creating Scanlist";
#else
    cout << "[info] finish creating Scanlist";
#endif
    return scan_list;
}