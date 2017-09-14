/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   mask.cpp
 * Author: mschartn
 * 
 * Created on June 27, 2017, 12:58 PM
 */

#include "HorizonMask.h"
using namespace std;
using namespace VieVS;

HorizonMask::HorizonMask(){
    type_ = Category::none;
}

HorizonMask::HorizonMask(const vector<double> &el_mask_deg)
{
    if (el_mask_deg.empty()){
        type_ = Category::none;
    } else {
        for(int i=0; i<el_mask_deg.size(); ++i){
            if (i%2==0)
                azimuth_.push_back(el_mask_deg[i]*deg2rad);
            else
                elevation_.push_back(el_mask_deg[i]*deg2rad);
        }


        if (azimuth_.size()%2==0){
            type_ = Category::line;
            if (*azimuth_.end() != twopi) {
                azimuth_.push_back(twopi);
                elevation_.push_back(elevation_[elevation_.size() - 1]);
            }


        } else {
            type_ = Category::step;
            if (*azimuth_.end() != twopi) {
                azimuth_.push_back(twopi);
                elevation_.push_back(elevation_[elevation_.size() - 1]);
            }
        }
    }
}


HorizonMask::~HorizonMask() = default;

bool HorizonMask::visible(const PointingVector &pv) const noexcept {
    bool visible = true;

    double az = pv.getAz();
    az = fmod(az,twopi);
    if(az<0){
        az+=twopi;
    }

    double el = pv.getEl();

    switch (type_){
        case Category::none:
            break;
        case Category::line:{
            int i = 1;

            while(az>azimuth_[i]){
                ++i;
            }

            int begin = i-1;
            int end = i;
            double delta = az-azimuth_[begin];
            double el_mask = elevation_[begin] + (elevation_[end]-elevation_[begin])/(azimuth_[end]-azimuth_[begin])*delta;
            if(el<el_mask){
                visible = false;
            }


            break;
        }
        case Category::step:{
            int i = 1;
            while(az>azimuth_[i]){
                ++i;
            }

            double el_mask = elevation_[i-1];
            if(el<el_mask){
                visible = false;
            }

            break;
        }
    }

    return visible;
}
