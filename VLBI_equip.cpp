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
    
    VLBI_equip::VLBI_equip(vector<string> all_channelNames, vector<double> corresponding_SEFDs):
    channelNames{all_channelNames}, SEFDs{corresponding_SEFDs}{}

    VLBI_equip::~VLBI_equip() {
    }
    
    ostream& operator<<(ostream& out, const VLBI_equip& equip){
        cout << "SEFD:\n";
        for (int i=0; i<equip.SEFDs.size(); ++i){
            cout << "    " << equip.channelNames[i] << ": " << equip.SEFDs[i] << "\n";
        }
        return out;
    }

}
