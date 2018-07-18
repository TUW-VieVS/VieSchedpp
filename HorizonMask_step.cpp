//
// Created by matth on 11.04.2018.
//

#include "HorizonMask_step.h"
using namespace std;
using namespace VieVS;

HorizonMask_step::HorizonMask_step(const std::vector<double> &azimuths, const std::vector<double> &elevations):
        azimuth_{azimuths}, elevation_{elevations} {
}

bool HorizonMask_step::visible(const PointingVector &pv) const noexcept {
    double az = pv.getAz();
    az = fmod(az,twopi);
    if(az<0){
        az+=twopi;
    }

    double el = pv.getEl();
    double el_mask = az2el(az);

    return el >= el_mask;

}

std::string HorizonMask_step::vexOutput() const noexcept {
    ostringstream  out;

    out << "        horizon_map_az = ";
    for(int i=0; i<azimuth_.size(); ++i){
        out << (azimuth_.at(static_cast<unsigned int>(i))*rad2deg);
        if(i==0){
            out << " deg";
        }
        if(i!=azimuth_.size()-1){
            out << " : ";
        }
    }
    out << ";\n";

    out << "        horizon_map_el = ";
    for(int i=0; i<elevation_.size(); ++i){
        out << elevation_.at(static_cast<unsigned int>(i))*rad2deg;
        if(i==0){
            out << " deg";
        }
        if(i!=elevation_.size()-1){
            out << " : ";
        }
    }
    out << ";\n";

    return out.str();
}

pair<vector<double>, vector<double>> HorizonMask_step::getHorizonMask() const {
    vector<double> az_;
    vector<double> el_;

    for(int az = 0; az <=360; az+=1){
        double azrad = static_cast<double>(az)*deg2rad;
        double el = az2el(azrad);

        az_.push_back(azrad);
        el_.push_back(el);
    }

    return {az_,el_};
}

double HorizonMask_step::az2el(double az) const noexcept {
    unsigned long i = 1;
    while(az>azimuth_.at(i)){
        ++i;
    }
    return elevation_.at(i-1);
}
