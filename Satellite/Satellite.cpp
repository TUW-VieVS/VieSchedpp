//
// Created by hwolf on 2/7/19.
//

#include "Satellite.h"

using namespace std;
using namespace VieVS;
unsigned long Satellite::nextId = 0;

Satellite::Satellite()
    : header( "" ), line1( "" ), line2( "" ), VieVS_NamedObject::VieVS_NamedObject( "", "", nextId++ ) {
    pTleData = NULL;
    pSGP4Data = NULL;
}


Satellite::Satellite( std::string hdr, std::string l1, std::string l2 )
    : header( hdr ), line1( l1 ), line2( l2 ), VieVS_NamedObject::VieVS_NamedObject( hdr, hdr, nextId++ ) {
    pTleData = new Tle( hdr, l1, l2 );
    pSGP4Data = new SGP4( *pTleData );
}


Satellite::~Satellite() {
    // delete pSGP4Data;
    // delete pTleData;
}

const std::string Satellite::getHeader() const noexcept { return this->header; }

const std::string Satellite::getLine1() const noexcept { return this->line1; }

const std::string Satellite::getLine2() const noexcept { return this->line2; }

SGP4* Satellite::getSGP4Data() { return this->pSGP4Data; }

std::vector<std::vector<Satellite::SatPass>> Satellite::GeneratePassList( const VieVS::Network& network,
                                                                          const DateTime& start_time,
                                                                          const DateTime& end_time,
                                                                          const int time_step ) {
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "start generating passlists for stations";
#else
    cout << "[info] start generating passlists for stations";
#endif
    std::vector<SatPass> pass_list;
    std::vector<std::vector<SatPass>> PassList_;
    std::vector<VieVS::Station> stations = network.getStations();
    for ( const auto& station : stations ) {
        pass_list.clear();
        CoordGeodetic user_geo( station.getPosition().getLat(), station.getPosition().getLon(),
                                station.getPosition().getHeight() / 1000, true );
        Observer obs( user_geo );
        SGP4* sgp4( this->pSGP4Data );
        DateTime aos_time;
        DateTime los_time;
        bool found_aos = false;

        DateTime previous_time( start_time );
        DateTime current_time( start_time );

        while ( current_time < end_time ) {
            bool end_of_pass = false;

            /*
             * calculate satellite position
             */
            Eci eci = sgp4->FindPosition( current_time );
            CoordTopocentric topo = obs.GetLookAngle( eci );
            VieVS::PointingVector pv = VieVS::PointingVector( 1, 0 );
            pv.setAz( topo.azimuth );
            pv.setEl( topo.elevation );

            if ( !found_aos && station.isVisible( pv, 0 ) ) {
                /*
                 * aos hasnt occured yet, but the satellite is now above horizon
                 * this must have occured within the last time_step
                 */
                if ( start_time == current_time ) {
                    /*
                     * satellite was already above the horizon at the start,
                     * so use the start time
                     */
                    aos_time = start_time;
                } else {
                    /*
                     * find the point at which the satellite crossed the horizon
                     */
                    aos_time = FindCrossingPoint( station, *sgp4, previous_time, current_time, true );
                }
                found_aos = true;
            } else if ( found_aos && !station.isVisible( pv, 0 ) ) {
                found_aos = false;
                /*
                 * end of pass, so move along more than time_step
                 */
                end_of_pass = true;
                /*
                 * already have the aos, but now the satellite is below the horizon,
                 * so find the los
                 */
                los_time = FindCrossingPoint( station, *sgp4, previous_time, current_time, false );

                struct SatPass pd;
                pd.start = aos_time;
                pd.end = los_time;
                pd.StationID = station.getId();
                pd.SatelliteID = this->getId();
                pass_list.push_back( pd );
            }

            /*
             * save current time
             */
            previous_time = current_time;

            if ( end_of_pass ) {
                /*
                 * at the end of the pass move the time along by 30mins
                 */
                current_time = current_time + TimeSpan( 0, 30, 0 );
            } else {
                /*
                 * move the time along by the time step value
                 */
                current_time = current_time + TimeSpan( 0, 0, time_step );
            }

            if ( current_time > end_time ) {
                /*
                 * dont go past end time
                 */
                current_time = end_time;
            }
        };

        if ( found_aos ) {
            /*
             * satellite still above horizon at end of search period, so use end
             * time as los
             */
            struct SatPass pd;
            pd.start = aos_time;
            pd.end = end_time;
            pd.StationID = station.getId();
            pd.SatelliteID = this->getId();
            pass_list.push_back( pd );
        }
        PassList_.push_back( pass_list );
    }
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "finish generating passlists for stations sucessfully";
#else
    cout << "[info] finish generating passlists for stations sucessfully";
#endif
    return PassList_;
}

DateTime Satellite::FindCrossingPoint( const VieVS::Station& station, const SGP4& sgp4, const DateTime& initial_time1,
                                       const DateTime& initial_time2, bool finding_aos ) {
    CoordGeodetic user_geo( station.getPosition().getLat(), station.getPosition().getLon(),
                            station.getPosition().getHeight() / 1000, true );
    Observer obs( user_geo );
    DateTime time1( initial_time1 );
    DateTime time2( initial_time2 );
    DateTime middle_time;

    bool running = true;
    // int cnt = 0;
    // while (running && cnt++ < 16)
    while ( running ) {
        middle_time = time1.AddSeconds( ( time2 - time1 ).TotalSeconds() / 2.0 );
        /*
         * calculate satellite position
         */
        Eci eci = sgp4.FindPosition( middle_time );
        CoordTopocentric topo = obs.GetLookAngle( eci );
        VieVS::PointingVector pv = VieVS::PointingVector( 1, 0 );
        pv.setAz( topo.azimuth );
        pv.setEl( topo.elevation );
        if ( station.isVisible( pv, 0 ) ) {
            /*
             * satellite above horizon
             */
            if ( finding_aos ) {
                time2 = middle_time;
            } else {
                time1 = middle_time;
            }
        } else {
            if ( finding_aos ) {
                time1 = middle_time;
            } else {
                time2 = middle_time;
            }
        }
        if ( ( time2 - time1 ).TotalSeconds() < 0.0001 )  // if the timespan
        {
            /*
             * two times are within a millisecond, stop
             */
            running = false;  // to quit the while loop
        }
    }
    return middle_time;
}

vector<Satellite> Satellite::readSatelliteFile( std::string filename_ ) {
    vector<Satellite> satellites_;
    try {
        std::ifstream fid( filename_ );
        // fid.exceptions ( ifstream::eofbit | ifstream::failbit | ifstream::badbit );
        std::string line;
        std::string hdr;
        std::string line1;
        std::string line2;
        if ( fid.is_open() ) {
            while ( getline( fid, line ) ) {
                if ( isalpha( line[0] ) ) {
                    hdr = line;
                    continue;
                } else if ( line[0] == '1' ) {
                    line1 = line;
                    continue;
                } else if ( line[0] == '2' ) {
                    line2 = line;
                    if ( !hdr.empty() && !line1.empty() && !line.empty() ) {
                        Satellite sat( hdr, line1, line2 );
                        satellites_.push_back( sat );
                        hdr = "";
                        line1 = "";
                        line2 = "";
                    } else {
#ifdef VIESCHEDPP_LOG
                        BOOST_LOG_TRIVIAL( error ) << "wrong file format for satellite input" << filename_;
#else
                        cout << "[error] wrong file format for satellite input" << filename_;
#endif
                        satellites_.clear();
                    }

                    continue;
                } else {
                    cout << "[error] unable to read satellite file" << endl;
                }
            }
            fid.close();
        } else {
            std::cout << "[error] unable to open " << filename_;
        }
    } catch ( std::exception const& e ) {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( error ) << "unable to open " << filename_;
#else
        cout << "[error] unable to open " << filename_;
#endif
        terminate();
    }
    return satellites_;
}