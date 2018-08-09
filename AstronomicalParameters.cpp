//
// Created by matth on 21.05.2018.
//

#include "AstronomicalParameters.h"
using namespace VieVS;

std::vector<double> AstronomicalParameters::earth_velocity; ///< velocity of the earth

std::vector<double> AstronomicalParameters::earth_nutX; ///< nutation x in one hour steps from IAU2006a model
std::vector<double> AstronomicalParameters::earth_nutY; ///< nutation y in one hour steps from IAU2006a model
std::vector<double> AstronomicalParameters::earth_nutS; ///< nutation s in one hour steps from IAU2006a model
std::vector<unsigned int> AstronomicalParameters::earth_nutTime; ///< corresponding times of nut_x nut_y nut_s entries

std::vector<double> AstronomicalParameters::sun_radc; ///< right ascension and declination of sun