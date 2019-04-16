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

#include "CableWrap_HaDc.h"
using namespace VieVS;
using namespace std;

CableWrap_HaDc::CableWrap_HaDc( double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg )
    : AbstractCableWrap( axis1_low_deg, axis1_up_deg, axis2_low_deg, axis2_up_deg ) {}

bool CableWrap_HaDc::anglesInside( const PointingVector &p ) const noexcept {
    double ax1 = p.getHa();
    double ax2 = p.getDc();

    return axisInsideCableWrap( ax1, ax2 );
}

void CableWrap_HaDc::unwrapAzNearAz( PointingVector &new_pointingVector, double az_old ) const noexcept {}

AbstractCableWrap::CableWrapFlag CableWrap_HaDc::cableWrapFlag( double unaz ) const noexcept {
    return AbstractCableWrap::CableWrapFlag::n;
}

bool CableWrap_HaDc::unwrapAzInSection( PointingVector &pv, char section ) const noexcept { return section != '-'; }

std::pair<std::string, std::string> CableWrap_HaDc::getMotions() const noexcept { return {"ha", "dec"}; }

std::string CableWrap_HaDc::vexPointingSectors() const noexcept { return pointingSector( "ha", "dec", '-' ); }
