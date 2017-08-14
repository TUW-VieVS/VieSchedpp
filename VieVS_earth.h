/**
 * @file VieVS_earth.h
 * @brief class VieVS_earth
 *
 *
 * @author Matthias Schartner
 * @date 31.07.2017
 */

#ifndef VIEVS_EARTH_H
#define VIEVS_EARTH_H

#include <vector>

namespace VieVS {
    class VieVS_earth {
    public:
        static std::vector<double> velocity; ///< velocity of the earth, is calculated in VLBI_initializer::initializeEarth()
    };
}


#endif //VIEVS_SCHEDULER_CLION_VIEVS_EARTH_H
