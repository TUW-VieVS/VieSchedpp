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

#ifndef VIESCHEDPP_SATELLITE_H
#define VIESCHEDPP_SATELLITE_H

#include "AbstractSource.h"

namespace VieVS {

class Satellite : public AbstractSource {
   public:
    /**
     * @brief pre calculated parameters
     * @author Matthias Schartner
     */
    struct PreCalculated {
        unsigned int refTime_;
        std::vector<double> sourceInCrs;  ///< source vector in celestrial reference frame
    };


    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param src_name name of the source
     * @param src_name2 alternative name of source
     * @param src_flux flux information per band
     */
    Satellite( const std::string &src_name, const std::string &src_name2,
               std::unordered_map<std::string, std::unique_ptr<AbstractFlux>> &src_flux );


    const std::vector<double> &getSourceInCrs( unsigned int time ) const override;

    double getRa( unsigned int time ) const noexcept override;

    double getDe( unsigned int time ) const noexcept override;


   private:
    std::string tle0;
    std::string tle1;
    std::string tle2;

    PreCalculated preCalculated;
};
}  // namespace VieVS
#endif  // VIESCHEDPP_SATELLITE_H
