/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_pointingVector.h
 * Author: mschartn
 *
 * Created on July 6, 2017, 1:24 PM
 */

#ifndef VLBI_POINTINGVECTOR_H
#define VLBI_POINTINGVECTOR_H
#include <iostream>
#include <boost/date_time.hpp>

#include "VieVS_constants.h"

using namespace std;

namespace VieVS{
    class VLBI_pointingVector {
    public:
        VLBI_pointingVector();
        VLBI_pointingVector(int staid, int srcid);

        VLBI_pointingVector(const VLBI_pointingVector &other) = default;

        VLBI_pointingVector(VLBI_pointingVector &&other) = default;

        VLBI_pointingVector &operator=(const VLBI_pointingVector &other) = default;

        VLBI_pointingVector &operator=(VLBI_pointingVector &&other) = default;

        virtual ~VLBI_pointingVector() {}


        int getStaid(){return staid;}
        int getSrcid(){return srcid;}
        double getAz(){return az;}
        double getEl(){return el;}
        unsigned int getTime(){return time;}
        
        void setAz(double new_az){az = new_az;};
        void setEl(double new_el){el = new_el;};
        void setTime(unsigned int new_time){time = new_time;};
        
    private:
        int staid;
        int srcid;
        double az;
        double el;
        unsigned int time;
    };
}
#endif /* VLBI_POINTINGVECTOR_H */

