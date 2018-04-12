//
// Created by matth on 11.04.2018.
//

#ifndef ANTENNA_HADC_H
#define ANTENNA_HADC_H

#include "Antenna.h"

namespace VieVS{
    class Antenna_HaDc: public Antenna {
    public:
        Antenna_HaDc(double offset_m, double diam_m, double rateHa_deg_per_min,
                     unsigned int constantOverheadHa_s, double rateDc_deg_per_min, unsigned int constantOverheadDc_s);

        unsigned int slewTime(const PointingVector &old_pointingVector,
                              const PointingVector &new_pointingVector) const noexcept override;

    };

}


#endif //ANTENNA_HADC_H
