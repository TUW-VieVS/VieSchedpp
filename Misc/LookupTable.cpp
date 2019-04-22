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

#include "LookupTable.h"


using namespace VieVS;
using namespace std;

std::vector<double> VieVS::LookupTable::sinLookupTable;
std::vector<double> VieVS::LookupTable::cosLookupTable;
std::vector<double> VieVS::LookupTable::acosLookupTable;
vector<vector<vector<float>>> VieVS::LookupTable::angularDistanceLookup = {};


void LookupTable::initialize() {
    for ( int i = 0; i < twopi * 1001; ++i ) {
        double x = i * 0.001;
        double val = sin( x );
        sinLookupTable.push_back( val );
    }

    for ( int i = 0; i < twopi * 1001; ++i ) {
        double x = i * 0.001;
        double val = cos( x );
        cosLookupTable.push_back( val );
    }

    for ( int i = 0; i < 2001; ++i ) {
        double x = -1 + i * 0.001;
        double val = acos( x );
        acosLookupTable.push_back( val );
    }

    unsigned int maxEl = 91;
    unsigned int sizeAz = 181;
    vector<vector<vector<float>>> storage;

    for ( unsigned int thisEl = 0; thisEl < maxEl; ++thisEl ) {
        unsigned int sizeEl = maxEl - thisEl;
        double thisEl_rad = thisEl * deg2rad;

        vector<vector<float>> thisStorage( sizeAz, vector<float>( sizeEl, 0 ) );

        for ( int deltaAz = 0; deltaAz < sizeAz; ++deltaAz ) {
            double deltaAz_rad = deltaAz * deg2rad;
            for ( int deltaEl = 0; deltaEl < sizeEl; ++deltaEl ) {
                double deltaEl_rad = deltaEl * deg2rad;

                double tmp = sinLookup( thisEl_rad ) * sinLookup( thisEl_rad + deltaEl_rad ) +
                             cosLookup( thisEl_rad ) * cosLookup( thisEl_rad + deltaEl_rad ) * cosLookup( deltaAz_rad );
                double angle = acosLookup( tmp );

                thisStorage[deltaAz][deltaEl] = static_cast<float>( angle );
            }
        }
        angularDistanceLookup.push_back( move( thisStorage ) );
    }
}


float LookupTable::angularDistance( const PointingVector &p1, const PointingVector &p2 ) noexcept {
    return angularDistance( p1.getAz(), p1.getEl(), p2.getAz(), p2.getEl() );
}


double LookupTable::sinLookup( double x ) { return sinLookupTable[lround( x * 1000 )]; }


double LookupTable::cosLookup( double x ) { return cosLookupTable[lround( x * 1000 )]; }


double LookupTable::acosLookup( double x ) { return acosLookupTable.at( lround( x * 1000 ) + 1000 ); }


float LookupTable::angularDistance( double phi1, double theta1, double phi2, double theta2 ) noexcept {
    if ( phi1 > phi2 ) {
        swap( phi1, phi2 );
    }
    if ( theta1 > theta2 ) {
        swap( theta1, theta2 );
    }

    double deltaPhi_tmp = ( phi2 - phi1 ) * rad2deg;
    while ( deltaPhi_tmp > 180 ) {
        deltaPhi_tmp = deltaPhi_tmp - 360;
    }

    long deltaPhi = abs( lround( deltaPhi_tmp ) );

    auto thisTheta = lround( theta1 * rad2deg );
    auto deltaTheta = lround( ( theta2 - theta1 ) * rad2deg );

    return angularDistanceLookup[thisTheta][deltaPhi][deltaTheta];
}
