//
// Created by mschartn on 31.07.17.
//

#ifndef VIEVS_NUTATION_H
#define VIEVS_NUTATION_H

#include <vector>

using namespace std;

namespace VieVS {

    class VieVS_nutation {
    public:
        static vector<double> nut_x;
        static vector<double> nut_y;
        static vector<double> nut_s;
        static vector<unsigned int> nut_time;
    };
}

#endif //VIEVS_NUTATION_H
