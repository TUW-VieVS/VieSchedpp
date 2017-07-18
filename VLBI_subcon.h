/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_subcon.h
 * Author: mschartn
 *
 * Created on June 29, 2017, 5:51 PM
 */

#ifndef VLBI_SUBCON_H
#define VLBI_SUBCON_H
#include <vector>
#include <utility>

#include "VLBI_station.h"
#include "VLBI_source.h"
#include "VLBI_scan.h"

using namespace std;

namespace VieVS{
    class VLBI_subcon {
    public:
        
        VLBI_subcon();
        
        virtual ~VLBI_subcon();
        
        void addScan(VLBI_scan scan);
        
        void addScan(VLBI_scan scan1, VLBI_scan scan2);
        
        int getNumberSingleScans(){return n1scans;}
        
        int getNumberSubnettingScans(){return n2scans;}
        
        void calcStartTimes(vector<VLBI_station> &stations, vector<VLBI_source> &sources);

        void constructAllBaselines();

        void updateAzEl(vector<VLBI_station> &stations, vector<VLBI_source> &sources);

        void calcAllBaselineDurations(vector<VLBI_station> &stations, vector<VLBI_source> &sources, double mjdStart);

        void calcAllScanDurations(vector<VLBI_station>& stations, vector<VLBI_source>& sources);

    private:
        int n1scans;
        vector<VLBI_scan> subnet1;
        int n2scans;
        vector<pair<VLBI_scan,VLBI_scan> > subnet2;
    };
}
#endif /* VLBI_SUBCON_H */

