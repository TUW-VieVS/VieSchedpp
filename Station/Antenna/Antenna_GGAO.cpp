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
    double az_vel = AbstractAntenna::getRate1() * rad2deg;
    double el_vel = AbstractAntenna::getRate2() * rad2deg;

    double az_beg = old_pointingVector.getAz() * rad2deg;  // starting point
    double el_beg = old_pointingVector.getEl() * rad2deg;  // starting point

    double az_end = new_pointingVector.getAz() * rad2deg;  // ending point
    double el_end = new_pointingVector.getEl() * rad2deg;  // ending point

    double az_pk1 = 192;
    double az_pk2 = 552;
    double el_pk = 42.;
    double fudge = 1.0;
    double half_width = el_pk;

    double az_pk1_lft = az_pk1 - half_width;
    double az_pk1_rt = az_pk1 + half_width;
    double az_pk2_lft = az_pk2 - half_width;
    double az_pk2_rt = az_pk2 + half_width;

    double az_acc = az_vel / az_off;
    double el_acc = el_vel / el_off;

    if ( az_beg > az_end ) {
        swap( az_beg, az_end );
        swap( el_beg, el_end );
    }

    double az_slewt = slew_time( az_beg, az_end, az_vel, az_acc );
    double el_slewt = slew_time( el_beg, el_end, el_vel, el_acc );

    double slew0 = max( az_slewt, el_slewt );

    // Above the mask
    if ( el_beg >= el_pk && el_end >= el_pk ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }
    // Both to the left of the first mask
    if ( az_beg <= az_pk1_lft && az_end <= az_pk1_lft ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }
    // Both to the right of the second mask
    if ( az_beg >= az_pk2_rt && az_end >= az_pk2_rt ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }
    // Both between the masks
    if ( ( az_beg >= az_pk1_rt && az_beg <= az_pk2_lft ) && ( az_end >= az_pk1_rt && az_end <= az_pk2_lft ) ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }

    // This handles case where starting and ending below mask and both starting and ending points are in same valley.
    // starting and ending below the peaks
    if ( ( el_beg <= el_pk && el_end <= el_pk ) ) {
        // Both to the left of the first mask.
        if ( az_beg <= az_pk1 && az_end <= az_pk1 ) {
            return static_cast<unsigned int>( ceil( slew0 ) );
        }
        if ( az_beg >= az_pk2 && az_end >= az_pk2 ) {
            return static_cast<unsigned int>( ceil( slew0 ) );
        }
        if ( ( az_beg >= az_pk1 && az_beg <= az_pk2 ) && ( az_end >= az_pk1 && az_end <= az_pk2 ) ) {
            return static_cast<unsigned int>( ceil( slew0 ) );
        }
    }

    // Handle some rare cases.  Both within LHS of mask or RHS of mask.   Assume normal slewing.
    if ( ( az_beg >= az_pk1_lft && az_beg <= az_pk1 ) && ( az_end >= az_pk1_lft && az_end <= az_pk1 ) ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }
    if ( ( az_beg >= az_pk2_lft && az_beg <= az_pk2 ) && ( az_end >= az_pk2_lft && az_end <= az_pk2 ) ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }
    if ( ( az_beg >= az_pk1 && az_beg <= az_pk1_rt ) && ( az_end >= az_pk1 && az_end <= az_pk1_rt ) ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }
    if ( ( az_beg >= az_pk2 && az_beg <= az_pk2_rt ) && ( az_end >= az_pk2 && az_end <= az_pk2_rt ) ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }

    // In the region of a peak and going up. This is OK if going up from right side of peak.
    if ( el_end > el_beg && el_end > el_pk ) {
        if ( ( az_beg > az_pk1 && az_beg < az_pk1_rt ) || ( az_beg > az_pk2 && az_beg < az_pk2_rt ) ) {
            return static_cast<unsigned int>( ceil( slew0 ) );
        }
    }
    // In the region of a peak and going done. This is OK if coming down from left side.
    if ( el_beg > el_end && el_beg > el_pk ) {
        if ( ( az_end > az_pk1_lft && az_end < az_pk1 ) || ( az_end > az_pk2_lft && az_end < az_pk2 ) ) {
            return static_cast<unsigned int>( ceil( slew0 ) );
        }
    }

    // For many of the remaining cases we split the motion into two or three line segments.
    // Each line segment starts or ends at a peak.
    double el_mid = el_pk + fudge;  // for many parts below assume that one line segment ends at a peak.

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
    if ( el_beg <= el_mid && el_end <= el_mid ) {  // both starting and ending points below a peak.
        // Break the problem into pieces.
        // 1. What peak do we have to climb?
        // 2. What peak do we descend.
        // 3. Did we go over both peaks.

        // 1. Find which peak we are climbing
        double az_mid1;
        if ( az_beg <= az_pk1 ) {
            az_mid1 = az_pk1_lft;
        } else {
            az_mid1 = az_pk2_lft;
        }
        // Find slew time for first segment.
        az_mid1 = max( az_beg, az_mid1 );  // handles rare case when within rectangular mask
        double az_slew1 = slew_time( az_beg, az_mid1, az_vel, az_acc );
        double el_slew1 = slew_time( el_beg, el_mid, el_vel, el_acc );

        // 2. Find which peak we are descending
        double az_mid2;
        if ( az_end >= az_pk2 ) {
            az_mid2 = az_pk2_rt;
        } else {
            az_mid2 = az_pk1_rt;
        }
        // Find slew time for second segment
        az_mid2 = min( az_mid2, az_end );  // handles rare case when within rectangular mask
        double az_slew2 = slew_time( az_mid2, az_end, az_vel, az_acc );
        double el_slew2 = slew_time( el_mid, el_end, el_vel, el_acc );

        // Slew values used for comparison of time.
        // Subtract 1/2 offset because we don't worry about stopping/starting
        double az_slew1p = az_slew1 - az_off / 2;
        double az_slew2p = az_slew2 - az_off / 2;
        double el_slew1p = el_slew1 - el_off / 2;
        double el_slew2p = el_slew2 - el_off / 2;

        double slewt;
        if ( az_slew1p >= el_slew1p && az_slew2p >= el_slew2p ) {
            // One very long slew in azimuth
            slewt = slew_time( az_beg, az_end, az_vel, az_acc );
        } else if ( az_slew1p >= el_slew1p && az_slew2p <= el_slew2p ) {
            // A long slew in Az followed by the descent in Elevation
            // Subtract 1/2 of the offset because this coincides with el starting.
            slewt = slew_time( az_beg, az_mid2, az_vel, az_acc ) + el_slew2 - az_off / 2;
        } else if ( az_slew1p <= el_slew1p && az_slew2p >= el_slew2p ) {
            slewt = el_slew1 + slew_time( az_mid1, az_end, az_vel, az_acc ) - az_off / 2;
        } else {
            slewt = el_slew1 + ( az_mid2 - az_mid1 ) / az_vel + el_slew2;
        }
        return static_cast<unsigned int>( ceil( slewt ) );
    }

    // SECOND CASE
    // Start in a valley and and above a peak
    // --OR--
    // Start above a peak and end in a valley.
    // In both cases ceck if we would hit a peak in the normal course of business.
    // If we don't can use the normal slewing.

    // First case. Start low, come up high.
    double az_mid1;
    if ( el_beg < el_end ) {
        if ( az_beg < az_pk1 ) {
            az_mid1 = az_pk1_lft;
        } else {
            az_mid1 = az_pk2_lft;
        }
    } else {
        // Start high, come down low
        if ( az_beg < az_pk1_rt ) {
            az_mid1 = az_pk1_rt;
        } else {
            az_mid1 = az_pk2_rt;
        }
    }

    az_mid1 = max( az_beg, az_mid1 );  // middle can't be before beginning
    az_mid1 = min( az_mid1, az_end );  // middle can't be after ending

    double az_slew1 = slew_time( az_beg, az_mid1, az_vel, az_acc );
    double el_slew1 = slew_time( el_beg, el_mid, el_vel, el_acc );

    // This is slew time used for comparison. Don't worry about stopping
    double az_slew1p = abs( az_beg - az_mid1 ) / az_vel + az_off / 2;
    double el_slew1p = abs( el_beg - el_mid ) / el_vel + el_off / 2;

    double slewt;
    if ( el_beg < el_end ) {
        if ( az_slew1p >= el_slew1p ) {
            return static_cast<unsigned int>( ceil( slew0 ) );
        }  // Don't hit side on the way up. Normal slew.
           // Two possibilities.
           // 1. A long slew in elevation
           // 2. A slew in elevation followed by one in azimuth
        double az_slew2 = slew_time( az_mid1, az_end, az_vel, az_acc );
        slewt = max( el_slewt, el_slew1p + az_slew2 );
    } else {
        if ( el_slew1p > az_slew1p ) {
            return static_cast<unsigned int>( ceil( slew0 ) );
        }  // don't hit top on the way down. Normal slew
        // Two possibilities.
        // 1. A long slew in azimuth
        // 2. A slew in azimuth followed by one in elevation.
        double el_slew2 = slew_time( el_mid, el_end, el_vel, el_acc );
        // Use az_slew1p because antenna is still moving. It will stop while el is moving.
        slewt = max( az_slewt, az_slew1p + el_slew2 );
    }

    return static_cast<unsigned int>( ceil( slewt ) );
}

double Antenna_GGAO::slew_time( double x1, double x2, double vel, double acc ) {
    double dist = abs( x1 - x2 );
    double t_acc = vel / acc;

    double slew_time;
    if ( dist <= acc * t_acc * t_acc ) {
        slew_time = 2.0 * sqrt( dist / acc );
    } else {
        slew_time = dist / vel + t_acc;
    }
    return slew_time;
}

unsigned int Antenna_GGAO::slewTimeTracking( const PointingVector &old_pointingVector,
                                             const PointingVector &new_pointingVector ) const noexcept {

    double az_off = 0;
    double el_off = 0;
    double az_vel = AbstractAntenna::getRate1() * rad2deg;
    double el_vel = AbstractAntenna::getRate2() * rad2deg;

    double az_beg = old_pointingVector.getAz() * rad2deg;  // starting point
    double el_beg = old_pointingVector.getEl() * rad2deg;  // starting point

    double az_end = new_pointingVector.getAz() * rad2deg;  // ending point
    double el_end = new_pointingVector.getEl() * rad2deg;  // ending point

    double az_pk1 = 192;
    double az_pk2 = 552;
    double el_pk = 42.;
    double fudge = 1.0;
    double half_width = el_pk;

    double az_pk1_lft = az_pk1 - half_width;
    double az_pk1_rt = az_pk1 + half_width;
    double az_pk2_lft = az_pk2 - half_width;
    double az_pk2_rt = az_pk2 + half_width;

    double az_acc = 0;
    double el_acc = 0;

    if ( az_beg > az_end ) {
        swap( az_beg, az_end );
        swap( el_beg, el_end );
    }

    double az_slewt = slew_time( az_beg, az_end, az_vel, az_acc );
    double el_slewt = slew_time( el_beg, el_end, el_vel, el_acc );

    double slew0 = max( az_slewt, el_slewt );

    // Above the mask
    if ( el_beg >= el_pk && el_end >= el_pk ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }
    // Both to the left of the first mask
    if ( az_beg <= az_pk1_lft && az_end <= az_pk1_lft ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }
    // Both to the right of the second mask
    if ( az_beg >= az_pk2_rt && az_end >= az_pk2_rt ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }
    // Both between the masks
    if ( ( az_beg >= az_pk1_rt && az_beg <= az_pk2_lft ) && ( az_end >= az_pk1_rt && az_end <= az_pk2_lft ) ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }

    // This handles case where starting and ending below mask and both starting and ending points are in same valley.
    // starting and ending below the peaks
    if ( ( el_beg <= el_pk && el_end <= el_pk ) ) {
        // Both to the left of the first mask.
        if ( az_beg <= az_pk1 && az_end <= az_pk1 ) {
            return static_cast<unsigned int>( ceil( slew0 ) );
        }
        if ( az_beg >= az_pk2 && az_end >= az_pk2 ) {
            return static_cast<unsigned int>( ceil( slew0 ) );
        }
        if ( ( az_beg >= az_pk1 && az_beg <= az_pk2 ) && ( az_end >= az_pk1 && az_end <= az_pk2 ) ) {
            return static_cast<unsigned int>( ceil( slew0 ) );
        }
    }

    // Handle some rare cases.  Both within LHS of mask or RHS of mask.   Assume normal slewing.
    if ( ( az_beg >= az_pk1_lft && az_beg <= az_pk1 ) && ( az_end >= az_pk1_lft && az_end <= az_pk1 ) ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }
    if ( ( az_beg >= az_pk2_lft && az_beg <= az_pk2 ) && ( az_end >= az_pk2_lft && az_end <= az_pk2 ) ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }
    if ( ( az_beg >= az_pk1 && az_beg <= az_pk1_rt ) && ( az_end >= az_pk1 && az_end <= az_pk1_rt ) ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }
    if ( ( az_beg >= az_pk2 && az_beg <= az_pk2_rt ) && ( az_end >= az_pk2 && az_end <= az_pk2_rt ) ) {
        return static_cast<unsigned int>( ceil( slew0 ) );
    }

    // In the region of a peak and going up. This is OK if going up from right side of peak.
    if ( el_end > el_beg && el_end > el_pk ) {
        if ( ( az_beg > az_pk1 && az_beg < az_pk1_rt ) || ( az_beg > az_pk2 && az_beg < az_pk2_rt ) ) {
            return static_cast<unsigned int>( ceil( slew0 ) );
        }
    }
    // In the region of a peak and going done. This is OK if coming down from left side.
    if ( el_beg > el_end && el_beg > el_pk ) {
        if ( ( az_end > az_pk1_lft && az_end < az_pk1 ) || ( az_end > az_pk2_lft && az_end < az_pk2 ) ) {
            return static_cast<unsigned int>( ceil( slew0 ) );
        }
    }

    // For many of the remaining cases we split the motion into two or three line segments.
    // Each line segment starts or ends at a peak.
    double el_mid = el_pk + fudge;  // for many parts below assume that one line segment ends at a peak.

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
    if ( el_beg <= el_mid && el_end <= el_mid ) {  // both starting and ending points below a peak.
        // Break the problem into pieces.
        // 1. What peak do we have to climb?
        // 2. What peak do we descend.
        // 3. Did we go over both peaks.

        // 1. Find which peak we are climbing
        double az_mid1;
        if ( az_beg <= az_pk1 ) {
            az_mid1 = az_pk1_lft;
        } else {
            az_mid1 = az_pk2_lft;
        }
        // Find slew time for first segment.
        az_mid1 = max( az_beg, az_mid1 );  // handles rare case when within rectangular mask
        double az_slew1 = slew_time( az_beg, az_mid1, az_vel, az_acc );
        double el_slew1 = slew_time( el_beg, el_mid, el_vel, el_acc );

        // 2. Find which peak we are descending
        double az_mid2;
        if ( az_end >= az_pk2 ) {
            az_mid2 = az_pk2_rt;
        } else {
            az_mid2 = az_pk1_rt;
        }
        // Find slew time for second segment
        az_mid2 = min( az_mid2, az_end );  // handles rare case when within rectangular mask
        double az_slew2 = slew_time( az_mid2, az_end, az_vel, az_acc );
        double el_slew2 = slew_time( el_mid, el_end, el_vel, el_acc );

        // Slew values used for comparison of time.
        // Subtract 1/2 offset because we don't worry about stopping/starting
        double az_slew1p = az_slew1 - az_off / 2;
        double az_slew2p = az_slew2 - az_off / 2;
        double el_slew1p = el_slew1 - el_off / 2;
        double el_slew2p = el_slew2 - el_off / 2;

        double slewt;
        if ( az_slew1p >= el_slew1p && az_slew2p >= el_slew2p ) {
            // One very long slew in azimuth
            slewt = slew_time( az_beg, az_end, az_vel, az_acc );
        } else if ( az_slew1p >= el_slew1p && az_slew2p <= el_slew2p ) {
            // A long slew in Az followed by the descent in Elevation
            // Subtract 1/2 of the offset because this coincides with el starting.
            slewt = slew_time( az_beg, az_mid2, az_vel, az_acc ) + el_slew2 - az_off / 2;
        } else if ( az_slew1p <= el_slew1p && az_slew2p >= el_slew2p ) {
            slewt = el_slew1 + slew_time( az_mid1, az_end, az_vel, az_acc ) - az_off / 2;
        } else {
            slewt = el_slew1 + ( az_mid2 - az_mid1 ) / az_vel + el_slew2;
        }
        return static_cast<unsigned int>( ceil( slewt ) );
    }

    // SECOND CASE
    // Start in a valley and and above a peak
    // --OR--
    // Start above a peak and end in a valley.
    // In both cases ceck if we would hit a peak in the normal course of business.
    // If we don't can use the normal slewing.

    // First case. Start low, come up high.
    double az_mid1;
    if ( el_beg < el_end ) {
        if ( az_beg < az_pk1 ) {
            az_mid1 = az_pk1_lft;
        } else {
            az_mid1 = az_pk2_lft;
        }
    } else {
        // Start high, come down low
        if ( az_beg < az_pk1_rt ) {
            az_mid1 = az_pk1_rt;
        } else {
            az_mid1 = az_pk2_rt;
        }
    }

    az_mid1 = max( az_beg, az_mid1 );  // middle can't be before beginning
    az_mid1 = min( az_mid1, az_end );  // middle can't be after ending

    double az_slew1 = slew_time( az_beg, az_mid1, az_vel, az_acc );
    double el_slew1 = slew_time( el_beg, el_mid, el_vel, el_acc );

    // This is slew time used for comparison. Don't worry about stopping
    double az_slew1p = abs( az_beg - az_mid1 ) / az_vel + az_off / 2;
    double el_slew1p = abs( el_beg - el_mid ) / el_vel + el_off / 2;

    double slewt;
    if ( el_beg < el_end ) {
        if ( az_slew1p >= el_slew1p ) {
            return static_cast<unsigned int>( ceil( slew0 ) );
        }  // Don't hit side on the way up. Normal slew.
        // Two possibilities.
        // 1. A long slew in elevation
        // 2. A slew in elevation followed by one in azimuth
        double az_slew2 = slew_time( az_mid1, az_end, az_vel, az_acc );
        slewt = max( el_slewt, el_slew1p + az_slew2 );
    } else {
        if ( el_slew1p > az_slew1p ) {
            return static_cast<unsigned int>( ceil( slew0 ) );
        }  // don't hit top on the way down. Normal slew
        // Two possibilities.
        // 1. A long slew in azimuth
        // 2. A slew in azimuth followed by one in elevation.
        double el_slew2 = slew_time( el_mid, el_end, el_vel, el_acc );
        // Use az_slew1p because antenna is still moving. It will stop while el is moving.
        slewt = max( az_slewt, az_slew1p + el_slew2 );
    }

    return static_cast<unsigned int>( ceil( slewt ) );
}

std::string Antenna_GGAO::toVex( Axis axis ) const {
    string str;
    if ( axis == Axis::axis1 ) {
        str = ( boost::format(
                    "        antenna_motion = %3s: %3.0f deg/min: %3d sec; ***VEX2***: %5.2f deg/sec^2; * special slew "
                    "model to avoid slewing through 42 deg area around azimuth 192 elevation 0 \n" ) %
                "az" % ( getRate1() * rad2deg * 60 ) % ( getCon1() ) % ( getRate1() * rad2deg ) )
                  .str();
    }
    if ( axis == Axis::axis2 ) {
        str = ( boost::format(
                    "        antenna_motion = %3s: %3.0f deg/min: %3d sec; ***VEX2***: %5.2f deg/sec^2; * special slew "
                    "model to avoid slewing through 42 deg area around azimuth 192 elevation 0\n" ) %
                "el" % ( getRate2() * rad2deg * 60 ) % ( getCon2() ) % ( getRate2() * rad2deg ) )
                  .str();
    }
    return str;
}
