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

#include "VLBI_pointingVector.h"
#include "VLBI_baseline.h"
#include "VLBI_scanTimes.h"
#include "VLBI_station.h"
#include "VLBI_source.h"


using namespace std;
namespace VieVS{

    class VLBI_scan {
    public:
        VLBI_scan();
        VLBI_scan(vector<VLBI_pointingVector> pointingVectors, vector<unsigned int> endOfLastScan, int minimumNumberOfStations);

        const VLBI_scanTimes &getTimes() const {
            return times;
        }

        int getNSta(){
            return nsta;
        }

        int getStationId(int i){
            return pointingVectors.at(i).getStaid();
        }

        int getSourceId(){
            return pointingVectors[0].getSrcid();
        }

        VLBI_pointingVector& getPointingVector(int i){
            return pointingVectors.at(i);
        }

        vector<VLBI_baseline> &getBaselines() {
            return baselines;
        }

        bool removeElement(int idx);

        int findIdxOfStationId(int id);

        void setPointingVector(int i, VLBI_pointingVector& pointingVector){pointingVectors[i] = pointingVector;}

        void addTimes(int idx, unsigned int setup, unsigned int source, unsigned int slew, unsigned int tape,
                      unsigned int calib);

        void addTimes(int idx, VLBI_scan& other, int idx_other);

        void constructBaselines();

        void updateSlewtime(int idx, unsigned int new_slewtime);

        bool checkIdleTimes(vector<unsigned int> maxIdle);

        void calcBaselineScanDuration(vector<VLBI_station>& stations,VLBI_source& sources, double mjdStart);

        bool scanDuration(vector<VLBI_station> &stations, VLBI_source &source);

//        int idxLatestStation(unsigned int &time);
//        void updateStation(int idx,unsigned int slewtime, VLBI_pointingVector p);

        virtual ~VLBI_scan();
    private:
        int nsta;
        int srcid;

        int minimumNumberOfStations;

        VLBI_scanTimes times;
        vector<VLBI_pointingVector> pointingVectors;
        vector<VLBI_baseline> baselines;
    };
}
#endif /* VLBI_SCAN_H */

