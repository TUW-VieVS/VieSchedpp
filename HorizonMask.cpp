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

#include "HorizonMask.h"
using namespace std;
using namespace VieVS;
unsigned long HorizonMask::nextId = 0;

HorizonMask::HorizonMask():VieVS_Object(nextId++){
}
