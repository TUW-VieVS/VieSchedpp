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
#include <boost/date_time.hpp>

#include "VieVS_constants.h"

using namespace std;
namespace VieVS{
    class VLBI_skyCoverage {
    public:
        VLBI_skyCoverage();
        VLBI_skyCoverage(int nStations);
        virtual ~VLBI_skyCoverage();
    private:
        int nStations;
        
        vector<double> az;
        vector<double> el;
        vector<boost::posix_time::ptime> startTime;
        vector<boost::posix_time::ptime> endTime;
        vector<int> stationId;
        vector<int> sourceId;
    };
}
#endif /* SKYCOVERAGE_H */

