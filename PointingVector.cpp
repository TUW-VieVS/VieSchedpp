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

PointingVector::PointingVector() = default;

PointingVector::PointingVector(int staid, int srcid): staid_{staid}, srcid_{srcid}{
}

