//
// Created by matth on 06.05.2018.
//

#ifndef UTILITY_H
#define UTILITY_H

#include <boost/format.hpp>
#include <cmath>

#include "Constants.h"

namespace VieVS::util{

    std::string ra2dms(double angle);

    std::string dc2hms(double angle);

    double wrapToPi(double angle);

    template<typename T> T absDiff(const T&a, const T&b) {
        return (a > b) ? (a - b) : (b - a);
    }



}


#endif //UTILITY_H
