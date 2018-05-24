//
// Created by matth on 12.08.2017.
//

#include "StationEndposition.h"
using namespace std;
using namespace VieVS;

int StationEndposition::nextId = 0;

StationEndposition::StationEndposition(int nsta) : VieVS_Object(nsta) {
    stationAvailable_ = vector<char>(static_cast<unsigned long>(nsta), false);
    stationPossible_ = std::vector<char>(static_cast<unsigned long>(nsta), false);
    finalPosition_ = vector< boost::optional<PointingVector> >(static_cast<unsigned long>(nsta));

    // if there is no subcon the earliest scan start is set to zero to be save
    earliestScanStart_ = numeric_limits<unsigned int>::max();
}

void StationEndposition::addPointingVectorAsEndposition(const PointingVector &pv) {
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

void StationEndposition::checkStationPossibility(const Station &thisStation) {
    int staid = thisStation.getId();

    if(!thisStation.getPARA().available){
        stationPossible_[staid] = false;
        return;
    }

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


unsigned int StationEndposition::requiredEndpositionTime(int staid) const {

    // check if station has a required endposition, otherwise use earliest scan start.
    if(finalPosition_[staid].is_initialized()){
        return finalPosition_[staid]->getTime();
    }else{
        return earliestScanStart_;
    }
}

bool StationEndposition::checkStationPossibility(const std::vector<Station> &stations) {

    for(const auto &any:stations){
        checkStationPossibility(any);
    }
    return count(stationPossible_.begin(),stationPossible_.end(), true) >= 2;
}

std::set<int> StationEndposition::getObservedSources() const noexcept {
    set<int> obsSrc;

    for (auto pv : finalPosition_) {
        if(pv.is_initialized()){
            obsSrc.insert(pv->getSrcid());
        }
    }

    return std::move(obsSrc);
}

void StationEndposition::setStationAvailable(const std::vector<Station> &stations) {

    for(int i=0; i<stations.size(); ++i){
        stationAvailable_[i] = stations[i].getPARA().available;
    }
}


