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

#include <set>
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
            } else {
                ++i;
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
            times.alignStartTimes();
        }

        return scan_valid;
    }

    void VLBI_scan::updateSlewtime(int idx, unsigned int new_slewtime){
        times.updateSlewtime(idx,new_slewtime);
    }

    void
    VLBI_scan::calcBaselineScanDuration(vector<VLBI_station> &stations, VLBI_source& source, double mjdStart) {

        for (int ibl = 0; ibl < baselines.size(); ++ibl) {
            VLBI_baseline& thisBaseline = baselines[ibl];
            int staid1 = thisBaseline.getStaid1();
            int staid2 = thisBaseline.getStaid2();
            unsigned int startTime = thisBaseline.getStartTime();

            double date1 = 2400000.5;
            double date2 = mjdStart + startTime/86400;
            double gmst = iauGmst82(date1, date2);

            unordered_map<string,unsigned int> durations;
            unordered_map<string,double> flux = source.observedFlux(gmst,stations[staid1].dx(staid2),stations[staid1].dy(staid2),stations[staid1].dz(staid2));
            for(auto& any:flux){
                string fluxname = any.first;
                double SEFD_src = any.second;

                // TODO: check if band is in station and source
                bool bandsFound = true;
                double SEFD_sta1 = stations[staid1].getSEFD(fluxname);
                double SEFD_sta2 = stations[staid2].getSEFD(fluxname);
                double minSNR_sta1 = stations[staid1].getMinSNR(fluxname);
                double minSNR_sta2 = stations[staid2].getMinSNR(fluxname);


                unordered_map<string,double> minSNRs_src = source.getMinSNR();
                double minSNR_src = 0;
                auto it_src = minSNRs_src.find(fluxname);
                if (it_src != minSNRs_src.end()){
                    minSNR_src = it_src->second;
                }


                double maxminSNR = minSNR_src;
                if (minSNR_sta1>minSNR_src){
                    maxminSNR = minSNR_sta1;
                } if (minSNR_sta2>minSNR_src){
                    maxminSNR = minSNR_sta2;
                }


                double maxCorSynch1 = stations[staid1].getWaitCorsynch();
                double maxCorSynch = maxCorSynch1;
                double maxCorSynch2 = stations[staid2].getWaitCorsynch();
                if (maxCorSynch2 > maxCorSynch){
                    maxCorSynch = maxCorSynch2;
                }

                if (bandsFound){

                    double anum = (1.75*maxminSNR / SEFD_src);
                    double anu1 = SEFD_sta1*SEFD_sta2;
                    // TODO: do not hardcode observing mode!!!
                    double anu2 = 1024 * 1.0e6 * 16 * 2;

                    double new_duration = anum*anum *anu1/anu2 + maxCorSynch;
                    new_duration = ceil(new_duration);
                    unsigned int new_duration_uint = (unsigned int) new_duration;
                    durations.insert(make_pair(fluxname,new_duration_uint));
                }else{
                    cerr << "WARNING: duration of band " << fluxname << " ignored\n";
                }
            }
            thisBaseline.setObservedFlux(flux);
            thisBaseline.setScanDuration(durations);
            cout << "worked!";
        }


    }

    bool VLBI_scan::scanDuration(vector<VLBI_station> &stations, VLBI_source &source) {

        bool scanDurationsValid = false;
        bool scanValid = true;

        vector<unsigned int> minscanTimes(nsta,source.getMinScanTime());
        vector<unsigned int> maxScanTimes(nsta,source.getMaxScanTime());

        for (int i = 0; i < nsta; ++i) {
            unsigned int stationMinScanTime = stations[pointingVectors[i].getStaid()].getMinScanTime();
            unsigned int stationMaxScanTime = stations[pointingVectors[i].getStaid()].getMaxScanTime();

            if(minscanTimes[i]<stationMinScanTime){
                minscanTimes[i] = stationMinScanTime;
            }
            if(maxScanTimes[i]>stationMaxScanTime){
                maxScanTimes[i] = stationMaxScanTime;
            }
        }

        vector<unsigned int> scanTimes(nsta);
        do{
            scanDurationsValid = true;

            vector<int> eraseStations1;
            vector<int> eraseStations2;
            for (int i = 0; i < nsta; ++i) {
                scanTimes[i] = 0;
            }


            for (int i = 0; i < baselines.size(); ++i) {
                VLBI_baseline& thisBaseline = baselines[i];
                int staid1 = thisBaseline.getStaid1();
                int staidx1 = findIdxOfStationId(staid1);
                int staid2 = thisBaseline.getStaid2();
                int staidx2 = findIdxOfStationId(staid2);

                unsigned int duration = thisBaseline.getScanDuration();
                if(scanTimes[staidx1]<duration){
                    scanTimes[staidx1] = duration;
                }
                if(scanTimes[staidx2]<duration){
                    scanTimes[staidx2] = duration;
                }

                if(duration>maxScanTimes[staidx1] || duration>maxScanTimes[staidx2]){
                    eraseStations1.push_back(staidx1);
                    eraseStations2.push_back(staidx2);
                    scanDurationsValid = false;
                }
            }


            if(eraseStations1.size()>0){
                int eraseThis;

                vector<int> counter(nsta);
                for (int i = 0; i < eraseStations1.size(); ++i) {
                    counter[eraseStations1[i]]++;
                    counter[eraseStations2[i]]++;
                }

                int max = 0;
                vector<int> maxIdx;
                for (int i = 0; i< nsta; ++i){
                    if(counter[i] == max){
                        maxIdx.push_back(i);
                    }
                    if(counter[i] > max){
                        max=counter[i];
                        maxIdx.clear();
                        maxIdx.push_back(i);
                    }
                }

                if(maxIdx.size()==1){
                    eraseThis = maxIdx[0];
                } else {

                    vector<double> maxFlux(maxIdx.size());
                    vector<int> maxFluxIdx(maxIdx.size());
                    for(int i=0; i<maxIdx.size(); ++i){
                        int thisIdx = maxIdx[i];
                        maxFluxIdx[i] = thisIdx;
                        int id = pointingVectors[thisIdx].getStaid();
                        maxFlux[i] = stations[id].getMaxSEFT();
                    }

                    if(maxFluxIdx.size()==1){
                        eraseThis = maxFluxIdx[0];
                    } else {
                        vector<unsigned int> thisScanStartTimes(maxFluxIdx.size());
                        for(int i=0; i<maxFluxIdx.size(); ++i){
                            thisScanStartTimes.push_back(times.getEndOfSlewTime(maxFluxIdx[i]));
                        }

                        int maxmaxFluxIdx = distance(thisScanStartTimes.begin(), max_element(thisScanStartTimes.begin(), thisScanStartTimes.end()));
                        eraseThis = maxFluxIdx[maxmaxFluxIdx];
                    }
                }

                scanValid = removeElement(eraseThis);
                scanTimes.erase(scanTimes.begin()+eraseThis);

                if (!scanValid){
                    break;
                }
            }

        }while(!scanDurationsValid);

        times.addScanTimes(scanTimes);


        return scanValid;
    }

    int VLBI_scan::findIdxOfStationId(int id) {
        for (int idx = 0; idx < nsta; ++idx) {
            if(pointingVectors[idx].getStaid()==id){
                return idx;
            }
        }
        return -1;
    }


}
