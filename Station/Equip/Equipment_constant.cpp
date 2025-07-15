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

/*
 * File:   equip.cpp
 * Author: mschartn
 *
 * Created on June 27, 2017, 11:54 AM
 */

#include "Equipment_constant.h"


using namespace std;
using namespace VieVS;

Equipment_constant::Equipment_constant( unordered_map<string, double> SEFDs )
    : AbstractEquipment(), SEFD_{ std::move( SEFDs ) } {}


double Equipment_constant::getMaxSEFD() const noexcept {
    double maxSEFD = 0;
    for ( auto &any : SEFD_ ) {
        if ( any.second > maxSEFD ) {
            maxSEFD = any.second;
        }
    }
    return maxSEFD;
}


std::string Equipment_constant::shortSummary( const std::string &band ) const noexcept {
    if ( SEFD_.find( band ) == SEFD_.end() ) {
        return ( boost::format( "%7s %7s %7s %7s" ) % "---" % "---" % "---" % "---" ).str();
    }
    return ( boost::format( "%7.0f %7s %7s %7s" ) % SEFD_.at( band ) % "---" % "---" % "---" ).str();
}

std::string Equipment_constant::sefd_skdFormat() const noexcept {
    std::vector<std::string> bands;
    bands.reserve( SEFD_.size() );  // Optional but more efficient
    for ( const auto& entry : SEFD_ ) {
        bands.push_back( entry.first );
    }

    // Step 2: Sort the keys
    std::sort( bands.begin(), bands.end() );

    // Step 3: Build the formatted string using sorted keys
    std::string o;
    for ( const auto& band : bands ) {
        o.append( ( boost::format( "%s %6.0f " ) % band % SEFD_.at( band ) ).str() );
    }
    return o;
}
