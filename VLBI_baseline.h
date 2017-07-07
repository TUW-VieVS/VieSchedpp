/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   baseline.h
 * Author: mschartn
 *
 * Created on June 29, 2017, 3:25 PM
 */

#ifndef BASELINE_H
#define BASELINE_H
#include <iostream>
#include "VLBI_obs.h"
#include "VieVS_constants.h"

using namespace std;
namespace VieVS{
    class VLBI_baseline {
    public:
        VLBI_baseline();
        virtual ~VLBI_baseline();
    private:
        VLBI_obs sta1;
        VLBI_obs sta2;
    };
}
#endif /* BASELINE_H */

