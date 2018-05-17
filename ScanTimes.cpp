//
// Created by mschartn on 14.07.17.
//

#include "ScanTimes.h"
using namespace std;
using namespace VieVS;
int ScanTimes::nextId = 0;
ScanTimes::ScanTimes(unsigned int nsta): VieVS_Object(nextId++) {
    endOfLastScan_.resize(nsta);
    endOfFieldSystemTime_.resize(nsta);
    endOfSlewTime_.resize(nsta);
    endOfIdleTime_.resize(nsta);
    endOfPreobTime_.resize(nsta);
    endOfObservingTime_.resize(nsta);
}

void
ScanTimes::addTimes(int idx, unsigned int fieldSystem, unsigned int slew, unsigned int preob) noexcept {
    endOfFieldSystemTime_[idx] = endOfLastScan_[idx] + fieldSystem;
    endOfSlewTime_[idx] = endOfFieldSystemTime_[idx] + slew;
    endOfIdleTime_[idx] = endOfSlewTime_[idx];
    endOfPreobTime_[idx] = endOfIdleTime_[idx] + preob;
}

void ScanTimes::removeElement(int idx) noexcept {

    endOfLastScan_.erase(next(endOfLastScan_.begin(),idx));
    endOfFieldSystemTime_.erase(next(endOfFieldSystemTime_.begin(),idx));
    endOfSlewTime_.erase(next(endOfSlewTime_.begin(),idx));
    endOfIdleTime_.erase(next(endOfIdleTime_.begin(),idx));
    endOfPreobTime_.erase(next(endOfPreobTime_.begin(),idx));
    endOfObservingTime_.erase(next(endOfObservingTime_.begin(),idx));

    alignStartTimes(ScanTimes::AlignmentAnchor::start);
}

void ScanTimes::updateSlewtime(int idx, unsigned int new_slewtime) noexcept {
    unsigned int currentSlewtime = getSlewTime(idx);
    if (currentSlewtime != new_slewtime) {
        unsigned int preobTime = getPreobTime(idx);
        unsigned int observingTime = getObservingTime(idx);

        int delta = new_slewtime - currentSlewtime;
        endOfSlewTime_[idx] = endOfSlewTime_[idx] + delta;
        endOfIdleTime_[idx] = endOfSlewTime_[idx] - delta;
        endOfPreobTime_[idx] = endOfIdleTime_[idx] + preobTime;
        endOfObservingTime_[idx] = endOfPreobTime_[idx] + observingTime;
    }

}

void ScanTimes::alignStartTimes(AlignmentAnchor anchor) noexcept {
    auto nsta = static_cast<int>(endOfSlewTime_.size());

    switch(anchor){
        case AlignmentAnchor::start:{
            unsigned int latestSlewTime = *max_element(endOfSlewTime_.begin(),endOfSlewTime_.end());

            for (int i = 0; i < nsta; ++i) {
                unsigned int preob = getPreobTime(i);
                unsigned int obs = getObservingTime(i);
                endOfIdleTime_[i] = latestSlewTime;
                endOfPreobTime_[i] = endOfIdleTime_[i] + preob;
                endOfObservingTime_[i] = endOfPreobTime_[i] + obs;
            }
            break;
        }
        case AlignmentAnchor::end:{
            unsigned int latestObservingEnd = *max_element(endOfObservingTime_.begin(),endOfObservingTime_.end());

            for (int i = 0; i < nsta; ++i) {
                unsigned int preob = getPreobTime(i);
                unsigned int obs = getObservingTime(i);
                endOfObservingTime_[i] = latestObservingEnd;
                endOfPreobTime_[i] = latestObservingEnd - obs;
                endOfIdleTime_[i] = endOfPreobTime_[i] - preob;
            }
            break;

        }
        case AlignmentAnchor::individual:{
            vector<int> idxs = util::sortIndexes(endOfSlewTime_);
            unsigned int maxSlewEnd = *max_element(endOfSlewTime_.begin(),endOfSlewTime_.end());
            auto it = max_element(endOfObservingTime_.begin(),endOfObservingTime_.end());
            unsigned int maxObsEnd  = *it;
            unsigned int minObsStart = getObservingStart(distance(endOfObservingTime_.begin(),it));


            for(int i=0; i < nsta; ++i){
                int idx = idxs[i];
                unsigned int obs = getObservingTime(i);

                unsigned int thisObsTime = getObservingTime(idx);
                unsigned int thisPreobTime = getPreobTime(idx);
                if(endOfObservingTime_[idx]-minObsStart<=thisObsTime){
                    // anchor: end of observation is end of max observation time of all antennas
                    minObsStart = maxObsEnd-thisObsTime;
                    endOfIdleTime_[i] = minObsStart;
                    endOfPreobTime_[i] = minObsStart+thisPreobTime;
                    endOfObservingTime_[i] = maxObsEnd;

                }else{
                    unsigned int thisObsStart = endOfSlewTime_[idx];
                    if(thisObsStart<maxSlewEnd){
                        if(maxSlewEnd+thisObsTime>maxObsEnd){
                            // anchor: end of observation is end of max observation of all stations... maybe this is never true!
                            endOfIdleTime_[i] = maxObsEnd-thisObsTime;
                            endOfPreobTime_[i] = maxObsEnd-thisObsTime+thisPreobTime;
                            endOfObservingTime_[i] = maxObsEnd;

                        }else{
                            // anchor: start of observing time is maximum slew time of all antennas
                            endOfIdleTime_[i] = maxSlewEnd;
                            endOfPreobTime_[i] = maxSlewEnd+thisPreobTime;
                            endOfObservingTime_[i] = maxSlewEnd+thisObsTime;
                        }
                    }else{
                        // leave everything as it is (observation time is already correct)

//                        endOfIdleTime_[i] = getSlewEnd(idx);
//                        endOfPreobTime_[i] = getSlewEnd(idx)+thisPreobTime;
//                        endOfObservingTime_[i] = getObservingEnd(idx);
                    }

                }

            }



            break;
        }
    }
}

void ScanTimes::setStartTime(unsigned int scanStart) noexcept {
    for (unsigned int &i : endOfPreobTime_) {
        i = scanStart;
    }
}

void ScanTimes::addScanTimes(const vector<unsigned int> &scanTimes) noexcept {
    for (int i = 0; i < endOfSlewTime_.size(); ++i) {
        endOfObservingTime_[i] = endOfPreobTime_[i]+scanTimes[i];
    }
}

void ScanTimes::addScanTimes(unsigned int scanTimes) noexcept {
    for (int i = 0; i < endOfSlewTime_.size(); ++i) {
        endOfObservingTime_[i] = endOfPreobTime_[i]+scanTimes;
    }
}

void ScanTimes::addTagalongStation(const VieVS::PointingVector &pv_start, const VieVS::PointingVector &pv_end,
                                   unsigned int slewtime, unsigned int currentTime, unsigned int fieldSystem,
                                   unsigned int preob) {
    endOfLastScan_.push_back(currentTime);
    endOfFieldSystemTime_.push_back(currentTime+fieldSystem);
    endOfSlewTime_.push_back(currentTime+fieldSystem+slewtime);
    endOfIdleTime_.push_back(pv_start.getTime()-preob);
    endOfPreobTime_.push_back(pv_start.getTime());
    endOfObservingTime_.push_back(pv_end.getTime());
}

bool ScanTimes::substractPreobTimeFromStartTime(unsigned int preob) {
    bool valid = true;
    for(int i=0; i<endOfObservingTime_.size(); ++i){
        endOfIdleTime_[i] = endOfPreobTime_[i]-preob;
        if(endOfIdleTime_[i] < endOfSlewTime_[i]){
            valid = false;
        }
    }
    return valid;
}

const unsigned int ScanTimes::getObservingStart() const noexcept{
    unsigned int start = numeric_limits<unsigned int>::max();
    for(unsigned int x:endOfPreobTime_){
        if(x<start){
            start = x;
        }
    }
    return start;
}

const unsigned int ScanTimes::getObservingEnd() const noexcept{
    unsigned int end = 0;
    for(unsigned int x:endOfObservingTime_){
        if(x>end){
            end = x;
        }
    }
    return end;
}

const unsigned int ScanTimes::getObservingTime() const noexcept{
    return getObservingEnd()- getObservingStart();
}

const unsigned int ScanTimes::getScanStart() const noexcept {
    unsigned int start = numeric_limits<unsigned int>::max();
    for(unsigned int x:endOfLastScan_){
        if(x<start){
            start = x;
        }
    }
    return start;
}

const unsigned int ScanTimes::getScanEnd() const noexcept {
    // todo: postob
    return getObservingEnd();
}



