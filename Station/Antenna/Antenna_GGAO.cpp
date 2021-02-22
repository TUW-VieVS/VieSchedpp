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

#include "Antenna_GGAO.h"

using namespace std;
using namespace VieVS;


Antenna_GGAO::Antenna_GGAO( double offset_m, double diam_m, double rateAz_deg_per_min,
                            unsigned int constantOverheadAz_s, double rateEl_deg_per_min,
                            unsigned int constantOverheadEl_s )
    : AbstractAntenna( offset_m, diam_m, rateAz_deg_per_min, constantOverheadAz_s, rateEl_deg_per_min,
                       constantOverheadEl_s ) {}


unsigned int Antenna_GGAO::slewTime( const PointingVector &old_pointingVector,
                                     const PointingVector &new_pointingVector ) const noexcept {
    // WARNING: THIS CODE IS AN EXACT COPY OF SKED'S ggao_slew.f FILE. IT DOES NOT COPY THE NAMING CONVENTIONS AND CODE
    // STYLE CONVENTIONS WITHIN VieSched++

    double az_off = AbstractAntenna::getCon1();
    double el_off = AbstractAntenna::getCon2();
    double az_rate = AbstractAntenna::getRate1()*rad2deg;
    double el_rate = AbstractAntenna::getRate2()*rad2deg;

    double az1 = old_pointingVector.getAz()*rad2deg;  // starting point
    double el1 = old_pointingVector.getEl()*rad2deg;  // starting point

    double az2 = new_pointingVector.getAz()*rad2deg;  // ending point
    double el2 = new_pointingVector.getEl()*rad2deg;  // ending point


    double az_pk1 = 192.;             // location of peaks in az
    double az_pk2 = ( 192. + 360. );  // location of peaks in az
    double el_pk = 42.;               // height of peaks
    double fudge = 1.;
    double half_width = el_pk;

    double az_pk1_lft = az_pk1 - half_width;  // left limit of peak1
    double az_pk1_rt = az_pk1 + half_width;   // right limit of peak1
    double az_pk2_lft = az_pk2 - half_width;  // left limit of peak2
    double az_pk2_rt = az_pk2 + half_width;   // right limit of peak1

    double el_mid = el_pk;

    // Make sure we are always moving from larger to smaller
    double az_beg;
    double el_beg;
    double az_end;
    double el_end;
    if ( az1 <= az2 ) {
        az_beg = az1;
        el_beg = el1;
        az_end = az2;
        el_end = el2;
    } else {
        az_beg = az2;
        el_beg = el2;
        az_end = az1;
        el_end = el1;
    }


    double az_slew1 = az_off + abs( az_end - az_beg ) / az_rate;
    double el_slew1 = el_off + abs( el_end - el_beg ) / el_rate;
    double az_slew2 = 0;
    double el_slew2 = 0;
    double slew0 = max( az_slew1, el_slew1 );  // nominal slew time
    auto slew0_ui = static_cast<unsigned int>( ceil( slew0 ) );

    // Above the mask
    if(el_beg >= el_pk && el_end >= el_pk){
        return slew0_ui;
    }
    // both on the left
    if(az_beg <= az_pk1_lft && az_end <= az_pk1_lft){
        return slew0_ui;
    }
    // both on the right
    if(az_beg >= az_pk2_rt && az_end >= az_pk2_rt){
        return slew0_ui;
    }
    // both between
    if((az_beg >= az_pk1_rt && az_end >= az_pk1_rt) && (az_beg <= az_pk2_lft && az_end <= az_pk2_lft)){
        return slew0_ui;
    }

    // This handles case where starting and ending below mask and both starting and ending points are in same valley
    if ( el_beg <= el_pk && el_end <= el_pk ) {
        // before first peak
        if ( az_beg <= az_pk1 && az_end <= az_pk1 ) {
            return slew0_ui;
        }
        // after second peak
        if ( az_beg >= az_pk2 && az_end >= az_pk2 ) {
            return slew0_ui;
        }
        // in between peaks
        if ( ( az_beg >= az_pk1 && az_beg <= az_pk2 ) && ( az_end >= az_pk1 && az_end <= az_pk2 ) ) {
            return slew0_ui;
        }
    }


    // In the region of a peak and going up. This is OK if going up from right side of peak.
    if ( el_end > el_beg && el_end > el_pk ) {
        if ( ( az_beg > az_pk1 && az_beg < az_pk1_rt ) || ( az_beg > az_pk2 && az_beg < az_pk2_rt ) ) {
            return slew0_ui;
        }
    }

    // In the region of a peak and going done. This is OK if coming down from left side.
    if ( el_beg > el_end && el_beg > el_pk ) {
        if ( ( az_end > az_pk1_lft && az_end < az_pk1 ) || ( az_end > az_pk2_lft && az_end < az_pk2 ) ) {
            return slew0_ui;
        }
    }

    // For many of the remaining cases we split the motion into two or three line segments.
    // Each line segment starts or ends at a peak.
    //      el_mid = el_pk+fudge
    // for many parts below assume that one line segment ends at a peak.


    // FIRST CASE.
    // The beginning and ending elevation are below the peak.
    // This means that we start in one valley and end in another.
    // (The case where we started and ended in the same valley are covered above.)

    // We split the calculation into several segments.
    // 1. To the top of a peak.
    // 2. Down from a peak.  (May not be the first peak as before.
    // 3. Optional:  travel time between the peaks.
    // For segments 1&2:
    //    For the elevation time we add in the full-offset since we come to a stop.
    //    For the azimuth time we add in only 1/2 the offset since we only have to account for starting acceleration.
    // For segment 3
    //    We  don't have to account for azimuth acceleration since we are already at speed.

    double slewt = 0;  // total slew time

    if ( el_beg <= el_mid && el_end <= el_mid ) {
        el_mid = el_pk + fudge;

        // Break the problem into pieces.
        // 1. What peak do we have to climb?
        // 2. What peak do we descend.
        // 3. Did we go over both peaks.

        // 1. Find time to climb first peak.
        double az_mid1;
        if ( az_beg <= az_pk1 ) {
            az_mid1 = az_pk1 - half_width;
        } else {
            az_mid1 = az_pk2 - half_width;
        }
        // Find slew time for first segment.
        if ( az_beg >= az_mid1 ) {
            az_mid1 = az_beg;
            az_slew1 = 0.;
        } else {
            // only half the offset because don't have to account for stopping
            az_slew1 = az_off / 2.0 + ( az_mid1 - az_beg ) / az_rate;
        }
        el_slew1 = el_off / 2.0 + ( el_mid - el_beg ) / el_rate;

        // 2. Find time to descend the second peak
        double az_mid2;
        if ( az_end <= az_pk2 ) {
            az_mid2 = az_pk1 + half_width;
        } else {
            az_mid2 = az_pk2 + half_width;
        }
        if ( az_mid2 <= az_end ) {
            // only half the offset because don't have to account for stopping
            az_slew2 = az_off / 2.0 + ( az_end - az_mid2 ) / az_rate;
        } else {
            az_mid2 = az_end;
            az_slew2 = 0.0;
        }
        el_slew2 = el_off / 2.0 + ( el_mid - el_end ) / el_rate;

        // Several possibilities.
        if ( az_slew1 >= el_slew1 && az_slew2 >= el_slew2 ) {
            // One long line segment.
            slewt = az_off + abs( az_end - az_beg ) / az_rate;
        } else {
            if ( az_slew1 >= el_slew1 && az_slew2 <= el_slew2 ) {
                // Two segments.
                // rationale is that some of stopping is shared between two paths
                slewt = az_off / 2. + ( az_mid2 - az_beg ) / az_rate + el_off / 2. + ( el_mid - el_end ) / el_rate +
                        max( az_off, el_off ) / 2.;
            } else {
                if ( az_slew1 <= el_slew1 && az_slew2 >= el_slew2 ) {
                    // Two segments
                    slewt = el_off / 2. + ( el_mid - el_beg ) / el_rate + az_off / 2. + ( az_end - az_mid1 ) / az_rate +
                            max( az_off, el_off ) / 2.;
                } else {
                    // Three segments
                    // Removing offset from az term reduces RMS from 1.56-->1.47
                    slewt = el_off + ( el_mid - el_beg ) / el_rate + ( az_mid2 - az_mid1 ) / az_rate + el_off +
                            ( el_mid - el_end ) / el_rate;
                }
            }
        }
        return static_cast<unsigned int>(ceil(slewt));
    }

    // SECOND CASE
    // Start in a valley and and above a peak
    // --OR--
    // Start above a peak and end in a valley.
    // In both cases ceck if we would hit a peak in the normal course of business.
    // If we don't can use the normal slewing.
    el_mid = el_pk + fudge;
    double az_mid1;
    if ( az_beg <= az_pk1_rt ) {
        az_mid1 = az_pk1;
    } else {
        az_mid1 = az_pk2;
    }

    // Do we need to worry about left or right peak?
    if ( el_beg < el_end ) {
        az_mid1 = az_mid1 - half_width;
    } else {
        az_mid1 = az_mid1 + half_width;
    }
    az_mid1 = min( az_mid1, az_end );
    // Find time for both segments.

    az_slew1 = az_off / 2.0 + ( az_mid1 - az_beg ) / az_rate;
    el_slew1 = el_off / 2.0 + abs( el_mid - el_beg ) / el_rate;
    az_slew2 = az_off / 2.0 + ( az_end - az_mid1 ) / az_rate;
    el_slew2 = el_off / 2.0 + abs( el_end - el_mid ) / el_rate;
    // Several options

    // Sort of the default
    // If we are going up, Want Az_slew1>el_slew.  Won't hit left side.
    // If we are going down want az_slew <el_slew. Won't hit right side
    if ( ( az_slew1 >= el_slew1 && el_beg < el_end ) || ( az_slew1 <= el_slew1 && el_beg >= el_end ) ) {
        slewt = slew0;
        return slew0_ui;
    }

    slewt = max( az_slew1, el_slew1 ) + max( az_slew2, el_slew2 );
    return static_cast<unsigned int>( ceil( slewt ) );
}