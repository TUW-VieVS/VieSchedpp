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

double HorizonMask::minElevation = 0;

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
                azimuth_.push_back(el_mask_deg.at(i)*deg2rad);
            else
                elevation_.push_back(el_mask_deg.at(i)*deg2rad);
        }


        if (el_mask_deg.size()%2==0){
            type_ = Category::line;
            if (azimuth_.back() != twopi) {
                azimuth_.push_back(twopi);
                elevation_.push_back(elevation_.at(elevation_.size() - 1));
            }


        } else {
            type_ = Category::step;
            if (azimuth_.back() != twopi) {
                azimuth_.push_back(twopi);
                elevation_.push_back(elevation_.at(elevation_.size() - 1));
            }
        }
    }
}



bool HorizonMask::visible(const PointingVector &pv) const noexcept {

    double az = pv.getAz();
    az = fmod(az,twopi);
    if(az<0){
        az+=twopi;
    }

    double el = pv.getEl();
    if (el < minElevation) {
        return false;
    }

    switch (type_){
        case Category::none:
            break;
        case Category::line:{
            int i = 1;

            while(az>azimuth_.at(i)){
                ++i;
            }

            int begin = i-1;
            int end = i;
            double delta = az-azimuth_.at(begin);
            double el_mask = elevation_.at(begin) + (elevation_.at(end)-elevation_.at(begin))/(azimuth_.at(end)-azimuth_.at(begin))*delta;
            if(el<el_mask){
                return false;
            }


            break;
        }
        case Category::step:{
            int i = 1;
            while(az>azimuth_.at(i)){
                ++i;
            }

            double el_mask = elevation_.at(i-1);
            if(el<el_mask){
                return false;
            }

            break;
        }
    }

    return true;
}
