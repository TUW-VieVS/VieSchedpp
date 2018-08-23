//
// Created by matth on 11.04.2018.
//

#ifndef ANTENNA_AZEL_H
#define ANTENNA_AZEL_H

#include "AbstractAntenna.h"

namespace VieVS{
    class Antenna_AzEl : public AbstractAntenna {
    public:
        Antenna_AzEl(double offset_m, double diam_m, double rateAz_deg_per_min,
                     unsigned int constantOverheadAz_s, double rateEl_deg_per_min, unsigned int constantOverheadEl_s);

        unsigned int slewTime(const PointingVector &old_pointingVector,
                              const PointingVector &new_pointingVector) const noexcept override;

    private:

    };
}


#endif //ANTENNA_AZEL_H
