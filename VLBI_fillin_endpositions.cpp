//
// Created by matth on 12.08.2017.
//

#include "VLBI_fillin_endpositions.h"

namespace VieVS {


    VLBI_fillin_endpositions::VLBI_fillin_endpositions() {

    }

    VLBI_fillin_endpositions::VLBI_fillin_endpositions(const std::vector<VLBI_scan> &bestScans,
                                                       const std::vector<VLBI_station> &stations) {

        unsigned long nsta = stations.size();

        stationUnused = std::vector<char>(nsta, true);
        stationPossible = std::vector<char>(nsta, true);
        finalPosition = vector<VLBI_pointingVector>(nsta);
        for (int i = 0; i < nsta; ++i) {
            finalPosition[i] = VLBI_pointingVector(i, numeric_limits<int>::max());
        }

        vector<unsigned int> earliestScanStart(nsta, numeric_limits<unsigned int>::max());
        vector<unsigned int> availableTime(nsta, numeric_limits<unsigned int>::max());

        // if there is no subcon the earliest scan start is set to zero to be save
        unsigned int totalEarliestScanStart = numeric_limits<unsigned int>::max();
        if (bestScans.size() == 0) {
            totalEarliestScanStart = 0;
        }

        // first the earliest scan start of each station is searched and stored
        for (auto &any: bestScans) {
            for (int i = 0; i < any.getNSta(); ++i) {
                const VLBI_pointingVector &pv = any.getPointingVector(i);
                int staid = pv.getStaid();

                unsigned int thisEndOfIdleTime = any.getTimes().getEndOfCalibrationTime(i);
                if (thisEndOfIdleTime < earliestScanStart[staid]) {
                    earliestScanStart[staid] = thisEndOfIdleTime;
                    finalPosition[staid] = pv;
                    stationUnused[staid] = false;
                }
            }
        }

        // the total earliest scan start is searched... this is the upper limit for unused stations
        for (int staid = 0; staid < nsta; ++staid) {
            if (earliestScanStart[staid] < totalEarliestScanStart) {
                totalEarliestScanStart = earliestScanStart[staid];
            }
        }

        // used stations: available time is the time between the current position and the earliest scan time
        for (int staid = 0; staid < nsta; ++staid) {
            if (!stationUnused[staid]) {
                unsigned int staBeginning = stations[staid].getCurrentTime();
                unsigned int staEnd = earliestScanStart[staid];
                if (staBeginning > staEnd) {
                    availableTime[staid] = 0;
                } else {
                    availableTime[staid] = staEnd - staBeginning;
                }
            }
        }

        // unused stations: available time is the time between the current position and the total earliest scan time
        for (int staid = 0; staid < nsta; ++staid) {
            if (stationUnused[staid]) {
                finalPosition[staid].setTime(totalEarliestScanStart);
                unsigned int staBeginning = stations[staid].getCurrentTime();
                if (staBeginning > totalEarliestScanStart) {
                    availableTime[staid] = 0;
                } else {
                    availableTime[staid] = totalEarliestScanStart - staBeginning;
                }
            }
        }

        // checks if it is possible for a station to carry out a fillin scan
        for (int staid = 0; staid < nsta; ++staid) {
            unsigned int deltaT = availableTime[staid];
            const VLBI_station &thisSta = stations[staid];
            int assumedSlewTime = 5;
            if (deltaT < thisSta.getWaitSetup() + thisSta.getWaitSource() +
                         assumedSlewTime + thisSta.getWaitCalibration() +
                         thisSta.getWaitTape() + thisSta.getMinScanTime()) {
                stationPossible[staid] = false;
            }
        }

        // checks if a station is not available
        for (int staid = 0; staid < nsta; ++staid) {
            if (stationUnused[staid]) {
                const VLBI_station &thisSta = stations[staid];
                if (!thisSta.available()) {
                    stationPossible[staid] = false;
                    availableTime[staid] = 0;
                }
            }
        }

        numberOfPossibleStations = count(stationPossible.begin(), stationPossible.end(), true);
    }
}