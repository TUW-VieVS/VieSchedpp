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
 * @file AbstractFlux.h
 * @brief class AbstractFlux
 *
 * @author Matthias Schartner
 * @date 28.06.2017
 */

#ifndef FLUX_H
#define FLUX_H


#include <boost/lexical_cast.hpp>
#include <iostream>
#include <vector>

#include "../../Misc/Constants.h"
#include "../../Misc/VieVS_Object.h"


namespace VieVS {
/**
 * @class AbstractFlux
 * @brief representation of source flux
 *
 * @author Matthias Schartner
 * @date 28.06.2017
 *
 * This class serves as the base class for all flux implementations.
 *
 */
class AbstractFlux : public VieVS_Object {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param wavelength wavelength of flux information in meters
     */
    explicit AbstractFlux( double wavelength );


    /**
     * @brief virtual destructor
     * @author Matthias Schartner
     */
    virtual ~AbstractFlux() = default;


    /**
     * @brief maximum possible flux density
     * @author Matthias Schartner
     *
     * @return maximum possible flux density
     */
    virtual double getMaximumFlux() const noexcept = 0;


    /**
     * @brief flux density based on uv
     * @author Matthias Schartner
     *
     * @param u projected baseline length
     * @param v projected baseline length
     * @return flux density for this constellation
     */
    virtual double observedFlux( double u, double v ) const noexcept = 0;

    /**
     * @brief returns true if flux model needs UV information to calculate flux density
     * @author Matthias Schartner
     *
     * can be used for constant flux models to avoid calculating uv
     *
     * @return true if model needs uv information, otherwise false
     */
    virtual bool needsUV() const noexcept = 0;

    /**
     * @brief getter for wavelength
     * @author Matthias Schartner
     *
     * @return wavelength in meters
     */
    double getWavelength() const { return wavelength_; }


   private:
    static unsigned long nextId;  ///< next id for this object type

    double wavelength_;  ///< wavelength of the band whose flux information is stored
};

}  // namespace VieVS
#endif /* FLUX_H */
