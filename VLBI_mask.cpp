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
    
    VLBI_mask::VLBI_mask(vector<double> el_mask_deg)
    {
        if (el_mask_deg.empty()){
            type = category::none;
        } else {
            for(int i=0; i<el_mask_deg.size(); ++i){
                if (i%2==0)
                    knots.push_back(el_mask_deg[i]*deg2rad);
                else
                    values.push_back(el_mask_deg[i]*deg2rad);
            }

            if (knots.size()%2==0){
                type = category::line;
            } else {
                type = category::step;
            }
        }
    }


    VLBI_mask::~VLBI_mask() {
    }
}