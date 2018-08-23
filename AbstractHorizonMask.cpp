/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   mask.cpp
 * Author: mschartn
 * 
 * Created on June 27, 2017, 12:58 PM
 */

#include "AbstractHorizonMask.h"
using namespace std;
using namespace VieVS;
unsigned long AbstractHorizonMask::nextId = 0;

AbstractHorizonMask::AbstractHorizonMask() : VieVS_Object(nextId++) {
}
