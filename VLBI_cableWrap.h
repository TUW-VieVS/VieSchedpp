/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   cableWrap.h
 * Author: mschartn
 *
 * Created on June 27, 2017, 11:47 AM
 */

#ifndef VLBI_CABLEWRAP_H
#define VLBI_CABLEWRAP_H
#include <iostream>
#include <boost/format.hpp>
#include <cmath>

#include "VLBI_pointingVector.h"
#include "VieVS_constants.h"
using namespace std;

namespace VieVS{
    class VLBI_cableWrap {
    public:
        VLBI_cableWrap();
        
        VLBI_cableWrap(double lim1_low_deg, double lim1_up_deg, double lim2_low_deg, double lim2_up_deg);
        
        virtual ~VLBI_cableWrap();
                
        bool anglesInside(VLBI_pointingVector p);
        
        void calcUnwrappedAz(VLBI_pointingVector& old_pointingVector, VLBI_pointingVector& new_pointingVector);
        
        friend ostream& operator<<(ostream& out, const VLBI_cableWrap& sta);
        
    private:
        double lim1_low;
        double lim1_up; 
        double lim2_low;
        double lim2_up;
        
        double n_low;
        double n_up;
        double c_low;
        double c_start;
        double w_low;
        double w_start;
        
        double axis1_range;
    };
}
#endif /* VLBI_CABLEWRAP_H */

