/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   baseline.cpp
 * Author: mschartn
 * 
 * Created on June 29, 2017, 3:25 PM
 */

#include "VLBI_baseline.h"

namespace VieVS{
    VLBI_baseline::VLBI_baseline(){
    }

    VLBI_baseline::VLBI_baseline(int srcid, int staid1, int staid2, unsigned int startTime)
            : srcid(srcid), staid1(staid1), staid2(staid2), startTime{startTime}{
    }

    void VLBI_baseline::setScanDuration(unordered_map<string,unsigned int>& scanDurations) {
        VLBI_baseline::scanDurations = scanDurations;

        auto x = max_element(scanDurations.begin(), scanDurations.end(),
                             [](const pair<string,unsigned int>& p1, const pair<string,unsigned int>& p2) {
                                 return p1.second < p2.second; });

        scanDuration = x->second;
    }

    string VLBI_baseline::longestScanDurationBand() {
        string band;
        unsigned int max = 0;
        for(const auto& any:scanDurations ){
            if(any.second>max){
                band = any.first;
                max = any.second;
            }
        }
        return band;
    }

}
