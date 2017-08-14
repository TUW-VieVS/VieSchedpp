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

    VLBI_scan::VLBI_scan(vector<VLBI_pointingVector> pointingVectors, vector<unsigned int> endOfLastScan,
                         int minimumNumberOfStations, scanType type) :
            srcid{pointingVectors.at(0).getSrcid()}, pointingVectors{pointingVectors}, nsta{pointingVectors.size()},
            type{type},
            minimumNumberOfStations{minimumNumberOfStations}, score{0} {
        times = VLBI_scanTimes(nsta);
        times.setEndOfLastScan(endOfLastScan);
        pointingVectors_endtime.reserve(nsta);
        baselines.reserve((nsta * (nsta - 1)) / 2);
    }

    VLBI_scan::VLBI_scan(vector<VLBI_pointingVector> &pv, VLBI_scanTimes &times, vector<VLBI_baseline> &bl,
                         int minNumSta) :
            srcid{pv[0].getSrcid()}, nsta{pv.size()}, pointingVectors{move(pv)}, minimumNumberOfStations{minNumSta},
            score{0}, times{move(times)}, baselines{move(bl)} {
        pointingVectors_endtime.reserve(nsta);
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
            long idx = distance(eosl.begin(), it);

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

//            vector<pair<string, unsigned int> > durations;
            vector<pair<string, double> > flux = source.observedFlux(gmst, stations[staid1].dx(staid2),
                                                                     stations[staid1].dy(staid2),
                                                                     stations[staid1].dz(staid2));
            unsigned int maxScanDuration = 0;
            for(auto& any:flux){
                string fluxname = any.first;
                double SEFD_src = any.second;

                // TODO: check if band is in station and source
                bool bandsFound = true;
                double SEFD_sta1 = stations[staid1].getEquip().getSEFD(fluxname);
                double SEFD_sta2 = stations[staid2].getEquip().getSEFD(fluxname);
                double minSNR_sta1 = stations[staid1].getMinSNR(fluxname);
                double minSNR_sta2 = stations[staid2].getMinSNR(fluxname);


                vector<pair<string, double> > minSNRs_src = source.getMinSNR();
                double minSNR_src = 0;
                for (auto &any_minFlux:minSNRs_src) {
                    if (any_minFlux.first == fluxname) {
                        minSNR_src = any_minFlux.second;
                        break;
                    }
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
                    if (new_duration_uint > maxScanDuration) {
                        maxScanDuration = new_duration_uint;
                    }

//                    durations.push_back(make_pair(fluxname, new_duration_uint));
                }else{
                    cerr << "WARNING: duration of band " << fluxname << " ignored\n";
                }
            }
//            thisBaseline.setObservedFlux(flux);
//            thisBaseline.setScanDuration(durations);
            thisBaseline.setScanDuration(maxScanDuration);
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

        vector<unsigned int> scanTimes;
        do{
            scanDurationsValid = true;

            vector<int> eraseStations1;
            vector<int> eraseStations2;
            scanTimes = minscanTimes;


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
                        double thisMaxFlux = stations[id].getEquip().getMaxSEFD();
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
                minscanTimes.erase(minscanTimes.begin() + eraseThis);

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

    void VLBI_scan::calcScore_numberOfObservations(unsigned long maxObs) {
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
            finalScore += astas[thisStaId] * bl_counter[thisStaId] / (nsta - 1);
        }

        single_scores.averageStations = finalScore;
    }

    void VLBI_scan::calcScore_averageSources(vector<double> &asrcs) {
        unsigned long maxBl = (nsta * (nsta - 1)) / 2;
        unsigned long nbl = baselines.size();
        single_scores.averageSources = asrcs[srcid] * nbl / maxBl;
    }

    void VLBI_scan::calcScore_duration(unsigned int minTime, unsigned int maxTime) {
        unsigned int thisEndTime = times.maxTime();
        double score = 1 - ((double) thisEndTime - (double) minTime) / (maxTime - minTime);
        single_scores.duration = score;
    }

    void VLBI_scan::calcScore_skyCoverage(vector<VLBI_skyCoverage> &skyCoverages) {

        double score = 0;
        for (int i = 0; i < skyCoverages.size(); ++i) {
            double thisSore = skyCoverages[i].calcScore(pointingVectors) / nsta;
            score += thisSore;
        }

        single_scores.skyCoverage = score;
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

            double oldAz;
            double newAz = numeric_limits<double>::quiet_NaN();

            unsigned int slewtime;
            bool stationValid;
            do {
                oldAz = newAz;
                oldSlewEnd = newSlewEnd;
                new_p.setTime(newSlewEnd);
                thisSta.updateAzEl(source, new_p);
                stationValid = thisSta.isVisible(new_p);
                if (!stationValid) {
                    scanValid = removeElement(i);
                    if (!scanValid) {
                        return false;
                    }
                }

                stations[staid].getCableWrap().calcUnwrappedAz(stations[staid].getCurrentPointingVector(), new_p);
                newAz = new_p.getAz();
                if (oldAz != numeric_limits<double>::quiet_NaN() && abs(oldAz - newAz) > .5 * pi) {
                    bool secure = stations[staid].getCableWrap().unwrapAzNearNeutralPoint(new_p);
                    if (!secure) {
                        cout
                                << "### DEBUG OPPORTUNITY ###\n    axis 1 is close to limit and the ambigurity is not save!\n";
                        scanValid = removeElement(i);
                        if (!scanValid) {
                            return false;
                        }

                    }
                }
                slewtime = stations[staid].slewTime(new_p);

                if (slewtime > stations[staid].getMaxSlewtime()) {
                    scanValid = removeElement(i);
                    if (!scanValid) {
                        return false;
                    }

                }
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

            double oldAz = pointingVectors[i].getAz();
            double newAz;

            for (unsigned int j = scanStart; j < scanEnd; j += 10) {
                pv.setTime(j);
                thisSta.updateAzEl(source, pv);
                stationValid = thisSta.isVisible(pv);
                if (!stationValid) {
                    scanValid = removeElement(i);
                    if (!scanValid) {
                        return false;
                    }
                    break;
                }

                thisSta.getCableWrap().unwrapAzNearAz(pv, oldAz);
                newAz = pv.getAz();
                if (abs(newAz - oldAz) > .5 * pi) {
                    stationValid = false;
                    cout << "### DEBUG OPPORTUNITY ###\n    change of calbe wrap while observing!\n";
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
            thisSta.updateAzEl(source, pv);
            stationValid = thisSta.isVisible(pv);
            if (!stationValid) {
                scanValid = removeElement(i);
                if (!scanValid) {
                    return false;
                }
                continue;
            }

            thisSta.getCableWrap().unwrapAzNearAz(pv, oldAz);
            newAz = pv.getAz();
            if (abs(newAz - oldAz) > .5 * pi) {
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
        calcScore_numberOfObservations(nmaxbl);
        calcScore_averageStations(astas, nmaxsta);
        calcScore_averageSources(asrcs);
        calcScore_duration(minTime, maxTime);
        calcScore_skyCoverage(skyCoverages);
        sumScores();


    }

    void VLBI_scan::output(unsigned long observed_scan_nr, vector<VLBI_station> &stations, VLBI_source &source,
                           boost::posix_time::ptime &sessionStart) {
        unsigned long nmaxsta = stations.size();

        stringstream buffer1;
        buffer1 << "|-------------";
        for (int i = 0; i < nmaxsta - 1; ++i) {
            buffer1 << "-----------";
        }
        buffer1 << "----------| \n";
        cout << buffer1.str();

        string sname = source.getName();
        double sra = source.getRa() * rad2deg / 15;
        double sde = source.getDe() * rad2deg;
        stringstream buffer2;
        buffer2 << boost::format("| scan %4d to source: %8s (id: %4d) ") % observed_scan_nr %
                   sname % srcid;
        switch (type) {
            case VLBI_scan::scanType::single:
                buffer2 << "(single source scan)";
                break;
            case VLBI_scan::scanType::subnetting:
                buffer2 << "(subnetting scan)";
                break;
            case VLBI_scan::scanType::fillin:
                buffer2 << "(fillin mode scan)";
                break;
        }

        while (buffer2.str().size() < buffer1.str().size() - 3) {
            buffer2 << " ";
        }
        buffer2 << "| \n";
        cout << buffer2.str();
        unsigned int maxValue = numeric_limits<unsigned int>::max();

        vector<unsigned int> slewStart(nmaxsta, maxValue);
        vector<unsigned int> slewEnd(nmaxsta, maxValue);
        vector<unsigned int> ideling(nmaxsta, maxValue);
        vector<unsigned int> scanStart(nmaxsta, maxValue);
        vector<unsigned int> scanEnd(nmaxsta, maxValue);


        for (int idx = 0; idx < nsta; ++idx) {
            int staid = pointingVectors[idx].getStaid();
            slewStart[staid] = times.getEndOfSourceTime(idx);
            slewEnd[staid] = times.getEndOfSlewTime(idx);
            ideling[staid] = times.getEndOfIdleTime(idx);
            scanStart[staid] = times.getEndOfCalibrationTime(idx);
            scanEnd[staid] = times.getEndOfScanTime(idx);
        }
        cout << "| slew start | ";
        for (auto &t:slewStart) {
            if (t != maxValue) {
                boost::posix_time::ptime thisTime = sessionStart + boost::posix_time::seconds(t);
                cout << thisTime.time_of_day() << " | ";
            } else {
                cout << "         | ";
            }
        }
        cout << "\n";

        cout << "| slew end   | ";
        for (auto &t:slewEnd) {
            if (t != maxValue) {
                boost::posix_time::ptime thisTime = sessionStart + boost::posix_time::seconds(t);
                cout << thisTime.time_of_day() << " | ";
            } else {
                cout << "         | ";
            }
        }
        cout << "\n";

        cout << "| idle end   | ";
        for (auto &t:ideling) {
            if (t != maxValue) {
                boost::posix_time::ptime thisTime = sessionStart + boost::posix_time::seconds(t);
                cout << thisTime.time_of_day() << " | ";
            } else {
                cout << "         | ";
            }
        }
        cout << "\n";

        cout << "| scan start | ";
        for (auto &t:scanStart) {
            if (t != maxValue) {
                boost::posix_time::ptime thisTime = sessionStart + boost::posix_time::seconds(t);
                cout << thisTime.time_of_day() << " | ";
            } else {
                cout << "         | ";
            }
        }
        cout << "\n";

        cout << "| scan end   | ";
        for (auto &t:scanEnd) {
            if (t != maxValue) {
                boost::posix_time::ptime thisTime = sessionStart + boost::posix_time::seconds(t);
                cout << thisTime.time_of_day() << " | ";
            } else {
                cout << "         | ";
            }
        }
        cout << "\n";
    }

    boost::optional<VLBI_scan> VLBI_scan::copyScan(vector<int> &ids) {

        vector<VLBI_pointingVector> pv;
        pv.reserve(nsta);
        VLBI_scanTimes t = times;
        vector<VLBI_baseline> bl;
        bl.reserve(baselines.size());

        int counter = 0;
        for (auto &any:pointingVectors) {
            int id = any.getStaid();
            if (find(ids.begin(), ids.end(), id) != ids.end()) {
                pv.push_back(pointingVectors[counter]);
            }
            ++counter;
        }
        if (pv.size() < minimumNumberOfStations) {
            return boost::none;
        }

        for (int i = (int) nsta - 1; i >= 0; --i) {
            int thisId = pointingVectors[i].getStaid();
            if (find(ids.begin(), ids.end(), thisId) == ids.end()) {
                t.removeElement(i);
            }
        }

        for (int j = 0; j < baselines.size(); ++j) {
            VLBI_baseline &thisBl = baselines[j];
            int staid1 = thisBl.getStaid1();
            int staid2 = thisBl.getStaid2();

            if (find(ids.begin(), ids.end(), staid1) != ids.end() &&
                find(ids.begin(), ids.end(), staid2) != ids.end()) {
                bl.push_back(baselines[j]);
            }
        }
        return VLBI_scan(pv, t, bl, minimumNumberOfStations);
    }

    boost::optional<VLBI_scan>
    VLBI_scan::possibleFillinScan(vector<VLBI_station> &stations, VLBI_source &source,
                                  const std::vector<char> &possible,
                                  const std::vector<char> &unused,
                                  const vector<VLBI_pointingVector> &pv_final_position) {

        // get all ids of possible stations. This is necessary to call copyScan
        vector<int> possible_ids;
        for (int i = 0; i < possible.size(); ++i) {
            if (possible[i]) {
                possible_ids.push_back(i);
            }
        }
        // create a copy of the scan with all possible stations
        boost::optional<VLBI_scan> tmp_fi_scan_opt = copyScan(possible_ids);
        // if the copy is invalid nothing should be returned
        if (!tmp_fi_scan_opt) {
            return boost::none;
        }
        // if the copy is valid save it under tmp_fi_scan
        VLBI_scan tmp_fi_scan = std::move(*tmp_fi_scan_opt);

        // look if all stations have enough time to participate at the next scan
        int pv_id = 0;
        while (pv_id < tmp_fi_scan.nsta) {
            int staid = tmp_fi_scan.getPointingVector(pv_id).getStaid();
            // this is the endtime of the fillin scan, this means this is also the start time to slew to the next source
            unsigned int endOfFillinScan = tmp_fi_scan.getTimes().getEndOfScanTime(pv_id);

            if (!unused[staid]) { // unused stationd... this means we have an desired end pointing vector
                int srcid = tmp_fi_scan.getSourceId();
                // create pointing vector at end of scan
                VLBI_pointingVector this_pv_scan_end(staid, srcid);
                VLBI_station &thisStation = stations[staid];
                this_pv_scan_end.setTime(endOfFillinScan);

                // calculate azimuth and elevation at end of scan
                thisStation.updateAzEl(source, this_pv_scan_end);
                bool visible = thisStation.isVisible(this_pv_scan_end);
                if (!visible) {
                    bool valid = tmp_fi_scan.removeElement(pv_id);
                    if (!valid) {
                        return boost::none;
                    }
                    continue;
                }

                // unwrap azimuth and elevation at end of scan
                thisStation.getCableWrap().calcUnwrappedAz(tmp_fi_scan.getPointingVector(pv_id), this_pv_scan_end);

                // calculate slewtime between end of scan and start of new scan (final position)
                int slewTime = thisStation.getAntenna().slewTime(this_pv_scan_end, pv_final_position[staid]);
                // check the available time
                int availableTime = pv_final_position[staid].getTime() - endOfFillinScan;
                int time_needed = slewTime + thisStation.getWaitCalibration() + thisStation.getWaitSetup() +
                                  thisStation.getWaitSource() + thisStation.getWaitTape();
                if (time_needed > availableTime) {
                    bool valid = tmp_fi_scan.removeElement(pv_id);
                    if (!valid) {
                        return boost::none;
                    }
                    continue;
                }

            } else { // if station is not used in the following scans (this means we have no disred end pointing vector
                // check if the end of the fillin scan if earlier as the required final position time
                if (endOfFillinScan > pv_final_position[staid].getTime()) {
                    bool valid = tmp_fi_scan.removeElement(pv_id);
                    if (!valid) {
                        return boost::none;
                    }
                    continue;
                }
            }
            //station is ok!
            ++pv_id;
        }

        return tmp_fi_scan;
    }


}
