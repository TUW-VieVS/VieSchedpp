/**
 * @file VieVS_nutation.h
 * @brief class VieVS_nutation
 *
 *
 * @author Matthias Schartner
 * @date 31.07.2017
 */

#ifndef VIEVS_NUTATION_H
#define VIEVS_NUTATION_H

#include <vector>

using namespace std;

namespace VieVS {

    class VieVS_nutation {
    public:
        static vector<double> nut_x; ///< nutation x in one hour steps from IAU2006a model
        static vector<double> nut_y; ///< nutation y in one hour steps from IAU2006a model
        static vector<double> nut_s; ///< nutation s in one hour steps from IAU2006a model
        static vector<unsigned int> nut_time; ///< corresponding times of nut_x nut_y nut_s entries
    };
}

#endif //VIEVS_NUTATION_H
