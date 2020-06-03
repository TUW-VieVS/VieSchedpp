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

#include "Equipment.h"


using namespace std;
using namespace VieVS;

unsigned long Equipment::nextId = 0;


Equipment::Equipment( unordered_map<string, double> SEFDs ) : VieVS_Object( nextId++ ), SEFD_{ std::move( SEFDs ) } {}


double Equipment::getMaxSEFD() const noexcept {
    double maxSEFD = 0;
    for ( auto &any : SEFD_ ) {
        if ( any.second > maxSEFD ) {
            maxSEFD = any.second;
        }
    }
    return maxSEFD;
}


std::string Equipment::shortSummary( const std::string &band ) const noexcept {
    if ( SEFD_.find( band ) == SEFD_.end() ) {
        return ( boost::format( "%7s %7s %7s %7s" ) % "---" % "---" % "---" % "---" ).str();
    }
    return ( boost::format( "%7.0f %7s %7s %7s" ) % SEFD_.at( band ) % "---" % "---" % "---" ).str();
}
