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

#include "HorizonMask_line.h"


using namespace std;
using namespace VieVS;


HorizonMask_line::HorizonMask_line( const vector<double> &azimuths, const vector<double> &elevations )
    : azimuth_{ azimuths }, elevation_{ elevations } {}


bool HorizonMask_line::visible( const PointingVector &pv ) const noexcept {
    double az = pv.getAz();
    az = fmod( az, twopi );
    if ( az < 0 ) {
        az += twopi;
    }

    double el = pv.getEl();
    double el_mask = az2el( az );

    return el >= el_mask;
}


string HorizonMask_line::vexOutput() const noexcept { return string(); }


std::pair<std::vector<double>, std::vector<double>> HorizonMask_line::getHorizonMask() const noexcept {
    vector<double> az_;
    vector<double> el_;

    for ( int az = 0; az <= 360; az += 1 ) {
        double azrad = static_cast<double>( az ) * deg2rad;
        double el = az2el( azrad );

        az_.push_back( azrad );
        el_.push_back( el );
    }

    return { az_, el_ };
}


double HorizonMask_line::az2el( double az ) const noexcept {
    unsigned long i = 1;
    while ( az > azimuth_.at( i ) ) {
        ++i;
    }
    unsigned long begin = i - 1;
    unsigned long end = i;
    double delta = az - azimuth_.at( begin );
    return elevation_.at( begin ) +
           ( elevation_.at( end ) - elevation_.at( begin ) ) / ( azimuth_.at( end ) - azimuth_.at( begin ) ) * delta;
}
