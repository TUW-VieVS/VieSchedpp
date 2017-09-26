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

const double pi = 3.141592653589793; ///< value of pi
const double twopi = 2 * pi; ///< value of 2 times pi
const double halfpi = pi / 2; ///< value of half pi

const double rad2deg = 180 / pi; ///< transforms radians to degrees
const double deg2rad = pi / 180; ///< transforms degrees to radians

const double omega = 1.00273781191135448 * twopi / 86400; ///< earth rotation velocity [rad/s]

const double speedOfLight = 299792458.0;

#endif /* CONSTANTS_H */

