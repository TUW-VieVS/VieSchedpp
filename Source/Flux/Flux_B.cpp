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

#include "Flux_B.h"

VieVS::Flux_B::Flux_B( double wavelength, const std::vector<double> &knots, const std::vector<double> &values )
    : AbstractFlux{wavelength}, knots_{knots}, values_{values} {}

double VieVS::Flux_B::getMaximumFlux() const noexcept {
    double maxFlux = 0;

    for ( auto flux_Jy : values_ ) {
        if ( flux_Jy > maxFlux ) {
            maxFlux = flux_Jy;
        }
    }

    return maxFlux;
}

double VieVS::Flux_B::observedFlux( double u, double v ) const noexcept {
    double observedFlux = 0;
    double pbase = sqrt( u * u + v * v ) / 1000.0;

    for ( int i = 1; i < knots_.size(); ++i ) {
        if ( knots_[i] > pbase ) {
            observedFlux = values_[i - 1];
            break;
        }
    }

    return observedFlux;
}
