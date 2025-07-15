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

/**
 * @file Flux_M.h
 * @brief class Flux_M
 *
 * @author Matthias Schartner
 * @date 12.04.2018
 *
 */

#ifndef VLBI_SCHEDULER_FLUX_M_H
#define VLBI_SCHEDULER_FLUX_M_H


#include "AbstractFlux.h"


namespace VieVS {

/**
 * @class Flux_M
 * @brief model based flux information
 *
 * @author Matthias Schartner
 * @date 12.04.2018
 */
class Flux_M : public AbstractFlux {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param wavelength wavelength in meters
     * @param flux flux density in jansky
     * @param majorAxis major axis angles
     * @param axialRatio axial ratios
     * @param positionAngle position angles
     */
    Flux_M( double wavelength, std::vector<double> flux, std::vector<double> majorAxis, std::vector<double> axialRatio,
            std::vector<double> positionAngle );


    /**
     * @brief maximum possible flux density
     * @author Matthias Schartner
     *
     * @return maximum possible flux density in Jansky
     */
    double getMaximumFlux() const noexcept override;


    /**
     * @brief observed flux density
     * @author Matthias Schartner
     *
     * @param u projected baseline length u
     * @param v projected baseline length v
     * @return observed flux density in jansky
     */
    double observedFlux( double u, double v ) const noexcept override;

    /**
     * @brief observed flux density
     * @author Matthias Schartner
     *
     * @param el elevation
     * @param dist distance
     * @return observed flux density in jansky
     */
    double observedFluxElDist( double el, double dist ) const noexcept override { return 0; }


    /**
     * @brief returns true if flux model needs UV information to calculate flux density
     * @author Matthias Schartner
     *
     * @return true
     */
    bool needsUV() const noexcept override { return true; };


    /**
     * @brief returns true if flux model needs elevation and distance information to calculate flux density
     * @author Matthias Schartner
     *
     * @return false
     */
    bool needsElDist() const noexcept override { return false; };

   private:
    std::vector<double> flux_;           ///< flux density
    std::vector<double> majorAxis_;      ///< major axis angle
    std::vector<double> axialRatio_;     ///< axial ratio
    std::vector<double> positionAngle_;  ///< position angle

    static double flcon1_;  ///< constant precalculated value
    //        static double flcon2_; ///< constant precalculated value
};
}  // namespace VieVS

#endif  // VLBI_SCHEDULER_FLUX_M_H
