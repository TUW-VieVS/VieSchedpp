/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_scan.cpp
 * Author: mschartn
 * 
 * Created on June 29, 2017, 3:27 PM
 */

#include "VLBI_scan.h"

namespace VieVS{
    VLBI_scan::VLBI_scan() {
    }
    
    VLBI_scan::VLBI_scan(vector<VLBI_pointingVector> pointingVectors, vector<unsigned int> endOfLastScan, int minimumNumberOfStations):
    srcid{pointingVectors.at(0).getSrcid()}, pointingVectors{pointingVectors}, nsta{pointingVectors.size()}, minimumNumberOfStations{minimumNumberOfStations}{
        times = VLBI_scanTimes(nsta);
        times.setEndOfLastScan(endOfLastScan);
    }

    VLBI_scan::~VLBI_scan() {
    }
    
    void VLBI_scan::constructBaselines(){
        for (int i=0; i<pointingVectors.size(); ++i){
            for (int j=i+1; j<pointingVectors.size(); ++j){

                unsigned int startTime1 = times.getEndOfIdleTime(i);
                unsigned int startTime2 = times.getEndOfIdleTime(j);
                if (startTime1> startTime2){
                    baselines.push_back(VLBI_baseline(pointingVectors[i].getSrcid(), pointingVectors[i].getStaid(),
                                                      pointingVectors[j].getStaid(), startTime1));
                } else {
                    baselines.push_back(VLBI_baseline(pointingVectors[i].getSrcid(), pointingVectors[i].getStaid(),
                                                      pointingVectors[j].getStaid(), startTime2));
                }
            }
        }
    }

    void VLBI_scan::addTimes(int idx, VLBI_scan& other, int idx_other){
        times.addTimes(idx,other.getTimes(),idx_other);
    }

//    int VLBI_scan::idxLatestStation(unsigned int &time) {
//        auto it = max_element(endOfCalibrationTime.begin(),endOfCalibrationTime.end());
//        time = *it;
//        int idx = distance(endOfCalibrationTime.begin(),it);
//        int idx = time
//        return pointingVectors[idx].getStaid();
//    }
//
//    void VLBI_scan::updateStation(int idx,unsigned int slewtime, VLBI_pointingVector p){
//        int oldSlewTime = endOfSlewTime[idx]-endOfSourceTime[idx];
//        int delta = slewtime-oldSlewTime;
//
//        endOfSlewTime[idx] = endOfSlewTime[idx]+delta;
//        endOfIdleTime[idx] = endOfIdleTime[idx]+delta;
//        endOfTapeTime[idx] = endOfTapeTime[idx]+delta;
//        endOfCalibrationTime[idx] = endOfCalibrationTime[idx]+delta;
//    }

    void VLBI_scan::addTimes(int idx, unsigned int setup, unsigned int source, unsigned int slew, unsigned int tape,
                             unsigned int calib) {
        times.addTimes(idx, setup, source, slew, tape, calib);
    }

    bool VLBI_scan::removeElement(int idx) {
        --nsta;
        if (nsta<minimumNumberOfStations){
            return false;
        }

        times.removeElement(idx);

        int staid = pointingVectors[idx].getStaid();
        pointingVectors.erase(pointingVectors.begin()+idx);

        int i=0;
        while (i<baselines.size()){
            if(baselines[i].getStaid1()==staid || baselines[i].getStaid2()==staid){
                baselines.erase(baselines.begin()+i);
            }
        }

        return true;
    }

    bool VLBI_scan::checkIdleTimes(vector<unsigned int> maxIdle) {

        bool scan_valid = true;
        bool idleTimeValid;
        unsigned int latestSlewTime;
        do {
            idleTimeValid = true;

            vector<unsigned int> eosl = times.getEndOfSlewTime();
            auto it= max_element(eosl.begin(), eosl.end());
            latestSlewTime = *it;
            int idx = distance(eosl.begin(),it);

            vector<unsigned int > dt(nsta);
            for (int i = 0; i < nsta; ++i) {
                dt[i] = latestSlewTime-eosl[i];
                if (dt[i]>maxIdle[i]){
                    scan_valid = removeElement(idx);
                    if (scan_valid){
                        maxIdle.erase(maxIdle.begin()+idx);
                        idleTimeValid = false;
                        break;
                    } else {
                        break;
                    }
                }
            }

        }while(!idleTimeValid && !scan_valid);

        if(scan_valid){
            times.alignStartTimes(latestSlewTime);
        }

        return scan_valid;
    }

    void VLBI_scan::updateSlewtime(int idx, unsigned int new_slewtime){
        times.updateSlewtime(idx,new_slewtime);
    }


}
