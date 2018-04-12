//
// Created by matth on 11.04.2018.
//

#ifndef CABLEWRAP_AZEL_H
#define CABLEWRAP_AZEL_H

#include "CableWrap.h"
namespace VieVS{
    class CableWrap_AzEl: public CableWrap {
    public:
        CableWrap_AzEl(double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg);

        bool anglesInside(const PointingVector &p) const noexcept override;

        void unwrapAzNearAz(PointingVector &new_pointingVector, double az_old) const noexcept override;

        void calcUnwrappedAz(const PointingVector &old_pointingVector,
                                     PointingVector &new_pointingVector) const noexcept override;

        bool unwrapAzInSection(PointingVector &pv, char section) const noexcept override;

        std::string cableWrapFlag(const PointingVector &pointingVector) const noexcept override;

        std::pair<std::string, std::string> getMotions() const noexcept override;

        std::string vexPointingSectors() const noexcept override;

    };
}


#endif //VLBI_SCHEDULER_CABLEWRAP_AZEL_H
