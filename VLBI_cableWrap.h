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
        
        VLBI_cableWrap(double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg);
        
        double neutralPoint(int axis);

        virtual ~VLBI_cableWrap();
                
        bool anglesInside(VLBI_pointingVector p);
        
        void calcUnwrappedAz(VLBI_pointingVector& old_pointingVector, VLBI_pointingVector& new_pointingVector);
        
        friend ostream& operator<<(ostream& out, const VLBI_cableWrap& sta);
        
    private:
        double axis1_low;
        double axis1_up;
        double axis2_low;
        double axis2_up;
        
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

