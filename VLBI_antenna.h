/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   antenna.h
 * Author: mschartn
 *
 * Created on June 27, 2017, 11:15 AM
 */

#ifndef VLBI_ANTENNA_H
#define VLBI_ANTENNA_H
#include <iostream>
#include <boost/format.hpp>

#include "VLBI_pointingVector.h"
#include "VieVS_constants.h"

using namespace std;
namespace VieVS{
    class VLBI_antenna {
    public:
        VLBI_antenna();
        
        VLBI_antenna(double offset_m, 
                     double diam_m, 
                     double rate1_deg_per_min,
                     double constantOverhead1_s, 
                     double rate2_deg_per_min,
                     double constantOverhead2_s);
        
        virtual ~VLBI_antenna();
        
        double getOffset()   const{return offset;}
        double getDiam()     const{return diam;}
        double getRate1()    const{return rate1;}
        double getCon1()     const{return con1;}
        double getRate2()    const{return rate2;}
        
        void scanStart(VLBI_pointingVector& old_pointingVector, VLBI_pointingVector& new_pointingVector, int wait_setup, int wait_source, int wait_tape, int wait_idle, int wait_calibration);
        
        friend ostream& operator<<(ostream& out, const VLBI_antenna& antenna);
        
    private:
        double offset;
        double diam;
        double rate1;
        double con1; 
        double rate2; 
        double con2;
    };
}
#endif /* VLBI_ANTENNA_H */

