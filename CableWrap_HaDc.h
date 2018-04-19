//
// Created by matth on 11.04.2018.
//

#ifndef CABLEWRAP_HADC_H
#define CABLEWRAP_HADC_H

#include "CableWrap.h"
namespace VieVS {
    class CableWrap_HaDc: public CableWrap  {
    public:
        CableWrap_HaDc(double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg);

        bool anglesInside(const PointingVector &p) const noexcept override;

        void unwrapAzNearAz(PointingVector &new_pointingVector, double az_old) const noexcept override;

        bool unwrapAzInSection(PointingVector &pv, char section) const noexcept override;

        std::string cableWrapFlag(const PointingVector &pointingVector) const noexcept override;

        std::pair<std::string, std::string> getMotions() const noexcept override;

        std::string vexPointingSectors() const noexcept override;

    };
}

#endif //VLBI_SCHEDULER_CABLEWRAP_HADC_H
