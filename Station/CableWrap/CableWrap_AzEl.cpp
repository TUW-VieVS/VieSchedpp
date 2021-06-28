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

#include "CableWrap_AzEl.h"


using namespace VieVS;
using namespace std;


CableWrap_AzEl::CableWrap_AzEl( double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg )
    : AbstractCableWrap( axis1_low_deg, axis1_up_deg, axis2_low_deg, axis2_up_deg ) {}


bool CableWrap_AzEl::anglesInside( const PointingVector &p ) const noexcept {
    double ax1 = p.getAz();
    double ax2 = p.getEl();

    return axisInsideCableWrap( ax1, ax2 );
}


void CableWrap_AzEl::unwrapAzNearAz( PointingVector &new_pointingVector, double az_old ) const noexcept {
    double az_new = new_pointingVector.getAz();

    double unaz_new;

    while ( az_new > minLow( Axis::axis1 ) ) {
        az_new = az_new - 2 * pi;
    }
    while ( az_new < minLow( Axis::axis1 ) ) {
        az_new = az_new + 2 * pi;
    }

    unaz_new = az_new;
    auto ambigurities = static_cast<int>( floor( ( maxUp( Axis::axis1 ) - unaz_new ) / ( 2 * pi ) ) );
    double this_unaz = unaz_new;

    for ( int i = 1; i <= ambigurities; ++i ) {
        double this_unaz_new = unaz_new + i * 2 * pi;
        if ( abs( this_unaz - az_old ) < abs( this_unaz_new - az_old ) ) {
            break;
        } else {
            this_unaz = this_unaz_new;
        }
    }
    new_pointingVector.setAz( this_unaz );
}


AbstractCableWrap::CableWrapFlag CableWrap_AzEl::cableWrapFlag( double unaz ) const noexcept {
    if ( unaz <= getNUp() && unaz >= getNLow() ) {
        return AbstractCableWrap::CableWrapFlag::n;
    }
    if ( unaz <= getCUp() && unaz >= getCLow() ) {
        return AbstractCableWrap::CableWrapFlag::cw;
    }
    if ( unaz <= getWUp() && unaz >= getWLow() ) {
        return AbstractCableWrap::CableWrapFlag::ccw;
    }

    return AbstractCableWrap::CableWrapFlag::n;
}


bool CableWrap_AzEl::unwrapAzInSection( PointingVector &pv, char section ) const noexcept {
    double az = pv.getAz();
    bool flag = false;

    if ( section == '-' ) {
        double n_center = (getNLow() + getNUp())/2.;
        while ( az > n_center ) {
            az -= 2 * pi;
        }
        while ( az < n_center ) {
            az += 2 * pi;
        }
        double diff_1 = abs(az - n_center);
        double diff_2 = abs((az - twopi) - n_center);
        if ( diff_2 < diff_1 ) {
            az = az - twopi;
        }
        if ( az > getNUp() || az < getNLow() ) {
            flag = true;
        }
    } else if ( section == 'C' ) {
        double c_center = (getCLow() + getCUp())/2.;
        while ( az > c_center ) {
            az -= 2 * pi;
        }
        while ( az < c_center ) {
            az += 2 * pi;
        }
        double diff_1 = abs(az - c_center);
        double diff_2 = abs((az - twopi) - c_center);
        if ( diff_2 < diff_1 ) {
            az = az - twopi;
        }
        if ( az > getCUp() || az < getCLow() ) {
            flag = true;
        }
    } else if ( section == 'W' ) {
        double w_center = (getWLow() + getWUp())/2.;
        while ( az > w_center ) {
            az -= 2 * pi;
        }
        while ( az < w_center ) {
            az += 2 * pi;
        }
        double diff_1 = abs(az - w_center);
        double diff_2 = abs((az - twopi) - w_center);
        if ( diff_2 < diff_1 ) {
            az = az - twopi;
        }
        if ( az > getWUp() || az < getWLow() ) {
            flag = true;
        }
    }

    pv.setAz( az );
    return flag;
}


std::pair<std::string, std::string> CableWrap_AzEl::getMotions() const noexcept { return { "az", "el" }; }


std::string CableWrap_AzEl::vexPointingSectors() const noexcept {
    return pointingSector( "az", "el", 'W' ) + pointingSector( "az", "el", '-' ) + pointingSector( "az", "el", 'C' );
}
