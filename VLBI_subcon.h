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
#include <limits>
#include <queue>

#include "VLBI_station.h"
#include "VLBI_source.h"
#include "VLBI_scan.h"
#include "VLBI_skyCoverage.h"

using namespace std;

namespace VieVS{
    class VLBI_subcon {
    public:
        VLBI_subcon();

        virtual ~VLBI_subcon() {};
        
        void addScan(VLBI_scan scan);

        int getNumberSingleScans() const {
            return n1scans;
        }

        int getNumberSubnettingScans() const {
            return n2scans;
        }

        const vector<VLBI_scan> &getSubnet1() const {
            return subnet1;
        }

        VLBI_scan getSingleSourceScan(int idx) const {
            return *(subnet1.begin() + idx);
        }

        pair<VLBI_scan, VLBI_scan> getDoubleSourceScan(int idx) const {
            return *(subnet2.begin() + idx);
        }

        void calcStartTimes(vector<VLBI_station> &stations, vector<VLBI_source> &sources);

        void constructAllBaselines();

        void updateAzEl(vector<VLBI_station> &stations, vector<VLBI_source> &sources);

        void calcAllBaselineDurations(vector<VLBI_station> &stations, vector<VLBI_source> &sources, double mjdStart);

        void calcAllScanDurations(vector<VLBI_station> &stations, vector<VLBI_source> &sources);

        void createSubcon2(vector<vector<int>> &subnettingSrcIds, int minStaPerSubcon);

        void generateScore(vector<VLBI_station> &stations,
                           vector<VLBI_skyCoverage> &skyCoverages);

        void precalcScore(vector<VLBI_station> &stations, vector<VLBI_source> &sources);

        void minMaxTime();

        void average_station_score(const vector<VLBI_station> &stations);

        void average_source_score(vector<VLBI_source> &vector);

        void calcScores();

        int rigorousScore(vector<VLBI_station> &stations, vector<VLBI_source> &sources,
                          vector<VLBI_skyCoverage> &skyCoverages, double mjdStart);

    private:
        int n1scans;
        vector<VLBI_scan> subnet1;
        vector<double> subnet1_score;

        int n2scans;
        vector<pair<VLBI_scan,VLBI_scan> > subnet2;
        vector<double> subnet2_score;

        unsigned long nmaxbl;
        unsigned int minTime;
        unsigned int maxTime;
        vector<double> astas;
        vector<double> asrcs;

    };
}
#endif /* VLBI_SUBCON_H */

