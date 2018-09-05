/**
 * @file Flux.h
 * @brief class Flux
 *
 *
 * @author Matthias Schartner
 * @date 28.06.2017
 */

#ifndef FLUX_H
#define FLUX_H
#include <vector>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include "Constants.h"
#include "VieVS_Object.h"

namespace VieVS{
    /**
     * @class Flux
     * @brief representation of source flux
     *
     * @author Matthias Schartner
     * @date 28.06.2017
     */
    class AbstractFlux : public VieVS_Object {
    public:

        explicit AbstractFlux(double wavelength);

        virtual ~AbstractFlux() = default;

        /**
         * @brief gets the maximum possible flux density
         *
         * This function is used at the beginning to junk out sources which do not fit the minimum requested flux
         * density.
         *
         * @return maximum possible flux density
         */
        virtual double getMaximumFlux() const noexcept = 0;

        /**
         * @brief calculates the flux density for a constellation
         *
         * @param u projected baseline length
         * @param v projected baseline length
         * @return flux density fot this constellation
         */
        virtual double observedFlux(double u, double v) const noexcept = 0;


        double getWavelength() const {
            return wavelength_;
        }

//        std::unique_ptr<Flux> clone() const;


    private:
//        virtual Flux* do_clone() const = 0;

        static unsigned long nextId;

        double wavelength_; ///< wavelength of the band whose flux information is stored
    };

}
#endif /* FLUX_H */

