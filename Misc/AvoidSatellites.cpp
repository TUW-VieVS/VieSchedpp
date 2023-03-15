//
// Created by mschartner on 7/22/22.
//

#include "AvoidSatellites.h"

using namespace VieVS;
using namespace std;

std::vector<std::shared_ptr<VieVS::Satellite>> AvoidSatellites::satellitesToAvoid =
    std::vector<std::shared_ptr<VieVS::Satellite>>();  ///< list of satellites to be avoided during scheduling

double AvoidSatellites::extraMargin = 0.2 * deg2rad;  /// set to 0.2 degree as a default
double AvoidSatellites::orbitError = 2000;            /// set to 2 km by default
double AvoidSatellites::orbitErrorPerDay = 2000;      /// set to 2 km by default
double AvoidSatellites::minElevation = 20 * deg2rad;  /// set minimum elevation to 20 degrees
int AvoidSatellites::frequency = 10;
double AvoidSatellites::outputPercentage = 0.10;
unordered_map<int, unordered_map<int, vector<pair<int, int>>>> AvoidSatellites::visible_{};

void AvoidSatellites::initialize( Network& network ) {
    unsigned int dt = 60;

    if ( satellitesToAvoid.size() > 1e3 ) {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( info ) << "this might take a while (" << network.getNSta() << " stations with "
                                  << satellitesToAvoid.size() << " satellites)";
#else
        cout << "[info] this might take a while ( " << network.getNSta() << " stations and " << satellitesToAvoid.size()
             << " satellites)\n";
#endif
    }

    unsigned long total = network.getNSta() * satellitesToAvoid.size();
    unsigned long counter = 0;
    double next_percentage = outputPercentage;

    for ( auto& sta : network.refStations() ) {
        unsigned long staid = sta.getId();
        for ( const auto& sat : satellitesToAvoid ) {
            ++counter;
            if ( next_percentage > 0 &&
                 static_cast<double>( counter ) / static_cast<double>( total ) > next_percentage ) {
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( info ) << boost::format( "%.2f%% done" ) % ( next_percentage * 100 );
#else
                cout << boost::format( "[info] %.2f%% done" ) % ( next_percentage * 100 );
#endif
                next_percentage += outputPercentage;
            }
            vector<pair<int, int>> tmp;
            unsigned long satid = sat->getId();

            PointingVector pv( staid, satid );
            pv.setTime( 0 );
            sta.calcAzEl_rigorous( sat, pv );

            bool visible = sta.isVisible( pv, AvoidSatellites::minElevation );
            unsigned int startTime = 0;
            unsigned int endTime = 0;

            for ( unsigned int i = dt; i < TimeSystem::duration; i += dt ) {
                pv.setTime( i );
                sta.calcAzEl_rigorous( sat, pv );

                bool nowVisible = sta.isVisible( pv, AvoidSatellites::minElevation );
                if ( !visible && nowVisible ) {
                    startTime = i - dt;
                    visible = true;
                }
                if ( visible && !nowVisible ) {
                    endTime = i;
                    tmp.emplace_back( startTime, endTime );
                    visible = false;
                    startTime = 0;
                    endTime = 0;
                }
            }


            if ( visible ) {
                endTime = TimeSystem::duration;
                tmp.emplace_back( startTime, endTime );
            }
            visible_[staid][satid] = tmp;
        }
    }
}
/// by default, check satellite position every 10 seconds
