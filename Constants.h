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

#include <vector>

constexpr double pi = 3.141592653589793; ///< value of pi
constexpr double twopi = 2 * pi; ///< value of 2 times pi
constexpr double halfpi = pi / 2; ///< value of half pi

constexpr double rad2deg = 180 / pi; ///< transforms radians to degrees
constexpr double deg2rad = pi / 180; ///< transforms degrees to radians

constexpr double omega = 1.00273781191135448 * twopi / 86400; ///< earth rotation velocity [rad/s]

constexpr double speedOfLight = 299792458.0;

#endif /* CONSTANTS_H */

