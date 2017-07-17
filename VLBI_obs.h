/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   obs.h
 * Author: mschartn
 *
 * Created on June 29, 2017, 3:24 PM
 */

#ifndef OBS_H
#define OBS_H
#include <iostream>
#include "VLBI_pointingVector.h"
#include "VieVS_constants.h"

using namespace std;
namespace VieVS{
    class VLBI_obs {
    public:
        VLBI_obs();
        virtual ~VLBI_obs();
    private:
        VLBI_pointingVector start;
        VLBI_pointingVector end;
    };
}
#endif /* OBS_H */

