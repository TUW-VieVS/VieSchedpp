//
// Created by matth on 11.04.2018.
//

#include "CableWrap_XYew.h"
using namespace VieVS;
using namespace std;

CableWrap_XYew::CableWrap_XYew(double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg)
        : CableWrap(axis1_low_deg, axis1_up_deg, axis2_low_deg, axis2_up_deg) {

}

bool CableWrap_XYew::anglesInside(const PointingVector &p) const noexcept {
    double az = p.getAz();
    double el = p.getEl();

    double cel = cos(el);
    double sel = sin(el);
    double caz = cos(az);
    double saz = sin(az);

    double ax1 = atan2(cel*caz,sel);
    double ax2 = asin(cel*saz);

    return axisInsideCableWrap(ax1, ax2);
}

void CableWrap_XYew::unwrapAzNearAz(PointingVector &new_pointingVector, double az_old) const noexcept {
}


CableWrap::CableWrapFlag CableWrap_XYew::cableWrapFlag(const PointingVector &pointingVector) const noexcept {
    return CableWrap::CableWrapFlag::n;
}

bool CableWrap_XYew::unwrapAzInSection(PointingVector &pv, char section) const noexcept {
    return section == '-';
}

std::pair<std::string, std::string> CableWrap_XYew::getMotions() const noexcept {
    return {"x","yew"};
}

std::string CableWrap_XYew::vexPointingSectors() const noexcept {
    return pointingSector("x","yew",'-');
}
