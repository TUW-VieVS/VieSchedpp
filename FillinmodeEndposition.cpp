//
// Created by matth on 12.08.2017.
//

#include "FillinmodeEndposition.h"
using namespace std;
using namespace VieVS;

int FillinmodeEndposition::nextId = 0;

FillinmodeEndposition::FillinmodeEndposition(const std::vector<Scan> &bestScans,
                                                   const std::vector<Station> &stations): VieVS_Object(nextId) {

    unsigned long nsta = stations.size();

    stationUnused_ = std::vector<char>(nsta, true);
    stationPossible_ = std::vector<char>(nsta, true);
    finalPosition_.reserve(nsta);
    for (int i = 0; i < nsta; ++i) {
        finalPosition_[i] = PointingVector(i, numeric_limits<int>::max());
    }

    vector<unsigned int> earliestScanStart(nsta, numeric_limits<unsigned int>::max());
    vector<unsigned int> availableTime(nsta, numeric_limits<unsigned int>::max());

    // if there is no subcon the earliest scan start is set to zero to be save
    unsigned int totalEarliestScanStart = numeric_limits<unsigned int>::max();
    if (bestScans.empty()) {
        totalEarliestScanStart = 0;
    }

    // first the earliest scan start of each station is searched and stored
    for (auto &any: bestScans) {
        for (int i = 0; i < any.getNSta(); ++i) {
            const PointingVector &pv = any.getPointingVector(i);
            int staid = pv.getStaid();

            unsigned int thisEndOfIdleTime = any.getTimes().getScanStart(i);
            if (thisEndOfIdleTime < earliestScanStart[staid]) {
                earliestScanStart[staid] = thisEndOfIdleTime;
                finalPosition_[staid] = pv;
                stationUnused_[staid] = false;
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
        if (!stationUnused_[staid]) {
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
        if (stationUnused_[staid]) {
            finalPosition_[staid].setTime(totalEarliestScanStart);
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
        const Station &thisSta = stations[staid];
        int assumedSlewTime = 5;
        const Station::WaitTimes wtimes = thisSta.getWaittimes();
        if (deltaT < wtimes.fieldSystem + wtimes.preob + assumedSlewTime + wtimes.postob + thisSta.getPARA().minScan ||
            !thisSta.getPARA().available || !thisSta.getPARA().availableForFillinmode) {
            stationPossible_[staid] = false;
        }
    }

    // checks if a station is not available
    for (int staid = 0; staid < nsta; ++staid) {
        if (stationUnused_[staid]) {
            const Station &thisSta = stations[staid];
            if (!thisSta.getPARA().available || !thisSta.getPARA().availableForFillinmode) {
                stationPossible_[staid] = false;
                availableTime[staid] = 0;
            }
        }
    }

    numberOfPossibleStations_ = static_cast<int>(count(stationPossible_.begin(), stationPossible_.end(), true));
}
