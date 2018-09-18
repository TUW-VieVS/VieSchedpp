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

#ifndef VLBI_SCHEDULER_FLUX_M_H
#define VLBI_SCHEDULER_FLUX_M_H

#include "AbstractFlux.h"
namespace VieVS {
    class Flux_M : public AbstractFlux {
    public:
        Flux_M(double wavelength, const std::vector<double> &flux, const std::vector<double> &majorAxis,
               const std::vector<double> &axialRatio, const std::vector<double> &positionAngle);

        double getMaximumFlux() const noexcept override;

        double observedFlux(double u, double v) const noexcept override;

    private:
//        Flux_M* do_clone() const override;

        std::vector<double> flux_; ///< flux density (type M)
        std::vector<double> majorAxis_; ///< major axis angle (type M)
        std::vector<double> axialRatio_; ///< axial ratio (type M)
        std::vector<double> positionAngle_; ///< position angle (type M)

        static double flcon1_; ///< constant precalculated value for model M
//        static double flcon2_; ///< constant precalculated value for model M

    };
}

#endif //VLBI_SCHEDULER_FLUX_M_H
