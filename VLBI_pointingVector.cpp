/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_pointingVector.cpp
 * Author: mschartn
 * 
 * Created on July 6, 2017, 1:24 PM
 */

#include "VLBI_pointingVector.h"
namespace VieVS{
    VLBI_pointingVector::VLBI_pointingVector() {
    }
    
    VLBI_pointingVector::VLBI_pointingVector(int staid, int srcid):
    staid{staid}, srcid{srcid}{
    }
    
    VLBI_pointingVector::~VLBI_pointingVector() {
    }
}
