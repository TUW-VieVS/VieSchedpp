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

Equipment::Equipment(const unordered_map<string, double> &SEFDs): SEFD_{SEFDs}, elevationDependentSEFD_{false}{

}

Equipment::Equipment(const std::unordered_map<std::string, double> SEFDs,
                     const std::unordered_map<std::string, double> y, const std::unordered_map<std::string, double> c0,
                     const std::unordered_map<std::string, double> c1):
        SEFD_{SEFDs}, elevationDependentSEFD_{true}, y_{y}, c0_{c0}, c1_{c1}{

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

double Equipment::getSEFD(const std::string &band, double el) const noexcept {
    double y = y_.at(band);
    double c0 = c0_.at(band);
    double c1 = c1_.at(band);

    double tmp = pow(sin(el),y);
    double tmp2 = c0 + c1/tmp;

    if(tmp2<1){
        return SEFD_.at(band);
    }else{
        return SEFD_.at(band)*tmp2;
    }
}


