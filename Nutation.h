/**
 * @file Nutation.h
 * @brief class Nutation
 *
 *
 * @author Matthias Schartner
 * @date 31.07.2017
 */

#ifndef NUTATION_H
#define NUTATION_H

#include <vector>

namespace VieVS {
    /**
     * @class Nutation
     * @brief stored nutation parameters in one hour steps
     *
     * This values can be used for interpolation. The iauXys06a model from the IAU SOFA library is used to estimate the
     * nutation parameters.
     *
     * @author Matthias Schartner
     * @date 31.07.2017
     */
    class Nutation {
    public:
        static thread_local std::vector<double> nutX; ///< nutation x in one hour steps from IAU2006a model
        static thread_local std::vector<double> nutY; ///< nutation y in one hour steps from IAU2006a model
        static thread_local std::vector<double> nutS; ///< nutation s in one hour steps from IAU2006a model
        static thread_local std::vector<unsigned int> nutTime; ///< corresponding times of nut_x nut_y nut_s entries
    };
}

#endif //NUTATION_H
