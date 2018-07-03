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

using namespace std;
using namespace VieVS;

unsigned int Scan::nScanSelections{0};
Scan::ScanSequence Scan::scanSequence;
unsigned long Scan::nextId = 0;



Scan::Scan(vector<PointingVector> &pointingVectors, vector<unsigned int> &endOfLastScan, ScanType type):
        VieVS_Object(nextId++), times_{ScanTimes(static_cast<unsigned int>(pointingVectors.size()))},
        pointingVectors_{move(pointingVectors)}, type_{type}, constellation_{ScanConstellation::single}, score_{0}{
    nsta_ = Scan::pointingVectors_.size();
    srcid_ = Scan::pointingVectors_.at(0).getSrcid();
    times_.setEndOfLastScan(endOfLastScan);
    observations_.reserve((nsta_ * (nsta_ - 1)) / 2);
}

Scan::Scan(vector<PointingVector> pv, ScanTimes times, vector<Observation> obs):
        VieVS_Object(nextId++), srcid_{pv[0].getSrcid()}, nsta_{pv.size()}, pointingVectors_{move(pv)},
        score_{0}, times_{move(times)}, observations_{move(obs)}, constellation_{ScanConstellation::subnetting},
        type_{ScanType::standard}{

    times_.giveNewId();

}

bool Scan::constructObservations(const Network &network, const Source &source) noexcept {
    observations_.clear();
    bool valid = false;
    unsigned long srcid = source.getId();

    // loop over all pointingVectors
    for (int i=0; i<pointingVectors_.size(); ++i){
        for (int j=i+1; j<pointingVectors_.size(); ++j){

            unsigned long staid1 = pointingVectors_[i].getStaid();
            unsigned long staid2 = pointingVectors_[j].getStaid();
            unsigned long blid = network.getBaseline(staid1,staid2).getId();

            // check if Baseline is ignored
            if(network.getBaseline(staid1,staid2).getParameters().ignore){
                continue;
            }
            // check if this source has to ignore this baseline
            if (!source.getPARA().ignoreBaselines.empty()) {
                const auto &PARA = source.getPARA();
                const Baseline &bl = network.getBaseline(staid1,staid2);
                unsigned long blid = bl.getId();
                if (find(PARA.ignoreBaselines.begin(), PARA.ignoreBaselines.end(), blid) !=
                    PARA.ignoreBaselines.end()) {
                    continue;
                }

            }

            // add new baseline
            unsigned int startTime = max({times_.getObservingStart(i), times_.getObservingStart(j)});
            observations_.emplace_back(blid, staid1, staid2, srcid, startTime);
            valid = true;
        }
    }
    return valid;
}

void Scan::addTimes(int idx, unsigned int fieldSystem, unsigned int slew, unsigned int preob) noexcept {
    times_.addTimes(idx, fieldSystem, slew, preob);
}

bool Scan::removeStation(int idx, const Source &source) noexcept {

    --nsta_;
    // check if you have still enough stations
    if (nsta_ < source.getPARA().minNumberOfStations) {
        return false;
    }

    // check if you want to remove a required station
    unsigned long staid = pointingVectors_[idx].getStaid();
    if (!source.getPARA().requiredStations.empty()) {
        const vector<unsigned long> &rsta = source.getPARA().requiredStations;
        if (find(rsta.begin(), rsta.end(), staid) != rsta.end()) {
            return false;
        }
    }

    // remove element from time
    times_.removeElement(idx);

    // erase the pointing vector
    pointingVectors_.erase(next(pointingVectors_.begin(),idx));
    if (!pointingVectorsEndtime_.empty()) {
        pointingVectorsEndtime_.erase(next(pointingVectorsEndtime_.begin(),idx));
    }

    // remove all observations with this station
    unsigned long nbl_before = observations_.size();
    int i=0;
    while (i<observations_.size()){
        if(observations_[i].containsStation(staid)){
            observations_.erase(next(observations_.begin(),i));
        } else {
            ++i;
        }
    }
    // check if there are any observations left
    return !(nbl_before != 0 && observations_.empty());
}

bool Scan::removeObservation(int idx_bl, const Source &source) noexcept {

    // remove observation
    unsigned long staid1 = observations_[idx_bl].getStaid1();
    unsigned long staid2 = observations_[idx_bl].getStaid2();
    observations_.erase(next(observations_.begin(),idx_bl));
    if (observations_.empty()) {
        return false;
    }

    // check if it necessary to remove a pointing vector (a station is no longer part of any baseline)
    bool scanValid = true;
    int counterStaid1 = 0;
    int counterStaid2 = 0;
    for (const auto& any: observations_) {
        if(any.containsStation(staid1)){
            ++counterStaid1;
        }
        if(any.containsStation(staid2)){
            ++counterStaid2;
        }
    }

    // remove station if necessary
    if(counterStaid1==0){
        boost::optional<unsigned long> idx_pv = findIdxOfStationId(staid1);
        scanValid = removeStation(static_cast<int>(*idx_pv), source);
    }
    if(scanValid && counterStaid2==0){
        boost::optional<unsigned long> idx_pv = findIdxOfStationId(staid2);
        scanValid = removeStation(static_cast<int>(*idx_pv), source);
    }
    return scanValid;
}


bool Scan::checkIdleTimes(std::vector<unsigned int> &maxIdle, const Source &source) noexcept {


    bool scan_valid = true;
    bool idleTimeValid;
    unsigned int latestSlewTime;
    // check if idle time is in valid range
    do {
        idleTimeValid = true;

        // get end of slew times
        const vector<unsigned int> &eosl = times_.getEndOfSlewTime();
        auto it= max_element(eosl.begin(), eosl.end());
        latestSlewTime = *it;
        long idx = distance(eosl.begin(), it);

        // check idle time for each station
        for (int i = 0; i < nsta_; ++i) {
            unsigned int dt = latestSlewTime-eosl[i];
            // if idle time is too long remove station which has latest slew arrival and restart
            if (dt>maxIdle[i]){
                scan_valid = removeStation(static_cast<int>(idx), source);
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

    // align start times at the end
    if(scan_valid){
        times_.alignStartTimes();
    }

    return scan_valid;
}

bool Scan::calcObservationDuration(const Network &network, const Source &source) noexcept {

    // check if it is a calibrator scan and there is a fixed scan duration for calibrator scans
    if(type_ == ScanType::calibrator){
        if(CalibratorBlock::targetScanLengthType == CalibratorBlock::TargetScanLengthType::seconds){
            unsigned int maxObservingTime = CalibratorBlock::scanLength;
            // set baseline scan duration to fixed scan duration
            for(auto &thisObservation:observations_){
                thisObservation.setObservingTime(maxObservingTime);
            }
            return true;
        }
    }

    // check if there is a fixed scan duration for this source
    boost::optional<unsigned int> fixedScanDuration = source.getPARA().fixedScanDuration;
    if (fixedScanDuration.is_initialized()) {
        unsigned int maxObservingTime = *fixedScanDuration;
        // set baseline scan duration to fixed scan duration
        for(auto &thisBaseline:observations_){
            thisBaseline.setObservingTime(maxObservingTime);
        }
        return true;
    }


    // loop over all observed baselines
    int idxObs = 0;
    while (idxObs < observations_.size()) {
        auto &thisObservation = observations_[idxObs];

        // get station ids from this baseline
        unsigned long staid1 = thisObservation.getStaid1();
        const Station &sta1 = network.getStation(staid1);
        unsigned long staid2 = thisObservation.getStaid2();
        const Station &sta2 = network.getStation(staid2);
        const Baseline &bl = network.getBaseline(staid1,staid2);

        // get baseline scan start time
        unsigned int startTime = thisObservation.getStartTime();

        // calculate greenwhich meridian sedirial time
        double date1 = 2400000.5;
        double date2 = TimeSystem::mjdStart + static_cast<double>(startTime) / 86400.0;
        double gmst = iauGmst82(date1, date2);

        unsigned int maxDuration = 0;

        // loop over each band
        bool flag_baselineRemoved = false;
        for (auto &band:ObservationMode::bands) {

            // calculate observed flux density for each band
            double SEFD_src = source.observedFlux(band, gmst, network.getDxyz(staid1,staid2));

            // calculate system equivalent flux density for each station
            double el1 = pointingVectors_[*findIdxOfStationId(staid1)].getEl();
            double SEFD_sta1 = sta1.getEquip().getSEFD(band, el1);
            double el2 = pointingVectors_[*findIdxOfStationId(staid2)].getEl();
            double SEFD_sta2 = sta2.getEquip().getSEFD(band, el2);

            // get minimum required SNR for each station, baseline and source
            double minSNR_sta1 = sta1.getPARA().minSNR.at(band);
            double minSNR_sta2 = sta2.getPARA().minSNR.at(band);
            double minSNR_bl = bl.getParameters().minSNR.at(band);
            double minSNR_src = source.getPARA().minSNR.at(band);

            // maximum required minSNR
            double maxminSNR = max({minSNR_src,minSNR_bl, minSNR_sta1, minSNR_sta2});

            // get maximum correlator synchronization time for
            double maxCorSynch1 = sta1.getWaittimes().midob;
            double maxCorSynch2 = sta2.getWaittimes().midob;
            double maxCorSynch = max({maxCorSynch1, maxCorSynch2});

            // calc required baseline scan duration
            double anum = (1.75*maxminSNR / SEFD_src);
            double anu1 = SEFD_sta1*SEFD_sta2;
            double anu2 = ObservationMode::sampleRate * 1.0e6 * ObservationMode::nChannels[band] * ObservationMode::bits;
            double new_duration = anum*anum *anu1/anu2 + maxCorSynch;
            new_duration = ceil(new_duration);
            auto new_duration_uint = static_cast<unsigned int>(new_duration);

            // check if required baseline scan duration is within min and max scan times of baselines
            unsigned int minScanBl = bl.getParameters().minScan;
            if(new_duration_uint<minScanBl){
                new_duration_uint = minScanBl;
            }
            unsigned int maxScanBl = bl.getParameters().maxScan;
            if(new_duration_uint>maxScanBl){
                bool scanValid = removeObservation(idxObs, source);
                if(!scanValid){
                    return false;
                }
                flag_baselineRemoved = true;
                break;
            }

            // change maxDuration if it is higher for this band
            if (new_duration_uint > maxDuration) {
                maxDuration = new_duration_uint;
            }
        }

        // if you have not removed the baseline increment counter and set the baseline scan duration
        if(!flag_baselineRemoved){
            ++idxObs;
            thisObservation.setObservingTime(maxDuration);
        }
    }

    return true;
}

bool Scan::scanDuration(const Network &network, const Source &source) noexcept {

    // check if it is a calibrator scan with a fixed scan duration
    if(type_ == ScanType::calibrator){
        if(CalibratorBlock::targetScanLengthType == CalibratorBlock::TargetScanLengthType::seconds){
            times_.addObservingTime(CalibratorBlock::scanLength);
            return true;
        }
    }

    // check if there is a fixed scan duration for this source
    boost::optional<unsigned int> fixedScanDuration = source.getPARA().fixedScanDuration;
    if (fixedScanDuration.is_initialized()) {
        times_.addObservingTime(*fixedScanDuration);
        return true;
    }

    // save minimum and maximum scan time
    vector<unsigned int> minscanTimes(nsta_, source.getPARA().minScan);
    vector<unsigned int> maxScanTimes(nsta_, source.getPARA().maxScan);
    for (int i = 0; i < nsta_; ++i) {
        unsigned long staid = pointingVectors_[i].getStaid();
        unsigned int stationMinScanTime = network.getStation(staid).getPARA().minScan;
        unsigned int stationMaxScanTime = network.getStation(staid).getPARA().maxScan;

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
        for (auto &thisObservation : observations_) {

            // get index of stations in this baseline
            unsigned long staid1 = thisObservation.getStaid1();
            boost::optional<unsigned long> staidx1o = findIdxOfStationId(staid1);
            unsigned long staidx1 = *staidx1o;
            unsigned long staid2 = thisObservation.getStaid2();
            boost::optional<unsigned long> staidx2o = findIdxOfStationId(staid2);
            unsigned long staidx2 = *staidx2o;

            // get scan duration of baseline
            unsigned int duration = thisObservation.getObservingTime();

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
                    unsigned long staid = pointingVectors_[thisIdx].getStaid();
                    double thisMaxSEFD = network.getStation(staid).getEquip().getMaxSEFD();
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

    times_.addObservingTimes(scanTimes);
    return true;
}

boost::optional<unsigned long> Scan::findIdxOfStationId(unsigned long id) const noexcept {
    for (unsigned long idx = 0; idx < nsta_; ++idx) {
        if(pointingVectors_[idx].getStaid()==id){
            return idx;
        }
    }
    return boost::none;
}

vector<unsigned long> Scan::getStationIds() const noexcept {
    vector<unsigned long> ids;
    for (int i = 0; i < nsta_; ++i) {
        ids.push_back(pointingVectors_[i].getStaid());
    }

    return std::move(ids);
}

double Scan::calcScore_numberOfObservations(unsigned long maxObs) const noexcept {
    unsigned long nbl = observations_.size();
    double thisScore = static_cast<double>(nbl) / static_cast<double>(maxObs);
    return thisScore;
}

double Scan::calcScore_idleTime() const noexcept {
    double interval = WeightFactors::idleTimeInterval;

    double score = 0;
    for(int idx=0; idx<nsta_; ++idx){
        unsigned int thisIdleTime = times_.getIdleTime(idx);
        score += static_cast<double>(thisIdleTime) / interval;
    }

    return score / nsta_;
}


double Scan::calcScore_averageStations(const vector<double> &astas, unsigned long nMaxSta) const noexcept {
    double finalScore = 0;
    unsigned long nMaxStaPossible = nMaxSta-1;

    for(const auto &pv:pointingVectors_){
        unsigned long staid = pv.getStaid();
        unsigned long nObs = getNObs(staid);
        finalScore += astas[staid] * nObs / nMaxStaPossible;
    }

    return finalScore;
}

double Scan::calcScore_averageBaselines(const std::vector<double> &abls) const noexcept{

    double finalScore = 0;
    for(const auto &obs:observations_){
        unsigned long blid = obs.getBlid();
        finalScore += abls[blid];
    }

    return finalScore;
}


double Scan::calcScore_averageSources(const vector<double> &asrcs, unsigned long nMaxBls) const noexcept {

    unsigned long nbl = observations_.size();
    return asrcs[srcid_] * nbl / nMaxBls;

}

double Scan::calcScore_duration(unsigned int minTime, unsigned int maxTime) const noexcept {
    unsigned int thisScanDuration = times_.getScanDuration();
    double score;
    if (maxTime - minTime == 0) {
        score = 1;
    } else {
        score = 1 - static_cast<double>(thisScanDuration - minTime) / static_cast<double>(maxTime - minTime);
    }
    return score;
}

double Scan::calcScore_lowElevation() {
    double score = 0;
    for (const auto &pv:pointingVectors_) {
        double el = pv.getEl();
        double f = 0;
        if (el > WeightFactors::lowElevationStartWeight) {
            f = 0;
        } else if (el < WeightFactors::lowElevationFullWeight) {
            f = 1;
        } else {
            f = (WeightFactors::lowElevationStartWeight - el) /
                (WeightFactors::lowElevationFullWeight - WeightFactors::lowElevationStartWeight);
        }
        score += f ;
    }
    return score / nsta_;
}


bool Scan::rigorousUpdate(const Network &network, const Source &source,
                          const boost::optional<StationEndposition> &endposition) noexcept {
    bool scanValid;

    //check if source is available during whole scan and iteratively check everything
    bool stationRemoved;
    do {
        stationRemoved = false;

        // calc earliest possible slew end times for each station:
        scanValid = rigorousSlewtime(network, source);
        if(!scanValid){
            return scanValid;
        }

        // align start times to earliest possible one:
        scanValid = rigorousScanStartTimeAlignment(network, source);
        if(!scanValid){
            return scanValid;
        }

        // check if source is available during whole scan
        scanValid = rigorousScanVisibility(network, source, stationRemoved);
        if(!scanValid){
            return scanValid;
        }
        if(stationRemoved){
            continue;
        }

        // check if end position can be reached
        scanValid = rigorousScanCanReachEndposition(network, source, endposition, stationRemoved);
        if(!scanValid){
            return scanValid;
        }

    } while (stationRemoved);

    return true;
}

bool Scan::rigorousSlewtime(const Network &network, const Source &source) noexcept {

    bool scanValid = true;

    // loop through all stations
    int ista = 0;
    while (ista < nsta_) {
        PointingVector &pv = pointingVectors_[ista];
        unsigned int slewStart = times_.getSlewStart(ista);
        const Station &thisStation = network.getStation(pv.getStaid());

        // old slew end time and new slew end time, required for iteration
        unsigned int oldSlewEnd = 0;
        unsigned int newSlewEnd = times_.getSlewEnd(ista);

        // big slew indicates if the slew distance is > 180 degrees
        bool bigSlew = false;

        // timeDiff is difference between two estimated slew rates in iteration.
        unsigned int timeDiff = numeric_limits<unsigned int>::max();

        bool stationRemoved = false;
        // iteratively calculate slew time
        while (timeDiff > 1) {
            // change slew times for iteration
            oldSlewEnd = newSlewEnd;
            double oldAz = pv.getAz();

            // calculate az, el for pointing vector for previouse time
            pv.setTime(oldSlewEnd);
            thisStation.calcAzEl(source,pv);
            if(!thisStation.isVisible(pv,source.getPARA().minElevation)){
                scanValid = removeStation(ista, source);
                if (!scanValid) {
                    return scanValid;
                }
                stationRemoved = true;
                break;
            }
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
                    stationRemoved = true;
                    break;
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
                stationRemoved = true;
                break;
            }

            // calculate new slew end time and time difference
            newSlewEnd = slewStart + *thisSlewtime;
            if (newSlewEnd > oldSlewEnd) {
                timeDiff = newSlewEnd - oldSlewEnd;
            } else {
                timeDiff = oldSlewEnd - newSlewEnd;
            }
        }
        // if no station was removed update slewtimes and increase counter... otherwise restart with same staid
        if(!stationRemoved){
            // update the slewtime
            times_.updateSlewtime(ista, newSlewEnd - slewStart);
            ++ista;
        }
    }

    return scanValid;
}

bool Scan::rigorousScanStartTimeAlignment(const Network &network, const Source &source) noexcept{
    bool scanValid;

    // iteratively align start times
    unsigned long nsta_beginning;
    do {
        nsta_beginning = nsta_;

        // align start times
        times_.alignStartTimes();
        scanValid = constructObservations(network, source);
        if (!scanValid) {
            return scanValid;
        }
        if(nsta_ != nsta_beginning){
            continue;
        }

        // calc baseline scan durations
        scanValid = calcObservationDuration(network, source);
        if (!scanValid) {
            return scanValid;
        }
        if(nsta_ != nsta_beginning){
            continue;
        }

        // calc scan durations
        scanValid = scanDuration(network, source);
        if (!scanValid) {
            return scanValid;
        }

    } while (nsta_ != nsta_beginning);

    return scanValid;
}

bool Scan::rigorousScanVisibility(const Network &network, const Source &source, bool &stationRemoved) noexcept{

    pointingVectorsEndtime_.clear();

    // loop over all stations
    int ista = 0;
    while (ista < nsta_) {

        // get all required members
        unsigned int scanStart = times_.getObservingStart(ista);
        unsigned int scanEnd = times_.getObservingEnd(ista);
        PointingVector &pv = pointingVectors_[ista];
        const Station &thisStation = network.getStation(pv.getStaid());

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

bool Scan::rigorousScanCanReachEndposition(const Network &network, const Source &thisSource,
                                           const boost::optional<StationEndposition> &endposition,
                                           bool &stationRemoved) {
    if(!endposition.is_initialized()){
        return true;
    }

    for(int idxSta=0; idxSta<nsta_; ++idxSta){
        // start position for slewing
        const PointingVector &slewStart = pointingVectorsEndtime_[idxSta];

        // get station
        unsigned long staid = slewStart.getStaid();
        const Station &thisSta = network.getStation(staid);
        const Station::WaitTimes &waitTimes = thisSta.getWaittimes();

        // check if there is a required endposition
        int possibleEndpositionTime;
        if(endposition->hasEndposition(staid)){

            // required endposition for slewing
            const PointingVector &thisEndposition = endposition->getFinalPosition(staid).get();

            // calculate slew time between pointing vectors
            unsigned int slewtime = thisSta.getAntenna().slewTime(slewStart,thisEndposition);

            // check if there is enough time
            possibleEndpositionTime =
                    times_.getObservingEnd(idxSta) + waitTimes.fieldSystem + slewtime + waitTimes.preob;
        }else{
            possibleEndpositionTime = times_.getObservingEnd(idxSta);
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


void Scan::addTagalongStation(const PointingVector &pv_start, const PointingVector &pv_end,
                              const std::vector<Observation> &observations,
                              unsigned int slewtime, const Station &station) {
    pointingVectors_.push_back(pv_start);
    pointingVectorsEndtime_.push_back(pv_end);
    ++nsta_;
    for(auto &any:observations){
        observations_.push_back(any);
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
                                 const std::vector<double> &abls, unsigned int minTime, unsigned int maxTime,
                                 const Network &network, const Source &source) {
    unsigned long nmaxsta = network.getNSta();
    unsigned long nmaxbl = network.getNBls();
    double this_score = 0;

    double weight_numberOfObservations = WeightFactors::weightNumberOfObservations;
    if (weight_numberOfObservations != 0) {
        this_score += calcScore_numberOfObservations(nmaxbl) * weight_numberOfObservations;
    }
    double weight_averageSources = WeightFactors::weightAverageSources;
    if (weight_averageSources != 0 && !asrcs.empty()) {
        this_score += calcScore_averageSources(asrcs, nmaxbl) * weight_averageSources;
    }
    double weight_averageStations = WeightFactors::weightAverageStations;
    if (weight_averageStations != 0 && !astas.empty()) {
        this_score += calcScore_averageStations(astas, nmaxsta) * weight_averageStations;
    }
    double weight_averageBaselines = WeightFactors::weightAverageBaselines;
    if (weight_averageBaselines != 0 && !abls.empty()) {
        this_score += calcScore_averageBaselines(abls) * weight_averageBaselines;
    }
    double weight_duration = WeightFactors::weightDuration;
    if (weight_duration != 0) {
        this_score += calcScore_duration(minTime, maxTime) * weight_duration;
    }
    double weight_idle = WeightFactors::weightIdleTime;
    if(weight_idle != 0) {
        this_score += calcScore_idleTime() * weight_idle;
    }

    double weightDeclination = WeightFactors::weightDeclination;
    if (weightDeclination != 0) {
        double dec = source.getDe();
        double f = 0;
        if (dec > WeightFactors::declinationStartWeight) {
            f = 0;
        } else if (dec < WeightFactors::declinationFullWeight) {
            f = 1;
        } else {
            f = (dec - WeightFactors::declinationFullWeight) /
                (WeightFactors::declinationStartWeight - WeightFactors::declinationFullWeight);
        }
        this_score += f * weightDeclination;
    }

    double weightLowElevation = WeightFactors::weightLowElevation;
    if (weightLowElevation != 0) {
        this_score += calcScore_lowElevation() * weightLowElevation;
    }

    return this_score;
}

double Scan::calcScore_secondPart(double this_score, const Network &network, const Source &source) {
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
            const vector<unsigned long> &target = scanSequence.targetSources[scanSequence.moduloScanSelctions];
            if(find(target.begin(),target.end(),source.getId()) != target.end()){
                this_score *= 100;
            }else{
                this_score /= 100;
            }
        }
    }

    this_score *= source.getPARA().weight * weight_stations(network.getStations()) * weight_baselines(network.getBaselines());

    return this_score;
}


void Scan::calcScore(const std::vector<double> &astas, const std::vector<double> &asrcs,
                     const std::vector<double> &abls, unsigned int minTime, unsigned int maxTime,
                     const Network &network, const Source &source) noexcept {

    double this_score = calcScore_firstPart(astas, asrcs, abls, minTime, maxTime, network, source);


    double weight_skyCoverage = WeightFactors::weightSkyCoverage;
    if (weight_skyCoverage != 0) {
        this_score += network.calcScore_skyCoverage(pointingVectors_) * weight_skyCoverage;
    }

    score_ = calcScore_secondPart(this_score,network,source);
}

void Scan::calcScore(const std::vector<double> &astas, const std::vector<double> &asrcs,
                     const std::vector<double> &abls, unsigned int minTime, unsigned int maxTime,
                     const Network &network, const Source &source,
                     unordered_map<unsigned long, double> &staids2skyCoverageScore) noexcept {

    double this_score = calcScore_firstPart(astas, asrcs, abls, minTime, maxTime, network, source);


    double weight_skyCoverage = WeightFactors::weightSkyCoverage;
    if (weight_skyCoverage != 0) {
        this_score += network.calcScore_skyCoverage(pointingVectors_,staids2skyCoverageScore) * weight_skyCoverage;
    }

    score_ = calcScore_secondPart(this_score, network, source);
}


void Scan::calcScore_subnetting(const std::vector<double> &astas, const std::vector<double> &asrcs,
                                const std::vector<double> &abls, unsigned int minTime, unsigned int maxTime,
                                const Network &network, const Source &source,
                                const unordered_map<unsigned long, double> &staids2skyCoverageScore) noexcept {

    double this_score = calcScore_firstPart(astas, asrcs, abls, minTime, maxTime, network, source);

    double weight_skyCoverage = WeightFactors::weightSkyCoverage;
    if (weight_skyCoverage != 0) {
        this_score += network.calcScore_skyCoverage_subnetting(pointingVectors_,staids2skyCoverageScore)
                      * weight_skyCoverage;
    }

    score_ = calcScore_secondPart(this_score, network, source);
}

void Scan::calcScore(unsigned int minTime, unsigned int maxTime, const Network &network, const Source &source,
                     double hiscore) {

    double this_score = calcScore_firstPart(vector<double>(), vector<double>(), vector<double>(), minTime, maxTime, network, source);


    score_ = calcScore_secondPart(this_score, network, source)*hiscore;

}


bool Scan::calcScore(const std::vector<double> &prevLowElevationScores, const std::vector<double> &prevHighElevationScores,
                     const Network &network, unsigned int minRequiredTime, unsigned int maxRequiredTime,
                     const Source &source) {
    double lowElevationSlopeStart = CalibratorBlock::lowElevationStartWeight;
    double lowElevationSlopeEnd = CalibratorBlock::lowElevationFullWeight;

    double highElevationSlopeStart = CalibratorBlock::highElevationStartWeight;
    double highElevationSlopeEnd = CalibratorBlock::highElevationFullWeight;

    double improvementLowElevation = 0;
    double improvementHighElevation = 0;

    int i=0;
    while( i<nsta_ ){
        const PointingVector &pv = pointingVectors_[i];
        unsigned long staid = pv.getStaid();
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

    double scoreBaselines = static_cast<double>(observations_.size())/ static_cast<double>(network.getNBls())*.5;

    double this_score = improvementLowElevation/nsta_ + improvementHighElevation/nsta_ + scoreDuration + scoreBaselines;

    this_score *= source.getPARA().weight * weight_stations(network.getStations()) * weight_baselines(network.getBaselines());

    score_ = this_score;
    return true;
}


void Scan::output(unsigned long observed_scan_nr, const Network &network, const Source &source,
                  ofstream &of) const noexcept {
    string type;
    string type2;
    switch (type_){
        case ScanType::highImpact: type = "high impact"; break;
        case ScanType::standard: type = "target"; break;
        case ScanType::fillin: type = "fillin mode"; break;
        case ScanType::calibrator: type = "calibrator"; break;
    }

    switch (constellation_){
        case ScanConstellation::single: type2 = "single source scan"; break;
        case ScanConstellation::subnetting: type2 = "subnetting scan"; break;
    }

    of << boost::format("Scan: no%04d  Source: %-8s (id: %3d) Ra: %s Dec %s Start_time: %s Stop_time: %s Type: %s %s (id: %d)\n")
       % observed_scan_nr % source.getName() % source.getId() % util::ra2dms(source.getRa()) % util::dc2hms(source.getDe())
       % TimeSystem::ptime2string(TimeSystem::internalTime2PosixTime(times_.getObservingStart()))
       % TimeSystem::ptime2string(TimeSystem::internalTime2PosixTime(times_.getObservingEnd()))
       % type % type2 %getId();

    for(int i=0; i<nsta_; ++i){
        const PointingVector &pv = pointingVectors_[i];
        const PointingVector &pve = pointingVectorsEndtime_[i];
        const Station &thisSta = network.getStation(pv.getStaid());
        double az = util::wrapToPi(pv.getAz())*rad2deg;

        of << boost::format("    %-8s: fs: %2d [s] slew: %3d [s] idle: %4d [s] preob: %3d [s] obs: %3d [s] (%s - %s) az: %8.4f unaz: %9.4f el: %7.4f (id: %d and %d)\n")
              % thisSta.getName() %times_.getFieldSystemTime(i) % times_.getSlewTime(i) % times_.getIdleTime(i) % times_.getPreobTime(i) %
                times_.getObservingTime(i)
              % TimeSystem::ptime2string(TimeSystem::internalTime2PosixTime(times_.getObservingStart(i)))
              % TimeSystem::ptime2string(TimeSystem::internalTime2PosixTime(times_.getObservingEnd(i)))
              % az % (pv.getAz()*rad2deg) % (pv.getEl()*rad2deg) %pv.getId() %pve.getId();
    }
    of << "*\n";
}

boost::optional<Scan> Scan::copyScan(const std::vector<unsigned long> &ids, const Source &source) const noexcept {

    vector<PointingVector> pv;
    pv.reserve(ids.size());
    ScanTimes t = times_;
    vector<Observation> obs;

    int counter = 0;
    // add all found pointing vectors to new pointing vector vector
    for (auto &any:pointingVectors_) {
        unsigned long id = any.getStaid();
        if (find(ids.begin(), ids.end(), id) != ids.end()) {
            pv.push_back(pointingVectors_[counter]);
        }
        ++counter;
    }
    // check if there are enough pointing vectors
    if (pv.size() < source.getPARA().minNumberOfStations) {
        return boost::none;
    }

    // check if there are some required stations for this source
    if (!source.getPARA().requiredStations.empty()) {
        const vector<unsigned long> &rsta = source.getPARA().requiredStations;
        for (auto &thisRequiredStationId:rsta) {
            if (find(ids.begin(), ids.end(), thisRequiredStationId) == ids.end()) {
                return boost::none;
            }
        }
    }

    // remove times of pointingVectors_(original scan) which are not found (not part of pv)
    for (auto i = static_cast<int>(nsta_ - 1); i >= 0; --i) {
        unsigned long thisId = pointingVectors_[i].getStaid();
        if (find(ids.begin(), ids.end(), thisId) == ids.end()) {
            t.removeElement(i);
        }
    }

    // move all observations whose stations were found to bl
    for (const auto &iobs : observations_) {
        const Observation &thisBl = iobs;
        unsigned long staid1 = thisBl.getStaid1();
        unsigned long staid2 = thisBl.getStaid2();

        if (find(ids.begin(), ids.end(), staid1) != ids.end() &&
            find(ids.begin(), ids.end(), staid2) != ids.end()) {
            obs.push_back(iobs);
        }
    }
    if (obs.empty()) {
        return boost::none;
    }

    return Scan(move(pv), move(t), move(obs));
}


//bool Scan::possibleFillinScan(const vector<Station> &stations, const Source &source,
//                                   const std::vector<char> &unused,
//                                   const vector<PointingVector> &pv_final_position) {
//    int pv_id = 0;
//    while (pv_id < nsta_) {
//        const PointingVector fillinScanStart = pointingVectors_[pv_id];
//        unsigned long staid = fillinScanStart.getStaid();
//
//        // this is the endtime of the fillin scan, this means this is also the start time to slew to the next source
//        unsigned int endOfFillinScan = times_.getObservingEnd(pv_id);
//        const PointingVector &finalPosition = pv_final_position[staid];
//
//        if (!unused[staid]) { // unused station... this means we have an desired end pointing vector
//
//            const Station &thisStation = stations[staid];
//
//            PointingVector fillinScanEnd(staid, srcid_);
//            if (pointingVectorsEndtime_.empty()) {
//                // create pointing vector at end of scan
//                fillinScanEnd.setTime(endOfFillinScan);
//
//                // calculate azimuth and elevation at end of scan
//                thisStation.calcAzEl(source, fillinScanEnd);
//
//                bool visible = thisStation.isVisible(fillinScanEnd, source.getPARA().minElevation);
//                if (!visible) {
//                    bool valid = removeStation(pv_id, source);
//                    if (!valid) {
//                        return false;
//                    }
//                    continue;
//                }
//
//                // unwrap azimuth and elevation at end of scan
//                thisStation.getCableWrap().calcUnwrappedAz(fillinScanStart, fillinScanEnd);
//            } else {
//                fillinScanEnd = pointingVectorsEndtime_[pv_id];
//            }
//
//            // calculate slewtime between end of scan and start of new scan (final position)
//            unsigned int slewTime = thisStation.getAntenna().slewTime(fillinScanEnd, finalPosition);
//
//            // check the available time
//            unsigned int pv_finalTime = finalPosition.getTime();
//            unsigned int availableTime;
//            if (pv_finalTime > endOfFillinScan) {
//                availableTime = pv_finalTime - endOfFillinScan;
//            } else {
//                availableTime = 0;
//            }
//
//            const Station::WaitTimes &wtimes = thisStation.getWaittimes();
//            int time_needed = slewTime + wtimes.fieldSystem + wtimes.preob;
//            if (time_needed > availableTime) {
//                bool valid = removeStation(pv_id, source);
//                if (!valid) {
//                    return false;
//                }
//                continue;
//            }
//
//        } else { // if station is not used in the following scans (this means we have no desired end pointing vector
//            // check if the end of the fillin scan if earlier as the required final position time
//            if (endOfFillinScan > finalPosition.getTime()) {
//                bool valid = removeStation(pv_id, source);
//                if (!valid) {
//                    return false;
//                }
//                continue;
//            }
//        }
//        //station is ok!
//        ++pv_id;
//    }
//    return true;
//}

double Scan::weight_stations(const std::vector<Station> &stations) {
    double weight = 1;
    for (const auto &any:pointingVectors_) {
        weight *= stations[any.getStaid()].getPARA().weight;
    }

    return weight;
}

double Scan::weight_baselines(const std::vector<Baseline> &baselines) {
    double weight = 1;
    for (const auto &any:observations_) {
        weight *= baselines[any.getBlid()].getParameters().weight;
    }

    return weight;
}


void Scan::setFixedScanDuration(unsigned int scanDuration) noexcept{
    times_.addObservingTime(scanDuration);
}

bool Scan::setScanTimes(const vector<unsigned int> &eols, unsigned int fieldSystemTime,
                        const vector<unsigned int> &slewTime, unsigned int preob,
                        unsigned int scanStart, const vector<unsigned int> &observingTimes) {
    times_.setEndOfLastScan(eols);
    for(int i=0; i < slewTime.size(); ++i){
        times_.addTimes(i, fieldSystemTime, slewTime.at(static_cast<unsigned long>(i)), 0);
    }
    times_.setStartTime(scanStart);
    bool valid = times_.substractPreobTimeFromStartTime(preob);
    times_.addObservingTimes(observingTimes);
    return valid;
}

void Scan::setPointingVectorsEndtime(vector<PointingVector> pv_end) {
    pointingVectorsEndtime_ = std::move(pv_end);
}

void Scan::createDummyObservations(const Network &network) {
    for(int i=0; i<nsta_; ++i) {
        unsigned long staid1 = pointingVectors_[i].getStaid();
        unsigned int dur1 = times_.getObservingTime(i);
        for (int j = i + 1; j < nsta_; ++j) {
            unsigned long staid2 = pointingVectors_[j].getStaid();
            unsigned int dur2 = times_.getObservingTime(j);

            unsigned int dur = std::max(dur1, dur2);
            unsigned long blid = network.getBlid(staid1,staid2);

            Observation obs(blid, staid1, staid2, srcid_, times_.getObservingStart(), dur);
            observations_.push_back(std::move(obs));
        }
    }
}

unsigned long Scan::getNObs(unsigned long staid) const noexcept {
    unsigned long n=0;
    for(const auto &any:observations_){
        if(any.containsStation(staid)){
            ++n;
        }
    }
    return n;
}


