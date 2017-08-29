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

#include "VLBI_mask.h"
namespace VieVS{
    VLBI_mask::VLBI_mask(){
        type = category::none;
    }
    
    VLBI_mask::VLBI_mask(const vector<double> &el_mask_deg)
    {
        if (el_mask_deg.empty()){
            type = category::none;
        } else {
            for(int i=0; i<el_mask_deg.size(); ++i){
                if (i%2==0)
                    azimuth.push_back(el_mask_deg[i]*deg2rad);
                else
                    elevation.push_back(el_mask_deg[i]*deg2rad);
            }

            if (azimuth.size()%2==0){
                type = category::line;
            } else {
                type = category::step;
            }
        }
    }


    VLBI_mask::~VLBI_mask() {
    }

    bool VLBI_mask::visible(const VLBI_pointingVector &pv) const noexcept {
        bool visible = true;

        double az = pv.getAz();
        az = fmod(az,twopi);
        if(az<0){
            az+=twopi;
        }

        double el = pv.getEl();

        switch (type){
            case category::none:
                break;
            case category::line:{
                int i = 1;

                while(az>azimuth[i]){
                    ++i;
                }

                int begin = i-1;
                int end = i;
                double delta = az-azimuth[begin];
                double el_mask = elevation[begin] + (elevation[end]-elevation[begin])/(azimuth[end]-azimuth[begin])*delta;
                if(el<el_mask){
                    visible = false;
                }


                break;
            }
            case category::step:{
                int i = 1;
                while(az>azimuth[i]){
                    ++i;
                }

                double el_mask = elevation[i-1];
                if(el<el_mask){
                    visible = false;
                }

                break;
            }
        }

        return visible;
    }
}