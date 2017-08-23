/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   equip.cpp
 * Author: mschartn
 * 
 * Created on June 27, 2017, 11:54 AM
 */

#include "VLBI_equip.h"
namespace VieVS{
    VLBI_equip::VLBI_equip(){}
    
    VLBI_equip::VLBI_equip(const vector<string> all_channelNames, const vector<double> corresponding_SEFDs){
        for (int i = 0; i < all_channelNames.size(); ++i) {
            SEFD.insert(make_pair(all_channelNames[i],corresponding_SEFDs[i]));
        }
    }

    VLBI_equip::~VLBI_equip() {
    }
    
    ostream& operator<<(ostream& out, const VLBI_equip& equip){
        cout << "SEFD:\n";
        for(auto& any: equip.SEFD){
            cout << "Band: " << any.first << " " << any.second;
        }
        return out;
    }

    double VLBI_equip::getMaxSEFD() const {
        double maxSEFD = 0;
        for(auto& any: SEFD){
            if(any.second>maxSEFD){
                maxSEFD = any.second;
            }
        }
        return maxSEFD;

    }

}
