//
// Created by matth on 11.04.2018.
//

#include "Antenna_XYew.h"
using namespace VieVS;
using namespace std;

Antenna_XYew::Antenna_XYew(double offset_m, double diam_m, double rateX_deg_per_min, unsigned int constantOverheadX_s,
                           double rateY_deg_per_min, unsigned int constantOverheadY_s) :
        Antenna(offset_m, diam_m, rateX_deg_per_min, constantOverheadX_s, rateY_deg_per_min, constantOverheadY_s) {
}

unsigned int
Antenna_XYew::slewTime(const PointingVector &old_pointingVector,
                       const PointingVector &new_pointingVector) const noexcept {

    double cel_old = cos(old_pointingVector.getEl());
    double sel_old = sin(old_pointingVector.getEl());
    double caz_old = cos(old_pointingVector.getAz());
    double saz_old = sin(old_pointingVector.getAz());

    double x_old = atan2(cel_old*caz_old,sel_old);
    double y_old = asin(cel_old*saz_old);

    double cel_new = cos(new_pointingVector.getEl());
    double sel_new = sin(new_pointingVector.getEl());
    double caz_new = cos(new_pointingVector.getAz());
    double saz_new = sin(new_pointingVector.getAz());

    double x_new = atan2(cel_new*caz_new,sel_new);
    double y_new = asin(cel_new*saz_new);

    double delta1 = abs(x_new-x_old);
    double delta2 = abs(y_new-y_old);

    unsigned int t_1  = slewTimePerAxis(delta1,Axis::axis1);
    unsigned int t_2  = slewTimePerAxis(delta2,Axis::axis2);

    return t_1 > t_2 ? t_1 : t_2;

}
