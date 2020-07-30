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
 * @file SourceList.h
 * @brief class SourceList
 *
 * @author Matthias Schartner
 * @date 29.07.2020
 */

#ifndef VIESCHEDPP_QUASAR_H
#define VIESCHEDPP_QUASAR_H

#include "AbstractSource.h"

namespace VieVS {

class Quasar : public AbstractSource {
   public:
    /**
     * @brief pre calculated parameters
     * @author Matthias Schartner
     */
    struct PreCalculated {
        std::vector<double> sourceInCrs;  ///< source vector in celestrial reference frame
    };

    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param src_name name of the source
     * @param src_name2 alternative name of source
     * @param src_ra_deg right ascension in degrees
     * @param src_de_deg declination in degrees
     * @param src_flux flux information per band
     */
    Quasar( const std::string &src_name, const std::string &src_name2, double src_ra_deg, double src_de_deg,
            std::unordered_map<std::string, std::unique_ptr<AbstractFlux>> &src_flux );

    /**
     * @brief getter for declination
     * @author Matthias Schartner
     *
     * @return declination of the source in radians
     */
    double getDe( unsigned int time ) const noexcept override { return de_; };

    double getDe() const noexcept { return de_; };

    /**
     * @brief getter for right ascension
     * @author Matthias Schartner
     *
     * @return right ascension of the source in radians
     */
    double getRa( unsigned int time ) const noexcept override { return ra_; };

    double getRa() const noexcept { return ra_; };

    double getSinDe( unsigned int time ) const noexcept { return sinDe_; };

    double getCosDe( unsigned int time ) const noexcept { return cosDe_; };


    /**
     * @brief get source position in CRS
     * @author Matthias Schartner
     *
     * @return source position vector
     */
    const std::vector<double> &getSourceInCrs( unsigned int time ) const override {
        return preCalculated_->sourceInCrs;
    }

   private:
    double ra_;     ///< source right ascension
    double de_;     ///< source declination
    double sinDe_;  ///< sine of declination
    double cosDe_;  ///< cosine of declination


    std::shared_ptr<PreCalculated> preCalculated_;  ///< pre calculated values
};

}  // namespace VieVS
#endif  // VIESCHEDPP_QUASAR_H
