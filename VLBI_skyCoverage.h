/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   skyCoverage.h
 * Author: mschartn
 *
 * Created on June 29, 2017, 11:28 AM
 */

#ifndef SKYCOVERAGE_H
#define SKYCOVERAGE_H
#include <vector>
#include <iostream>
#include <cmath>
#include <limits>
#include <boost/math/distributions/normal.hpp>
#include "VLBI_pointingVector.h"

using boost::math::normal;
using namespace std;
namespace VieVS{
    class VLBI_skyCoverage {
    public:
        VLBI_skyCoverage();

        VLBI_skyCoverage(vector<int> &staids);

        const vector<int> &getStaids() const {
            return staids;
        }

        double calcScore(vector<VLBI_pointingVector> &pvs);

        void update(VLBI_pointingVector &start, VLBI_pointingVector &end);

        virtual ~VLBI_skyCoverage();

        static vector<int> sta2sky;
    private:
        unsigned long nStations;
        vector<int> staids;

        vector<VLBI_pointingVector> pv_start;
        vector<VLBI_pointingVector> pv_end;
    };
}

#endif /* SKYCOVERAGE_H */

