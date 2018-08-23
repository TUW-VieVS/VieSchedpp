//
// Created by matth on 11.04.2018.
//

#include "Antenna_AzEl.h"
using namespace std;
using namespace VieVS;

Antenna_AzEl::Antenna_AzEl(double offset_m, double diam_m, double rateAz_deg_per_min, unsigned int constantOverheadAz_s,
                           double rateEl_deg_per_min, unsigned int constantOverheadEl_s)
        : AbstractAntenna(offset_m, diam_m, rateAz_deg_per_min, constantOverheadAz_s, rateEl_deg_per_min,
                          constantOverheadEl_s) {
}

unsigned int Antenna_AzEl::slewTime(const PointingVector &old_pointingVector,
                                           const PointingVector &new_pointingVector) const noexcept {

    double delta1 = abs(old_pointingVector.getAz()-new_pointingVector.getAz());
    double delta2 = abs(old_pointingVector.getEl()-new_pointingVector.getEl());

    unsigned int t_1  = slewTimePerAxis(delta1,Axis::axis1);
    unsigned int t_2  = slewTimePerAxis(delta2,Axis::axis2);

    return t_1 > t_2 ? t_1 : t_2;
}

