//
// Created by matth on 11.04.2018.
//

#include "Antenna_HaDc.h"
using namespace VieVS;
using namespace std;

Antenna_HaDc::Antenna_HaDc(double offset_m, double diam_m, double rateHa_deg_per_min, unsigned int constantOverheadHa_s,
                           double rateDc_deg_per_min, unsigned int constantOverheadDc_s)
        : AbstractAntenna(offset_m, diam_m, rateHa_deg_per_min, constantOverheadHa_s, rateDc_deg_per_min,
                          constantOverheadDc_s) {
}

unsigned int Antenna_HaDc::slewTime(const PointingVector &old_pointingVector,
                                    const PointingVector &new_pointingVector) const noexcept {

    double delta1 = abs(new_pointingVector.getHa() - old_pointingVector.getHa());
    double delta2 = abs(new_pointingVector.getDc() - old_pointingVector.getDc());

    unsigned int t_1  = slewTimePerAxis(delta1,Axis::axis1);
    unsigned int t_2  = slewTimePerAxis(delta2,Axis::axis2);

    return t_1 > t_2 ? t_1 : t_2;

}

