/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PointingVector.cpp
 * Author: mschartn
 * 
 * Created on July 6, 2017, 1:24 PM
 */

#include "PointingVector.h"
using namespace std;
using namespace VieVS;
int PointingVector::nextId = 0;

//PointingVector::PointingVector():VieVS_Object(nextId++), staid_{-1}, srcid_{-1}{
//}

PointingVector::PointingVector(int staid, int srcid):VieVS_Object(nextId++), staid_{staid}, srcid_{srcid}{
}

PointingVector::PointingVector(const PointingVector &other):
        VieVS_Object(nextId++), staid_{other.staid_}, srcid_{other.srcid_},
        az_{other.az_}, el_{other.el_}, ha_{other.ha_}, dc_{other.dc_}, time_{other.time_}{
}

void PointingVector::copyValuesFromOtherPv(const PointingVector &other) {
    staid_ = other.staid_;
    srcid_ = other.srcid_;
    az_ = other.az_;
    el_ = other.el_;
    ha_ = other.ha_;
    dc_ = other.dc_;
    time_ = other.time_;
}

