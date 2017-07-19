/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_scheduler.h
 * Author: mschartn
 *
 * Created on June 29, 2017, 2:29 PM
 */

#ifndef VLBI_SCHEDULER_H
#define VLBI_SCHEDULER_H
#include <vector>
#include <boost/date_time.hpp>
#include <bitset>

#include "VLBI_initializer.h"
#include "VLBI_subcon.h"
#include "VieVS_constants.h"

using namespace std;
namespace VieVS{
    class VLBI_scheduler {
    public:
        struct PARAMETERS { 
            bool subnetting = true;
            bool fillinmode = true;
            unsigned int minStaPerFillin = 2;
            double minAngleBetweenSubnettingSources = 120*deg2rad;
            double skyCoverageInterval = 3600;
            
            double weightNumberOfObs = 1/3;
            double weightSkyCoverage = 1/3;
            double weightScanEndTime = 1/3;
            
            boost::posix_time::ptime startTime;
            boost::posix_time::ptime endTime;
            double mjdStart;
            unsigned int currentTime;
        };
        
        struct PRECALC{
            vector<vector<int>> subnettingSrcIds;
        };
        
        VLBI_scheduler();
        
        VLBI_scheduler(VLBI_initializer init);
        
        void start();
        
        VLBI_subcon allVisibleScans();

        void precalcSubnettingSrcIds();
        
        virtual ~VLBI_scheduler();
        
    private:
        vector<VLBI_station> stations;
        vector<VLBI_source> sources;
        vector<VLBI_skyCoverage> skyCoverages;
        PARAMETERS PARA;
        PRECALC PRE;
        vector<VLBI_scan> scans;
    };
}
#endif /* VLBI_SCHEDULER_H */

