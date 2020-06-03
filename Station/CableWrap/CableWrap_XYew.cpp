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

#include "CableWrap_XYew.h"


using namespace VieVS;
using namespace std;


CableWrap_XYew::CableWrap_XYew( double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg )
    : AbstractCableWrap( axis1_low_deg, axis1_up_deg, axis2_low_deg, axis2_up_deg ) {}


bool CableWrap_XYew::anglesInside( const PointingVector &p ) const noexcept {
    double az = p.getAz();
    double el = p.getEl();

    double cel = cos( el );
    double sel = sin( el );
    double caz = cos( az );
    double saz = sin( az );

    double ax1 = atan2( cel * caz, sel );
    double ax2 = asin( cel * saz );

    return axisInsideCableWrap( ax1, ax2 );
}


void CableWrap_XYew::unwrapAzNearAz( PointingVector &new_pointingVector, double az_old ) const noexcept {}


AbstractCableWrap::CableWrapFlag CableWrap_XYew::cableWrapFlag( double unaz ) const noexcept {
    return AbstractCableWrap::CableWrapFlag::n;
}


bool CableWrap_XYew::unwrapAzInSection( PointingVector &pv, char section ) const noexcept { return section != '-'; }


std::pair<std::string, std::string> CableWrap_XYew::getMotions() const noexcept { return { "x", "yew" }; }


std::string CableWrap_XYew::vexPointingSectors() const noexcept { return pointingSector( "x", "yew", '-' ); }
