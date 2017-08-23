/**
 * @file VLBI_flux.h
 * @brief class VLBI_flux
 *
 *
 * @author Matthias Schartner
 * @date 28.06.2017
 */

#ifndef VLBI_FLUX_H
#define VLBI_FLUX_H
#include <vector>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include "VieVS_constants.h"

using namespace std;
namespace VieVS{

    // TODO: change this flux information into two classes for each model B and M
    class VLBI_flux {
    public:
        /**
         * @brief all possible flux information type
         */
        enum class fluxType {
            B, ///< baseline length / flux pairs
            M, ///< model parameters
            undefined ///< represents missing information
        };

        /**
         * @brief empty default constructor
         */
        VLBI_flux();

        /**
         * @brief constructor
         *
         * @param stype type of flux information (B or M)
         */
        VLBI_flux(string stype);

        /**
         * @brief adds flux information to object
         *
         * @param parameters all parameters spezified in flux.cat line
         * @return true if flux parameters could be used, false if something went wrong
         */
        bool addFluxParameters(const vector<string> &parameters);

        /**
         * @brief sets the wavelength of the band whose flux information is stored
         *
         * @param wavelength wavelength in meters
         */
        void setWavelength(double wavelength) {
            VLBI_flux::wavelength = wavelength;
        }

        /**
         * @brief gets the maximum possible flux density
         *
         * This function is used at the beginning to junk out sources which do not fit the minimum requested flux
         * density.
         *
         * @return maximum possible flux density
         */
        double getMaximumFlux() const;

        /**
         * @brief destructor
         */
        virtual ~VLBI_flux();

        friend ostream& operator<<(ostream& out, const VLBI_flux& src);

        /**
         * @brief calculates the flux density for a constellation
         *
         * @param u projected baseline length
         * @param v projected baseline length
         * @return flux density fot this constellation
         */
        double getFlux(double u, double v) const;

    private:

        double wavelength; ///< wavelength of the band whose flux information is stored

        fluxType type; ///< flux information type (B or M)

        vector<double> knots; ///< baseline length of flux information (type B)
        vector<double> values; ///< corresponding flux information for baseline length (type B)

        vector<double> flux; ///< flux density (type M)
        vector<double> majorAxis; ///< major axis angle (type M)
        vector<double> axialRatio; ///< axial ratio (type M)
        vector<double> positionAngle; ///< position angle (type M)


        //TODO: make this static
        double flcon1{(pi * pi) / (4.0 * 0.6931471)}; ///< constant precalculated value for model M
        double flcon2{pi / (3600.0 * 180.0 * 1000.0)}; ///< constant precalculated value for model M
    };

}
#endif /* VLBI_FLUX_H */

