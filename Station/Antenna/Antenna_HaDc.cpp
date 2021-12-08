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

#include "Antenna_HaDc.h"


using namespace VieVS;
using namespace std;


Antenna_HaDc::Antenna_HaDc( double offset_m, double diam_m, double rateHa_deg_per_min,
                            unsigned int constantOverheadHa_s, double rateDc_deg_per_min,
                            unsigned int constantOverheadDc_s )
    : AbstractAntenna( offset_m, diam_m, rateHa_deg_per_min, constantOverheadHa_s, rateDc_deg_per_min,
                       constantOverheadDc_s ) {}


unsigned int Antenna_HaDc::slewTime( const PointingVector &old_pointingVector,
                                     const PointingVector &new_pointingVector ) const noexcept {
    double delta1 = abs( new_pointingVector.getHa() - old_pointingVector.getHa() );
    double delta2 = abs( new_pointingVector.getDc() - old_pointingVector.getDc() );

    unsigned int t_1 = slewTimePerAxis( delta1, Axis::axis1 );
    unsigned int t_2 = slewTimePerAxis( delta2, Axis::axis2 );

    return t_1 > t_2 ? t_1 : t_2;
}
unsigned int Antenna_HaDc::slewTimeTracking( const PointingVector &old_pointingVector,
                                             const PointingVector &new_pointingVector ) const noexcept {
    double delta1 = abs( new_pointingVector.getHa() - old_pointingVector.getHa() );
    double delta2 = abs( new_pointingVector.getDc() - old_pointingVector.getDc() );

    unsigned int t_1 = slewTimePerAxis( delta1, Axis::axis1 ) - static_cast<unsigned int>( lround( getCon1() ) );
    unsigned int t_2 = slewTimePerAxis( delta2, Axis::axis2 ) - static_cast<unsigned int>( lround( getCon2() ) );

    return t_1 > t_2 ? t_1 : t_2;
}


std::string Antenna_HaDc::toVex( AbstractAntenna::Axis axis ) const {
    string str;
    if ( axis == Axis::axis1 ) {
        str = ( boost::format( "        antenna_motion = %3s: %3.0f deg/min: %3d sec: %5.2f deg/sec^2; \n" ) % "ha" %
                ( getRate1() * rad2deg * 60 ) % ( getCon1() ) % ( getRate1() * rad2deg ) )
                  .str();
    }
    if ( axis == Axis::axis2 ) {
        str = ( boost::format( "        antenna_motion = %3s: %3.0f deg/min: %3d sec: %5.2f deg/sec^2; \n" ) % "dec" %
                ( getRate2() * rad2deg * 60 ) % ( getCon2() ) % ( getRate2() * rad2deg ) )
                  .str();
    }
    return str;
}
