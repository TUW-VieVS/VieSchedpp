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
 * @file Equipment.h
 * @brief class Equipment
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
#include "../../Misc/VieVS_Object.h"


namespace VieVS {
/**
 * @class Equipment
 * @brief representation of VLBI equipment
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 */
class Equipment : public VieVS_Object {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param SEFDs SEFD per band - key is band name, value is SEFD
     */
    explicit Equipment( std::unordered_map<std::string, double> SEFDs );


    /**
     * @brief getter function for antenna SEFD information
     * @author Matthias Schartner
     *
     * @param band name of band
     * @param el elevation
     * @return SEFD of this band
     */
    virtual double getSEFD( const std::string &band, double el ) const noexcept {
        if ( SEFD_.find( band ) != SEFD_.end() ) {
            return SEFD_.at( band );
        } else {
            return 0;
        }
    };

    /**
     *
     */
    const std::vector<std::string> getBands() const noexcept {
        std::vector<std::string> v;
        for ( const auto &any : SEFD_ ) {
            v.push_back( any.first );
        }
        return v;
    }

    /**
     * @brief returns maximum SEFD of this antenna
     * @author Matthias Schartner
     *
     * @return maximum SEFD of this antenna
     */
    double getMaxSEFD() const noexcept;


    /**
     * @brief creates a short summary of SEFD parameters
     * @author Matthis Schartner
     *
     * @param band band name
     * @return short summary of SEFD parameters
     */
    virtual std::string shortSummary( const std::string &band ) const noexcept;

   private:
    static unsigned long nextId;  ///< next id for this object type

    std::unordered_map<std::string, double> SEFD_;  ///< SEFD information per band
};
}  // namespace VieVS
#endif /* EQUIPMENT_H */
