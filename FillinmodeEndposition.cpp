//
// Created by matth on 12.08.2017.
//

#include "FillinmodeEndposition.h"
using namespace std;
using namespace VieVS;

int FillinmodeEndposition::nextId = 0;

//FillinmodeEndposition::FillinmodeEndposition(const std::vector<Scan> &bestScans,
//                                                   const std::vector<Station> &stations): VieVS_Object(nextId) {
//
//    unsigned long nsta = stations.size();
//
//    stationAvailable_ = vector<char>(nsta, true);
//    stationPossible_ = std::vector<char>(nsta, true);
//    finalPosition_ = vector<boost::optional<PointingVector> >(nsta);
//
//    // if there is no subcon the earliest scan start is set to zero to be save
//    earliestScanStart_ = numeric_limits<unsigned int>::max();
//    if (bestScans.empty()) {
//        earliestScanStart_ = 0;
//    }
//
//    // first the earliest pointing vector of each station is searched and stored
//    for (auto &any: bestScans) {
//        const auto &times = any.getTimes();
//        for (int i = 0; i < any.getNSta(); ++i) {
//
//            const PointingVector &pv = any.getPointingVector(i);
//            int staid = pv.getStaid();
//
//            unsigned int thisScanStart = times.getScanStart(i);
//            if(finalPosition_[staid].is_initialized()){
//                if (thisScanStart < finalPosition_[staid]->getTime()) {
//                    finalPosition_[staid] = pv;
//                }
//            }else{
//                finalPosition_[staid] = pv;
//            }
//
//            if(thisScanStart<earliestScanStart_){
//                earliestScanStart_ = thisScanStart;
//            }
//        }
//    }
//
//    // loop through all stations
//    for (int staid = 0; staid < nsta; ++staid) {
//
//        const Station &thisStation = stations[staid];
//
//        // check if station is available
//        if(!thisStation.getPARA().available){
//            stationAvailable_[staid] = false;
//            stationPossible_[staid] = false;
//            continue;
//        }
//
//        // get start time of this station
//        unsigned int staStarttime = thisStation.getCurrentTime();
//
//        // estimat end time of this session, if there is a endposition then take this time, otherwise earliest scan start time
//        unsigned int staEndtime = 0;
//        if (finalPosition_[staid].is_initialized()) {
//            staEndtime = finalPosition_[staid]->getTime();
//        }else{
//            staEndtime = earliestScanStart_;
//        }
//
//        // calculate available time
//        unsigned int availableTime;
//        if (staEndtime > staStarttime) {
//            availableTime = staEndtime - staStarttime;
//        }else{
//            stationPossible_[staid] = false;
//            continue;
//        }
//
//        // calculate minimum required time. Assumtion: only 5 sec slew time, min scan time and no idle time.
//        const Station::WaitTimes wtimes = thisStation.getWaittimes();
//        unsigned int requiredTime = wtimes.fieldSystem + wtimes.preob + 5 + wtimes.postob + thisStation.getPARA().minScan;
//
//        // determine if station is possible
//        stationPossible_[staid] = availableTime > requiredTime;
//    }
//}

FillinmodeEndposition::FillinmodeEndposition(int nsta) : VieVS_Object(nsta) {
    stationAvailable_ = vector<char>(static_cast<unsigned long>(nsta), true);
    stationPossible_ = std::vector<char>(static_cast<unsigned long>(nsta), true);
    finalPosition_ = vector<boost::optional<PointingVector> >(static_cast<unsigned long>(nsta));

    // if there is no subcon the earliest scan start is set to zero to be save
    earliestScanStart_ = numeric_limits<unsigned int>::max();
}

void FillinmodeEndposition::addPointingVectorAsEndposition(const PointingVector &pv) {
    int staid = pv.getStaid();

    // check if there is already an earlier endposition
    if(finalPosition_[staid].is_initialized()){
        if(pv.getTime() < finalPosition_[staid]->getTime()){
            finalPosition_[staid] = pv;
        }
    }else{
        finalPosition_[staid] = pv;
    }

    if(pv.getTime()<earliestScanStart_){
        earliestScanStart_ = pv.getTime();
    }

}

void FillinmodeEndposition::checkStationPossibility(const Station &thisStation) {
    int staid = thisStation.getId();
    // get start time of this station
    unsigned int staStarttime = thisStation.getCurrentTime();

    // estimat end time of this session, if there is a endposition then take this time, otherwise earliest scan start time
    unsigned int staEndtime = 0;
    if (finalPosition_[staid].is_initialized()) {
        staEndtime = finalPosition_[staid]->getTime();
    }else{
        staEndtime = earliestScanStart_;
    }

    // calculate available time
    unsigned int availableTime;
    if (staEndtime > staStarttime) {
        availableTime = staEndtime - staStarttime;
    }else{
        stationPossible_[staid] = false;
        return;
    }

    // calculate minimum required time. Assumtion: only 5 sec slew time, min scan time and no idle time.
    const Station::WaitTimes wtimes = thisStation.getWaittimes();
    unsigned int requiredTime = wtimes.fieldSystem + wtimes.preob + 5 + wtimes.postob + thisStation.getPARA().minScan;

    // determine if station is possible
    stationPossible_[staid] = availableTime > requiredTime;
}


unsigned int FillinmodeEndposition::requiredEndpositionTime(int staid) const {

    // check if station has a required endposition, otherwise use earliest scan start.
    if(finalPosition_[staid].is_initialized()){
        return finalPosition_[staid]->getTime();
    }else{
        return earliestScanStart_;
    }
}

void FillinmodeEndposition::checkStationPossibility(const std::vector<Station> &stations) {
    for(const auto &any:stations){
        checkStationPossibility(any);
    }
}


