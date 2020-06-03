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

#include "Flux_M.h"


using namespace VieVS;
using namespace std;

double Flux_M::flcon1_{ ( pi * pi ) / ( 4.0 * 0.6931471 ) };  ///< constant precalculated value for model M
// double Flux_M::flcon2_{pi / (3600.0 * 180.0 * 1000.0)}; ///< constant precalculated value for model M

VieVS::Flux_M::Flux_M( double wavelength, std::vector<double> flux, std::vector<double> majorAxis,
                       std::vector<double> axialRatio, std::vector<double> positionAngle )
    : AbstractFlux{ wavelength },
      flux_{ std::move( flux ) },
      majorAxis_{ std::move( majorAxis ) },
      axialRatio_{ std::move( axialRatio ) },
      positionAngle_{ std::move( positionAngle ) } {}


double Flux_M::getMaximumFlux() const noexcept {
    double maxFlux = 0;

    for ( auto flux_Jy : flux_ ) {
        if ( flux_Jy > maxFlux ) {
            maxFlux = flux_Jy;
        }
    }

    return maxFlux;
}


double Flux_M::observedFlux( double u, double v ) const noexcept {
    double observedFlux = 0;

    double u_w = u / AbstractFlux::getWavelength();
    double v_w = v / AbstractFlux::getWavelength();

    for ( int i = 0; i < flux_.size(); ++i ) {
        double pa = positionAngle_[i];
        double ucospa = u_w * cos( pa );
        double usinpa = u_w * sin( pa );
        double vcospa = v_w * cos( pa );
        double vsinpa = v_w * sin( pa );

        double arg1 = ( vcospa + usinpa ) * ( vcospa + usinpa );
        double arg2 = ( axialRatio_[i] * ( ucospa - vsinpa ) ) * ( axialRatio_[i] * ( ucospa - vsinpa ) );
        double arg = -flcon1_ * ( arg1 + arg2 ) * majorAxis_[i] * majorAxis_[i];
        double f1 = flux_[i] * exp( arg );
        observedFlux += f1;
    }

    return observedFlux;
}

// Flux_M *Flux_M::do_clone() const {
//    return new Flux_M(*this);
//}
