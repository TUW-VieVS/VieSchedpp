//
// Created by matth on 11.04.2018.
//

#include "CableWrap_AzEl.h"
using namespace VieVS;
using namespace std;

CableWrap_AzEl::CableWrap_AzEl(double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg)
        : CableWrap(axis1_low_deg, axis1_up_deg, axis2_low_deg, axis2_up_deg) {
}

bool CableWrap_AzEl::anglesInside(const PointingVector &p) const noexcept {
    double ax1 = p.getAz();
    double ax2 = p.getEl();

    return axisInsideCableWrap(ax1, ax2);
}

void CableWrap_AzEl::unwrapAzNearAz(PointingVector &new_pointingVector, double az_old) const noexcept {
    double az_new = new_pointingVector.getAz();

    double unaz_new;

    while (az_new > minLow(Axis::axis1)) {
        az_new = az_new - 2 * pi;
    }
    while (az_new < minLow(Axis::axis1)) {
        az_new = az_new + 2 * pi;
    }


    unaz_new = az_new;
    auto ambigurities = static_cast<int>(floor((maxUp(Axis::axis1) - unaz_new) / (2 * pi)));
    double this_unaz = unaz_new;

    for (int i = 1; i <= ambigurities; ++i) {
        double this_unaz_new = unaz_new + i * 2 * pi;
        if (abs(this_unaz - az_old) < abs(this_unaz_new - az_old)) {
            break;
        } else {
            this_unaz = this_unaz_new;
        }
    }
    new_pointingVector.setAz(this_unaz);
}

CableWrap::CableWrapFlag CableWrap_AzEl::cableWrapFlag(const PointingVector &pointingVector) const noexcept {
    double az = pointingVector.getAz();

    if (az <= getNUp() && az >= getNLow()) {
        return CableWrap::CableWrapFlag::n;
    }
    if (az <= getCUp() && az >= getCLow()) {
        return CableWrap::CableWrapFlag::cw;
    }
    if (az <= getWUp() && az >= getWLow()) {
        return CableWrap::CableWrapFlag::ccw;
    }

    return CableWrap::CableWrapFlag::n;
}

bool CableWrap_AzEl::unwrapAzInSection(PointingVector &pv, char section) const noexcept {
    double az = pv.getAz();
    bool flag = false;

    if(section == '-') {
        while(az>getNLow()){
            az -= 2*pi;
        }
        while(az<getNLow()){
            az += 2*pi;
        }
        if(az>getNUp()){
            flag = true;
        }
    }else if(section == 'C'){
        while(az>getCLow()){
            az -= 2*pi;
        }
        while(az<getCLow()){
            az += 2*pi;
        }
        if(az>getCUp()){
            flag = true;
        }
    }else if(section == 'W'){
        while(az>getWLow()){
            az -= 2*pi;
        }
        while(az<getWLow()){
            az += 2*pi;
        }
        if(az>getWUp()){
            flag = true;
        }
    }

    pv.setAz(az);
    return flag;
}

std::pair<std::string, std::string> CableWrap_AzEl::getMotions() const noexcept {
    return {"az","el"};
}

std::string CableWrap_AzEl::vexPointingSectors() const noexcept {
    return pointingSector("az","el",'W') + pointingSector("az","el",'-') + pointingSector("az","el",'C');
}
