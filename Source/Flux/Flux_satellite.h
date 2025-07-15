//
// Created by mschartner on 2/19/25.
//

#ifndef VIESCHEDPP_FLUX_SATELLITE_H
#define VIESCHEDPP_FLUX_SATELLITE_H

#include "AbstractFlux.h"

namespace VieVS {

/**
 * @class Flux_constant
 * @brief model based flux information
 *
 * @author Matthias Schartner
 * @date 06.12.2021
 */
class Flux_satellite : public AbstractFlux {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param wavelength wavelength in meters
     * @param flux flux density in jansky
     */
    Flux_satellite( double wavelength, double flux, double db_loss );


    /**
     * @brief maximum possible flux density
     * @author Matthias Schartner
     *
     * @return maximum possible flux density in Jansky
     */
    double getMaximumFlux() const noexcept override { return flux_; };


    /**
     * @brief observed flux density
     * @author Matthias Schartner
     *
     * @param u projected baseline length u (unused)
     * @param v projected baseline length v (unused)
     * @return observed flux density in jansky
     */
    double observedFlux( double u, double v ) const noexcept override { return flux_; };

    /**
     * @brief returns true if flux model needs UV information to calculate flux density
     * @author Matthias Schartner
     *
     * @return false
     */
    bool needsUV() const noexcept override { return false; };


    /**
     * @brief returns true if flux model needs elevation and distance information to calculate flux density
     * @author Matthias Schartner
     *
     * @return true
     */
    bool needsElDist() const noexcept override { return true; };

    /**
     * @brief observed flux density
     * @author Matthias Schartner
     *
     * @param el elevation
     * @param dist distance
     * @return observed flux density in jansky
     */
    double observedFluxElDist( double el, double dist ) const noexcept override;

   private:
    double flux_;     ///< flux density
    double db_loss_;  ///< db loss at 5 degree elevation
};
}  // namespace VieVS


#endif  // VIESCHEDPP_FLUX_SATELLITE_H
