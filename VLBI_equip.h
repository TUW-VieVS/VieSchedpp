/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   equip.h
 * Author: mschartn
 *
 * Created on June 27, 2017, 11:54 AM
 */

#ifndef VLBI_EQUIP_H
#define VLBI_EQUIP_H

#include <vector>
#include <unordered_map>
#include <iostream>

using namespace std;

namespace VieVS{
    class VLBI_equip {
    public:
        VLBI_equip();
        VLBI_equip(vector<string> all_channelNames, vector<double> corresponding_SEFDs);
        virtual ~VLBI_equip();

        const unordered_map<string, double> &getSEFD() const {
            return SEFD;
        }

        friend ostream& operator<<(ostream& out, const VLBI_equip& equip);
        
    private:
        unordered_map<string,double> SEFD;
    };
}
#endif /* VLBI_EQUIP_H */

