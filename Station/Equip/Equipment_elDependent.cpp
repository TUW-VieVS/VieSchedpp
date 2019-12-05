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


Equipment_elDependent::Equipment_elDependent( std::unordered_map<std::string, double> SEFDs,
                                              std::unordered_map<std::string, double> SEFD_y,
                                              std::unordered_map<std::string, double> SEFD_c0,
                                              std::unordered_map<std::string, double> SEFD_c1 )
    : Equipment( std::move( SEFDs ) ), y_{std::move( SEFD_y )}, c0_{std::move( SEFD_c0 )}, c1_{std::move( SEFD_c1 )} {}


double Equipment_elDependent::getSEFD( const std::string &band, double el ) const noexcept {
    if ( Equipment::getSEFD( band, 0 ) == 0 ) {
        return 0;
    }


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


std::string Equipment_elDependent::shortSummary( const std::string &band ) const noexcept {
    if ( y_.find( band ) == y_.end() ) {
        return ( boost::format( "%7s %7s %7s %7s" ) % "" % "" % "" % "" ).str();
    }
    return ( boost::format( "%7.0f %7.4f %7.4f %7.4f" ) % Equipment::getSEFD( band, 0 ) % y_.at( band ) %
             c0_.at( band ) % c1_.at( band ) )
        .str();
}
