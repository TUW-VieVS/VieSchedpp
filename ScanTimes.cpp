//
// Created by mschartn on 14.07.17.
//

#include "ScanTimes.h"
using namespace std;
using namespace VieVS;

ScanTimes::ScanTimes() = default;

ScanTimes::ScanTimes(unsigned int nsta) {
    endOfLastScan_.resize(nsta);
    endOfSetupTime_.resize(nsta);
    endOfSourceTime_.resize(nsta);
    endOfSlewTime_.resize(nsta);
    endOfTapeTime_.resize(nsta);
    endOfIdleTime_.resize(nsta);
    endOfCalibrationTime_.resize(nsta);
    endOfScanTime_.resize(nsta);
}

void
ScanTimes::addTimes(int idx, unsigned int setup, unsigned int source, unsigned int slew, unsigned int tape,
                         unsigned int calib) noexcept {
    endOfSetupTime_[idx] = endOfLastScan_[idx] + setup;
    endOfSourceTime_[idx] = endOfSetupTime_[idx] + source;
    endOfSlewTime_[idx] = endOfSourceTime_[idx] + slew;
    endOfIdleTime_[idx] = endOfSlewTime_[idx];
    endOfTapeTime_[idx] = endOfIdleTime_[idx] + tape;
    endOfCalibrationTime_[idx] = endOfTapeTime_[idx] + calib;
}

void ScanTimes::removeElement(int idx) noexcept {

    if(idx >= endOfLastScan_.size()){
        bool flag = false;
    }

    int e = endOfLastScan_.size();

    if(!(e == endOfSetupTime_.size() &
            e == endOfSourceTime_.size() &
            e == endOfSlewTime_.size() &
            e == endOfIdleTime_.size() &
            e == endOfTapeTime_.size() &
            e == endOfCalibrationTime_.size() &
            e == endOfScanTime_.size())){
        bool flag = false;
    }

    unsigned int res;
    res = endOfLastScan_.at(idx);
    endOfLastScan_.erase(next(endOfLastScan_.begin(),idx));

    res = endOfSetupTime_.at(idx);
    endOfSetupTime_.erase(next(endOfSetupTime_.begin(),idx));

    res = endOfSourceTime_.at(idx);
    endOfSourceTime_.erase(next(endOfSourceTime_.begin(),idx));

    res = endOfSlewTime_.at(idx);
    endOfSlewTime_.erase(next(endOfSlewTime_.begin(),idx));

    res = endOfIdleTime_.at(idx);
    endOfIdleTime_.erase(next(endOfIdleTime_.begin(),idx));

    res = endOfTapeTime_.at(idx);
    endOfTapeTime_.erase(next(endOfTapeTime_.begin(),idx));

    res = endOfCalibrationTime_.at(idx);
    endOfCalibrationTime_.erase(next(endOfCalibrationTime_.begin(),idx));

    res = endOfScanTime_.at(idx);
    endOfScanTime_.erase(next(endOfScanTime_.begin(),idx));


    alignStartTimes();
}

void ScanTimes::updateSlewtime(int idx, unsigned int new_slewtime) noexcept {
    unsigned int currentSlewtime = endOfSlewTime_[idx] - endOfSourceTime_[idx];
    if (currentSlewtime != new_slewtime) {
        unsigned int tapeTime = endOfTapeTime_[idx] - endOfIdleTime_[idx];
        unsigned int calibrationTime = endOfCalibrationTime_[idx] - endOfTapeTime_[idx];
        unsigned int scanTime = endOfScanTime_[idx] - endOfCalibrationTime_[idx];

        int delta = new_slewtime - currentSlewtime;
        endOfSlewTime_[idx] = endOfSlewTime_[idx] + delta;
        endOfIdleTime_[idx] = endOfSlewTime_[idx] - delta;
        endOfTapeTime_[idx] = endOfIdleTime_[idx] + tapeTime;
        endOfCalibrationTime_[idx] = endOfTapeTime_[idx] + calibrationTime;
        endOfScanTime_[idx] = endOfCalibrationTime_[idx] + scanTime;
    }

}

void ScanTimes::alignStartTimes() noexcept {
    int nsta = endOfSlewTime_.size();

    unsigned int latestSlewTime = 0;
    for (int i = 0; i < nsta; ++i) {
        if(endOfSlewTime_[i]>latestSlewTime){
            latestSlewTime = endOfSlewTime_[i];
        }
    }

    for (int i = 0; i < nsta; ++i) {
        unsigned int tapeTime = endOfTapeTime_[i]-endOfIdleTime_[i];
        unsigned int calibrationTime = endOfCalibrationTime_[i]-endOfTapeTime_[i];
        unsigned int scanTime = endOfScanTime_[i]-endOfCalibrationTime_[i];

        endOfIdleTime_[i] = latestSlewTime;
        endOfTapeTime_[i] =  endOfIdleTime_[i] + tapeTime;
        endOfCalibrationTime_[i] = endOfTapeTime_[i] + calibrationTime;
        endOfScanTime_[i] = endOfCalibrationTime_[i] + scanTime;
    }
}

void ScanTimes::setStartTime(unsigned int scanStart) noexcept {
    for (int i = 0; i < endOfSlewTime_.size(); ++i) {
        endOfCalibrationTime_[i] = scanStart;
    }
}

void ScanTimes::addScanTimes(const vector<unsigned int> &scanTimes) noexcept {
    for (int i = 0; i < endOfSlewTime_.size(); ++i) {
        endOfScanTime_[i] = endOfCalibrationTime_[i]+scanTimes[i];
    }
}

void ScanTimes::addScanTimes(unsigned int scanTimes) noexcept {
    for (int i = 0; i < endOfSlewTime_.size(); ++i) {
        endOfScanTime_[i] = endOfCalibrationTime_[i]+scanTimes;
    }
}

unsigned int ScanTimes::maxTime() const noexcept {
    unsigned int max = 0;
    for (auto &thisTime: endOfScanTime_) {
        if (thisTime > max) {
            max = thisTime;
        }
    }
    return max;
}

unsigned int ScanTimes::scanStart() const noexcept {
    unsigned int min = numeric_limits<unsigned int>::max();
    for (auto &thisTime: endOfCalibrationTime_) {
        if (thisTime < min) {
            min = thisTime;
        }
    }
    return min;
}

void ScanTimes::addTagalongStation(const VieVS::PointingVector &pv_start, const VieVS::PointingVector &pv_end,
                                   unsigned int slewtime, unsigned int currentTime, unsigned int setup, unsigned int source, unsigned int tape,
                                   unsigned int calibration) {
    endOfLastScan_.push_back(currentTime);
    endOfSetupTime_.push_back(currentTime+setup);
    endOfSourceTime_.push_back(currentTime+setup+source);
    endOfSlewTime_.push_back(currentTime+setup+source+slewtime);
    endOfIdleTime_.push_back(pv_start.getTime()-calibration-tape);
    endOfTapeTime_.push_back(pv_start.getTime()-calibration);
    endOfCalibrationTime_.push_back(pv_start.getTime());
    endOfScanTime_.push_back(pv_end.getTime());
}

bool ScanTimes::substractPreobTimeFromStartTime(unsigned int preob) {
    bool error = false;
    for(int i=0; i<endOfScanTime_.size(); ++i){
        endOfTapeTime_[i] = endOfCalibrationTime_[i]-preob;
        endOfIdleTime_[i] = endOfTapeTime_[i];
        if(endOfIdleTime_[i] < endOfSlewTime_[i]){
            error = true;
        }
    }
    return error;
}



