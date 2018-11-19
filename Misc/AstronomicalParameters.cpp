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
 
#include "AstronomicalParameters.h"
using namespace VieVS;

std::vector<double> AstronomicalParameters::earth_velocity; ///< velocity of the earth

std::vector<double> AstronomicalParameters::earth_nutX; ///< nutation x in one hour steps from IAU2006a model
std::vector<double> AstronomicalParameters::earth_nutY; ///< nutation y in one hour steps from IAU2006a model
std::vector<double> AstronomicalParameters::earth_nutS; ///< nutation s in one hour steps from IAU2006a model
std::vector<unsigned int> AstronomicalParameters::earth_nutTime; ///< corresponding times of nut_x nut_y nut_s entries

std::vector<double> AstronomicalParameters::sun_radc; ///< right ascension and declination of sun
