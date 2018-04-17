/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Subcon.cpp
 * Author: mschartn
 * 
 * Created on June 29, 2017, 5:51 PM
 */

#include "Subcon.h"

using namespace std;
using namespace VieVS;
int Subcon::nextId = 0;

Subcon::Subcon(): VieVS_Object(nextId++), nSingleScans_{0}, nSubnettingScans_{0} {
}

void Subcon::addScan(const Scan &scan) noexcept {
    singleScans_.push_back(scan);
    nSingleScans_++;
}

void
Subcon::calcStartTimes(const vector<Station> &stations, const vector<Source> &sources) noexcept {

    int i=0;
    while(i<nSingleScans_){
        bool scanValid_slew = true;
        bool scanValid_idle = true;
        vector<unsigned  int> maxIdleTimes;

        int j=0;
        while(j<singleScans_[i].getNSta()){
            int staid = singleScans_[i].getStationId(j);

            const Station &thisSta = stations[staid];
            if (thisSta.getPARA().firstScan) {
                singleScans_[i].addTimes(j, 0, 0, 0);
                ++j;
                maxIdleTimes.push_back(thisSta.getPARA().maxWait);
                continue;
            }


            thisSta.getCableWrap().calcUnwrappedAz(thisSta.getCurrentPointingVector(),
                                                   singleScans_[i].referencePointingVector(j));
            auto slewtime = thisSta.slewTime(singleScans_[i].getPointingVector(j));

            if (!slewtime.is_initialized()) {
                scanValid_slew = singleScans_[i].removeStation(j, sources[singleScans_[i].getSourceId()]);
                if(!scanValid_slew){
                    break;
                }
            } else {
                maxIdleTimes.push_back(thisSta.getPARA().maxWait);
                const Station::WaitTimes wtimes = thisSta.getWaittimes();
                singleScans_[i].addTimes(j, wtimes.fieldSystem, *slewtime, wtimes.preob);
                ++j;
            }
        }

        if (scanValid_slew) {
            scanValid_idle = singleScans_[i].checkIdleTimes(maxIdleTimes, sources[singleScans_[i].getSourceId()]);
        }

        if (!scanValid_slew || !scanValid_idle){
            singleScans_.erase(next(singleScans_.begin(),i));
            --nSingleScans_;
        }else{
            ++i;
        }
    }
}

void Subcon::constructAllBaselines(const vector<Source> &sources) noexcept {
    int i = 0;
    while (i < nSingleScans_) {
        Scan &thisScan = singleScans_[i];
        const Source &thisSource = sources[thisScan.getSourceId()];
        bool scanValid = thisScan.constructBaselines(thisSource);
        if (scanValid) {
            ++i;
        } else {
            --nSingleScans_;
            singleScans_.erase(next(singleScans_.begin(),i));
        }
    }
}

void Subcon::updateAzEl(const vector<Station> &stations, const vector<Source> &sources) noexcept {
    int i = 0;
    while (i < nSingleScans_) {

        const Source &thisSource = sources[singleScans_[i].getSourceId()];
        bool scanValid_slew = true;
        bool scanValid_idle = true;
        vector<unsigned  int> maxIdleTimes;

        int j = 0;
        while (j < singleScans_[i].getNSta()) {
            int staid = singleScans_[i].getPointingVector(j).getStaid();
            PointingVector &thisPointingVector = singleScans_[i].referencePointingVector(j);
            thisPointingVector.setTime(singleScans_[i].getTimes().getScanStart(j));
            stations[staid].calcAzEl(thisSource, thisPointingVector);
            bool visible = stations[staid].isVisible(thisPointingVector, sources[singleScans_[i].getSourceId()].getPARA().minElevation);

            boost::optional<unsigned int> slewtime;

            if (visible){
                stations[staid].getCableWrap().calcUnwrappedAz(stations[staid].getCurrentPointingVector(),
                                                               thisPointingVector);
                slewtime = stations[staid].slewTime(thisPointingVector);
            }

            if (!visible || !slewtime.is_initialized()) {
                scanValid_slew = singleScans_[i].removeStation(j, sources[singleScans_[i].getSourceId()]);
                if(!scanValid_slew){
                    break;
                }
            } else {
                maxIdleTimes.push_back(stations[staid].getPARA().maxWait);
                singleScans_[i].updateSlewtime(j, *slewtime);
                ++j;
            }
        }


        if (scanValid_slew) {
            scanValid_idle = singleScans_[i].checkIdleTimes(maxIdleTimes, sources[singleScans_[i].getSourceId()]);
        }

        if (!scanValid_slew || !scanValid_idle){
            singleScans_.erase(next(singleScans_.begin(),i));
            --nSingleScans_;
        }else{
            ++i;
        }
    }
}

void
Subcon::calcAllBaselineDurations(const vector<Station> &stations,
                                      const vector<Source> &sources) noexcept {
    int i = 0;
    while ( i < nSingleScans_ ) {
        Scan& thisScan = singleScans_[i];
        bool scanValid = thisScan.calcBaselineScanDuration(stations, sources[thisScan.getSourceId()]);
        if (scanValid){
            ++i;
        } else {
            --nSingleScans_;
            singleScans_.erase(next(singleScans_.begin(),i));
        }
    }
}

void Subcon::calcAllScanDurations(const vector<Station> &stations,
                                       const vector<Source> &sources) noexcept {
    int i=0;
    while (i<nSingleScans_){
        Scan& thisScan = singleScans_[i];
        int srcid = thisScan.getSourceId();

        bool scanValid = thisScan.scanDuration(stations, sources[srcid]);
        if (scanValid){
            ++i;
        } else {
            --nSingleScans_;
            singleScans_.erase(next(singleScans_.begin(),i));
        }
    }
}

void Subcon::calcCalibratorScanDuration(const vector<Station> &stations, const vector<Source> &sources) {
    for(auto &thisScan:singleScans_) {
        thisScan.setFixedScanDuration(CalibratorBlock::scanLength);
    }
}


void Subcon::createSubnettingScans(const vector<vector<int>> &subnettingSrcIds, int minStaPerSubcon,
                                   const vector<Source> &sources) noexcept {
    vector<int> sourceIds(nSingleScans_);
    for (int i = 0; i < nSingleScans_; ++i) {
        sourceIds[i] = singleScans_[i].getSourceId();
    }

    for (int i = 0; i < nSingleScans_; ++i) {
        int firstSrcId = sourceIds[i];
        Scan &first = singleScans_[i];
        vector<int> secondSrcIds = subnettingSrcIds[firstSrcId];
        for (int secondSrcId : secondSrcIds) {
            vector<int> sta1 = first.getStationIds();

            auto it = find(sourceIds.begin(), sourceIds.end(), secondSrcId);
            if (it != sourceIds.end()) {
                long idx = distance(sourceIds.begin(), it);
                Scan &second = singleScans_[idx];
                vector<int> sta2 = second.getStationIds();


                vector<int> uniqueSta1;
                vector<int> uniqueSta2;
                vector<int> intersection;
                for (int any: sta1) {
                    if (find(sta2.begin(), sta2.end(), any) == sta2.end()) {
                        uniqueSta1.push_back(any);
                    } else {
                        intersection.push_back(any);
                    }
                }
                for (int any: sta2) {
                    if (find(sta1.begin(), sta1.end(), any) == sta1.end()) {
                        uniqueSta2.push_back(any);
                    }
                }

                if (uniqueSta1.size() + uniqueSta2.size() + intersection.size() < minStaPerSubcon) {
                    continue;
                }

                unsigned long nint = intersection.size();


                for (int igroup = 0; igroup <= nint; ++igroup) {

                    vector<int> data(nint, 1);
                    for (unsigned long ii = nint - igroup; ii < nint; ++ii) {
                        data.at(ii) = 2;
                    }


                    do {
                        vector<int> scan1sta{uniqueSta1};
                        vector<int> scan2sta{uniqueSta2};
                        for (unsigned long ii = 0; ii < nint; ++ii) {
                            if (data.at(ii) == 1) {
                                scan1sta.push_back(intersection[ii]);
                            } else {
                                scan2sta.push_back(intersection[ii]);
                            }
                        }
                        if (scan1sta.size() >= sources[firstSrcId].getPARA().minNumberOfStations &&
                            scan2sta.size() >= sources[secondSrcId].getPARA().minNumberOfStations) {

                            unsigned int firstTime = first.getTimes().getScanEnd();
                            unsigned int secondTime = second.getTimes().getScanEnd();
                            if (firstTime > secondTime) {
                                swap(firstTime, secondTime);
                            }

                            if (secondTime - firstTime > 600) {
                                continue;
                            }


                            boost::optional<Scan> new_first = first.copyScan(scan1sta, sources[firstSrcId]);
                            if (!new_first) {
                                continue;
                            }
                            new_first->setType(Scan::ScanType::subnetting);

                            boost::optional<Scan> new_second = second.copyScan(scan2sta, sources[secondSrcId]);
                            if (!new_second) {
                                continue;
                            }
                            new_second->setType(Scan::ScanType::subnetting);

                            ++nSubnettingScans_;
                            pair<Scan, Scan>tmp = make_pair(move(*new_first),move(*new_second));
//                            tmp.first = ;
//                            tmp.second = ;
                            subnettingScans_.push_back(move(tmp));
                        }
                    } while (next_permutation(std::begin(data), std::end(data)));
                }
            }
        }
    }
}

void Subcon::generateScore(const vector<Station> &stations, const vector<Source> &sources,
                           const vector<SkyCoverage> &skyCoverages, unsigned long nsrc) noexcept {
    precalcScore(stations, sources);
    unsigned long nmaxsta = stations.size();
    vector< vector <double> > firstScore(nsrc);
    for (auto &thisScan: singleScans_) {
        vector<double> firstScorePerPv(thisScan.getNSta(),0);
        const Source &thisSource = sources[thisScan.getSourceId()];
        thisScan.calcScore(nmaxsta, nMaxBaselines_, astas_, asrcs_, minRequiredTime_, maxRequiredTime_, skyCoverages,
                           stations, thisSource, firstScorePerPv);
        singleScanScores_.push_back(thisScan.getScore());
        firstScore[thisScan.getSourceId()] = std::move(firstScorePerPv);
    }


    for (auto &thisScans:subnettingScans_) {
        Scan &thisScan1 = thisScans.first;
        int srcid1 = thisScan1.getSourceId();
        const Source &thisSource1 = sources[thisScan1.getSourceId()];
        thisScan1.calcScore_subnetting(nmaxsta, nMaxBaselines_, astas_, asrcs_, minRequiredTime_, maxRequiredTime_,
                                       skyCoverages, stations, thisSource1,
                                       firstScore[srcid1]);
        double score1 = thisScan1.getScore();

        Scan &thisScan2 = thisScans.second;
        int srcid2 = thisScan2.getSourceId();
        const Source &thisSource2 = sources[thisScan2.getSourceId()];
        thisScan2.calcScore_subnetting(nmaxsta, nMaxBaselines_, astas_, asrcs_, minRequiredTime_, maxRequiredTime_,
                                       skyCoverages, stations, thisSource2,
                                       firstScore[srcid2]);
        double score2 = thisScan2.getScore();
        subnettingScanScores_.push_back(score1 + score2);
    }
}

void Subcon::generateScore(const std::vector<double> &lowElevatrionScore, const std::vector<double> &highElevationScore,
                           unsigned int nsta, const vector<Station> &stations, const std::vector<Source> &sources) {

    minMaxTime();
    unsigned int nMaxBl = (nsta*(nsta-1))/2;

    int i=0;
    while (i<nSingleScans_){
        Scan& thisScan = singleScans_[i];

        bool valid = thisScan.calcScore(lowElevatrionScore, highElevationScore, stations, minRequiredTime_,
                                        maxRequiredTime_, nMaxBl, sources[thisScan.getSourceId()]);
        if (valid){
            ++i;
            singleScanScores_.push_back(thisScan.getScore());
        } else {
            --nSingleScans_;
            singleScans_.erase(next(singleScans_.begin(),i));
        }
    }

    i=0;
    while (i<subnettingScans_.size()){
        Scan &thisScan1 = subnettingScans_[i].first;

        bool valid1 = thisScan1.calcScore(lowElevatrionScore, highElevationScore, stations, minRequiredTime_,
                                          maxRequiredTime_, nMaxBl, sources[thisScan1.getSourceId()]);
        double score1 = thisScan1.getScore();

        Scan &thisScan2 = subnettingScans_[i].first;

        bool valid2 = thisScan2.calcScore(lowElevatrionScore, highElevationScore, stations, minRequiredTime_,
                                          maxRequiredTime_, nMaxBl, sources[thisScan2.getSourceId()]);
        double score2 = thisScan2.getScore();

        if (valid1 && valid2){
            ++i;
            subnettingScanScores_.push_back(score1+score2);
        } else {
            --nSubnettingScans_;
            subnettingScans_.erase(next(subnettingScans_.begin(),i));
        }
    }
}


void Subcon::minMaxTime() noexcept {
    unsigned int maxTime = 0;
    unsigned int minTime = numeric_limits<unsigned int>::max();
    for (auto &thisScan: singleScans_) {
        unsigned int thisTime = thisScan.getTimes().getScanEnd();
        if (thisTime < minTime) {
            minTime = thisTime;
        }
        if (thisTime > maxTime) {
            maxTime = thisTime;
        }
    }
    for (auto &thisScan: subnettingScans_) {
        unsigned int thisTime1 = thisScan.first.getTimes().getScanEnd();
        if (thisTime1 < minTime) {
            minTime = thisTime1;
        }
        if (thisTime1 > maxTime) {
            maxTime = thisTime1;
        }
        unsigned int thisTime2 = thisScan.first.getTimes().getScanEnd();
        if (thisTime2 < minTime) {
            minTime = thisTime2;
        }
        if (thisTime2 > maxTime) {
            maxTime = thisTime2;
        }
    }
    minRequiredTime_ = minTime;
    maxRequiredTime_ = maxTime;
}


void Subcon::average_station_score(const vector<Station> &stations) noexcept {

    vector<double> staobs;
    for (auto &thisStation:stations) {
        staobs.push_back(thisStation.getNbls());
    }

    double meanStaobs = std::accumulate(std::begin(staobs), std::end(staobs), 0.0) / staobs.size();

    double total = 0;
    vector<double> staobs_score;
    for (auto &thisStaobs:staobs) {
        double diff = meanStaobs - thisStaobs;
        if (diff > 0) {
            staobs_score.push_back(diff);
            total += diff;
        } else {
            staobs_score.push_back(0);
        }
    }
    if (total != 0) {
        for (auto &thisStaobs_score:staobs_score) {
            thisStaobs_score /= total;
        }
    }
    astas_ = staobs_score;
}

void Subcon::average_source_score(const vector<Source> &sources) noexcept {
    vector<double> srcobs;
    for (auto &thisSource:sources) {
        srcobs.push_back(thisSource.getNbls());
    }
    double meanSrcobs = std::accumulate(std::begin(srcobs), std::end(srcobs), 0.0) / srcobs.size();

    double max = 0;
    vector<double> srcobs_score;
    for (auto &thisSrcobs:srcobs) {
        double diff = meanSrcobs - thisSrcobs;
        if (diff > 0) {
            srcobs_score.push_back(diff);
            if (diff > max) {
                max = diff;
            }
        } else {
            srcobs_score.push_back(0);
        }
    }
    if (max != 0) {
        for (auto &thisStaobs_score:srcobs_score) {
            thisStaobs_score /= max;
        }
    }
    asrcs_ = srcobs_score;
}

void Subcon::precalcScore(const vector<Station> &stations, const vector<Source> &sources) noexcept {

    unsigned long nsta = stations.size();

    nMaxBaselines_ = (nsta * (nsta - 1)) / 2;
    if (WeightFactors::weightDuration != 0) {
        minMaxTime();
    }

    if (WeightFactors::weightAverageStations != 0) {
        average_station_score(stations);
    }

    if (WeightFactors::weightAverageSources != 0) {
        average_source_score(sources);
    }

}

boost::optional<unsigned long>
Subcon::rigorousScore(const vector<Station> &stations, const vector<Source> &sources,
                           const vector<SkyCoverage> &skyCoverages) noexcept {

    vector<double> scores = singleScanScores_;
    scores.insert(scores.end(), subnettingScanScores_.begin(), subnettingScanScores_.end());



    std::priority_queue<std::pair<double, unsigned long> > q;
    for (unsigned long i = 0; i < scores.size(); ++i) {
        q.push(std::pair<double, unsigned long>(scores[i], i));
    }

    while (true) {
        if (q.empty()) {
            return boost::none;
        }
        unsigned long idx = q.top().second;
        q.pop();

        if (idx < nSingleScans_) {
            unsigned long thisIdx = idx;
            Scan &thisScan = singleScans_[thisIdx];
            const Source &thisSource = sources[thisScan.getSourceId()];
            bool flag = thisScan.rigorousUpdate(stations, sources[thisScan.getSourceId()]);
            if (!flag) {
                continue;
            }
            thisScan.calcScore(stations.size(), nMaxBaselines_, astas_, asrcs_, minRequiredTime_, maxRequiredTime_,
                               stations, thisSource, skyCoverages);
            double newScore = thisScan.getScore();

            q.push(make_pair(newScore, idx));

        } else {
            unsigned long thisIdx = idx - nSingleScans_;
            auto &thisScans = subnettingScans_[thisIdx];

            Scan &thisScan1 = thisScans.first;
            const Source &thisSource1 = sources[thisScan1.getSourceId()];
            bool flag1 = thisScan1.rigorousUpdate(stations, sources[thisScan1.getSourceId()]);
            if (!flag1) {
                continue;
            }
            thisScan1.calcScore(stations.size(), nMaxBaselines_, astas_, asrcs_, minRequiredTime_, maxRequiredTime_,
                                stations, thisSource1, skyCoverages);
            double newScore1 = thisScan1.getScore();

            Scan &thisScan2 = thisScans.second;
            const Source &thisSource2 = sources[thisScan2.getSourceId()];
            bool flag2 = thisScan2.rigorousUpdate(stations, sources[thisScan2.getSourceId()]);
            if (!flag2) {
                continue;
            }

            unsigned int maxTime1 = thisScan1.getTimes().getScanEnd();
            unsigned int maxTime2 = thisScan2.getTimes().getScanEnd();
            unsigned int deltaTime;
            if (maxTime1 > maxTime2) {
                deltaTime = maxTime1 - maxTime2;
            } else {
                deltaTime = maxTime2 - maxTime1;
            }
            if (deltaTime > 600) {
                continue;
            }

            thisScan2.calcScore(stations.size(), nMaxBaselines_, astas_, asrcs_, minRequiredTime_, maxRequiredTime_,
                                stations, thisSource2, skyCoverages);
            double newScore2 = thisScan2.getScore();
            double newScore = newScore1 + newScore2;

            q.push(make_pair(newScore, idx));
        }
        unsigned long newIdx = q.top().second;
        if (newIdx == idx) {
            return idx;
        }
    }
}

boost::optional<unsigned long> Subcon::rigorousScore(const vector<Station> &stations, const vector<Source> &sources,
                                                     const vector<SkyCoverage> &skyCoverages,
                                                     const vector<double> &prevLowElevationScores,
                                                     const vector<double> &prevHighElevationScores) {

    vector<double> scores = singleScanScores_;
    scores.insert(scores.end(), subnettingScanScores_.begin(), subnettingScanScores_.end());

    std::priority_queue<std::pair<double, unsigned long> > q;
    for (unsigned long i = 0; i < scores.size(); ++i) {
        q.push(std::pair<double, unsigned long>(scores[i], i));
    }

    while (true) {
        if (q.empty()) {
            return boost::none;
        }
        unsigned long idx = q.top().second;
        q.pop();

        if (idx < nSingleScans_) {
            unsigned long thisIdx = idx;
            Scan &thisScan = singleScans_[thisIdx];
            const Source &thisSource = sources[thisScan.getSourceId()];
            bool flag = thisScan.rigorousUpdate(stations, sources[thisScan.getSourceId()]);
            if (!flag) {
                continue;
            }
            flag = thisScan.calcScore(prevLowElevationScores, prevHighElevationScores, stations,
                                      minRequiredTime_, maxRequiredTime_, static_cast<unsigned int>(stations.size()),
                                      sources[thisScan.getSourceId()]);
            if (!flag) {
                continue;
            }
            double newScore = thisScan.getScore();

            q.push(make_pair(newScore, idx));

        } else {
            unsigned long thisIdx = idx - nSingleScans_;
            auto &thisScans = subnettingScans_[thisIdx];

            Scan &thisScan1 = thisScans.first;
            const Source &thisSource1 = sources[thisScan1.getSourceId()];
            bool flag1 = thisScan1.rigorousUpdate(stations, sources[thisScan1.getSourceId()]);
            if (!flag1) {
                continue;
            }
            flag1 = thisScan1.calcScore(prevLowElevationScores, prevHighElevationScores, stations,
                                        minRequiredTime_, maxRequiredTime_, static_cast<unsigned int>(stations.size()),
                                        sources[thisScan1.getSourceId()]);
            if (!flag1) {
                continue;
            }
            double newScore1 = thisScan1.getScore();

            Scan &thisScan2 = thisScans.second;
            const Source &thisSource2 = sources[thisScan2.getSourceId()];
            bool flag2 = thisScan2.rigorousUpdate(stations, sources[thisScan2.getSourceId()]);
            if (!flag2) {
                continue;
            }

            unsigned int maxTime1 = thisScan1.getTimes().getScanEnd();
            unsigned int maxTime2 = thisScan2.getTimes().getScanEnd();
            unsigned int deltaTime;
            if (maxTime1 > maxTime2) {
                deltaTime = maxTime1 - maxTime2;
            } else {
                deltaTime = maxTime2 - maxTime1;
            }
            if (deltaTime > 900) {
                continue;
            }

            flag2 = thisScan2.calcScore(prevLowElevationScores, prevHighElevationScores, stations,
                                        minRequiredTime_, maxRequiredTime_, static_cast<unsigned int>(stations.size()),
                                        sources[thisScan2.getSourceId()]);
            if (!flag2) {
                continue;
            }

            double newScore2 = thisScan2.getScore();
            double newScore = newScore1 + newScore2;

            q.push(make_pair(newScore, idx));
        }
        unsigned long newIdx = q.top().second;
        if (newIdx == idx) {
            return idx;
        }
    }

}


void Subcon::removeScan(unsigned long idx) noexcept {
    if (idx < nSingleScans_) {
        unsigned long thisIdx = idx;
        singleScans_.erase(next(singleScans_.begin(), static_cast<int>(thisIdx)));
        --nSingleScans_;
        singleScanScores_.erase(next(singleScanScores_.begin(), static_cast<int>(thisIdx)));

    } else {
        unsigned long thisIdx = idx - nSingleScans_;
        subnettingScans_.erase(next(subnettingScans_.begin(), static_cast<int>(thisIdx)));
        --nSubnettingScans_;
        subnettingScanScores_.erase(next(subnettingScanScores_.begin(), static_cast<int>(thisIdx)));

    }
}

void Subcon::clearSubnettingScans() {
    nSubnettingScans_ = 0;
    subnettingScans_.clear();
    subnettingScanScores_.clear();
}

void Subcon::changeScanTypes(Scan::ScanType type) {
    for(auto &scan:singleScans_){
        scan.setType(type);
    }
    for(auto &scans:subnettingScans_){
        scans.first.setType(type);
        scans.second.setType(type);
    }
}


