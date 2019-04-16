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

#include "Equipment_elDependent.h"
using namespace VieVS;
using namespace std;

Equipment_elDependent::Equipment_elDependent( const std::unordered_map<std::string, double> &SEFDs,
                                              const std::unordered_map<std::string, double> &SEFD_y,
                                              const std::unordered_map<std::string, double> &SEFD_c0,
                                              const std::unordered_map<std::string, double> &SEFD_c1 )
    : Equipment( SEFDs ), y_{SEFD_y}, c0_{SEFD_c0}, c1_{SEFD_c1} {}

double Equipment_elDependent::getSEFD( const std::string &band, double el ) const noexcept {
    double y = y_.at( band );
    double c0 = c0_.at( band );
    double c1 = c1_.at( band );

    double tmp = pow( sin( el ), y );
    double tmp2 = c0 + c1 / tmp;

    if ( tmp2 < 1 ) {
        return Equipment::getSEFD( band, el );
    } else {
        return Equipment::getSEFD( band, el ) * tmp2;
    }
}
