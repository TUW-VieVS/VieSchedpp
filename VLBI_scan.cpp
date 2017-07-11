/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_scan.cpp
 * Author: mschartn
 * 
 * Created on June 29, 2017, 3:27 PM
 */

#include "VLBI_scan.h"
namespace VieVS{
    VLBI_scan::VLBI_scan() {
    }
    
    VLBI_scan::VLBI_scan(vector<VLBI_pointingVector> pointingVectors):pointingVectors{pointingVectors}, nsta{pointingVectors.size()} {
    }

    VLBI_scan::~VLBI_scan() {
    }
}
