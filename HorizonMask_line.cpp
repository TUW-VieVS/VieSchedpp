//
// Created by matth on 11.04.2018.
//

#include "HorizonMask_line.h"
using namespace std;
using namespace VieVS;

HorizonMask_line::HorizonMask_line(const vector<double> &azimuths, const vector<double> &elevations):
        azimuth_{azimuths}, elevation_{elevations} {
}

bool HorizonMask_line::visible(const PointingVector &pv) const noexcept {
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
    unsigned long begin = i - 1;
    unsigned long end = i;
    double delta = az-azimuth_.at(begin);
    double el_mask = elevation_.at(begin) + (elevation_.at(end)-elevation_.at(begin))/(azimuth_.at(end)-azimuth_.at(begin))*delta;

    return el >= el_mask;
}

string HorizonMask_line::vexOutput() const noexcept {
    return string();
}

