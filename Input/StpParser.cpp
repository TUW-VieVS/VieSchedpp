//
// Created by mschartner on 12/3/21.
//

#include "StpParser.h"

using namespace VieVS;
using namespace std;
unsigned long StpParser::nextId = 0;

void StpParser::parse() {
    std::ifstream file( fname_ );

    // antenna parameters
    double slew_az = numeric_limits<double>::quiet_NaN();
    double slew_el = numeric_limits<double>::quiet_NaN();
    double accl_az = numeric_limits<double>::quiet_NaN();
    double accl_el = numeric_limits<double>::quiet_NaN();
    double dece_az = numeric_limits<double>::quiet_NaN();
    double dece_el = numeric_limits<double>::quiet_NaN();
    double tsettle_az = numeric_limits<double>::quiet_NaN();
    double tsettle_el = numeric_limits<double>::quiet_NaN();

    // cable wrap parameters
    double el_min = numeric_limits<double>::quiet_NaN();
    double el_max = numeric_limits<double>::quiet_NaN();
    double az_min = numeric_limits<double>::quiet_NaN();
    double az_cmin = numeric_limits<double>::quiet_NaN();
    double az_cmax = numeric_limits<double>::quiet_NaN();
    double az_max = numeric_limits<double>::quiet_NaN();

    // horizon mask parameters
    vector<double> hor_az;
    vector<double> hor_el;

    if ( file.is_open() ) {
        std::string line;
        while ( std::getline( file, line ) ) {
            boost::trim( line );
            if ( line[0] == '#' ) {
                continue;
            }
            vector<string> splitVector;
            boost::split( splitVector, line, boost::is_space(), boost::token_compress_on );
            if ( splitVector[0] == "LAST_UPDATE:" ) {
                version_ = splitVector[3];
            }
            if ( splitVector[0] == "COORD:" ) {
                auto x = boost::lexical_cast<double>( splitVector[3] );
                auto y = boost::lexical_cast<double>( splitVector[4] );
                auto z = boost::lexical_cast<double>( splitVector[5] );
                position_ = make_shared<Position>( x, y, z );
            }
            if ( splitVector[0] == "MOUNT:" ) {
                mount_ = splitVector[3];
            }

            // antenna parameters
            if ( splitVector[0] == "SLEW_AZ:" ) {
                slew_az = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "SLEW_EL:" ) {
                slew_el = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "ACCL_AZ:" ) {
                accl_az = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "ACCL_EL:" ) {
                accl_el = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "DECE_AZ:" ) {
                dece_az = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "DECE_EL:" ) {
                dece_el = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "TSETTLE_AZ:" ) {
                tsettle_az = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "TSETTLE_EL:" ) {
                tsettle_el = boost::lexical_cast<double>( splitVector[3] );
            }

            // cable wrap parameters
            if ( splitVector[0] == "EL_MIN:" ) {
                el_min = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "EL_MAX:" ) {
                el_max = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "AZ_RANGE:" ) {
                az_min = boost::lexical_cast<double>( splitVector[3] );
                az_cmin = boost::lexical_cast<double>( splitVector[4] );
                az_cmax = boost::lexical_cast<double>( splitVector[5] );
                az_max = boost::lexical_cast<double>( splitVector[6] );
            }

            // horizon mask parameters
            if ( splitVector[0] == "HOR_AZIM:" ) {
                for ( int i = 3; i < splitVector.size(); ++i ) {
                    const auto& v = splitVector[i];
                    hor_az.push_back( boost::lexical_cast<double>( v ) * deg2rad );
                }
            }
            if ( splitVector[0] == "HOR_ELEV:" ) {
                for ( int i = 3; i < splitVector.size(); ++i ) {
                    const auto& v = splitVector[i];
                    hor_el.push_back( boost::lexical_cast<double>( v ) * deg2rad );
                }
            }


            // general parameters
            if ( splitVector[0] == "PREOB:" ) {
                preob_ = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "POSTOB:" ) {
                postob_ = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "RECORDER:" ) {
                recorder_ = splitVector[3];
            }
        }
        file.close();

        if ( isnan( dece_az ) && !isnan( accl_az ) ) {
            dece_az = accl_az;
        }
        if ( isnan( dece_el ) && !isnan( accl_el ) ) {
            dece_el = accl_el;
        }

        if ( mount_ == "ALTAZ" && !isnan( slew_az ) && !isnan( accl_az ) && !isnan( dece_az ) && !isnan( tsettle_az ) &&
             !isnan( slew_el ) && !isnan( accl_el ) && !isnan( dece_el ) && !isnan( tsettle_el ) ) {
            antenna_ = make_shared<Antenna_AzEl_acceleration>( 0, 0, slew_az, accl_az, dece_az, tsettle_az, slew_el,
                                                               accl_el, dece_el, tsettle_el );
        } else if ( mount_ == "AZEL" && !isnan( slew_az ) && isnan( accl_az ) && isnan( dece_az ) &&
                    !isnan( tsettle_az ) && !isnan( slew_el ) && isnan( accl_el ) && isnan( dece_el ) &&
                    !isnan( tsettle_el ) ) {
            antenna_ = make_shared<Antenna_AzEl>( 0, 0, slew_az / 60, tsettle_az, slew_el / 60, tsettle_el );
        }

        if ( mount_ == "ALTAZ" && !isnan( az_min ) && !isnan( az_cmin ) && !isnan( az_cmax ) && !isnan( az_max ) &&
             !isnan( el_min ) && !isnan( el_max ) ) {
            cableWrap_ = make_shared<CableWrap_AzEl>( az_min, az_cmin, az_cmax, az_max, el_min, el_max );
        }

        if ( !hor_az.empty() && hor_az.size() == hor_el.size() ) {
            mask_ = make_shared<HorizonMask_step>( hor_az, hor_el );
        }
    }
}
