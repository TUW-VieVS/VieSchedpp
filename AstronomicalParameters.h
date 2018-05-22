//
// Created by matth on 21.05.2018.
//

#ifndef ASTRONOMICALPARAMETERS_H
#define ASTRONOMICALPARAMETERS_H

#include <vector>

namespace VieVS{
    class AstronomicalParameters {
    public:
        static thread_local std::vector<double> earth_velocity; ///< velocity of the earth

        static thread_local std::vector<double> earth_nutX; ///< nutation x in one hour steps from IAU2006a model
        static thread_local std::vector<double> earth_nutY; ///< nutation y in one hour steps from IAU2006a model
        static thread_local std::vector<double> earth_nutS; ///< nutation s in one hour steps from IAU2006a model
        static thread_local std::vector<unsigned int> earth_nutTime; ///< corresponding times of nut_x nut_y nut_s entries

        static thread_local std::vector<double> sun_radc; ///< right ascension and declination of sun

    };
}


#endif //ASTRONOMICALPARAMETERS_H
