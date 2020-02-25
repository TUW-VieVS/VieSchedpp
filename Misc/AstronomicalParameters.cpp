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

std::vector<double> AstronomicalParameters::earth_velocity;  ///< velocity of the earth

std::vector<double> AstronomicalParameters::earth_nutX;           ///< nutation x in one hour steps from IAU2006a model
std::vector<double> AstronomicalParameters::earth_nutY;           ///< nutation y in one hour steps from IAU2006a model
std::vector<double> AstronomicalParameters::earth_nutS;           ///< nutation s in one hour steps from IAU2006a model
std::vector<unsigned int> AstronomicalParameters::earth_nutTime;  ///< corresponding times of nut_x nut_y nut_s entries

std::vector<double> AstronomicalParameters::sun_ra;          ///< right ascension and declination of sun
std::vector<double> AstronomicalParameters::sun_dec;         ///< right ascension and declination of sun
std::vector<unsigned int> AstronomicalParameters::sun_time;  ///< right ascension and declination of sun

unsigned int AstronomicalParameters::getNutInterpolationIdx( unsigned int time ) {
    unsigned int nut_precalc_idx = 0;
    while ( AstronomicalParameters::earth_nutTime[nut_precalc_idx + 1] < time ) {
        ++nut_precalc_idx;
    }
    return nut_precalc_idx;
}

double AstronomicalParameters::getNutX( unsigned int time, unsigned int interpolationIdx ) {
    unsigned int delta = AstronomicalParameters::earth_nutTime[1] - AstronomicalParameters::earth_nutTime[0];
    unsigned int deltaTime = time - AstronomicalParameters::earth_nutTime[interpolationIdx];

    double X = AstronomicalParameters::earth_nutX[interpolationIdx] +
               ( AstronomicalParameters::earth_nutX[interpolationIdx + 1] -
                 AstronomicalParameters::earth_nutX[interpolationIdx] ) /
                   delta * deltaTime;
    return X;
}

double AstronomicalParameters::getNutY( unsigned int time, unsigned int interpolationIdx ) {
    unsigned int delta = AstronomicalParameters::earth_nutTime[1] - AstronomicalParameters::earth_nutTime[0];
    unsigned int deltaTime = time - AstronomicalParameters::earth_nutTime[interpolationIdx];

    double Y = AstronomicalParameters::earth_nutY[interpolationIdx] +
               ( AstronomicalParameters::earth_nutY[interpolationIdx + 1] -
                 AstronomicalParameters::earth_nutY[interpolationIdx] ) /
                   delta * deltaTime;
    return Y;
}

double AstronomicalParameters::getNutS( unsigned int time, unsigned int interpolationIdx ) {
    unsigned int delta = AstronomicalParameters::earth_nutTime[1] - AstronomicalParameters::earth_nutTime[0];
    unsigned int deltaTime = time - AstronomicalParameters::earth_nutTime[interpolationIdx];

    double S = AstronomicalParameters::earth_nutS[interpolationIdx] +
               ( AstronomicalParameters::earth_nutS[interpolationIdx + 1] -
                 AstronomicalParameters::earth_nutS[interpolationIdx] ) /
                   delta * deltaTime;
    return S;
}
