/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_subcon.cpp
 * Author: mschartn
 * 
 * Created on June 29, 2017, 5:51 PM
 */

#include "VLBI_subcon.h"
namespace VieVS{
    VLBI_subcon::VLBI_subcon(): n1scans{0}, n2scans{0} {
    }

    VLBI_subcon::~VLBI_subcon() {
    }
    
    void VLBI_subcon::addScan(VLBI_scan scan){
        subnet1.push_back(scan);
        n1scans++;
    }
    
    void VLBI_subcon::addScan(VLBI_scan scan1, VLBI_scan scan2){
        subnet2.push_back(make_pair(scan1,scan2));
        n2scans++;
    }
    
    void VLBI_subcon::calcSlewTimes(vector<VLBI_station> stations){
        for(int i=0; i<n1scans; ++i){
            vector<VLBI_pointingVector> pointingVectors = subnet1[i].getPointingVectors();
            for(int j=0; j<pointingVectors.size(); ++j){
                stations[pointingVectors[j].getStaid()].slewTo(pointingVectors[j]);
            }
        }
        
        for(int i=0; i<n2scans; ++i){
            
        }
    }
    
}
