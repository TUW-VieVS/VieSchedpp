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
    endOfLastScan_.erase(endOfLastScan_.begin()+idx);
    endOfSetupTime_.erase(endOfSetupTime_.begin()+idx);
    endOfSourceTime_.erase(endOfSourceTime_.begin()+idx);
    endOfSlewTime_.erase(endOfSlewTime_.begin()+idx);
    endOfIdleTime_.erase(endOfIdleTime_.begin()+idx);
    endOfTapeTime_.erase(endOfTapeTime_.begin()+idx);
    endOfCalibrationTime_.erase(endOfCalibrationTime_.begin()+idx);
    endOfScanTime_.erase(endOfScanTime_.begin()+idx);


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


