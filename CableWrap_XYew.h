//
// Created by matth on 11.04.2018.
//

#ifndef CABLEWRAP_XYEW_H
#define CABLEWRAP_XYEW_H

#include "CableWrap.h"

namespace VieVS {
    class CableWrap_XYew: public CableWrap  {
    public:
        CableWrap_XYew(double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg);

        bool anglesInside(const PointingVector &p) const noexcept override;

        void unwrapAzNearAz(PointingVector &new_pointingVector, double az_old) const noexcept override;

        bool unwrapAzInSection(PointingVector &pv, char section) const noexcept override;

        CableWrap::CableWrapFlag cableWrapFlag(const PointingVector &pointingVector) const noexcept override;

        std::pair<std::string, std::string> getMotions() const noexcept override;

        std::string vexPointingSectors() const noexcept override;

    };
}

#endif //CABLEWRAP_XYEW_H
