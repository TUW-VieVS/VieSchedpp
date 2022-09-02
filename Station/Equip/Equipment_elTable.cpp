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

#include "Equipment_elTable.h"

#include <utility>

using namespace std;
using namespace VieVS;

Equipment_elTable::Equipment_elTable( std::unordered_map<std::string, std::vector<double>> elevation,
                                      std::unordered_map<std::string, std::vector<double>> SEFD )
    : AbstractEquipment(), el_{ std::move( elevation ) }, SEFD_{ std::move( SEFD ) } {}


double Equipment_elTable::getSEFD( const string& band, double el ) const noexcept {
    if ( el_.find( band ) != el_.end() ) {
        const auto& tel = el_.at( band );
        const auto& tSEFD = SEFD_.at( band );

        if ( el <= tel.front() ) {
            return tSEFD[0];
        }
        if ( el >= tel.back() ) {
            return tSEFD.back();
        }

        unsigned int idx = 1;
        while ( el >= tel[idx] ) {
            ++idx;
        }
        double dy = tSEFD[idx] - tSEFD[idx - 1];
        double dx = ( el - tel[idx - 1] ) / ( tel[idx] - tel[idx - 1] );
        double y_ = tSEFD[idx - 1] + dy * dx;
        return y_;
    } else {
        return 999999999;
    }
    return 999999999;
}

std::string Equipment_elTable::shortSummary( const string& band ) const noexcept {
    if ( SEFD_.find( band ) == SEFD_.end() ) {
        return ( boost::format( "%7s %7s %7s %7s" ) % "---" % "---" % "---" % "---" ).str();
    }
    return ( boost::format( "%7s %7s %7s %7s" ) % "TABLE" % "---" % "---" % "---" ).str();
}

double Equipment_elTable::getMaxSEFD() const noexcept {
    double max = 0;
    for ( const auto& any : SEFD_ ) {
        for ( double v : any.second ) {
            if ( v > max ) {
                max = v;
            }
        }
    }
    if ( max == 0 ) {
        max = 99999;
    }
    return max;
}
std::string Equipment_elTable::sefd_skdFormat() const noexcept {
    string o;
    for ( const auto& any : SEFD_ ) {
        o.append( ( boost::format( "%s %6.0f " ) % any.first % any.second[0] ).str() );
    }
    return o;
}
