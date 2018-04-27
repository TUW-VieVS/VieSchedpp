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

#include "Scan.h"
#include "FillinmodeEndposition.h"

using namespace std;
using namespace VieVS;

unsigned int thread_local Scan::nScanSelections{0};
Scan::ScanSequence thread_local Scan::scanSequence;
int thread_local Scan::nextId{0};



Scan::Scan(vector<PointingVector> &pointingVectors, vector<unsigned int> &endOfLastScan, ScanType type):
        VieVS_Object(nextId++), times_{ScanTimes(static_cast<unsigned int>(pointingVectors.size()))},
        pointingVectors_{move(pointingVectors)}, type_{type}, constellation_{ScanConstellation::single}, score_{0}{
    nsta_ = Scan::pointingVectors_.size();
    srcid_ = Scan::pointingVectors_.at(0).getSrcid();
    times_.setEndOfLastScan(endOfLastScan);
    baselines_.reserve((nsta_ * (nsta_ - 1)) / 2);
}

Scan::Scan(vector<PointingVector> &pv, ScanTimes &times, vector<Baseline> &bl):
        VieVS_Object(nextId++), srcid_{pv[0].getSrcid()}, nsta_{pv.size()}, pointingVectors_{move(pv)},
        score_{0}, times_{move(times)}, baselines_{move(bl)}, constellation_{ScanConstellation::subnetting},
        type_{ScanType::standard}{
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
                const auto &PARA = source.getPARA();
                if (staid1 > staid2) {
                    swap(staid1, staid2);
                }
                if (find(PARA.ignoreBaselines.begin(), PARA.ignoreBaselines.end(), make_pair(staid1, staid2)) !=
                    PARA.ignoreBaselines.end()) {
                    continue;
                }

            }

            unsigned int startTime1 = times_.getScanStart(i);
            unsigned int startTime2 = times_.getScanStart(j);
            if (startTime1> startTime2){
                baselines_.emplace_back(pointingVectors_[i].getSrcid(), pointingVectors_[i].getStaid(),
                                        pointingVectors_[j].getStaid(), startTime1);
                valid = true;
            } else {
                baselines_.emplace_back(pointingVectors_[i].getSrcid(), pointingVectors_[i].getStaid(),
                                        pointingVectors_[j].getStaid(), startTime2);
                valid = true;
            }
        }
    }
    return valid;
}

void Scan::addTimes(int idx, unsigned int fieldSystem, unsigned int slew, unsigned int preob) noexcept {
    times_.addTimes(idx, fieldSystem, slew, preob);
}

bool Scan::removeStation(int idx, const Source &source) noexcept {
    --nsta_;
    if (nsta_ < source.getPARA().minNumberOfStations) {
        return false;
    }

    int staid = pointingVectors_[idx].getStaid();
    if (!source.getPARA().requiredStations.empty()) {
        const vector<int> &rsta = source.getPARA().requiredStations;
        if (find(rsta.begin(), rsta.end(), staid) != rsta.end()) {
            return false;
        }
    }

    times_.removeElement(idx);

    pointingVectors_.erase(next(pointingVectors_.begin(),idx));
    if (!pointingVectorsEndtime_.empty()) {
        pointingVectorsEndtime_.erase(next(pointingVectorsEndtime_.begin(),idx));
    }


    unsigned long nbl_before = baselines_.size();
    int i=0;
    while (i<baselines_.size()){
        if(baselines_[i].getStaid1()==staid || baselines_[i].getStaid2()==staid){
            baselines_.erase(next(baselines_.begin(),i));
        } else {
            ++i;
        }
    }
    return !(nbl_before != 0 && baselines_.empty());
}

bool Scan::removeBaseline(int idx_bl, const Source &source) noexcept {

    int staid1 = baselines_[idx_bl].getStaid1();
    int staid2 = baselines_[idx_bl].getStaid2();
    baselines_.erase(next(baselines_.begin(),idx_bl));
    if (baselines_.empty()) {
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
        boost::optional<int> idx_pv = findIdxOfStationId(staid1);
        scanValid = removeStation(*idx_pv, source);
    }
    if(counterStaid2==0){
        boost::optional<int> idx_pv = findIdxOfStationId(staid2);
        scanValid = removeStation(*idx_pv, source);
    }
    return scanValid;
}


bool Scan::checkIdleTimes(std::vector<unsigned int> &maxIdle, const Source &source) noexcept {

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
                scan_valid = removeStation(idx, source);
                if (scan_valid){
                    maxIdle.erase(next(maxIdle.begin(),idx));
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

bool Scan::calcBaselineScanDuration(const vector<Station> &stations, const Source &source) noexcept {

    // check if it is a calibrator scan and there is a fixed scan duration for calibrator scans
    if(type_ == ScanType::calibrator){
        if(CalibratorBlock::targetScanLengthType == CalibratorBlock::TargetScanLengthType::seconds){
            unsigned int maxScanDuration = CalibratorBlock::scanLength;
            // set baseline scan duration to fixed scan duration
            for(auto &thisBaseline:baselines_){
                thisBaseline.setScanDuration(maxScanDuration);
            }
            return true;
        }
    }

    // check if there is a fixed scan duration for this source
    boost::optional<unsigned int> fixedScanDuration = source.getPARA().fixedScanDuration;
    if (fixedScanDuration.is_initialized()) {
        unsigned int maxScanDuration = *fixedScanDuration;
        // set baseline scan duration to fixed scan duration
        for(auto &thisBaseline:baselines_){
            thisBaseline.setScanDuration(maxScanDuration);
        }
        return true;
    }


    // loop through all baselines
    int ibl = 0;
    while (ibl < baselines_.size()) {
        Baseline& thisBaseline = baselines_[ibl];

        // get station ids from this baseline
        int staid1 = thisBaseline.getStaid1();
        const Station &sta1 = stations[staid1];
        int staid2 = thisBaseline.getStaid2();
        const Station &sta2 = stations[staid2];

        // get baseline scan start time
        unsigned int startTime = thisBaseline.getStartTime();

        // calculate greenwhich meridian sedirial time
        double date1 = 2400000.5;
        double date2 = TimeSystem::mjdStart + static_cast<double>(startTime) / 86400.0;
        double gmst = iauGmst82(date1, date2);

        unsigned int maxScanDuration = 0;

        // loop over each band
        bool flag_baselineRemoved = false;
        for (auto &band:ObservationMode::bands) {

            // calculate observed flux density for each band
            double SEFD_src = source.observedFlux(band, gmst, stations[staid1].dx(staid2), stations[staid1].dy(staid2),
                                                  stations[staid1].dz(staid2));

            // calculate system equivalent flux density for each station
            double el1 = pointingVectors_[*findIdxOfStationId(staid1)].getEl();
            double SEFD_sta1 = sta1.getEquip().getSEFD(band, el1);
            double el2 = pointingVectors_[*findIdxOfStationId(staid2)].getEl();
            double SEFD_sta2 = sta2.getEquip().getSEFD(band, el2);

            // get minimum required SNR for each station, baseline and source
            double minSNR_sta1 = sta1.getPARA().minSNR.at(band);
            double minSNR_sta2 = sta2.getPARA().minSNR.at(band);
            double minSNR_bl = Baseline::PARA.minSNR[band][staid1][staid2];
            double minSNR_src = source.getPARA().minSNR.at(band);

            // maximum required minSNR
            double maxminSNR = max({minSNR_src,minSNR_bl, minSNR_sta1, minSNR_sta2});

            // get maximum correlator synchronization time for
            double maxCorSynch1 = stations[staid1].getWaittimes().midob;
            double maxCorSynch2 = stations[staid2].getWaittimes().midob;
            double maxCorSynch = max({maxCorSynch1,maxCorSynch2});

            // calc required baseline scan duration
            double anum = (1.75*maxminSNR / SEFD_src);
            double anu1 = SEFD_sta1*SEFD_sta2;
            double anu2 = ObservationMode::sampleRate * 1.0e6 * ObservationMode::nChannels[band] * ObservationMode::bits;
            double new_duration = anum*anum *anu1/anu2 + maxCorSynch;
            new_duration = ceil(new_duration);
            auto new_duration_uint = static_cast<unsigned int>(new_duration);

            // check if required baseline scan duration is within min and max scan times of baselines
            unsigned int minScanBl = Baseline::PARA.minScan[staid1][staid2];
            if(new_duration_uint<minScanBl){
                new_duration_uint = minScanBl;
            }
            unsigned int maxScanBl = Baseline::PARA.maxScan[staid1][staid2];
            if(new_duration_uint>maxScanBl){
                bool scanValid = removeBaseline(ibl, source);
                if(!scanValid){
                    return false;
                }
                flag_baselineRemoved = true;
                break;
            }

            // change maxScanDuration if it is higher for this band
            if (new_duration_uint > maxScanDuration) {
                maxScanDuration = new_duration_uint;
            }
        }

        // if you have not removed the baseline increment counter and set the baseline scan duration
        if(!flag_baselineRemoved){
            ++ibl;
            thisBaseline.setScanDuration(maxScanDuration);
        }
    }

    return true;
}

bool Scan::scanDuration(const vector<Station> &stations, const Source &source) noexcept {

    // check if it is a calibrator scan with a fixed scan duration
    if(type_ == ScanType::calibrator){
        if(CalibratorBlock::targetScanLengthType == CalibratorBlock::TargetScanLengthType::seconds){
            times_.addScanTimes(CalibratorBlock::scanLength);
            return true;
        }
    }

    // check if there is a fixed scan duration for this source
    boost::optional<unsigned int> fixedScanDuration = source.getPARA().fixedScanDuration;
    if (fixedScanDuration.is_initialized()) {
        times_.addScanTimes(*fixedScanDuration);
        return true;
    }

    // save minimum and maximum scan time
    vector<unsigned int> minscanTimes(nsta_, source.getPARA().minScan);
    vector<unsigned int> maxScanTimes(nsta_, source.getPARA().maxScan);
    for (int i = 0; i < nsta_; ++i) {
        int staid = pointingVectors_[i].getStaid();
        unsigned int stationMinScanTime = stations[staid].getPARA().minScan;
        unsigned int stationMaxScanTime = stations[staid].getPARA().maxScan;

        if(minscanTimes[i] < stationMinScanTime){
            minscanTimes[i] = stationMinScanTime;
        }
        if(maxScanTimes[i] > stationMaxScanTime){
            maxScanTimes[i] = stationMaxScanTime;
        }
    }

    // iteratively calculate scan times
    bool scanDurationsValid;
    vector<unsigned int> scanTimes;
    do{
        scanDurationsValid = true;

        // counter which counts how often a station should be removed due to too long observation time
        vector<int> counter(nsta_);

        // set scanTimes to minimum required scan times
        scanTimes = minscanTimes;

        // loop through all baselines
        for (auto &thisBaseline : baselines_) {

            // get index of stations in this baseline
            int staid1 = thisBaseline.getStaid1();
            boost::optional<int> staidx1o = findIdxOfStationId(staid1);
            int staidx1 = *staidx1o;
            int staid2 = thisBaseline.getStaid2();
            boost::optional<int> staidx2o = findIdxOfStationId(staid2);
            int staidx2 = *staidx2o;

            // get scan duration of baseline
            unsigned int duration = thisBaseline.getScanDuration();

            // if there is a higher required scan time update scanTimes
            if(scanTimes[staidx1]<duration){
                scanTimes[staidx1] = duration;
            }
            if(scanTimes[staidx2]<duration){
                scanTimes[staidx2] = duration;
            }

            // check if required duration is higher then maximum allowed scan time.
            // Increase counter for station which are observed too long
            if(duration>maxScanTimes[staidx1] || duration>maxScanTimes[staidx2]){
                counter[staidx1]++;
                counter[staidx2]++;
                scanDurationsValid = false;
            }
        }

        // if the scan duration is not valid
        if (!scanDurationsValid) {
            int eraseThis;

            // get index of all station which have the maximum amount of too long observations
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

            // if there is only one maximum remove this station
            if(maxIdx.size()==1){
                eraseThis = maxIdx[0];
            } else {

                // if more stations have the same maximum amount of too long observations look at the max SEFD and
                // remove station with highest SEFD
                double maxSEFD = 0;
                vector<int> maxSEFDId(maxIdx.size());
                for(int i=0; i<maxIdx.size(); ++i){
                    int thisIdx = maxIdx[i];
                    int staid = pointingVectors_[thisIdx].getStaid();
                    double thisMaxSEFD = stations[staid].getEquip().getMaxSEFD();
                    if (thisMaxSEFD == maxSEFD) {
                        maxSEFDId.push_back(thisIdx);
                    }
                    if (thisMaxSEFD > maxSEFD) {
                        maxSEFD = thisMaxSEFD;
                        maxSEFDId.clear();
                        maxSEFDId.push_back(i);
                    }
                }

                // if there is only one maximum remove this station
                if(maxSEFDId.size()==1){
                    eraseThis = maxSEFDId[0];
                } else {

                    // if more stations have the same maximum amount of too long observations and highest SEFD
                    // look at the earliest possible scan start time and remove station with latest scan start time
                    vector<unsigned int> thisScanStartTimes(maxSEFDId.size());
                    for (int i : maxSEFDId) {
                        thisScanStartTimes[(times_.getSlewEnd(i))];
                    }

                    // remove station with latest slew end time. If multiple have the same value simply pick one
                    long maxSlewEnd = distance(thisScanStartTimes.begin(), max_element(thisScanStartTimes.begin(),
                                                                                          thisScanStartTimes.end()));
                    eraseThis = maxSEFDId[maxSlewEnd];
                }
            }

            bool scanValid = removeStation(eraseThis, source);
            if (!scanValid){
                return false;
            }
            minscanTimes.erase(next(minscanTimes.begin(),eraseThis));
        }

    }while(!scanDurationsValid);

    times_.addScanTimes(scanTimes);
    return true;
}

boost::optional<int> Scan::findIdxOfStationId(int id) const noexcept {
    for (int idx = 0; idx < nsta_; ++idx) {
        if(pointingVectors_[idx].getStaid()==id){
            return idx;
        }
    }
    return boost::none;
}

vector<int> Scan::getStationIds() const noexcept {
    vector<int> ids;
    for (int i = 0; i < nsta_; ++i) {
        ids.push_back(pointingVectors_[i].getStaid());
    }

    return std::move(ids);
}

double Scan::calcScore_numberOfObservations(unsigned long maxObs) const noexcept {
    unsigned long nbl = baselines_.size();
    double thisScore = static_cast<double>(nbl) / static_cast<double>(maxObs);
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
    unsigned int thisEndTime = times_.getScanEnd();
    double score = 1 - static_cast<double>(thisEndTime - minTime) / static_cast<double>(maxTime - minTime);
    return score;
}

double Scan::calcScore_skyCoverage(const vector<SkyCoverage> &skyCoverages,
                                        const vector<Station> &stations) const noexcept {

    double score = 0;

    for (const auto &skyCoverage : skyCoverages) {
        double thisSore = skyCoverage.calcScore(pointingVectors_, stations) / nsta_;
        score += thisSore;
    }
    return score;
}


double Scan::calcScore_skyCoverage(const vector<SkyCoverage> &skyCoverages,
                                        const vector<Station> &stations,
                                        vector<double> &firstScorePerPv) const noexcept {

    double score = 0;

    for (const auto &skyCoverage : skyCoverages) {
        double thisSore = skyCoverage.calcScore(pointingVectors_, stations, firstScorePerPv) / nsta_;
        score += thisSore;
    }
    return score;
}

double Scan::calcScore_skyCoverage_subcon(const vector<SkyCoverage> &skyCoverages,
                                               const vector<Station> &stations,
                                               const vector<double> &firstScorePerPv) const noexcept {

    double score = 0;

    for (const auto &skyCoverage : skyCoverages) {
        double thisSore = skyCoverage.calcScore_subcon(pointingVectors_, stations, firstScorePerPv) / nsta_;
        score += thisSore;
    }
    return score;
}


bool Scan::rigorousUpdate(const vector<Station> &stations, const Source &source,
                          const boost::optional<FillinmodeEndposition> &endposition) noexcept {
    bool scanValid;

    // FIRST STEP: calc earliest possible slew end times for each station:
    scanValid = rigorousSlewtime(stations, source);
    if(!scanValid){
        return scanValid;
    }

    // SECOND STEP: check if source is available during whole scan and iteratively check everything
    bool stationRemoved;
    do {
        stationRemoved = false;

        // align start times to earliest possible one:
        scanValid = rigorousScanStartTimeAlignment(stations, source);
        if(!scanValid){
            return scanValid;
        }

        // check if source is available during whole scan
        scanValid = rigorousScanVisibility(stations, source, stationRemoved);
        if(!scanValid){
            return scanValid;
        }
        if(stationRemoved){
            continue;
        }

        // check if end position can be reached
        scanValid = rigorousScanCanReachEndposition(stations, source, endposition, stationRemoved);
        if(!scanValid){
            return scanValid;
        }

    } while (stationRemoved);

    return true;
}

bool Scan::rigorousSlewtime(const std::vector<Station> &stations, const Source &source) noexcept {

    bool scanValid = true;

    // loop through all stations
    int ista = 0;
    while (ista < nsta_) {
        PointingVector &pv = pointingVectors_[ista];
        unsigned int slewStart = times_.getSlewStart(ista);
        const Station &thisStation = stations[pv.getStaid()];

        // old slew end time and new slew end time, required for iteration
        unsigned int oldSlewEnd = 0;
        unsigned int newSlewEnd = times_.getSlewEnd(ista);

        // big slew indicates if the slew distance is > 180 degrees
        bool bigSlew = false;

        // timeDiff is difference between two estimated slew rates in iteration.
        unsigned int timeDiff = numeric_limits<unsigned int>::max();

        // iteratively calculate slew time
        while (timeDiff > 1) {
            // change slew times for iteration
            oldSlewEnd = newSlewEnd;
            double oldAz = pv.getAz();

            // calculate az, el for pointing vector for previouse time
            pv.setTime(oldSlewEnd);
            thisStation.calcAzEl(source,pv);
            thisStation.getCableWrap().calcUnwrappedAz(thisStation.getCurrentPointingVector(), pv);

            // if you have a "big slew" unwrap the azimuth near the old azimuth
            if (bigSlew) {
                thisStation.getCableWrap().unwrapAzNearAz(pv, oldAz);
            } else {
                thisStation.getCableWrap().calcUnwrappedAz(thisStation.getCurrentPointingVector(), pv);
            }
            double newAz = pv.getAz();

            // check if you are near the cable wrap limits and the software decides to slew the other direction
            if (std::abs(oldAz - newAz) > .5 * pi) {
                // big slew detected, this means you are somewhere close to the cable wrap limit.
                // from this point on you calc your unwrapped Az near the azimuth, which is further away from this limit

                // if there was already a bigSlew flag you know that both azimuth are unsafe... remove station
                if (bigSlew) {
                    scanValid = removeStation(ista, source);
                    if (!scanValid) {
                        return scanValid;
                    }
                    continue;
                }
                // otherwise set the big slew flag
                bigSlew = true;
            }

            // calculate new slewtime
            auto thisSlewtime = thisStation.slewTime(pv);
            if (!thisSlewtime.is_initialized()) {
                scanValid = removeStation(ista, source);
                if (!scanValid) {
                    return scanValid;
                }
                continue;
            }

            // calculate new slew end time and time difference
            newSlewEnd = slewStart + *thisSlewtime;
            if (newSlewEnd > oldSlewEnd) {
                timeDiff = newSlewEnd - oldSlewEnd;
            } else {
                timeDiff = oldSlewEnd - newSlewEnd;
            }
        }
        // update the slewtime
        times_.updateSlewtime(ista, newSlewEnd - slewStart);
        ++ista;
    }

    return scanValid;
}

bool Scan::rigorousScanStartTimeAlignment(const std::vector<Station> &stations, const Source &source) noexcept{
    bool scanValid = true;

    // iteratively align start times
    unsigned long nsta_beginning;
    do {
        nsta_beginning = nsta_;

        // align start times
        times_.alignStartTimes();
        scanValid = constructBaselines(source);
        if (!scanValid) {
            return scanValid;
        }
        if(nsta_ != nsta_beginning){
            continue;
        }

        // calc baseline scan durations
        scanValid = calcBaselineScanDuration(stations, source);
        if (!scanValid) {
            return scanValid;
        }
        if(nsta_ != nsta_beginning){
            continue;
        }

        // calc scan durations
        scanValid = scanDuration(stations, source);
        if (!scanValid) {
            return scanValid;
        }

    } while (nsta_ != nsta_beginning);

    return scanValid;
}

bool Scan::rigorousScanVisibility(const std::vector<Station> &stations, const Source &source, bool &stationRemoved) noexcept{

    pointingVectorsEndtime_.clear();

    // loop over all stations
    int ista = 0;
    while (ista < nsta_) {

        // get all required members
        unsigned int scanStart = times_.getScanStart(ista);
        unsigned int scanEnd = times_.getScanEnd(ista);
        PointingVector &pv = pointingVectors_[ista];
        const Station &thisStation = stations[pv.getStaid()];

        // create moving pointing vector which is used to check visibility during scan
        PointingVector moving_pv(pv.getStaid(), pv.getSrcid());
        moving_pv.setAz(pv.getAz());
        moving_pv.setEl(pv.getEl());

        // loop over whole scan time in 30 second steps.
        // Ignore last 30seconds because it is in a next step checked at end time
        for (unsigned int time = scanStart; scanStart < scanEnd; scanStart += 30) {

            // check if there is no change in slew direction (no change in azimuth ambigurity)
            double oldAz = moving_pv.getAz();
            moving_pv.setTime(scanStart);
            thisStation.calcAzEl(source, moving_pv);
            thisStation.getCableWrap().unwrapAzNearAz(moving_pv, oldAz);
            double newAz = moving_pv.getAz();

            // check if there is a change in azimuth ambigurity during scan
            if (std::abs(oldAz - newAz) > .5 * pi) {
                stationRemoved = true;
                return removeStation(ista, source);
            }

            // check if source is visible during scan
            bool flag = thisStation.isVisible(moving_pv, source.getPARA().minElevation);
            if (!flag) {
                stationRemoved = true;
                return removeStation(ista, source);
            }

            if (time == scanStart) {
                pointingVectors_[ista].copyValuesFromOtherPv(moving_pv);
            }
        }

        // check if source is visible at scan end time
        double oldAz = moving_pv.getAz();
        moving_pv.setTime(scanEnd);
        thisStation.calcAzEl(source, moving_pv);
        thisStation.getCableWrap().unwrapAzNearAz(moving_pv, oldAz);
        double newAz = moving_pv.getAz();

        // check if there is a change in azimuth ambigurity during scan
        if (std::abs(oldAz - newAz) > .5 * pi) {
            stationRemoved = true;
            return removeStation(ista, source);
        }

        // check if source is visible during scan
        bool flag = thisStation.isVisible(moving_pv, source.getPARA().minElevation);
        if (!flag) {
            stationRemoved = true;
            return removeStation(ista, source);
        }

        // save pointing vector for end time
        pointingVectorsEndtime_.push_back(moving_pv);
        ++ista;
    }

    return true;
}

bool Scan::rigorousScanCanReachEndposition(const std::vector<Station> &stations, const Source &thisSource,
                                           const boost::optional<FillinmodeEndposition> &endposition,
                                           bool &stationRemoved) {
    if(!endposition.is_initialized()){
        return true;
    }

    for(int idxSta=0; idxSta<nsta_; ++idxSta){
        // start position for slewing
        const PointingVector &slewStart = pointingVectorsEndtime_[idxSta];

        // get station
        int staid = slewStart.getStaid();
        const Station &thisSta = stations[staid];
        const Station::WaitTimes &waitTimes = thisSta.getWaittimes();

        // check if there is a required endposition
        int possibleEndpositionTime;
        if(endposition->hasEndposition(staid)){

            // required endposition for slewing
            const PointingVector &thisEndposition = endposition->getFinalPosition(staid).get();

            // calculate slew time between pointing vectors
            unsigned int slewtime = thisSta.getAntenna().slewTime(slewStart,thisEndposition);

            // check if there is enough time
            possibleEndpositionTime = times_.getScanEnd(idxSta) + waitTimes.fieldSystem + slewtime + waitTimes.preob;
        }else{
            possibleEndpositionTime = times_.getScanEnd(idxSta);
        }

        // get minimum required endpositon time
        int requiredEndpositionTime = endposition->requiredEndpositionTime(staid);

        if(possibleEndpositionTime > requiredEndpositionTime){
            stationRemoved = true;
            return removeStation(idxSta, thisSource);
        }
    }
    return true;
}


void Scan::addTagalongStation(const PointingVector &pv_start, const PointingVector &pv_end, const std::vector<Baseline> &baselines,
                              unsigned int slewtime, const Station &station) {
    pointingVectors_.push_back(pv_start);
    pointingVectorsEndtime_.push_back(pv_end);
    ++nsta_;
    for(auto &any:baselines){
        baselines_.push_back(any);
    }
    if(station.getPARA().firstScan){
        times_.addTagalongStation(pv_start, pv_end, 0, 0, 0, 0);
    }else{
        times_.addTagalongStation(pv_start, pv_end, slewtime,
                                  station.getCurrentTime(),
                                  station.getWaittimes().fieldSystem,
                                  station.getWaittimes().preob);
    }
}

double Scan::calcScore_firstPart(const std::vector<double> &astas, const std::vector<double> &asrcs,
                                 unsigned int minTime, unsigned int maxTime, const std::vector<Station> &stations,
                                 const Source &source) {
    unsigned long nmaxsta = stations.size();
    unsigned long nmaxbl = nmaxsta * (nmaxsta - 1) / 2;
    double this_score = 0;

    double weight_numberOfObservations = WeightFactors::weightNumberOfObservations;
    if (weight_numberOfObservations != 0) {
        this_score += calcScore_numberOfObservations(nmaxbl) * weight_numberOfObservations;
    }
    double weight_averageSources = WeightFactors::weightAverageSources;
    if (weight_averageSources != 0) {
        this_score += calcScore_averageSources(asrcs) * weight_averageSources;
    }
    double weight_averageStations = WeightFactors::weightAverageStations;
    if (weight_averageStations != 0) {
        this_score += calcScore_averageStations(astas, nmaxsta) * weight_averageStations;
    }
    double weight_duration = WeightFactors::weightDuration;
    if (weight_duration != 0) {
        this_score += calcScore_duration(minTime, maxTime) * weight_duration;
    }

    double weightDeclination = WeightFactors::weightDeclination;
    if (weightDeclination != 0) {
        double dec = source.getDe();
        double f = 0;
        if (dec < WeightFactors::declinationStartWeight) {
            f = 0;
        } else if (dec > WeightFactors::declinationFullWeight) {
            f = 1;
        } else {
            f = (dec - WeightFactors::declinationStartWeight) /
                (WeightFactors::declinationFullWeight - WeightFactors::declinationStartWeight);
        }
        this_score += f * weightDeclination;
    }

    double weightLowElevation = WeightFactors::weightLowElevation;
    if (weightLowElevation != 0) {
        this_score += calcScore_lowElevation() * weightLowElevation;
    }

    this_score *= source.getPARA().weight * weight_stations(stations) * weight_baselines();
    return this_score;
}

double Scan::calcScore_secondPart(double this_score, const Source &source) {
    if (source.getPARA().tryToFocusIfObservedOnce) {
        unsigned int nscans = source.getNscans();
        if (nscans > 0) {
            if(*source.getPARA().tryToFocusOccurrency == Source::TryToFocusOccurrency::once){
                if(*source.getPARA().tryToFocusType == Source::TryToFocusType::additive){
                    this_score += *source.getPARA().tryToFocusFactor;
                }else{
                    this_score *= *source.getPARA().tryToFocusFactor;
                }
            }else{
                if(*source.getPARA().tryToFocusType == Source::TryToFocusType::additive){
                    this_score += (nscans * *source.getPARA().tryToFocusFactor);
                }else{
                    this_score *= (nscans * *source.getPARA().tryToFocusFactor);
                }
            }
        }
    }

    if(scanSequence.customScanSequence){
        if(nScanSelections != 0 && scanSequence.targetSources.find(scanSequence.moduloScanSelctions) != scanSequence.targetSources.end()){
            const vector<int> &target = scanSequence.targetSources[scanSequence.moduloScanSelctions];
            if(find(target.begin(),target.end(),source.getId()) != target.end()){
                this_score *= 100;
            }else{
                this_score /= 100;
            }
        }
    }
    return this_score;
}


void Scan::calcScore(const std::vector<double> &astas, const std::vector<double> &asrcs,
                     unsigned int minTime, unsigned int maxTime, const std::vector<Station> &stations,
                     const Source &source, const std::vector<SkyCoverage> &skyCoverages) noexcept {

    double this_score = calcScore_firstPart(astas, asrcs, minTime, maxTime, stations, source);


    double weight_skyCoverage = WeightFactors::weightSkyCoverage;
    if (weight_skyCoverage != 0) {
        this_score += calcScore_skyCoverage(skyCoverages, stations) * weight_skyCoverage;
    }

    score_ = calcScore_secondPart(this_score,source);
}

void Scan::calcScore(const std::vector<double> &astas,
                     const std::vector<double> &asrcs, unsigned int minTime, unsigned int maxTime,
                     const std::vector<SkyCoverage> &skyCoverages, const std::vector<Station> &stations,
                     const Source &source,
                     std::vector<double> &firstScorePerPV) noexcept {

    double this_score = calcScore_firstPart(astas, asrcs, minTime, maxTime, stations, source);


    double weight_skyCoverage = WeightFactors::weightSkyCoverage;
    if (weight_skyCoverage != 0) {
        this_score += calcScore_skyCoverage(skyCoverages, stations, firstScorePerPV) * weight_skyCoverage;
    }

    score_ = calcScore_secondPart(this_score,source);
}


void Scan::calcScore_subnetting(const std::vector<double> &astas,
                                const std::vector<double> &asrcs, unsigned int minTime, unsigned int maxTime,
                                const std::vector<SkyCoverage> &skyCoverages, const std::vector<Station> &stations,
                                const Source &source, const std::vector<double> &firstScorePerPV) noexcept {

    double this_score = calcScore_firstPart(astas, asrcs, minTime, maxTime, stations, source);

    double weight_skyCoverage = WeightFactors::weightSkyCoverage;
    if (weight_skyCoverage != 0) {
        this_score += calcScore_skyCoverage_subcon(skyCoverages, stations, firstScorePerPV) * weight_skyCoverage * .5;
    }

    score_ = calcScore_secondPart(this_score,source);
}

bool Scan::calcScore(const std::vector<double> &prevLowElevationScores, const std::vector<double> &prevHighElevationScores,
                     const vector<Station> &stations, unsigned int minRequiredTime, unsigned int maxRequiredTime,
                     const Source &source) {
    unsigned long nmaxsta = stations.size();
    unsigned long nmaxbl = nmaxsta * (nmaxsta - 1) / 2;

    double lowElevationSlopeStart = CalibratorBlock::lowElevationStartWeight;
    double lowElevationSlopeEnd = CalibratorBlock::lowElevationFullWeight;

    double highElevationSlopeStart = CalibratorBlock::highElevationStartWeight;
    double highElevationSlopeEnd = CalibratorBlock::highElevationFullWeight;

    double improvementLowElevation = 0;
    double improvementHighElevation = 0;

    int i=0;
    while( i<nsta_ ){
        const PointingVector &pv = pointingVectors_[i];
        int staid = pv.getStaid();
        double el = pv.getEl();

        double lowElScore;
        if(el>lowElevationSlopeStart) {
            lowElScore = 0;
        }else if(el<lowElevationSlopeEnd) {
            lowElScore = 1;
        } else {
            lowElScore = (lowElevationSlopeStart-el)/(lowElevationSlopeStart-lowElevationSlopeEnd);
        }
        double deltaLowElScore = lowElScore-prevLowElevationScores[staid];
        if(deltaLowElScore>0){
            improvementLowElevation += deltaLowElScore;
        }


        double highElScore;
        if(el<highElevationSlopeStart) {
            highElScore = 0;
        }else if(el>highElevationSlopeEnd) {
            highElScore = 1;
        } else {
            highElScore = (el-highElevationSlopeStart)/(highElevationSlopeEnd-lowElevationSlopeStart);
        }
        double deltaHighElScore = highElScore-prevHighElevationScores[staid];
        if(deltaHighElScore>0){
            improvementHighElevation += deltaHighElScore;
        }

        if(deltaLowElScore+deltaHighElScore == 0){
            bool scanValid = removeStation(i, source);
            if(!scanValid){
                return false;
            }
        }
        ++i;
    }

    double scoreDuration = calcScore_duration(minRequiredTime, maxRequiredTime)*.1;

    double scoreBaselines = static_cast<double>(baselines_.size())/ static_cast<double>(nmaxbl)*.5;

    double this_score = improvementLowElevation/nsta_ + improvementHighElevation/nsta_ + scoreDuration + scoreBaselines;

    this_score *= source.getPARA().weight * weight_stations(stations) * weight_baselines();

    score_ = this_score;
    return true;
}


void
Scan::output(unsigned long observed_scan_nr, const vector<Station> &stations, const Source &source,
                  ofstream &of) const noexcept {
    string type;
    string type2;
    switch (type_){
        case ScanType::standard: type = "target"; break;
        case ScanType::fillin: type = "fillin"; break;
        case ScanType::calibrator: type = "calibrator"; break;
    }

    switch (constellation_){
        case ScanConstellation::single: type2 = "single source scan"; break;
        case ScanConstellation::subnetting: type2 = "subnetting scan"; break;
    }

    of << boost::format("Scan: no%04d  Source: %-8s (id: %3d) Ra: %s Dec %s Start_time: %s Stop_time: %s Type: %s %s (id: %d)\n")
       % observed_scan_nr % source.getName() % source.getId() % Miscellaneous::ra2dms(source.getRa()) % Miscellaneous::dc2hms(source.getDe())
       % TimeSystem::ptime2string(TimeSystem::internalTime2PosixTime(times_.getScanStart()))
       % TimeSystem::ptime2string(TimeSystem::internalTime2PosixTime(times_.getScanEnd()))
       % type % type2 %getId();

    for(int i=0; i<nsta_; ++i){
        const auto &pv = pointingVectors_[i];
        const auto &thisSta = stations[pv.getStaid()];
        double az = pv.getAz()*rad2deg;
        while(az<=0){
            az+=360;
        }
        az = fmod(az,360);

        of << boost::format("    %-8s: fs: %2d [s] slew: %3d [s] idle: %3d [s] preob: %3d [s] obs: %3d [s] (%s - %s) az: %8.4f unaz: %8.4f el: %7.4f (id: %d)\n")
              % thisSta.getName() %times_.getFieldSystemTime(i) % times_.getSlewTime(i) % times_.getIdleTime(i) % times_.getPreobTime(i) % times_.getScanTime(i)
              % TimeSystem::ptime2string(TimeSystem::internalTime2PosixTime(times_.getScanStart(i)))
              % TimeSystem::ptime2string(TimeSystem::internalTime2PosixTime(times_.getScanEnd(i)))
              % az % (pv.getAz()*rad2deg) % (pv.getEl()*rad2deg) %pv.getId();
    }
}

boost::optional<Scan> Scan::copyScan(const std::vector<int> &ids, const Source &source) const noexcept {

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
    if (pv.size() < source.getPARA().minNumberOfStations) {
        return boost::none;
    }

    if (!source.getPARA().requiredStations.empty()) {
        const vector<int> &rsta = source.getPARA().requiredStations;
        for (auto &thisRequiredStationId:rsta) {
            if (find(ids.begin(), ids.end(), thisRequiredStationId) == ids.end()) {
                return boost::none;
            }
        }
    }


    for (int i = (int) nsta_ - 1; i >= 0; --i) {
        int thisId = pointingVectors_[i].getStaid();
        if (find(ids.begin(), ids.end(), thisId) == ids.end()) {
            t.removeElement(i);
        }
    }

    for (const auto &baseline : baselines_) {
        const Baseline &thisBl = baseline;
        int staid1 = thisBl.getStaid1();
        int staid2 = thisBl.getStaid2();

        if (find(ids.begin(), ids.end(), staid1) != ids.end() &&
            find(ids.begin(), ids.end(), staid2) != ids.end()) {
            bl.push_back(baseline);
        }
    }
    if (bl.empty()) {
        return boost::none;
    }

    return Scan(pv, t, bl);
}


bool Scan::possibleFillinScan(const vector<Station> &stations, const Source &source,
                                   const std::vector<char> &unused,
                                   const vector<PointingVector> &pv_final_position) {
    int pv_id = 0;
    while (pv_id < nsta_) {
        const PointingVector fillinScanStart = pointingVectors_[pv_id];
        int staid = fillinScanStart.getStaid();

        // this is the endtime of the fillin scan, this means this is also the start time to slew to the next source
        unsigned int endOfFillinScan = times_.getScanEnd(pv_id);
        const PointingVector &finalPosition = pv_final_position[staid];

        if (!unused[staid]) { // unused station... this means we have an desired end pointing vector

            const Station &thisStation = stations[staid];

            PointingVector fillinScanEnd(staid, srcid_);
            if (pointingVectorsEndtime_.empty()) {
                // create pointing vector at end of scan
                fillinScanEnd.setTime(endOfFillinScan);

                // calculate azimuth and elevation at end of scan
                thisStation.calcAzEl(source, fillinScanEnd);

                bool visible = thisStation.isVisible(fillinScanEnd, source.getPARA().minElevation);
                if (!visible) {
                    bool valid = removeStation(pv_id, source);
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
            unsigned int slewTime = thisStation.getAntenna().slewTime(fillinScanEnd, finalPosition);

            // check the available time
            unsigned int pv_finalTime = finalPosition.getTime();
            unsigned int availableTime;
            if (pv_finalTime > endOfFillinScan) {
                availableTime = pv_finalTime - endOfFillinScan;
            } else {
                availableTime = 0;
            }

            const Station::WaitTimes &wtimes = thisStation.getWaittimes();
            int time_needed = slewTime + wtimes.fieldSystem + wtimes.preob;
            if (time_needed > availableTime) {
                bool valid = removeStation(pv_id, source);
                if (!valid) {
                    return false;
                }
                continue;
            }

        } else { // if station is not used in the following scans (this means we have no desired end pointing vector
            // check if the end of the fillin scan if earlier as the required final position time
            if (endOfFillinScan > finalPosition.getTime()) {
                bool valid = removeStation(pv_id, source);
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

double Scan::weight_stations(const std::vector<Station> &stations) {
    double weight = 0;
    for (const auto &any:pointingVectors_) {
        weight += stations[any.getStaid()].getPARA().weight;
    }
    weight /= pointingVectors_.size();

    return weight;
}

double Scan::weight_baselines() {
    double weight = 0;
    for (const auto &any:baselines_) {
        weight += Baseline::PARA.weight[any.getStaid1()][any.getStaid2()];
    }
    weight /= baselines_.size();

    return weight;
}

double Scan::calcScore_lowElevation() {
    double score = 0;
    for (const auto &pv:pointingVectors_) {
        double el = pv.getEl();
        double f = 0;
        if (el < WeightFactors::lowElevationStartWeight) {
            f = 0;
        } else if (el > WeightFactors::lowElevationFullWeight) {
            f = 1;
        } else {
            f = (el - WeightFactors::lowElevationStartWeight) /
                (WeightFactors::lowElevationFullWeight - WeightFactors::lowElevationStartWeight);
        }
        score += f;
    }
    return score / nsta_;
}

void Scan::setFixedScanDuration(unsigned int scanDuration) noexcept{
    times_.addScanTimes(scanDuration);
}

bool Scan::setScanTimes(const vector<unsigned int> &eols, unsigned int fieldSystemTime,
                        const vector<unsigned int> &slewTime, unsigned int preob,
                        unsigned int scanStart, const vector<unsigned int> &scanDurations) {
    times_.setEndOfLastScan(eols);
    for(int i=0; i < slewTime.size(); ++i){
        times_.addTimes(i, fieldSystemTime, slewTime.at(static_cast<unsigned long>(i)), 0);
    }
    times_.setStartTime(scanStart);
    bool valid = times_.substractPreobTimeFromStartTime(preob);
    times_.addScanTimes(scanDurations);
    return valid;
}

void Scan::setPointingVectorsEndtime(vector<PointingVector> pv_end) {
    pointingVectorsEndtime_ = std::move(pv_end);
}









