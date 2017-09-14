/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Scan.cpp
 * Author: mschartn
 * 
 * Created on June 29, 2017, 3:27 PM
 */

#include <set>
#include "Scan.h"

using namespace std;
using namespace VieVS;

Scan::Scan() {
}

Scan::Scan(const vector<PointingVector> &pointingVectors, const vector<unsigned int> &endOfLastScan,
                     int minimumNumberOfStations, ScanType type) :
        pointingVectors_{std::move(pointingVectors)},
        type_{type},
        minNumberOfStations_{minimumNumberOfStations}, score_{0} {
    nsta_ = Scan::pointingVectors_.size();
    srcid_ = Scan::pointingVectors_.at(0).getSrcid();
    times_ = ScanTimes(nsta_);
    times_.setEndOfLastScan(endOfLastScan);
    baselines_.reserve((nsta_ * (nsta_ - 1)) / 2);
}

Scan::Scan(const vector<PointingVector> &pv, const ScanTimes &times,
                     const vector<Baseline> &bl, int minNumSta) :
        srcid_{pv[0].getSrcid()}, nsta_{pv.size()}, pointingVectors_{move(pv)}, minNumberOfStations_{minNumSta},
        score_{0}, times_{move(times)}, baselines_{move(bl)} {
}

bool Scan::constructBaselines(const Source &source) noexcept {
    baselines_.clear();
    bool valid = false;
    for (int i=0; i<pointingVectors_.size(); ++i){
        for (int j=i+1; j<pointingVectors_.size(); ++j){

            int staid1 = pointingVectors_[i].getStaid();
            int staid2 = pointingVectors_[j].getStaid();

            if(Baseline::PARA.ignore[staid1][staid2]){
                continue;
            }
            if (!source.getPARA().ignoreBaselines.empty()) {
                auto &PARA = source.getPARA();
                if (staid1 > staid2) {
                    swap(staid1, staid2);
                }
                if (find(PARA.ignoreBaselines.begin(), PARA.ignoreBaselines.end(), make_pair(staid1, staid2)) !=
                    PARA.ignoreBaselines.end()) {
                    continue;
                }

            }

            unsigned int startTime1 = times_.getEndOfIdleTime(i);
            unsigned int startTime2 = times_.getEndOfIdleTime(j);
            if (startTime1> startTime2){
                baselines_.push_back(Baseline(pointingVectors_[i].getSrcid(), pointingVectors_[i].getStaid(),
                                                  pointingVectors_[j].getStaid(), startTime1));
                valid = true;
            } else {
                baselines_.push_back(Baseline(pointingVectors_[i].getSrcid(), pointingVectors_[i].getStaid(),
                                                  pointingVectors_[j].getStaid(), startTime2));
                valid = true;
            }
        }
    }
    return valid;
}

void Scan::addTimes(int idx, unsigned int setup, unsigned int source, unsigned int slew, unsigned int tape,
                         unsigned int calib) noexcept {
    times_.addTimes(idx, setup, source, slew, tape, calib);
}

bool Scan::removeStation(int idx) noexcept {
    --nsta_;
    if (nsta_<minNumberOfStations_){
        return false;
    }

    times_.removeElement(idx);

    int staid = pointingVectors_[idx].getStaid();
    pointingVectors_.erase(pointingVectors_.begin()+idx);
    if (pointingVectorsEndtime_.size() > 0) {
        pointingVectorsEndtime_.erase(pointingVectorsEndtime_.begin() + idx);
    }


    unsigned long nbl_before = baselines_.size();
    int i=0;
    while (i<baselines_.size()){
        if(baselines_[i].getStaid1()==staid || baselines_[i].getStaid2()==staid){
            baselines_.erase(baselines_.begin()+i);
        } else {
            ++i;
        }
    }
    return !(nbl_before != 0 && baselines_.size() == 0);
}

bool Scan::removeBaseline(int idx_bl) noexcept {

    int staid1 = baselines_[idx_bl].getStaid1();
    int staid2 = baselines_[idx_bl].getStaid2();
    baselines_.erase(baselines_.begin()+idx_bl);
    if (baselines_.size()==0){
        return false;
    }

    // check if it necessary to remove a pointing vector (a station is no longer part of any baseline)
    bool scanValid = true;
    int counterStaid1 = 0;
    int counterStaid2 = 0;
    for (const auto& any: baselines_) {
        int thisStaid1 = any.getStaid1();
        int thisStaid2 = any.getStaid2();

        if(thisStaid1 == staid1 || thisStaid2 == staid1){
            ++counterStaid1;
        }
        if(thisStaid1 == staid2 || thisStaid2 == staid2){
            ++counterStaid2;
        }
    }

    if(counterStaid1==0){
        int idx_pv = findIdxOfStationId(staid1);
        scanValid = removeStation(idx_pv);
    }
    if(counterStaid2==0){
        int idx_pv = findIdxOfStationId(staid2);
        scanValid = removeStation(idx_pv);
    }
    return scanValid;
}


bool Scan::checkIdleTimes(vector<unsigned int> maxIdle) noexcept {

    bool scan_valid = true;
    bool idleTimeValid;
    unsigned int latestSlewTime;
    do {
        idleTimeValid = true;

        vector<unsigned int> eosl = times_.getEndOfSlewTime();
        auto it= max_element(eosl.begin(), eosl.end());
        latestSlewTime = *it;
        long idx = distance(eosl.begin(), it);

        vector<unsigned int > dt(nsta_);
        for (int i = 0; i < nsta_; ++i) {
            dt[i] = latestSlewTime-eosl[i];
            if (dt[i]>maxIdle[i]){
                scan_valid = removeStation(idx);
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
        times_.alignStartTimes();
    }

    return scan_valid;
}

void Scan::updateSlewtime(int idx, unsigned int new_slewtime) noexcept {
    times_.updateSlewtime(idx,new_slewtime);
}

bool
Scan::calcBaselineScanDuration(const vector<Station> &stations, const Source &source) noexcept {

    bool flag_scanValid = true;
    int ibl = 0;
    while (ibl < baselines_.size()) {
        Baseline& thisBaseline = baselines_[ibl];
        int staid1 = thisBaseline.getStaid1();
        int staid2 = thisBaseline.getStaid2();
        unsigned int startTime = thisBaseline.getStartTime();

        double date1 = 2400000.5;
        double date2 = TimeSystem::mjdStart + startTime / 86400;
        double gmst = iauGmst82(date1, date2);

//            vector<pair<string, unsigned int> > durations;
        vector<pair<string, double> > flux = source.observedFlux(gmst, stations[staid1].dx(staid2),
                                                                 stations[staid1].dy(staid2),
                                                                 stations[staid1].dz(staid2));
        unsigned int maxScanDuration = 0;

        bool flag_baselineRemoved = false;
        for(auto& any:flux){
            string band = any.first;
            double SEFD_src = any.second;

            double SEFD_sta1 = stations[staid1].getEquip().getSEFD(band);
            double SEFD_sta2 = stations[staid2].getEquip().getSEFD(band);
            double minSNR_sta1 = stations[staid1].getMinSNR(band);
            double minSNR_sta2 = stations[staid2].getMinSNR(band);

            double minSNR_bl = Baseline::PARA.minSNR[band][staid1][staid2];

            double minSNR_src = source.getMinSNR(band);

            double maxminSNR = minSNR_src;
            if (minSNR_sta1>maxminSNR){
                maxminSNR = minSNR_sta1;
            }
            if (minSNR_sta2>maxminSNR){
                maxminSNR = minSNR_sta2;
            }
            if (minSNR_bl>maxminSNR){
                maxminSNR = minSNR_bl;
            }

            double maxCorSynch1 = stations[staid1].getWaitCorsynch();
            double maxCorSynch = maxCorSynch1;
            double maxCorSynch2 = stations[staid2].getWaitCorsynch();
            if (maxCorSynch2 > maxCorSynch){
                maxCorSynch = maxCorSynch2;
            }

            double anum = (1.75*maxminSNR / SEFD_src);
            double anu1 = SEFD_sta1*SEFD_sta2;
            double anu2 = ObservationMode::sampleRate * 1.0e6 * ObservationMode::nChannels[band] * ObservationMode::bits;

            double new_duration = anum*anum *anu1/anu2 + maxCorSynch;
            new_duration = ceil(new_duration);
            unsigned int new_duration_uint = (unsigned int) new_duration;

            unsigned int minScanBl = Baseline::PARA.minScan[staid1][staid2];
            if(new_duration_uint<minScanBl){
                new_duration_uint = minScanBl;
            }
            unsigned int maxScanBl = Baseline::PARA.maxScan[staid1][staid2];
            if(new_duration_uint>maxScanBl){
                flag_scanValid = removeBaseline(ibl);
                flag_baselineRemoved = true;
                break;
            }

            if (new_duration_uint > maxScanDuration) {
                maxScanDuration = new_duration_uint;
            }
        }

        if(!flag_baselineRemoved){
            ++ibl;
        }
        if(!flag_scanValid){
            return false;
        }

        thisBaseline.setScanDuration(maxScanDuration);
    }

    return true;
}

bool Scan::scanDuration(const vector<Station> &stations, const Source &source) noexcept {

    bool scanDurationsValid;
    bool scanValid = true;

    boost::optional<unsigned int> fixedScanDuration = source.getFixedScanDuration();
    if (fixedScanDuration.is_initialized()) {
        vector<unsigned int> scanTimes(nsta_, *fixedScanDuration);
        times_.addScanTimes(scanTimes);
        return scanValid;
    }



    vector<unsigned int> minscanTimes(nsta_,source.getMinScanTime());
    vector<unsigned int> maxScanTimes(nsta_,source.getMaxScanTime());

    for (int i = 0; i < nsta_; ++i) {
        unsigned int stationMinScanTime = stations[pointingVectors_[i].getStaid()].getMinScanTime();
        unsigned int stationMaxScanTime = stations[pointingVectors_[i].getStaid()].getMaxScanTime();

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


        for (int i = 0; i < baselines_.size(); ++i) {
            Baseline& thisBaseline = baselines_[i];
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

            vector<int> counter(nsta_);
            for (int i = 0; i < eraseStations1.size(); ++i) {
                counter[eraseStations1[i]]++;
                counter[eraseStations2[i]]++;
            }

            int max = 0;
            vector<int> maxIdx;
            for (int i = 0; i< nsta_; ++i){
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
                    int id = pointingVectors_[thisIdx].getStaid();
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
                        thisScanStartTimes[(times_.getEndOfSlewTime(maxFluxIdx[i]))];
                    }

                    long maxmaxFluxIdx = distance(thisScanStartTimes.begin(),
                                                  max_element(thisScanStartTimes.begin(),
                                                              thisScanStartTimes.end()));
                    eraseThis = maxFluxIdx[maxmaxFluxIdx];
                }
            }

            scanValid = removeStation(eraseThis);
            minscanTimes.erase(minscanTimes.begin() + eraseThis);

            if (!scanValid){
                break;
            }
        }

    }while(!scanDurationsValid);

    times_.addScanTimes(scanTimes);
    return scanValid;
}

int Scan::findIdxOfStationId(int id) const noexcept {
    for (int idx = 0; idx < nsta_; ++idx) {
        if(pointingVectors_[idx].getStaid()==id){
            return idx;
        }
    }
    return -1;
}

vector<int> Scan::getStationIds() const noexcept {
    vector<int> ids;
    for (int i = 0; i < nsta_; ++i) {
        ids.push_back(pointingVectors_[i].getStaid());
    }

    return std::move(ids);
}

bool Scan::removeAllBut(const vector<int> &station_ids) noexcept {
    int i = 0;
    bool valid = true;
    while (i < nsta_) {
        int thisId = pointingVectors_[i].getStaid();
        if (find(station_ids.begin(), station_ids.end(), thisId) == station_ids.end()) {
            valid = removeStation(i);
            if (!valid) {
                break;
            }
        } else {
            ++i;
        }
    }
    return valid;
}

unsigned int Scan::maxTime() const noexcept {
    return times_.maxTime();
}

double Scan::calcScore_numberOfObservations(unsigned long maxObs) const noexcept {
    int nbl = baselines_.size();
    double thisScore = (double) nbl / (double) maxObs;
    return thisScore;
}

double Scan::calcScore_averageStations(const vector<double> &astas, unsigned long nmaxsta) const noexcept {
    double finalScore = 0;

    vector<unsigned long> bl_counter(nmaxsta);
    for (auto &bl:baselines_) {
        ++bl_counter[bl.getStaid1()];
        ++bl_counter[bl.getStaid2()];
    }

    for (auto &pv:pointingVectors_) {
        int thisStaId = pv.getStaid();
        finalScore += astas[thisStaId] * bl_counter[thisStaId] / (nsta_ - 1);
    }

    return finalScore;
}

double Scan::calcScore_averageSources(const vector<double> &asrcs) const noexcept {
    unsigned long maxBl = (nsta_ * (nsta_ - 1)) / 2;
    unsigned long nbl = baselines_.size();
    return asrcs[srcid_] * nbl / maxBl;
}

double Scan::calcScore_duration(unsigned int minTime, unsigned int maxTime) const noexcept {
    unsigned int thisEndTime = times_.maxTime();
    double score = 1 - ((double) thisEndTime - (double) minTime) / (maxTime - minTime);
    return score;
}

double Scan::calcScore_skyCoverage(const vector<SkyCoverage> &skyCoverages,
                                        const vector<Station> &stations) const noexcept {

    double score = 0;

    for (int i = 0; i < skyCoverages.size(); ++i) {
        double thisSore = skyCoverages[i].calcScore(pointingVectors_, stations) / nsta_;
        score += thisSore;
    }
    return score;
}


double Scan::calcScore_skyCoverage(const vector<SkyCoverage> &skyCoverages,
                                        const vector<Station> &stations,
                                        vector<double> &firstScorePerPv) const noexcept {

    double score = 0;

    for (int i = 0; i < skyCoverages.size(); ++i) {
        double thisSore = skyCoverages[i].calcScore(pointingVectors_, stations, firstScorePerPv) / nsta_;
        score += thisSore;
    }
    return score;
}

double Scan::calcScore_skyCoverage_subcon(const vector<SkyCoverage> &skyCoverages,
                                               const vector<Station> &stations,
                                               const vector<double> &firstScorePerPv) const noexcept {

    double score = 0;

    for (int i = 0; i < skyCoverages.size(); ++i) {
        double thisSore = skyCoverages[i].calcScore_subcon(pointingVectors_, stations, firstScorePerPv) / nsta_;
        score += thisSore;
    }
    return score;
}


bool Scan::rigorousUpdate(const vector<Station> &stations, const Source &source) noexcept {
    int srcid = source.getId();
    bool scanValid = true;
    pointingVectorsEndtime_.resize(nsta_);
    // FIRST STEP: calc earliest possible slew end times for each station:
    int ista = 0;
    while (ista < nsta_) {
        PointingVector &pv = pointingVectors_[ista];
        unsigned int slewStart = times_.getEndOfSourceTime(ista);
        unsigned int slewEnd = times_.getEndOfSlewTime(ista);
        const Station &thisStation = stations[pv.getStaid()];

        unsigned int oldSlewEnd = 0;
        unsigned int newSlewEnd = slewEnd;

        bool bigSlew = false;

        unsigned int timeDiff = newSlewEnd - oldSlewEnd;
        while (timeDiff > 1) {
            oldSlewEnd = newSlewEnd;
            pv.setTime(oldSlewEnd);

            double oldAz = pv.getAz();
            if (bigSlew) {
                thisStation.getCableWrap().unwrapAzNearAz(pv, oldAz);
            } else {
                thisStation.getCableWrap().calcUnwrappedAz(thisStation.getCurrentPointingVector(), pv);
            }
            double newAz = pv.getAz();

            // big slew detected... this means you are somewhere close to the cable wrap limit...
            // from this point on you calc your unwrapped Az near the azimuth, which is further away from this limit
            if (std::abs(oldAz - newAz) > .5 * pi) {
                // if there was already a bigSlew flag you know that both azimuth are unsafe...
                if (bigSlew) {
                    scanValid = removeStation(ista);
                    if (!scanValid) {
                        return false;
                    }
                    continue;
                }
                // otherwise set the big slew flag
                bigSlew = true;
            }

            unsigned int thisSlewtime = thisStation.slewTime(pv);
            if (thisSlewtime > thisStation.getMaxSlewtime()) {
                scanValid = removeStation(ista);
                if (!scanValid) {
                    return false;
                }
                continue;
            }

            newSlewEnd = slewStart + thisSlewtime;
            if (newSlewEnd > oldSlewEnd) {
                timeDiff = newSlewEnd - oldSlewEnd;
            } else {
                timeDiff = oldSlewEnd - newSlewEnd;
            }
        }
        times_.updateSlewtime(ista, newSlewEnd - slewStart);
        ++ista;
    }


    // SECOND STEP: check if source is available during whole scan
    bool stationRemoved;
    do {
        stationRemoved = false;

        // SECOND.FIRST STEP: align start times to earliest possible one:
        unsigned long nsta_beginning;
        do {
            nsta_beginning = nsta_;

            times_.alignStartTimes();
            scanValid = constructBaselines(source);
            if (!scanValid) {
                return false;
            }
            scanValid = calcBaselineScanDuration(stations, source);
            if (!scanValid) {
                return false;
            }
            scanValid = scanDuration(stations, source);
            if (!scanValid) {
                return false;
            }

        } while (nsta_ != nsta_beginning);


        // SECOND STEP: check if source is available during whole scan
        ista = 0;
        while (ista < nsta_ && !stationRemoved) {
            unsigned int scanStart = times_.getEndOfCalibrationTime(ista);
            unsigned int scanEnd = times_.getEndOfScanTime(ista);
            PointingVector &pv = pointingVectors_[ista];
            const Station &thisStation = stations[pv.getStaid()];

            PointingVector moving_pv(pv.getStaid(), pv.getSrcid());
            moving_pv.setAz(pv.getAz());
            moving_pv.setEl(pv.getEl());

            for (unsigned int time = scanStart; scanStart < scanEnd; scanStart += 30) {
                moving_pv.setTime(scanStart);
                double oldAz = moving_pv.getAz();
                thisStation.getCableWrap().calcUnwrappedAz(thisStation.getCurrentPointingVector(), moving_pv);
                double newAz = moving_pv.getAz();

                if (std::abs(oldAz - newAz) > .5 * pi) {
                    scanValid = removeStation(ista);
                    if (!scanValid) {
                        return false;
                    }
                    stationRemoved = true;
                    continue;
                }

                if (time == scanStart) {
                    pointingVectors_[ista] = moving_pv;
                }
            }
            moving_pv.setTime(scanEnd);
            double oldAz = moving_pv.getAz();
            thisStation.getCableWrap().calcUnwrappedAz(thisStation.getCurrentPointingVector(), moving_pv);
            double newAz = moving_pv.getAz();

            if (std::abs(oldAz - newAz) > .5 * pi) {
                scanValid = removeStation(ista);
                if (!scanValid) {
                    return false;
                }
                stationRemoved = true;
                continue;
            }
            pointingVectorsEndtime_[ista] = move(moving_pv);
            ++ista;
        }
    } while (stationRemoved);
    return true;
}


bool Scan::rigorousUpdate2(const vector<Station> &stations, const Source &source) noexcept {
    int srcid = source.getId();
    bool scanValid;

    int i = 0;

    while (i < nsta_) {

        PointingVector &pv = pointingVectors_[i];
        unsigned int slewStart = times_.getEndOfSourceTime(i);
        unsigned int slewEnd = times_.getEndOfSlewTime(i);
        int staid = pv.getStaid();
        const Station & thisSta = stations[staid];

        PointingVector new_p(staid, srcid);
        unsigned int oldSlewEnd;
        unsigned int newSlewEnd = slewEnd;

        double oldAz;
        double newAz = numeric_limits<double>::quiet_NaN();

        unsigned int slewtime;
        bool stationValid;

        unsigned int timeDiff;
        do {
            oldAz = newAz;
            oldSlewEnd = newSlewEnd;
            new_p.setTime(newSlewEnd);
            thisSta.calcAzEl(source, new_p);
            stationValid = thisSta.isVisible(new_p);
            if (!stationValid) {
                scanValid = removeStation(i);
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
                    scanValid = removeStation(i);
                    if (!scanValid) {
                        return false;
                    }

                }
            }
            slewtime = stations[staid].slewTime(new_p);

            if (slewtime > stations[staid].getMaxSlewtime()) {
                scanValid = removeStation(i);
                if (!scanValid) {
                    return false;
                }

            }
            newSlewEnd = slewStart + slewtime;

            if (oldSlewEnd > newSlewEnd) {
                timeDiff = oldSlewEnd - newSlewEnd;
            } else {
                timeDiff = newSlewEnd - oldSlewEnd;
            }

        } while (timeDiff > 1);

        if (stationValid) {
            pointingVectors_[i] = new_p;
            times_.updateSlewtime(i, slewtime);
            ++i;
        }
    }

    times_.alignStartTimes();

    scanValid = constructBaselines(source);
    if (!scanValid) {
        return false;
    }
    scanValid = calcBaselineScanDuration(stations, source);
    if (!scanValid) {
        return false;
    }
    scanValid = scanDuration(stations, source);
    if (!scanValid) {
        return false;
    }

    i = 0;
    while (i < nsta_) {

        unsigned int scanStart = times_.getEndOfCalibrationTime(i);
        unsigned int scanEnd = times_.getEndOfScanTime(i);
        int thisStaId = pointingVectors_[i].getStaid();

        PointingVector pv(thisStaId, srcid);
        const Station &thisSta = stations[thisStaId];

        bool stationValid = true;

        double oldAz = pointingVectors_[i].getAz();
        double newAz;

        for (unsigned int j = scanStart; j < scanEnd; j += 10) {
            pv.setTime(j);
            thisSta.calcAzEl(source, pv);
            stationValid = thisSta.isVisible(pv);
            if (!stationValid) {
                scanValid = removeStation(i);
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
                scanValid = removeStation(i);
                if (!scanValid) {
                    return false;
                }
                break;
            }

            if (j == scanStart) {
                pointingVectors_[i] = pv;
            }
        }

        if (!stationValid) {
            continue;
        }

        pv.setTime(scanEnd);
        thisSta.calcAzEl(source, pv);
        stationValid = thisSta.isVisible(pv);
        if (!stationValid) {
            scanValid = removeStation(i);
            if (!scanValid) {
                return false;
            }
            continue;
        }

        thisSta.getCableWrap().unwrapAzNearAz(pv, oldAz);
        newAz = pv.getAz();
        if (abs(newAz - oldAz) > .5 * pi) {
            scanValid = removeStation(i);
            if (!scanValid) {
                return false;
            }
            continue;
        }


        pointingVectorsEndtime_.push_back(pv);

        ++i;
    }
    return true;
}

void Scan::calcScore(unsigned long nmaxsta, unsigned long nmaxbl, const vector<double> &astas,
                          const vector<double> &asrcs, unsigned int minTime, unsigned int maxTime,
                          const vector<SkyCoverage> &skyCoverages,
                          const vector<Station> &stations) noexcept {
    double this_score = 0;

    double weight_numberOfObservations = WeightFactors::weightNumberOfObservations;
    if (weight_numberOfObservations != 0) {
        this_score += calcScore_numberOfObservations(nmaxbl) * WeightFactors::weightNumberOfObservations;
    }
    double weight_averageSources = WeightFactors::weightAverageSources;
    if (weight_averageSources != 0) {
        this_score += calcScore_averageSources(asrcs) * WeightFactors::weightAverageSources;
    }
    double weight_averageStations = WeightFactors::weightAverageStations;
    if (weight_averageStations != 0) {
        this_score += calcScore_averageStations(astas, nmaxsta) * WeightFactors::weightAverageStations;
    }
    double weight_duration = WeightFactors::weightDuration;
    if (weight_duration != 0) {
        this_score += calcScore_duration(minTime, maxTime) * WeightFactors::weightDuration;
    }
    double weight_skyCoverage = WeightFactors::weightSkyCoverage;
    if (weight_skyCoverage != 0) {
        this_score += calcScore_skyCoverage(skyCoverages, stations) * WeightFactors::weightSkyCoverage;
    }

    score_ = this_score;
}

void Scan::calcScore(unsigned long nmaxsta, unsigned long nmaxbl, const vector<double> &astas,
                          const vector<double> &asrcs, unsigned int minTime, unsigned int maxTime,
                          const vector<SkyCoverage> &skyCoverages,
                          vector<double> &firstScorePerPV, const vector<Station> &stations) noexcept {
    double this_score = 0;

    double weight_numberOfObservations = WeightFactors::weightNumberOfObservations;
    if (weight_numberOfObservations != 0) {
        this_score += calcScore_numberOfObservations(nmaxbl) * WeightFactors::weightNumberOfObservations;
    }
    double weight_averageSources = WeightFactors::weightAverageSources;
    if (weight_averageSources != 0) {
        this_score += calcScore_averageSources(asrcs) * WeightFactors::weightAverageSources;
    }
    double weight_averageStations = WeightFactors::weightAverageStations;
    if (weight_averageStations != 0) {
        this_score += calcScore_averageStations(astas, nmaxsta) * WeightFactors::weightAverageStations;
    }
    double weight_duration = WeightFactors::weightDuration;
    if (weight_duration != 0) {
        this_score += calcScore_duration(minTime, maxTime) * WeightFactors::weightDuration;
    }
    double weight_skyCoverage = WeightFactors::weightSkyCoverage;
    if (weight_skyCoverage != 0) {
        this_score += calcScore_skyCoverage(skyCoverages, stations, firstScorePerPV) *
                      WeightFactors::weightSkyCoverage;
    }

    score_ =  this_score;
}


void Scan::calcScore_subcon(unsigned long nmaxsta, unsigned long nmaxbl, const vector<double> &astas,
                                 const vector<double> &asrcs, unsigned int minTime, unsigned int maxTime,
                                 const vector<SkyCoverage> &skyCoverages,
                                 const vector<double> &firstScorePerPV,
                                 const vector<Station> &stations) noexcept {
    double this_score = 0;

    double weight_numberOfObservations = WeightFactors::weightNumberOfObservations;
    if (weight_numberOfObservations != 0) {
        this_score += calcScore_numberOfObservations(nmaxbl) * WeightFactors::weightNumberOfObservations;
    }
    double weight_averageSources = WeightFactors::weightAverageSources;
    if (weight_averageSources != 0) {
        this_score += calcScore_averageSources(asrcs) * WeightFactors::weightAverageSources;
    }
    double weight_averageStations = WeightFactors::weightAverageStations;
    if (weight_averageStations != 0) {
        this_score += calcScore_averageStations(astas, nmaxsta) * WeightFactors::weightAverageStations;
    }
    double weight_duration = WeightFactors::weightDuration;
    if (weight_duration != 0) {
        this_score += calcScore_duration(minTime, maxTime) * WeightFactors::weightDuration;
    }
    double weight_skyCoverage = WeightFactors::weightSkyCoverage;
    if (weight_skyCoverage != 0) {
        this_score += calcScore_skyCoverage_subcon(skyCoverages, stations, firstScorePerPV) *
                      WeightFactors::weightSkyCoverage;
    }

    score_ =  this_score;
}

void
Scan::output(unsigned long observed_scan_nr, const vector<Station> &stations, const Source &source,
                  ofstream &of) const noexcept {
    unsigned long nmaxsta = stations.size();

    stringstream buffer1;
    buffer1 << "|-------------";
    for (int i = 0; i < nmaxsta - 1; ++i) {
        buffer1 << "-----------";
    }
    buffer1 << "----------| \n";
    of << buffer1.str();

    string sname = source.getName();
    double sra = source.getRa() * rad2deg / 15;
    double sde = source.getDe() * rad2deg;
    stringstream buffer2;
    buffer2 << boost::format("| scan %4d to source: %8s (id: %4d) ") % observed_scan_nr % sname % srcid_;
    switch (type_) {
        case Scan::ScanType::single:
            buffer2 << "(single source scan)";
            break;
        case Scan::ScanType::subnetting:
            buffer2 << "(subnetting scan)";
            break;
        case Scan::ScanType::fillin:
            buffer2 << "(fillin mode scan)";
            break;
    }

    while (buffer2.str().size() < buffer1.str().size() - 3) {
        buffer2 << " ";
    }
    buffer2 << "| \n";
    of << buffer2.str();
    unsigned int maxValue = numeric_limits<unsigned int>::max();

    vector<unsigned int> slewStart(nmaxsta, maxValue);
    vector<unsigned int> slewEnd(nmaxsta, maxValue);
    vector<unsigned int> ideling(nmaxsta, maxValue);
    vector<unsigned int> scanStart(nmaxsta, maxValue);
    vector<unsigned int> scanEnd(nmaxsta, maxValue);


    for (int idx = 0; idx < nsta_; ++idx) {
        int staid = pointingVectors_[idx].getStaid();
        slewStart[staid] = times_.getEndOfSourceTime(idx);
        slewEnd[staid] = times_.getEndOfSlewTime(idx);
        ideling[staid] = times_.getEndOfIdleTime(idx);
        scanStart[staid] = times_.getEndOfCalibrationTime(idx);
        scanEnd[staid] = times_.getEndOfScanTime(idx);
    }
    of << "| slew start | ";
    for (auto &t:slewStart) {
        if (t != maxValue) {
            boost::posix_time::ptime thisTime = TimeSystem::startTime + boost::posix_time::seconds(t);
            of << thisTime.time_of_day() << " | ";
        } else {
            of << "         | ";
        }
    }
    of << "\n";

    of << "| slew end   | ";
    for (auto &t:slewEnd) {
        if (t != maxValue) {
            boost::posix_time::ptime thisTime = TimeSystem::startTime + boost::posix_time::seconds(t);
            of << thisTime.time_of_day() << " | ";
        } else {
            of << "         | ";
        }
    }
    of << "\n";

    of << "| idle end   | ";
    for (auto &t:ideling) {
        if (t != maxValue) {
            boost::posix_time::ptime thisTime = TimeSystem::startTime + boost::posix_time::seconds(t);
            of << thisTime.time_of_day() << " | ";
        } else {
            of << "         | ";
        }
    }
    of << "\n";

    of << "| scan start | ";
    for (auto &t:scanStart) {
        if (t != maxValue) {
            boost::posix_time::ptime thisTime = TimeSystem::startTime + boost::posix_time::seconds(t);
            of << thisTime.time_of_day() << " | ";
        } else {
            of << "         | ";
        }
    }
    of << "\n";

    of << "| scan end   | ";
    for (auto &t:scanEnd) {
        if (t != maxValue) {
            boost::posix_time::ptime thisTime = TimeSystem::startTime + boost::posix_time::seconds(t);
            of << thisTime.time_of_day() << " | ";
        } else {
            of << "         | ";
        }
    }
    of << "\n";


    // TODO: EVERYTHING UNTER THIS LINE IS JUST FOR DEBUGGING!!!
    if (pointingVectors_.size() != nsta_ || pointingVectorsEndtime_.size() != nsta_ ||
        times_.getEndOfSlewTime().size() != nsta_) {
        of << "ERROR #1\n";
    }

    for (int i = 0; i < nsta_; ++i) {
        unsigned int pTime = pointingVectors_[i].getTime();
        unsigned int tTime = times_.getEndOfCalibrationTime(i);
        if (pTime != tTime) {
            of << "ERROR #2\n";
        }

        unsigned int pTime2 = pointingVectorsEndtime_[i].getTime();
        unsigned int tTime2 = times_.getEndOfScanTime(i);
        if (pTime != tTime) {
            of << "ERROR #3\n";
        }

    }

}

boost::optional<Scan> Scan::copyScan(const vector<int> &ids) const noexcept {


    vector<PointingVector> pv;
    pv.reserve(ids.size());
    ScanTimes t = times_;
    vector<Baseline> bl;

    int counter = 0;
    for (auto &any:pointingVectors_) {
        int id = any.getStaid();
        if (find(ids.begin(), ids.end(), id) != ids.end()) {
            pv.push_back(pointingVectors_[counter]);
        }
        ++counter;
    }
    if (pv.size() < minNumberOfStations_) {
        return boost::none;
    }

    for (int i = (int) nsta_ - 1; i >= 0; --i) {
        int thisId = pointingVectors_[i].getStaid();
        if (find(ids.begin(), ids.end(), thisId) == ids.end()) {
            t.removeElement(i);
        }
    }

    for (int j = 0; j < baselines_.size(); ++j) {
        const Baseline &thisBl = baselines_[j];
        int staid1 = thisBl.getStaid1();
        int staid2 = thisBl.getStaid2();

        if (find(ids.begin(), ids.end(), staid1) != ids.end() &&
            find(ids.begin(), ids.end(), staid2) != ids.end()) {
            bl.push_back(baselines_[j]);
        }
    }
    if (bl.size() == 0) {
        return boost::none;
    }

    return Scan(pv, t, bl, minNumberOfStations_);
}


bool Scan::possibleFillinScan(const vector<Station> &stations, const Source &source,
                                   const std::vector<char> &unused,
                                   const vector<PointingVector> &pv_final_position) {
    int pv_id = 0;
    while (pv_id < nsta_) {
        const PointingVector fillinScanStart = pointingVectors_[pv_id];
        int staid = fillinScanStart.getStaid();

        // this is the endtime of the fillin scan, this means this is also the start time to slew to the next source
        unsigned int endOfFillinScan = times_.getEndOfScanTime(pv_id);
        const PointingVector &finalPosition = pv_final_position[staid];

        if (!unused[staid]) { // unused station... this means we have an desired end pointing vector

            const Station &thisStation = stations[staid];

            PointingVector fillinScanEnd;
            if (pointingVectorsEndtime_.size() == 0) {
                // create pointing vector at end of scan
                fillinScanEnd = PointingVector(staid, srcid_);
                fillinScanEnd.setTime(endOfFillinScan);

                // calculate azimuth and elevation at end of scan
                thisStation.calcAzEl(source, fillinScanEnd);

                bool visible = thisStation.isVisible(fillinScanEnd);
                if (!visible) {
                    bool valid = removeStation(pv_id);
                    if (!valid) {
                        return false;
                    }
                    continue;
                }

                // unwrap azimuth and elevation at end of scan
                thisStation.getCableWrap().calcUnwrappedAz(fillinScanStart, fillinScanEnd);
            } else {
                fillinScanEnd = pointingVectorsEndtime_[pv_id];
            }

            // calculate slewtime between end of scan and start of new scan (final position)
            int slewTime = thisStation.getAntenna().slewTime(fillinScanEnd, finalPosition);

            // check the available time
            unsigned int pv_finalTime = finalPosition.getTime();
            unsigned int availableTime;
            if (pv_finalTime > endOfFillinScan) {
                availableTime = pv_finalTime - endOfFillinScan;
            } else {
                availableTime = 0;
            }

            int time_needed = slewTime + thisStation.getWaitCalibration() + thisStation.getWaitSetup() +
                              thisStation.getWaitSource() + thisStation.getWaitTape();
            if (time_needed > availableTime) {
                bool valid = removeStation(pv_id);
                if (!valid) {
                    return false;
                }
                continue;
            }

        } else { // if station is not used in the following scans (this means we have no desired end pointing vector
            // check if the end of the fillin scan if earlier as the required final position time
            if (endOfFillinScan > finalPosition.getTime()) {
                bool valid = removeStation(pv_id);
                if (!valid) {
                    return false;
                }
                continue;
            }
        }
        //station is ok!
        ++pv_id;
    }
    return true;
}

boost::optional<Scan>
Scan::possibleFillinScan(const vector<Station> &stations, const Source &source,
                              const std::vector<char> &possible, const std::vector<char> &unused,
                              const vector<PointingVector> &pv_final_position) const noexcept {

    // get all ids of possible stations. This is necessary to call copyScan
    vector<int> possible_ids;
    for (int i = 0; i < possible.size(); ++i) {
        if (possible[i]) {
            possible_ids.push_back(i);
        }
    }
    // create a copy of the scan with all possible stations
    boost::optional<Scan> tmp_fi_scan_opt = copyScan(possible_ids);

    // if the copy is invalid nothing should be returned
    if (!tmp_fi_scan_opt) {
        return boost::none;
    }

    // if the copy is valid save it under fi_scan
    Scan fi_scan = std::move(*tmp_fi_scan_opt);

    // look if all stations have enough time to participate at the next scan
    int pv_id = 0;
    while (pv_id < fi_scan.nsta_) {
        int staid = fi_scan.getPointingVector(pv_id).getStaid();

        // this is the endtime of the fillin scan, this means this is also the start time to slew to the next source
        unsigned int endOfFillinScan = fi_scan.getTimes().getEndOfScanTime(pv_id);

        if (!unused[staid]) { // unused station... this means we have an desired end pointing vector
            int srcid = fi_scan.getSourceId();

            // create pointing vector at end of scan
            PointingVector this_pv_scan_end(staid, srcid);
            const Station &thisStation = stations[staid];
            this_pv_scan_end.setTime(endOfFillinScan);

            // calculate azimuth and elevation at end of scan
            thisStation.calcAzEl(source, this_pv_scan_end);
            bool visible = thisStation.isVisible(this_pv_scan_end);
            if (!visible) {
                bool valid = fi_scan.removeStation(pv_id);
                if (!valid) {
                    return boost::none;
                }
                continue;
            }

            // unwrap azimuth and elevation at end of scan
            thisStation.getCableWrap().calcUnwrappedAz(fi_scan.getPointingVector(pv_id), this_pv_scan_end);

            // calculate slewtime between end of scan and start of new scan (final position)
            int slewTime = thisStation.getAntenna().slewTime(this_pv_scan_end, pv_final_position[staid]);

            // check the available time
            unsigned int pv_finalTime = pv_final_position[staid].getTime();
            unsigned int availableTime;
            if (pv_finalTime > endOfFillinScan) {
                availableTime = pv_finalTime - endOfFillinScan;
            } else {
                availableTime = 0;
            }

            int time_needed = slewTime + thisStation.getWaitCalibration() + thisStation.getWaitSetup() +
                              thisStation.getWaitSource() + thisStation.getWaitTape();
            if (time_needed > availableTime) {
                bool valid = fi_scan.removeStation(pv_id);
                if (!valid) {
                    return boost::none;
                }
                continue;
            }

        } else { // if station is not used in the following scans (this means we have no desired end pointing vector
            // check if the end of the fillin scan if earlier as the required final position time
            if (endOfFillinScan > pv_final_position[staid].getTime()) {
                bool valid = fi_scan.removeStation(pv_id);
                if (!valid) {
                    return boost::none;
                }
                continue;
            }
        }
        //station is ok!
        ++pv_id;
    }

    return fi_scan;
}

