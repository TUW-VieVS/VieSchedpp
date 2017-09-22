/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cableWrap.cpp
 * Author: mschartn
 * 
 * Created on June 27, 2017, 11:47 AM
 */

#include "CableWrap.h"
using namespace std;
using namespace VieVS;

CableWrap::CableWrap(){}

CableWrap::CableWrap(double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg):
        axis1Low_{axis1_low_deg * deg2rad}, axis1Up_{axis1_up_deg * deg2rad},
        axis2Low_{axis2_low_deg * deg2rad}, axis2Up_{axis2_up_deg * deg2rad} {

    if ((axis1Up_ - axis1Low_) > twopi) {
        double overlapping = (axis1Up_ - axis1Low_) - twopi;
        if (overlapping>twopi){
            cerr << "ERROR: cable wrap limits to large!";
        }
        wLow_ = axis1Low_;
        wUp_ = axis1Low_ + overlapping / 2;
        nLow_ = axis1Low_ + overlapping / 2;
        nUp_ = axis1Up_ - overlapping / 2;
        cLow_ = axis1Up_ - overlapping / 2;
        cUp_ = axis1Up_;
    }else {
        wLow_ = axis1Low_;
        wUp_ = axis1Low_;
        nLow_ = axis1Low_;
        nUp_ = axis1Up_;
        cLow_ = axis1Up_;
        cUp_ = axis1Up_;
    }
}

CableWrap::~CableWrap() {
}

bool CableWrap::anglesInside(const PointingVector &p) const noexcept {
    double az = p.getAz();
    double el = p.getEl();
    if ((axis1Up_ - axis1UpOffset_ - axis1Low_ + axis1LowOffset_) < 2 * pi) {
        if (az < fmod(axis1Low_ + axis1LowOffset_, twopi) || az > fmod(axis1Up_ - axis1UpOffset_, twopi) ||
            el < axis2Low_ + axis2LowOffset_ || el > axis2Up_ - axis2UpOffset_) {
            return false;
        }
    } else {
        if (el < axis2Low_ + axis2LowOffset_ || el > axis2Up_ - axis2UpOffset_) {
            return false;
        }
    }

    return true;
}

bool CableWrap::unwrapAzNearNeutralPoint(PointingVector &new_pointingVector) const noexcept {
    double az_old = neutralPoint(1);
    double az_new = new_pointingVector.getAz();

    double unaz_new;

    while (az_new > axis1Low_ + axis1LowOffset_) {
        az_new = az_new - 2 * pi;
    }
    while (az_new < axis1Low_ + axis1LowOffset_) {
        az_new = az_new + 2 * pi;
    }


    unaz_new = az_new;
    auto ambigurities = static_cast<int>(floor((axis1Up_ - axis1UpOffset_ - unaz_new) / (2 * pi)));
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

    bool secure = this_unaz > axis1Low_ + axis1LowOffset_ + 3 * deg2rad &&
                  this_unaz < axis1Up_ - axis1UpOffset_ - 3 * deg2rad;

    return secure;
}

void CableWrap::calcUnwrappedAz(const PointingVector &old_pointingVector,
                                     PointingVector &new_pointingVector) const noexcept {
    double az_old = old_pointingVector.getAz();
    double az_new = new_pointingVector.getAz();

    double unaz_new;

    while (az_new > axis1Low_ + axis1LowOffset_) {
        az_new = az_new - 2 * pi;
    }
    while (az_new < axis1Low_ + axis1LowOffset_) {
        az_new = az_new + 2 * pi;
    }


    unaz_new = az_new;
    auto ambigurities = static_cast<int>(floor((axis1Up_ - axis1UpOffset_ - unaz_new) / (2 * pi)));
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

double CableWrap::neutralPoint(int axis) const noexcept {
    if (axis==1){
        return (axis1Low_+axis1Up_)/2;
    } else if(axis==2){
        return (axis2Low_+axis2Up_)/2;
    } else {
        cerr<< "Wrong axis number, use 1 or 2!\n";
    }
    return 0;
}

namespace VieVS {
    ostream &operator<<(ostream &out, const CableWrap &cable) noexcept {
        cout << "cable wrap: \n";
        double axis1_lowdeg = cable.axis1Low_ * rad2deg;
        double axis1_updeg = cable.axis1Up_ * rad2deg;
        double axis2_lowdeg = cable.axis2Low_ * rad2deg;
        double axis2_updeg = cable.axis2Up_ * rad2deg;

        cout << boost::format("    axis1: %4.0f [deg] to %4.0f [deg]\n") % axis1_lowdeg % axis1_updeg;
        cout << boost::format("    axis2: %4.0f [deg] to %4.0f [deg]\n") % axis2_lowdeg % axis2_updeg;

        return out;
    }
}

void CableWrap::setMinimumOffsets(double axis1_low_offset, double axis1_up_offset,
                                       double axis2_low_offset, double axis2_up_offset) noexcept {
    axis1LowOffset_ = axis1_low_offset * deg2rad;
    axis1UpOffset_ = axis1_up_offset * deg2rad;
    axis2LowOffset_ = axis2_low_offset * deg2rad;
    axis2UpOffset_ = axis2_up_offset * deg2rad;
}

void CableWrap::unwrapAzNearAz(PointingVector &new_pointingVector, double az_old) const noexcept {
    double az_new = new_pointingVector.getAz();

    double unaz_new;

    while (az_new > axis1Low_ + axis1LowOffset_) {
        az_new = az_new - 2 * pi;
    }
    while (az_new < axis1Low_ + axis1LowOffset_) {
        az_new = az_new + 2 * pi;
    }


    unaz_new = az_new;
    auto ambigurities = static_cast<int>(floor((axis1Up_ - axis1UpOffset_ - unaz_new) / (2 * pi)));
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

std::string CableWrap::cableWrapFlag(const PointingVector &pointingVector) const noexcept {
    double az = pointingVector.getAz();

    if (az <= nUp_ && az >= nLow_) {
        return "-";
    }
    if (az <= cUp_ && az >= cLow_) {
        return "C";
    }
    if (az <= wUp_ && az >= wLow_) {
        return "W";
    }
    return "?";
}

