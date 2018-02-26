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

Antenna::Antenna(const std::string &type, double offset_m, double diam_m, double rate1_deg_per_min,
                 double constantOverhead1_s, double rate2_deg_per_min, double constantOverhead2_s) :
                 offset_{offset_m}, diam_{diam_m}, rate1_{rate1_deg_per_min*deg2rad/60}, con1_{constantOverhead1_s},
                 rate2_{rate2_deg_per_min*deg2rad/60}, con2_{constantOverhead2_s}{

    if (type == "AZEL")
        axisType_ = AxisType::AZEL;
    else if(type == "HADC")
        axisType_ = AxisType::HADC;
    else if(type == "XYNS")
        axisType_ = AxisType::XYNS;
    else if(type == "XYEW")
        axisType_ = AxisType::XYEW;
    else if(type == "RICH")
        axisType_ = AxisType::RICH;
    else if(type == "SEST")
        axisType_ = AxisType::SEST;
    else if(type == "ALGO")
        axisType_ = AxisType::ALGO;
    else
        axisType_ = AxisType::undefined;

}

unsigned int Antenna::slewTime(const PointingVector &old_pointingVector,
                                    const PointingVector &new_pointingVector) const noexcept {
    // TODO acceleration is hardcoded
    double acc1 = rate1_;
    double acc2 = rate2_;

    unsigned int slewtime;
    switch(axisType_){
        case AxisType::AZEL:{
            double delta1 = abs(old_pointingVector.getAz()-new_pointingVector.getAz());
            double delta2 = abs(old_pointingVector.getEl()-new_pointingVector.getEl());


            double t_1  = slewTimePerAxis(delta1,rate1_,acc1)+con1_;
            double t_2  = slewTimePerAxis(delta2,rate2_,acc2)+con2_;

            if(t_1>t_2){
                slewtime = static_cast<unsigned int>(ceil(t_1));
            }else{
                slewtime = static_cast<unsigned int>(ceil(t_2));
            }
            break;
        }
        case AxisType::HADC:{
            double delta1 = abs(new_pointingVector.getHa() - old_pointingVector.getHa());
            double delta2 = abs(new_pointingVector.getDc() - old_pointingVector.getDc());

            double t_1  = slewTimePerAxis(delta1,rate1_,acc1)+con1_;
            double t_2  = slewTimePerAxis(delta2,rate2_,acc2)+con2_;

            if(t_1>t_2){
                slewtime = static_cast<unsigned int>(ceil(t_1));
            }else{
                slewtime = static_cast<unsigned int>(ceil(t_2));
            }
            break;
        }
        case AxisType::XYEW:{
            double cel_old = cos(old_pointingVector.getEl());
            double sel_old = sin(old_pointingVector.getEl());
            double caz_old = cos(old_pointingVector.getAz());
            double saz_old = sin(old_pointingVector.getAz());

            double x_old = atan2(cel_old*caz_old,sel_old);
            double y_old = asin(cel_old*saz_old);

            double cel_new = cos(new_pointingVector.getEl());
            double sel_new = sin(new_pointingVector.getEl());
            double caz_new = cos(new_pointingVector.getAz());
            double saz_new = sin(new_pointingVector.getAz());

            double x_new = atan2(cel_new*caz_new,sel_new);
            double y_new = asin(cel_new*saz_new);

            double delta1 = abs(x_new-x_old);
            double delta2 = abs(y_new-y_old);

            double t_1  = slewTimePerAxis(delta1,rate1_,acc1)+con1_;
            double t_2  = slewTimePerAxis(delta2,rate2_,acc2)+con2_;

            if(t_1>t_2){
                slewtime = static_cast<unsigned int>(ceil(t_1));
            }else{
                slewtime = static_cast<unsigned int>(ceil(t_2));
            }
            break;
        }
        case AxisType::XYNS:
        case AxisType::RICH:
        case AxisType::SEST:
        case AxisType::ALGO:
        case AxisType::undefined:{
            cerr << "ERROR axis type is not yet implementet for slewtime calculation!;\n";
            slewtime = numeric_limits<unsigned int>::max();
            break;
        }
    }
    return slewtime;
}

double Antenna::slewTimePerAxis(double delta, double rate, double acc) const noexcept {
    double t_acc = rate/acc;
    double s_acc = 2*(acc*t_acc*t_acc/2);
    double t;
    if (delta<s_acc){
        t = 2*sqrt(delta/acc);
    } else {
        t = 2 * t_acc + (delta - s_acc) / rate;
    }
    return t;
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
