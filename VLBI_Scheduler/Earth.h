/**
 * @file Earth.h
 * @brief class Earth
 *
 *
 * @author Matthias Schartner
 * @date 31.07.2017
 */

#ifndef EARTH_H
#define EARTH_H

#include <vector>

namespace VieVS {
    /**
     * @class Earth
     * @brief earth bond information required for scheduling
     *
     * @author Matthias Schartner
     * @date 31.07.2017
    */
    class Earth {
    public:
        static thread_local std::vector<double> velocity; ///< velocity of the earth, is calculated in VLBI_initializer::initializeEarth()
    };
}


#endif //EARTH_H
