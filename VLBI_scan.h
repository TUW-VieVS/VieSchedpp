/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_scan.h
 * Author: mschartn
 *
 * Created on June 29, 2017, 3:27 PM
 */

#ifndef VLBI_SCAN_H
#define VLBI_SCAN_H


#include <iostream>
#include <vector>
#include <boost/format.hpp>
#include <boost/date_time.hpp>
#include <utility>
#include <limits>

#include "VLBI_pointingVector.h"
#include "VLBI_baseline.h"
#include "VLBI_scanTimes.h"
#include "VLBI_station.h"
#include "VLBI_source.h"
#include "VLBI_skyCoverage.h"
//#include "VieVS_helper.h"


using namespace std;
namespace VieVS{

    class VLBI_scan {
    public:
        struct SCORESTRUCT {
            double nunmberOfObservations = 0;
            double averageStations = 0;
            double averageSources = 0;
            double duration = 0;
            double skyCoverage = 0;
        };

        VLBI_scan();

        VLBI_scan(vector<VLBI_pointingVector> pointingVectors, vector<unsigned int> endOfLastScan, int minimumNumberOfStations);

        VLBI_scan(vector<VLBI_pointingVector> &pv, VLBI_scanTimes &times, vector<VLBI_baseline> &bl, int minNumSta);

        VLBI_scan(const VLBI_scan &other) = default;

        VLBI_scan(VLBI_scan &&other) = default;

        VLBI_scan &operator=(const VLBI_scan &other) = default;

        VLBI_scan &operator=(VLBI_scan &&other) = default;

        virtual ~VLBI_scan() {}
        
        const VLBI_scanTimes &getTimes() const {
            return times;
        }

        unsigned long getNSta() {
            return nsta;
        }

        int getStationId(int idx) {
            return pointingVectors[idx].getStaid();
        }

        int getSourceId(){
            return pointingVectors[0].getSrcid();
        }

        VLBI_pointingVector &getPointingVector(int idx) {
            return pointingVectors[idx];
        }

        VLBI_pointingVector &getPointingVectors_endtime(int idx) {
            return pointingVectors_endtime[idx];
        }

        vector<VLBI_baseline> &getBaselines() {
            return baselines;
        }

        int getMinimumNumberOfStations() const {
            return minimumNumberOfStations;
        }

        double getScore() const {
            return score;
        }

        bool removeElement(int idx);

        int findIdxOfStationId(int id);

        void setPointingVector(int i, VLBI_pointingVector &pointingVector) {
            pointingVectors[i] = pointingVector;
        }

        void addTimes(int idx, unsigned int setup, unsigned int source, unsigned int slew, unsigned int tape,
                      unsigned int calib);

        void constructBaselines();

        void updateSlewtime(int idx, unsigned int new_slewtime);

        bool checkIdleTimes(vector<unsigned int> maxIdle);

        void calcBaselineScanDuration(vector<VLBI_station>& stations,VLBI_source& sources, double mjdStart);

        bool scanDuration(vector<VLBI_station> &stations, VLBI_source &source);

        vector<int> getStationIds();

        bool removeAllBut(vector<int> &station_ids);

        void calcScore(unsigned long nmaxsta, unsigned long nmaxbl, vector<double> &astas, vector<double> &asrcs,
                       unsigned int minTime, unsigned int maxTime, vector<VLBI_skyCoverage> &skyCoverages);

        void calcScore_nunmberOfObservations(unsigned long maxObs);

        void calcScore_averageStations(vector<double> &astas, unsigned long nmaxsta);

        void calcScore_averageSources(vector<double> &asrcs);

        void calcScore_duration(unsigned int minTime, unsigned int maxTime);

        void calcScore_skyCoverage(vector<VLBI_skyCoverage> &skyCoverages);

        void sumScores();

        unsigned int maxTime() const;

        bool rigorousUpdate(vector<VLBI_station> &stations, VLBI_source &source, double mjdStart);

        VLBI_scan copyScan(vector<int> &scan1sta, bool &valid);

        unsigned long getNBl() {
            return baselines.size();
        }

        void output(unsigned long nr, vector<VLBI_station> &stations, VLBI_source &source,
                    boost::posix_time::ptime sessionStart);

    private:
        unsigned long nsta;
        int srcid;

        int minimumNumberOfStations;

        SCORESTRUCT single_scores;

        double score;

        VLBI_scanTimes times;
        vector<VLBI_pointingVector> pointingVectors;
        vector<VLBI_pointingVector> pointingVectors_endtime;
        vector<VLBI_baseline> baselines;

    };
}
#endif /* VLBI_SCAN_H */

