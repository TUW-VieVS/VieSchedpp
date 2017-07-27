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
        baselines.clear();
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

        vector<unsigned int> scanTimes(minscanTimes);
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

                    double maxFlux = 0;
                    vector<int> maxFluxIdx(maxIdx.size());
                    for(int i=0; i<maxIdx.size(); ++i){
                        int thisIdx = maxIdx[i];
                        int id = pointingVectors[thisIdx].getStaid();
                        double thisMaxFlux = stations[id].getMaxSEFT();
                        if (thisMaxFlux == maxFlux) {
                            maxFluxIdx.push_back(thisIdx);
                        }
                        if (thisMaxFlux > maxFlux) {
                            maxFlux = thisMaxFlux;
                            maxFluxIdx.clear();
                            maxFluxIdx.push_back(i);
                        }
                    }

                    if(maxFluxIdx.size()==1){
                        eraseThis = maxFluxIdx[0];
                    } else {
                        vector<unsigned int> thisScanStartTimes(maxFluxIdx.size());
                        for(int i=0; i<maxFluxIdx.size(); ++i){
                            thisScanStartTimes[(times.getEndOfSlewTime(maxFluxIdx[i]))];
                        }

                        long maxmaxFluxIdx = distance(thisScanStartTimes.begin(),
                                                      max_element(thisScanStartTimes.begin(),
                                                                  thisScanStartTimes.end()));
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

    vector<int> VLBI_scan::getStationIds() {
        vector<int> ids;
        for (int i = 0; i < nsta; ++i) {
            ids.push_back(pointingVectors[i].getStaid());
        }

        return ids;
    }

    bool VLBI_scan::removeAllBut(vector<int> &station_ids) {
        int i = 0;
        bool valid = true;
        while (i < nsta) {
            int thisId = pointingVectors[i].getStaid();
            if (find(station_ids.begin(), station_ids.end(), thisId) == station_ids.end()) {
                valid = removeElement(i);
                if (!valid) {
                    break;
                }
            } else {
                ++i;
            }
        }
        if (!valid) {
            cout << "HERE!";
        }
        return valid;
    }

    unsigned int VLBI_scan::maxTime() const {
        return times.maxTime();
    }

    void VLBI_scan::calcScore_nunmberOfObservations(unsigned long maxObs) {
        int nbl = baselines.size();
        double thisScore = (double) nbl / (double) maxObs;
        single_scores.nunmberOfObservations = thisScore;
    }

    void VLBI_scan::calcScore_averageStations(vector<double> &astas, unsigned long nmaxsta) {
        double finalScore = 0;

        vector<unsigned long> bl_counter(nmaxsta);
        for (auto &bl:baselines) {
            ++bl_counter[bl.getStaid1()];
            ++bl_counter[bl.getStaid2()];
        }

        for (auto &pv:pointingVectors) {
            int thisStaId = pv.getStaid();
            finalScore += astas[thisStaId] * bl_counter[thisStaId] / (nmaxsta - 1);
        }

        single_scores.averageStations = finalScore;
    }

    void VLBI_scan::calcScore_averageSources(vector<double> &asrcs, unsigned long nmaxbl) {
        unsigned long nbl = baselines.size();
        single_scores.averageSources = asrcs[srcid] * nbl / nmaxbl;
    }

    void VLBI_scan::calcScore_duration(unsigned int minTime, unsigned int maxTime, unsigned long nmaxsta) {
        unsigned int thisEndTime = times.maxTime();
        double score = 1 - ((double) thisEndTime - (double) minTime) / (maxTime - minTime);
        single_scores.duration = score * nsta / nmaxsta;
    }

    void VLBI_scan::calcScore_skyCoverage(vector<VLBI_skyCoverage> &skyCoverages, unsigned long nmaxsta) {
        vector<vector<int>> pv2sky(skyCoverages.size());

        vector<int> sta2sky = VLBI_skyCoverage::sta2sky;

        for (int i = 0; i < nsta; ++i) {
            VLBI_pointingVector &pv = pointingVectors[i];
            int thisStaId = pv.getStaid();
            int skyCovId = sta2sky[thisStaId];

            pv2sky[skyCovId].push_back(i);
        }


        vector<double> singleScores(skyCoverages.size(), 0);
        for (int i = 0; i < skyCoverages.size(); ++i) {
            vector<int> pvIds = pv2sky[i];

            vector<VLBI_pointingVector> pvs;
            for (auto &id:pvIds) {
                pvs.push_back(pointingVectors[id]);
            }
            singleScores[i] = skyCoverages[i].calcScore(pvs);
        }


        single_scores.skyCoverage = accumulate(singleScores.begin(), singleScores.end(), 0.0) / nmaxsta;
    }

    void VLBI_scan::sumScores() {
        score = single_scores.skyCoverage +
                single_scores.averageSources +
                single_scores.averageStations +
                single_scores.nunmberOfObservations +
                single_scores.duration;
    }

    bool VLBI_scan::rigorousUpdate(vector<VLBI_station> &stations, VLBI_source &source, double mjdStart) {
        bool flag = true;
        int srcid = source.getId();

        bool scanValid = true;
        int i = 0;
        while (i < nsta) {

            VLBI_pointingVector &pv = pointingVectors[i];
            unsigned int slewStart = times.getEndOfSourceTime(i);
            unsigned int slewEnd = times.getEndOfSlewTime(i);
            int staid = pv.getStaid();
            VLBI_station &thisSta = stations[staid];

            VLBI_pointingVector new_p(staid, srcid);
            unsigned int oldSlewEnd;
            unsigned int newSlewEnd = slewEnd;
            unsigned int slewtime;
            bool stationValid = true;
            do {
                oldSlewEnd = newSlewEnd;
                new_p.setTime(newSlewEnd);

                stationValid = thisSta.isVisible(source, new_p);
                if (!stationValid) {
                    scanValid = removeElement(i);
                    if (!scanValid) {
                        return false;
                    }
                }
                slewtime = stations[staid].unwrapAzGetSlewTime(new_p);
                newSlewEnd = slewStart + slewtime;
            } while (abs(newSlewEnd - oldSlewEnd) > 1);

            if (stationValid) {
                pointingVectors[i] = new_p;
                times.updateSlewtime(i, slewtime);
                ++i;
            }
        }

        times.alignStartTimes();

        constructBaselines();
        calcBaselineScanDuration(stations, source, mjdStart);
        scanValid = scanDuration(stations, source);

        if (!scanValid) {
            return false;
        }

        i = 0;
        while (i < nsta) {

            unsigned int scanStart = times.getEndOfCalibrationTime(i);
            unsigned int scanEnd = times.getEndOfScanTime(i);
            int thisStaId = pointingVectors[i].getStaid();

            VLBI_pointingVector pv(thisStaId, srcid);
            VLBI_station thisSta = stations[thisStaId];

            bool stationValid = true;

            for (unsigned int j = scanStart; j < scanEnd; j += 10) {
                pv.setTime(j);
                stationValid = thisSta.isVisible(source, pv);
                if (!stationValid) {
                    scanValid = removeElement(i);
                    if (!scanValid) {
                        return false;
                    }
                    break;
                }
                if (j == scanStart) {
                    pointingVectors[i] = pv;
                }
            }

            if (!stationValid) {
                continue;
            }

            pv.setTime(scanEnd);
            stationValid = thisSta.isVisible(source, pv);
            if (!stationValid) {
                scanValid = removeElement(i);
                if (!scanValid) {
                    return false;
                }
                continue;
            }

            pointingVectors_endtime.push_back(pv);

            ++i;
        }
        return true;
    }

    void VLBI_scan::calcScore(unsigned long nmaxsta, unsigned long nmaxbl, vector<double> &astas, vector<double> &asrcs,
                              unsigned int minTime, unsigned int maxTime, vector<VLBI_skyCoverage> &skyCoverages) {
        calcScore_nunmberOfObservations(nmaxbl);
        calcScore_averageStations(astas, nmaxsta);
        calcScore_averageSources(asrcs, nmaxbl);
        calcScore_duration(minTime, maxTime, nmaxsta);
        calcScore_skyCoverage(skyCoverages, nmaxsta);
        sumScores();


    }


}
