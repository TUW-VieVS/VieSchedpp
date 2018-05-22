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

namespace VieVS{
    /**
     * @class Flux
     * @brief representation of source flux
     *
     * @author Matthias Schartner
     * @date 28.06.2017
     */
    class Flux {
    public:
        /**
         * @brief all possible flux information type
         */
        enum class FluxType {
            B, ///< baseline length / flux pairs
            M, ///< model parameters
            undefined ///< represents missing information
        };

        /**
         * @brief empty default constructor
         */
        Flux();

        /**
         * @brief constructor
         *
         * @param stype type of flux information (B or M)
         */
        Flux(std::string stype);


        /**
         * @brief adds flux information to object
         *
         * @param parameters all parameters spezified in flux.cat line
         * @return true if flux parameters could be used, false if something went wrong
         */
        bool addFluxParameters(const std::vector<std::string> &parameters) noexcept;

        /**
         * @brief sets the wavelength of the band whose flux information is stored
         *
         * @param wavelength wavelength in meters
         */
        void setWavelength(double wavelength) noexcept {
            Flux::wavelength_ = wavelength;
        }

        /**
         * @brief gets the maximum possible flux density
         *
         * This function is used at the beginning to junk out sources which do not fit the minimum requested flux
         * density.
         *
         * @return maximum possible flux density
         */
        double getMaximumFlux() const noexcept;


        /**
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param src flux information that should be printed to stream
         * @return stream object
         */
        friend std::ostream &operator<<(std::ostream &out, const Flux &src) noexcept;

        /**
         * @brief calculates the flux density for a constellation
         *
         * @param u projected baseline length
         * @param v projected baseline length
         * @return flux density fot this constellation
         */
        double observedFlux(double u, double v) const noexcept;

    private:

        double wavelength_; ///< wavelength of the band whose flux information is stored

        FluxType type_; ///< flux information type (B or M)

        std::vector<double> knots_; ///< baseline length of flux information (type B)
        std::vector<double> values_; ///< corresponding flux information for baseline length (type B)

        std::vector<double> flux_; ///< flux density (type M)
        std::vector<double> majorAxis_; ///< major axis angle (type M)
        std::vector<double> axialRatio_; ///< axial ratio (type M)
        std::vector<double> positionAngle_; ///< position angle (type M)


        //TODO: make this static
        static double flcon1_; ///< constant precalculated value for model M
        static double flcon2_; ///< constant precalculated value for model M
    };

}
#endif /* FLUX_H */

