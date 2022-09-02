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
 * @file Equipment_constant.h
 * @brief class Equipment_constant
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 */

#ifndef EQUIPMENT_H
#define EQUIPMENT_H


#include <boost/format.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "AbstractEquipment.h"


namespace VieVS {
/**
 * @class Equipment_constant
 * @brief representation of VLBI equipment
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 */
class Equipment_constant : public AbstractEquipment {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param SEFDs SEFD per band - key is band name, value is SEFD
     */
    explicit Equipment_constant( std::unordered_map<std::string, double> SEFDs );


    /**
     * @brief getter function for antenna SEFD information
     * @author Matthias Schartner
     *
     * @param band name of band
     * @param el elevation
     * @return SEFD of this band
     */
    double getSEFD( const std::string &band, double el ) const noexcept override {
        if ( SEFD_.find( band ) != SEFD_.end() ) {
            return SEFD_.at( band );
        } else {
            return 0;
        }
    };

    //    /**
    //     *
    //     */
    //    std::vector<std::string> getBands() const noexcept override {
    //        std::vector<std::string> v;
    //        for ( const auto &any : SEFD_ ) {
    //            v.push_back( any.first );
    //        }
    //        return v;
    //    }

    /**
     * @brief returns maximum SEFD of this antenna
     * @author Matthias Schartner
     *
     * @return maximum SEFD of this antenna
     */
    double getMaxSEFD() const noexcept override;


    /**
     * @brief creates a short summary of SEFD parameters
     * @author Matthis Schartner
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
    std::unordered_map<std::string, double> SEFD_;  ///< SEFD information per band
};
}  // namespace VieVS
#endif /* EQUIPMENT_H */
