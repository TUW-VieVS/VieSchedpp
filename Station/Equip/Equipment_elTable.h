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
 * @file Equipment_elModel.h
 * @brief class Equipment_elModel
 *
 * @author Matthias Schartner
 * @date 12.04.2018
 */

#ifndef EQUIPMENT_ELTABLE_H
#define EQUIPMENT_ELTABLE_H


#include "AbstractEquipment.h"


namespace VieVS {

/**
 * @class Equipment_elModel
 * @brief representation of elevation dependent VLBI equipment
 *
 * @author Matthias Schartner
 * @date 12.04.2018
 */
class Equipment_elTable : public AbstractEquipment {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * Elevation dependent SEFD values represented through lookup table.
     *
     * @param elevations elevation angle knots
     * @param SEFD SEFD per band - key is band name, value is SEFD
     */
    Equipment_elTable( std::unordered_map<std::string, std::vector<double>> elevation,
                       std::unordered_map<std::string, std::vector<double>> SEFD );


    /**
     * @brief getter function for antenna SEFD information
     * @author Matthias Schartner
     *
     * @param band name of band
     * @param el elevation
     * @return SEFD of this band
     */
    double getSEFD( const std::string &band, double el ) const noexcept override;

    /**
     * @brief returns maximum SEFD of this antenna
     * @author Matthias Schartner
     *
     * @return maximum SEFD of this antenna
     */
    double getMaxSEFD() const noexcept override;

    /**
     * @brief creates a short summary of SEFD parameters
     * @author Matthias Schartner
     *
     * @param band band name
     * @return short summary of SEFD parameters
     */
    std::string shortSummary( const std::string &band ) const noexcept override;
    /**
     * @brief create $STATIONS SEFD summary in .skd format
     * @author Matthis Schartner
     *
     * @return string of SEFD summary
     */
    std::string sefd_skdFormat() const noexcept override;

    /**
     * @brief create $STATIONS elevation dependent SEFD summary in .skd format
     * @author Matthis Schartner
     *
     * @return string of elevation dependent SEFD summary summary
     */
    std::string elevationDependence_skdFormat() const noexcept override { return ""; };

   private:
    std::unordered_map<std::string, std::vector<double>> el_;    ///< elevation angle
    std::unordered_map<std::string, std::vector<double>> SEFD_;  ///< corresponding SEFD value
};
}  // namespace VieVS

#endif  // EQUIPMENT_ELTABLE_H
