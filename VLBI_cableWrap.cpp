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

#include "VLBI_cableWrap.h"
namespace VieVS{
    VLBI_cableWrap::VLBI_cableWrap(){}
    
    VLBI_cableWrap::VLBI_cableWrap(double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg):
            axis1_low{axis1_low_deg * deg2rad}, axis1_up{axis1_up_deg * deg2rad},
            axis2_low{axis2_low_deg * deg2rad}, axis2_up{axis2_up_deg * deg2rad} {

        if ((axis1_up - axis1_low) > twopi) {
            double overlapping = axis1_range-twopi;
            if (overlapping>twopi){
                cerr << "ERROR: cable wrap limits to large!";
            }
            n_low = axis1_low;
            n_up = axis1_low+overlapping/2;
            c_low = axis1_low+overlapping/2;
            c_start = axis1_up-overlapping/2;
            w_low = axis1_up-overlapping/2;
            w_start = axis1_up;
        }else {
            n_low = axis1_low;
            n_up= axis1_up;
            c_low = axis1_up;
            c_start = axis1_up;
            w_low = axis1_low;
            w_start = axis1_low;
        }
    }

    VLBI_cableWrap::~VLBI_cableWrap() {
    }
    
    bool VLBI_cableWrap::anglesInside(VLBI_pointingVector& p){
        double az = p.getAz();
        double el = p.getEl();
        if ((axis1_up - axis1_up_offset - axis1_low + axis1_low_offset) < 2 * pi) {
            if (az < fmod(axis1_low + axis1_low_offset, twopi) || az > fmod(axis1_up - axis1_up_offset, twopi) ||
                el < axis2_low + axis2_low_offset || el > axis2_up - axis2_up_offset) {
                return false;
            }
        } else {
            if (el < axis2_low + axis2_low_offset || el > axis2_up - axis2_up_offset) {
                return false;
            }
        }
        
        return true;
    }

    bool VLBI_cableWrap::unwrapAzNearNeutralPoint(VLBI_pointingVector &new_pointingVector) {
        double az_old = neutralPoint(1);
        double az_new = new_pointingVector.getAz();

        double unaz_new;

        while (az_new > axis1_low + axis1_low_offset) {
            az_new = az_new - 2 * pi;
        }
        while (az_new < axis1_low + axis1_low_offset) {
            az_new = az_new + 2 * pi;
        }


        unaz_new = az_new;
        int ambigurities = (int) floor((axis1_up - axis1_up_offset - unaz_new) / (2 * pi));
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

        bool secure;
        if (this_unaz > axis1_low + axis1_low_offset + 3 * deg2rad &&
            this_unaz < axis1_up - axis1_up_offset - 3 * deg2rad) {
            secure = true;
        } else {
            secure = false;
        }

        return secure;
    }

    void VLBI_cableWrap::calcUnwrappedAz(VLBI_pointingVector& old_pointingVector, VLBI_pointingVector& new_pointingVector){
        double az_old = old_pointingVector.getAz();
        double az_new = new_pointingVector.getAz();
        
        double unaz_new;

        while (az_new > axis1_low + axis1_low_offset) {
            az_new = az_new - 2 * pi;
        }
        while (az_new < axis1_low + axis1_low_offset) {
            az_new = az_new + 2 * pi;
        }


        unaz_new = az_new;
        int ambigurities = (int) floor((axis1_up - axis1_up_offset - unaz_new) / (2 * pi));
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

    double VLBI_cableWrap::neutralPoint(int axis){
        if (axis==1){
            return (axis1_low+axis1_up)/2;
        } else if(axis==2){
            return (axis2_low+axis2_up)/2;
        } else {
            cerr<< "Wrong axis number, use 1 or 2!\n";
        }
        return 0;
    }

    ostream& operator<<(ostream& out, const VLBI_cableWrap& cable){
        cout << "cable wrap: \n";
        double axis1_lowdeg = cable.axis1_low*rad2deg;
        double axis1_updeg  = cable.axis1_up*rad2deg;
        double axis2_lowdeg = cable.axis2_low*rad2deg;
        double axis2_updeg  = cable.axis2_up*rad2deg;
        
        cout << boost::format("    axis1: %4.0f [deg] to %4.0f [deg]\n") % axis1_lowdeg % axis1_updeg;
        cout << boost::format("    axis2: %4.0f [deg] to %4.0f [deg]\n") % axis2_lowdeg % axis2_updeg;
        
        return out;
    }

    void VLBI_cableWrap::setMinimumOffsets(double axis1_low_offset, double axis1_up_offset,
                                           double axis2_low_offset, double axis2_up_offset) {
        this->axis1_low_offset = axis1_low_offset * deg2rad;
        this->axis1_up_offset = axis1_up_offset * deg2rad;
        this->axis2_low_offset = axis2_low_offset * deg2rad;
        this->axis2_up_offset = axis2_up_offset * deg2rad;
    }

    void VLBI_cableWrap::unwrapAzNearAz(VLBI_pointingVector &new_pointingVector, double az_old) {
        double az_new = new_pointingVector.getAz();

        double unaz_new;

        while (az_new > axis1_low + axis1_low_offset) {
            az_new = az_new - 2 * pi;
        }
        while (az_new < axis1_low + axis1_low_offset) {
            az_new = az_new + 2 * pi;
        }


        unaz_new = az_new;
        int ambigurities = (int) floor((axis1_up - axis1_up_offset - unaz_new) / (2 * pi));
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
