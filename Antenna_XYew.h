//
// Created by matth on 11.04.2018.
//

#ifndef ANTENNA_XYEW_H
#define ANTENNA_XYEW_H

#include "AbstractAntenna.h"

namespace VieVS{
    class Antenna_XYew : public AbstractAntenna {
    public:
        Antenna_XYew(double offset_m, double diam_m, double rateX_deg_per_min, unsigned int constantOverheadX_s,
                     double rateY_deg_per_min, unsigned int constantOverheadY_s);

        unsigned int slewTime(const PointingVector &old_pointingVector,
                              const PointingVector &new_pointingVector) const noexcept override;

    };
}



#endif //ANTENNA_XYEW_H
