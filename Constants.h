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
 * @file VieVS_constants.h
 * @brief constants used in this program
 *
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

constexpr double pi = 3.141592653589793; ///< value of pi
constexpr double twopi = 2 * pi; ///< value of 2 times pi
constexpr double halfpi = pi / 2; ///< value of half pi

constexpr double rad2deg = 180 / pi; ///< transforms radians to degrees
constexpr double deg2rad = pi / 180; ///< transforms degrees to radians

constexpr double omega = 1.00273781191135448 * twopi / 86400; ///< earth rotation velocity [rad/s]

constexpr double speedOfLight = 299792458.0;

#endif /* CONSTANTS_H */

