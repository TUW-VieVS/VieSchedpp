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

#include "Quasar.h"
using namespace std;
using namespace VieVS;

Quasar::Quasar( const string& src_name, const string& src_name2, double src_ra_deg, double src_de_deg,
                unordered_map<std::string, std::unique_ptr<AbstractFlux>>& src_flux )
    : AbstractSource( src_name, src_name2, src_flux ), ra_{ src_ra_deg * deg2rad }, de_{ src_de_deg * deg2rad } {
    PreCalculated preCalculated = PreCalculated();
    preCalculated.sourceInCrs.resize( 3 );
    double sinDe = sin( de_ );
    double cosDe = cos( de_ );
    preCalculated.sourceInCrs[0] = cosDe * cos( ra_ );
    preCalculated.sourceInCrs[1] = cosDe * sin( ra_ );
    preCalculated.sourceInCrs[2] = sinDe;

    preCalculated_ = make_shared<PreCalculated>( move( preCalculated ) );
}

bool Quasar::checkForNewEvent( unsigned int time, bool& hardBreak ) noexcept {
    bool b = AbstractSource::checkForNewEvent( time, hardBreak );

    if ( getSunDistance( time, nullptr ) < getPARA().minSunDistance ) {
        referencePARA().available = false;
    }

    return b;
}
