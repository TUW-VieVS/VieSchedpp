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

    VLBI_baseline::~VLBI_baseline() {
    }

}
