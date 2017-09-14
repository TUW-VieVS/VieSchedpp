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

Antenna::Antenna(double offset_m,
                 double diam_m,
                 double rate1_deg_per_min,
                 double constantOverhead1_s,
                 double rate2_deg_per_min,
                 double constantOverhead2_s):
                 offset_{offset_m},
                 diam_{diam_m},
                 rate1_{rate1_deg_per_min*deg2rad/60},
                 con1_{constantOverhead1_s},
                 rate2_{rate2_deg_per_min*deg2rad/60},
                 con2_{constantOverhead2_s}{}

unsigned int Antenna::slewTime(const PointingVector &old_pointingVector,
                                    const PointingVector &new_pointingVector) const noexcept {

    double delta1 = abs(old_pointingVector.getAz()-new_pointingVector.getAz());
    double delta2 = abs(old_pointingVector.getEl()-new_pointingVector.getEl());
    double acc1 = 1*deg2rad; // TODO acceleration is hardcoded
    double acc2 = 1*deg2rad;

    double t_acc_1 = rate1_/acc1;
    double t_acc_2 = rate2_/acc2;

    double s_acc_1 = 2*(acc1*t_acc_1*t_acc_1/2);
    double s_acc_2 = 2*(acc2*t_acc_2*t_acc_2/2);

    double t_1, t_2;
    if (delta1<s_acc_1){
        t_1 = 2*sqrt(delta1/acc1);
    } else {
        t_1 = 2 * t_acc_1 + (delta1 - s_acc_1) / rate1_;
    }
    if (delta2<s_acc_2){
        t_2 = 2*sqrt(delta2/acc2);
    } else {
        t_2 = 2 * t_acc_2 + (delta2 - s_acc_2) / rate2_;
    }

    auto slewtime = (unsigned int) ceil(t_1);
    if(t_2>t_1){
        slewtime = (unsigned int) ceil(t_2);
    }

    return slewtime;
}

namespace VieVS {
    ostream &operator<<(ostream &out, const Antenna &antenna) noexcept {
        double rate1degs = antenna.rate1_ * rad2deg;
        double rate2degs = antenna.rate2_ * rad2deg;
        cout << "Antenna: " << antenna.diam_ << " [m] \n";
        cout << boost::format("    slew rate axis1: %6.2f [deg/s]\n") % rate1degs;
        cout << boost::format("    slew rate axis2: %6.2f [deg/s]\n") % rate2degs;
        return out;
    }
}
