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

#include "Antenna_XYew.h"


using namespace VieVS;
using namespace std;


Antenna_XYew::Antenna_XYew( double offset_m, double diam_m, double rateX_deg_per_min, unsigned int constantOverheadX_s,
                            double rateY_deg_per_min, unsigned int constantOverheadY_s )
    : AbstractAntenna( offset_m, diam_m, rateX_deg_per_min, constantOverheadX_s, rateY_deg_per_min,
                       constantOverheadY_s ) {}


unsigned int Antenna_XYew::slewTime( const PointingVector &old_pointingVector,
                                     const PointingVector &new_pointingVector ) const noexcept {
    double cel_old = cos( old_pointingVector.getEl() );
    double sel_old = sin( old_pointingVector.getEl() );
    double caz_old = cos( old_pointingVector.getAz() );
    double saz_old = sin( old_pointingVector.getAz() );

    double x_old = atan2( cel_old * caz_old, sel_old );
    double y_old = asin( cel_old * saz_old );

    double cel_new = cos( new_pointingVector.getEl() );
    double sel_new = sin( new_pointingVector.getEl() );
    double caz_new = cos( new_pointingVector.getAz() );
    double saz_new = sin( new_pointingVector.getAz() );

    double x_new = atan2( cel_new * caz_new, sel_new );
    double y_new = asin( cel_new * saz_new );

    double delta1 = abs( x_new - x_old );
    double delta2 = abs( y_new - y_old );

    unsigned int t_1 = slewTimePerAxis( delta1, Axis::axis1 );
    unsigned int t_2 = slewTimePerAxis( delta2, Axis::axis2 );

    return t_1 > t_2 ? t_1 : t_2;
}

unsigned int Antenna_XYew::slewTimeTracking( const PointingVector &old_pointingVector,
                                             const PointingVector &new_pointingVector ) const noexcept {
    double cel_old = cos( old_pointingVector.getEl() );
    double sel_old = sin( old_pointingVector.getEl() );
    double caz_old = cos( old_pointingVector.getAz() );
    double saz_old = sin( old_pointingVector.getAz() );

    double x_old = atan2( cel_old * caz_old, sel_old );
    double y_old = asin( cel_old * saz_old );

    double cel_new = cos( new_pointingVector.getEl() );
    double sel_new = sin( new_pointingVector.getEl() );
    double caz_new = cos( new_pointingVector.getAz() );
    double saz_new = sin( new_pointingVector.getAz() );

    double x_new = atan2( cel_new * caz_new, sel_new );
    double y_new = asin( cel_new * saz_new );

    double delta1 = abs( x_new - x_old );
    double delta2 = abs( y_new - y_old );

    unsigned int t_1 = slewTimePerAxis( delta1, Axis::axis1 ) - static_cast<unsigned int>( lround( getCon1() ) );
    unsigned int t_2 = slewTimePerAxis( delta2, Axis::axis2 ) - static_cast<unsigned int>( lround( getCon2() ) );

    return t_1 > t_2 ? t_1 : t_2;
}

std::string Antenna_XYew::toVex( AbstractAntenna::Axis axis ) const {
    string str;
    if ( axis == Axis::axis1 ) {
        str = ( boost::format( "        antenna_motion = %3s: %3.0f deg/min: %3d sec: %5.2f deg/sec^2; \n" ) % "x" %
                ( getRate1() * rad2deg * 60 ) % ( getCon1() ) % ( getRate1() * rad2deg ) )
                  .str();
    }
    if ( axis == Axis::axis1 ) {
        str = ( boost::format( "        antenna_motion = %3s: %3.0f deg/min: %3d sec: %5.2f deg/sec^2; \n" ) % "yew" %
                ( getRate2() * rad2deg * 60 ) % ( getCon2() ) % ( getRate2() * rad2deg ) )
                  .str();
    }
    return str;
}
