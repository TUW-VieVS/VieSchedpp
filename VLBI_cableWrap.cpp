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
                         axis1_low{axis1_low_deg*deg2rad}, axis1_up{axis1_up_deg*deg2rad}, axis2_low{axis2_low_deg*deg2rad}, axis2_up{axis2_up_deg*deg2rad}{
        axis1_range = axis1_up-axis1_low;

        if (axis1_range>twopi){
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
        if (axis1_range<2*pi){
            if (az < fmod(axis1_low,twopi) || az > fmod(axis1_up,twopi) || el < axis2_low || el > axis2_up){
                return false;
            }
        } else {
            if (el < axis2_low || el > axis2_up){
                return false;
            }
        }
        
        return true;
    }

    void VLBI_cableWrap::calcUnwrappedAz(VLBI_pointingVector& old_pointingVector, VLBI_pointingVector& new_pointingVector){
        double az_old = old_pointingVector.getAz();
        double az_new = new_pointingVector.getAz();
        
        double unaz_new;
        
        while (az_new > axis1_low){
            az_new = az_new - 2 * pi;
        }
        while (az_new < axis1_up){
            az_new = az_new + 2 * pi;
        }
        unaz_new = az_new;
        
        if(az_new+twopi < axis1_up){
            double unaz_new2 = az_new+twopi;
            if (abs(unaz_new2-az_old) < abs(unaz_new-az_old)){
                new_pointingVector.setAz(unaz_new2);
            } else {
                new_pointingVector.setAz(unaz_new);
            }
        } else {
            new_pointingVector.setAz(unaz_new);
        }
        
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

}
