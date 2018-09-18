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

#ifndef VLBI_SCHEDULER_FLUX_B_H
#define VLBI_SCHEDULER_FLUX_B_H

#include "AbstractFlux.h"

namespace VieVS{
    class Flux_B : public AbstractFlux {
    public:
        Flux_B(double wavelength, const std::vector<double> &knots, const std::vector<double> &values);

        double getMaximumFlux()  const noexcept override;

        double observedFlux(double u, double v) const noexcept override;

    private:
//        Flux_B* do_clone() const override;

        std::vector<double> knots_; ///< baseline length of flux information (type B)
        std::vector<double> values_; ///< corresponding flux information for baseline length (type B)
    };
}


#endif //VLBI_SCHEDULER_FLUX_B_H
