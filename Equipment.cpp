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

#include "Equipment.h"
using namespace std;
using namespace VieVS;

Equipment::Equipment(){}

Equipment::Equipment(const unordered_map<string, double> SEFDs): SEFD_{SEFDs}{

}

Equipment::~Equipment() {
}

namespace VieVS{
    ostream &operator<<(ostream &out, const Equipment &equip) noexcept {
        cout << "SEFD_:\n";
        for(auto& any: equip.SEFD_){
            cout << "Band: " << any.first << " " << any.second;
        }
        return out;
    }
}


double Equipment::getMaxSEFD() const noexcept {
    double maxSEFD = 0;
    for(auto& any: SEFD_){
        if(any.second>maxSEFD){
            maxSEFD = any.second;
        }
    }
    return maxSEFD;

}

