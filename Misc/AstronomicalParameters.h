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
* @file AstronomicalParameters.h
* @brief class AstronomicalParameters
*
* @author Matthias Schartner
* @date 22.05.2018
*/

#ifndef ASTRONOMICALPARAMETERS_H
#define ASTRONOMICALPARAMETERS_H

#include <vector>

namespace VieVS{

    /**
     * @class AstronomicalParameters
     * @brief astronomical parameters needed for scheduling
     *
     * serves as a lookup table for some astronomical parameters
     *
     * @author Matthias Schartner
     * @date 22.05.2018
     */
    class AstronomicalParameters {
    public:
        static std::vector<double> earth_velocity; ///< velocity of the earth in m/s

        static std::vector<double> earth_nutX; ///< nutation x in one hour steps from IAU2006a model
        static std::vector<double> earth_nutY; ///< nutation y in one hour steps from IAU2006a model
        static std::vector<double> earth_nutS; ///< nutation s in one hour steps from IAU2006a model
        static std::vector<unsigned int> earth_nutTime; ///< corresponding times of nut_x nut_y nut_s entries

        static std::vector<double> sun_radc; ///< right ascension and declination of sun in radians

    };
}


#endif //ASTRONOMICALPARAMETERS_H
