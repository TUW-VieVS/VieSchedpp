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

#include "Satellite.h"

using namespace std;
using namespace VieVS;

Satellite::Satellite( const std::string& hdr, const std::string& l1, const std::string& l2,
                      unordered_map<std::string, std::unique_ptr<AbstractFlux>>& src_flux )
    : AbstractSource( hdr, util::simplify( hdr ), src_flux ),
      header_{ hdr },
      line1_{ l1 },
      line2_{ l2 },
      pSGP4Data_{ SGP4( Tle( hdr, l1, l2 ) ) } {}

std::vector<double> Satellite::getSourceInCrs( unsigned int time,
                                               const std::shared_ptr<const Position>& sta_pos ) const {
    auto srcRaDe = getRaDe( time, sta_pos );
    double cosDe = cos( srcRaDe.second );

    return { cosDe * cos( srcRaDe.first ), cosDe * sin( srcRaDe.first ), sin( srcRaDe.second ) };
}

pair<double, double> Satellite::calcRaDe( unsigned int time, const std::shared_ptr<const Position>& sta_pos ) const {
    DateTime currentTime = internalTime2sgpt4Time( time );
    Eci eci = pSGP4Data_.FindPosition( currentTime );
    CoordGeodetic station;
    if ( sta_pos != nullptr ) {
        station = CoordGeodetic( sta_pos->getLat(), sta_pos->getLon(), sta_pos->getAltitude()/1000., true );
    }

    Eci stat( currentTime, station );

    // difference vector between station and satellite
    Vector x_sat = eci.Position();
    Vector x_stat = stat.Position();
    Vector xd = x_sat - x_stat;

    // calculation of right ascension and declination for satellite
    double r = sqrt( xd.x * xd.x + xd.y * xd.y + xd.z * xd.z );
    double de = asin( xd.z / r );
    double ra;
    if ( eci.Position().y / r > 0 ) {
        ra = acos( xd.x / r * 1 / cos( de ) );
    } else {
        ra = 2 * pi - acos( xd.x / r * 1 / cos( de ) );
    }
    return make_pair( ra, de );
}
