//
// Created by mschartner on 12/6/21.
//

#ifndef VIESCHEDPP_FLUX_CONSTANT_H
#define VIESCHEDPP_FLUX_CONSTANT_H


#include "AbstractFlux.h"


namespace VieVS {

/**
 * @class Flux_constant
 * @brief model based flux information
 *
 * @author Matthias Schartner
 * @date 06.12.2021
 */
class Flux_constant : public AbstractFlux {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param wavelength wavelength in meters
     * @param flux flux density in jansky
     */
    Flux_constant( double wavelength, double flux );


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


   private:
    double flux_;  ///< flux density
};
}  // namespace VieVS


#endif  // VIESCHEDPP_FLUX_CONSTANT_H
