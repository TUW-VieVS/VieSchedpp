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

#include "Equipment_elModel.h"


using namespace VieVS;
using namespace std;


Equipment_elModel::Equipment_elModel( std::unordered_map<std::string, double> SEFDs,
                                      std::unordered_map<std::string, double> SEFD_y,
                                      std::unordered_map<std::string, double> SEFD_c0,
                                      std::unordered_map<std::string, double> SEFD_c1 )
    : AbstractEquipment(),
      SEFDs_{ std::move( SEFDs ) },
      y_{ std::move( SEFD_y ) },
      c0_{ std::move( SEFD_c0 ) },
      c1_{ std::move( SEFD_c1 ) } {}


double Equipment_elModel::getSEFD( const std::string &band, double el ) const noexcept {
    if ( SEFDs_.at( band ) == 0 ) {
        return 0;
    }


    double y = y_.at( band );
    double c0 = c0_.at( band );
    double c1 = c1_.at( band );

    double tmp = pow( sin( el ), y );
    double tmp2 = c0 + c1 / tmp;

    if ( tmp2 < 1 ) {
        return SEFDs_.at( band );
    } else {
        return SEFDs_.at( band ) * tmp2;
    }
}


std::string Equipment_elModel::shortSummary( const std::string &band ) const noexcept {
    if ( y_.find( band ) == y_.end() ) {
        return ( boost::format( "%7s %7s %7s %7s" ) % "---" % "---" % "---" % "---" ).str();
    }
    return ( boost::format( "%7.0f %7.4f %7.4f %7.4f" ) % SEFDs_.at( band ) % y_.at( band ) % c0_.at( band ) %
             c1_.at( band ) )
        .str();
}


double Equipment_elModel::getMaxSEFD() const noexcept {
    double maxSEFD = 0;
    for ( auto &any : SEFDs_ ) {
        if ( any.second > maxSEFD ) {
            maxSEFD = any.second;
        }
    }
    return maxSEFD;
}

std::string Equipment_elModel::sefd_skdFormat() const noexcept {
    string o;
    for ( const auto &any : SEFDs_ ) {
        o.append( ( boost::format( "%s %6.0f " ) % any.first % any.second ).str() );
    }
    return o;
}
std::string Equipment_elModel::elevationDependence_skdFormat() const noexcept {
    string o;
    for ( const auto &any : SEFDs_ ) {
        string band = any.first;
        double y = y_.at( band );
        double c0 = c0_.at( band );
        double c1 = c1_.at( band );
        o.append( ( boost::format( "%s %.4f %.4f %.4f " ) % band % y % c0 % c1 ).str() );
    }
    return o;
}
