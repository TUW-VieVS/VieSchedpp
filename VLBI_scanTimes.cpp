//
// Created by mschartn on 14.07.17.
//

#include "VLBI_scanTimes.h"
namespace VieVS{


    VLBI_scanTimes::VLBI_scanTimes() {}

    VLBI_scanTimes::VLBI_scanTimes(int nsta) {
        endOfLastScan.resize(nsta);
        endOfSetupTime.resize(nsta);
        endOfSourceTime.resize(nsta);
        endOfSlewTime.resize(nsta);
        endOfTapeTime.resize(nsta);
        endOfIdleTime.resize(nsta);
        endOfCalibrationTime.resize(nsta);
        endOfScanTime.resize(nsta);
    }

    void
    VLBI_scanTimes::addTimes(int idx, unsigned int setup, unsigned int source, unsigned int slew, unsigned int tape,
                             unsigned int calib) {
        endOfSetupTime[idx] = endOfLastScan[idx] + setup;
        endOfSourceTime[idx] = endOfSetupTime[idx] + source;
        endOfSlewTime[idx] = endOfSourceTime[idx] + slew;
        endOfIdleTime[idx] = endOfSlewTime[idx];
        endOfTapeTime[idx] = endOfIdleTime[idx] + tape;
        endOfCalibrationTime[idx] = endOfTapeTime[idx] + calib;
    }

    void VLBI_scanTimes::removeElement(int idx) {
        endOfLastScan.erase(endOfLastScan.begin()+idx);
        endOfSetupTime.erase(endOfSetupTime.begin()+idx);
        endOfSourceTime.erase(endOfSourceTime.begin()+idx);
        endOfSlewTime.erase(endOfSlewTime.begin()+idx);
        endOfIdleTime.erase(endOfIdleTime.begin()+idx);
        endOfTapeTime.erase(endOfTapeTime.begin()+idx);
        endOfCalibrationTime.erase(endOfCalibrationTime.begin()+idx);
        endOfScanTime.erase(endOfScanTime.begin()+idx);

        unsigned int maxEndOfSlewTime;
        for (int i = 0; i < endOfSlewTime.size(); ++i) {

        }

        alignStartTimes();
    }

    void VLBI_scanTimes::updateSlewtime(int idx, unsigned int new_slewtime) {
        unsigned int currentSlewtime = endOfSlewTime[idx] - endOfSourceTime[idx];
        if (currentSlewtime != new_slewtime) {
            unsigned int tapeTime = endOfTapeTime[idx] - endOfIdleTime[idx];
            unsigned int calibrationTime = endOfCalibrationTime[idx] - endOfTapeTime[idx];
            unsigned int scanTime = endOfScanTime[idx] - endOfCalibrationTime[idx];

            int delta = new_slewtime - currentSlewtime;
            endOfSlewTime[idx] = endOfSlewTime[idx] + delta;
            endOfIdleTime[idx] = endOfSlewTime[idx] - delta;
            endOfTapeTime[idx] = endOfIdleTime[idx] + tapeTime;
            endOfCalibrationTime[idx] = endOfTapeTime[idx] + calibrationTime;
            endOfScanTime[idx] = endOfCalibrationTime[idx] + scanTime;
        }

    }

    void VLBI_scanTimes::alignStartTimes() {
        int nsta = endOfSlewTime.size();

        unsigned int latestSlewTime = 0;
        for (int i = 0; i < nsta; ++i) {
            if(endOfSlewTime[i]>latestSlewTime){
                latestSlewTime = endOfSlewTime[i];
            }
        }

        for (int i = 0; i < nsta; ++i) {
            unsigned int tapeTime = endOfTapeTime[i]-endOfIdleTime[i];
            unsigned int calibrationTime = endOfCalibrationTime[i]-endOfTapeTime[i];
            unsigned int scanTime = endOfScanTime[i]-endOfCalibrationTime[i];

            endOfIdleTime[i] = latestSlewTime;
            endOfTapeTime[i] =  endOfIdleTime[i] + tapeTime;
            endOfCalibrationTime[i] = endOfTapeTime[i] + calibrationTime;
            endOfScanTime[i] = endOfCalibrationTime[i] + scanTime;
        }
    }

    void VLBI_scanTimes::addScanTimes(const vector<unsigned int> &scanTimes) {
        for (int i = 0; i < endOfSlewTime.size(); ++i) {
            endOfScanTime[i] = endOfCalibrationTime[i]+scanTimes[i];
        }
    }

    unsigned int VLBI_scanTimes::maxTime() const {
        unsigned int max = 0;
        for (auto &thisTime: endOfScanTime) {
            if (thisTime > max) {
                max = thisTime;
            }
        }
        return max;
    }


}