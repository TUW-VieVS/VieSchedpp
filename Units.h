//
// Created by mschartn on 25.04.18.
//

#ifndef VLBI_SCHEDULER_UNITS_H
#define VLBI_SCHEDULER_UNITS_H

#include <boost/format.hpp>
#include <cmath>

#include "Constants.h"

namespace VieVS{
    class Units {
    public:
        static std::string ra2dms(double angle) noexcept;
        static std::string dc2hms(double angle) noexcept;
    };
}


#endif //VLBI_SCHEDULER_UNITS_H
