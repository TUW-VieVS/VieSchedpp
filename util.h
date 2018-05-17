//
// Created by matth on 06.05.2018.
//

#ifndef UTILITY_H
#define UTILITY_H

#include <boost/format.hpp>
#include <cmath>
#include <vector>
#include <numeric>

#include "Constants.h"

namespace VieVS::util{

    std::string ra2dms(double angle);

    std::string dc2hms(double angle);

    double wrapToPi(double angle);

    template<typename T>
    T absDiff(const T&a, const T&b) {
        return (a > b) ? (a - b) : (b - a);
    }

    template <typename T>
    std::vector<int> sortIndexes(const std::vector<T> &v){
        // initialize original index locations
        std::vector<int> idx(v.size());
        std::iota(idx.begin(), idx.end(), 0);

        // sort indexes based on comparing values in v
        std::sort(idx.begin(), idx.end(), [&v](int i1, int i2) {
            return v[i1] < v[i2];}
        );

        return idx;
    }


}


#endif //UTILITY_H
