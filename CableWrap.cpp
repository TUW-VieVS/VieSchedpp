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

int CableWrap::nextId = 0;

CableWrap::CableWrap(double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg, string cwp):
        VieVS_Object(nextId++),
        axis1Low_{axis1_low_deg * deg2rad}, axis1Up_{axis1_up_deg * deg2rad},
        axis2Low_{axis2_low_deg * deg2rad}, axis2Up_{axis2_up_deg * deg2rad}{


    if (cwp == "AZEL")
        cableWrapType_ = CableWrapType::AZEL;
    else if(cwp == "HADC")
        cableWrapType_ = CableWrapType::HADC;
    else if(cwp == "XYNS")
        cableWrapType_ = CableWrapType::XYNS;
    else if(cwp == "XYEW")
        cableWrapType_ = CableWrapType::XYEW;
    else if(cwp == "RICH")
        cableWrapType_ = CableWrapType::RICH;
    else if(cwp == "SEST")
        cableWrapType_ = CableWrapType::SEST;
    else if(cwp == "ALGO")
        cableWrapType_ = CableWrapType::ALGO;
    else
        cableWrapType_ = CableWrapType::undefined;


    if ((axis1Up_ - axis1Low_) > twopi) {
        double overlapping = (axis1Up_ - axis1Low_) - twopi;
        if (overlapping>twopi){
            cerr << "ERROR: cable wrap limits to large!;";
        }
        wLow_ = axis1Low_;
        wUp_ = axis1Low_ + overlapping;
        nLow_ = axis1Low_ + overlapping;
        nUp_ = axis1Up_ - overlapping;
        cLow_ = axis1Up_ - overlapping;
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


bool CableWrap::anglesInside(const PointingVector &p) const noexcept {
    double ax1;
    double ax2;

    switch (cableWrapType_){
        case CableWrapType::AZEL:{
            ax1 = p.getAz();
            ax2 = p.getEl();

            break;
        }
        case CableWrapType::HADC:{
            ax1 = p.getHa();
            ax2 = p.getDc();

            break;
        }
        case CableWrapType::XYEW:{
            double az = p.getAz();
            double el = p.getEl();

            double cel = cos(el);
            double sel = sin(el);
            double caz = cos(az);
            double saz = sin(az);
            ax1 = atan2(cel*caz,sel);
            ax2 = asin(cel*saz);
            break;
        }
        case CableWrapType::XYNS:
        case CableWrapType::RICH:
        case CableWrapType::SEST:
        case CableWrapType::ALGO:
        case CableWrapType::undefined:{
            cerr << "ERROR axis type is not yet implementet for cable wrap visibility calculation!;\n";
            return false;
        }
    }


    if ((axis1Up_ - axis1UpOffset_ - axis1Low_ + axis1LowOffset_) < 2 * pi) {
        if (ax1 < fmod(axis1Low_ + axis1LowOffset_, twopi) || ax1 > fmod(axis1Up_ - axis1UpOffset_, twopi) ||
            ax2 < axis2Low_ + axis2LowOffset_ || ax2 > axis2Up_ - axis2UpOffset_) {
            return false;
        }
    } else {
        if (ax2 < axis2Low_ + axis2LowOffset_ || ax2 > axis2Up_ - axis2UpOffset_) {
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

    if(cableWrapType_ == CableWrapType::AZEL){
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

}

double CableWrap::neutralPoint(int axis) const noexcept {
    if (axis==1){
        return (axis1Low_+axis1Up_)/2;
    } else if(axis==2){
        return (axis2Low_+axis2Up_)/2;
    } else {
        cerr<< "ERROR: Wrong axis number, use 1 or 2!;\n";
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

    if(cableWrapType_ == CableWrapType::AZEL){
        if (az <= nUp_ && az >= nLow_) {
            return "-";
        }
        if (az <= cUp_ && az >= cLow_) {
            return "C";
        }
        if (az <= wUp_ && az >= wLow_) {
            return "W";
        }
    }else{
        return "-";
    }
}

bool CableWrap::unwrapAzInSection(PointingVector &pv, char section) const noexcept{
    double az = pv.getAz();
    bool flag = false;
    if (cableWrapType_ == CableWrapType::AZEL) {
        if(section == '-') {
            while(az>nLow_){
                az -= 2*pi;
            }
            az +=2*pi;
            if(az>nUp_){
                flag = true;
//                cerr << "azimuth error! Flag: '-' limits: "<< nLow_*rad2deg << " - " << nUp_*rad2deg << " calculated:" << az*rad2deg << "\n";
            }
        }else if(section == 'C'){
            while(az>cLow_){
                az -= 2*pi;
            }
            az +=2*pi;
            if(az>cUp_){
                flag = true;
//                cerr << "azimuth error! Flag: '-' limits: "<< cLow_*rad2deg << " - " << cUp_*rad2deg << " calculated:" << az*rad2deg << "\n";
            }
        }else if(section == 'W'){
            while(az>wLow_){
                az -= 2*pi;
            }
            az +=2*pi;
            if(az>wUp_){
                flag = true;
//                cerr << "azimuth error! Flag: '-' limits: "<< wLow_*rad2deg << " - " << wUp_*rad2deg << " calculated:" << az*rad2deg << "\n";
            }
        }
    }

    pv.setAz(az);
    return flag;
}

pair<double, double> CableWrap::getLimits(char section) const {
    double lim1, lim2;

    if (cableWrapType_ == CableWrapType::AZEL) {
        if(section == '-') {
            lim1 = nLow_;
            lim2 = nUp_;
        }else if(section == 'C'){
            lim1 = cLow_;
            lim2 = cUp_;
        }else {
            lim1 = wLow_;
            lim2 = wUp_;
        }
    }else{
        lim1 = axis1Low_;
        lim2 = axis1Up_;
    }
    return {lim1,lim2};
}

