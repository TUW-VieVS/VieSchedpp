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

#ifndef EQUIPMENT_ELDEPENDENT_H
#define EQUIPMENT_ELDEPENDENT_H


#include "AbstractEquipment.h"


namespace VieVS {

/**
 * @class Equipment_elModel
 * @brief representation of elevation dependent VLBI equipment
 *
 * @author Matthias Schartner
 * @date 12.04.2018
 */
class Equipment_elModel : public AbstractEquipment {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param SEFDs SEFD per band - key is band name, value is SEFD
     * @param SEFD_y elevation dependent SEFD parameter "y" per band - key is band name, value is parameter
     * @param SEFD_c0 elevation dependent SEFD parameter "c1" per band - key is band name, value is parameter
     * @param SEFD_c1 elevation dependent SEFD parameter "c2" per band - key is band name, value is parameter
     */
    Equipment_elModel( std::unordered_map<std::string, double> SEFDs, std::unordered_map<std::string, double> SEFD_y,
                       std::unordered_map<std::string, double> SEFD_c0,
                       std::unordered_map<std::string, double> SEFD_c1 );


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
    std::string elevationDependence_skdFormat() const noexcept override;

   private:
    std::unordered_map<std::string, double> SEFDs_;  ///< SEFD parameters
    std::unordered_map<std::string, double> y_;      ///< elevation dependent SEFD parameter "y"
    std::unordered_map<std::string, double> c0_;     ///< elevation dependent SEFD parameter "c0"
    std::unordered_map<std::string, double> c1_;     ///< elevation dependent SEFD parameter "c1"
};
}  // namespace VieVS

#endif  // EQUIPMENT_ELDEPENDENT_H
