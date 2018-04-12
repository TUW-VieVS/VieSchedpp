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
    unsigned long i = 1;
    while(az>azimuth_.at(i)){
        ++i;
    }
    double el_mask = elevation_.at(i-1);

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
