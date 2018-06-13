/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   antenna.cpp
 * Author: mschartn
 * 
 * Created on June 27, 2017, 11:15 AM
 */

#include "Antenna.h"
using namespace std;
using namespace VieVS;
unsigned long Antenna::nextId = 0;

Antenna::Antenna(double offset_m, double diam_m, double rate1_deg_per_min,
                 unsigned int constantOverhead1_s, double rate2_deg_per_min, unsigned int constantOverhead2_s):
        VieVS_Object(nextId++), offset_{offset_m}, diam_{diam_m}, rate1_{rate1_deg_per_min*deg2rad/60},
        con1_{constantOverhead1_s}, rate2_{rate2_deg_per_min*deg2rad/60}, con2_{constantOverhead2_s}{
}

unsigned int Antenna::slewTimePerAxis(double delta, Axis axis) const noexcept {
    double rate, acc;
    unsigned int constantOverhead;
    switch(axis){
        case Axis::axis1:{
            rate = rate1_;
            acc = rate1_;
            constantOverhead = con1_;
            break;
        }
        case Axis::axis2:{
            rate = rate2_;
            acc = rate2_;
            constantOverhead = con2_;
            break;
        }
    }

    double t_acc = rate/acc;
    double s_acc = 2*(acc*t_acc*t_acc/2);
    double t;
    if (delta<s_acc){
        t = 2*sqrt(delta/acc);
    } else {
        t = 2 * t_acc + (delta - s_acc) / rate;
    }

    return static_cast<unsigned int>(ceil(t)) + constantOverhead;
}
