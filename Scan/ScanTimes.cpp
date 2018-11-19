/* 
 *  VieSched++ Very Long Baseline Interferometry (VLBI) Scheduling Software
 *  Copyright (C) 2018  Matthias Schartner
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScanTimes.h"
using namespace std;
using namespace VieVS;
unsigned long ScanTimes::nextId = 0;
ScanTimes::AlignmentAnchor ScanTimes::anchor = ScanTimes::AlignmentAnchor::start;

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
    endOfObservingTime_[idx] = endOfIdleTime_[idx] + preob;
}

void ScanTimes::removeElement(int idx) noexcept {

    endOfLastScan_.erase(next(endOfLastScan_.begin(),idx));
    endOfFieldSystemTime_.erase(next(endOfFieldSystemTime_.begin(),idx));
    endOfSlewTime_.erase(next(endOfSlewTime_.begin(),idx));
    endOfIdleTime_.erase(next(endOfIdleTime_.begin(),idx));
    endOfPreobTime_.erase(next(endOfPreobTime_.begin(),idx));
    endOfObservingTime_.erase(next(endOfObservingTime_.begin(),idx));

    alignStartTimes();
}

void ScanTimes::setSlewTime(int idx, unsigned int new_slewtime) noexcept {
    unsigned int currentSlewtime = getSlewDuration(idx);
    if (currentSlewtime != new_slewtime) {
        unsigned int preobTime = getPreobDuration(idx);
        unsigned int observingTime = getObservingDuration(idx);

        endOfSlewTime_[idx] = endOfFieldSystemTime_[idx] + new_slewtime;
        endOfIdleTime_[idx] = endOfSlewTime_[idx];
        endOfPreobTime_[idx] = endOfIdleTime_[idx] + preobTime;
        endOfObservingTime_[idx] = endOfPreobTime_[idx] + observingTime;
    }
}

void ScanTimes::removeIdleTime() {
    auto nsta = static_cast<int>(endOfSlewTime_.size());

    for(int idx=0; idx<nsta; ++idx){
        unsigned int preob = getPreobDuration(idx);
        unsigned int obs = getObservingDuration(idx);

        endOfIdleTime_[idx] = endOfSlewTime_[idx];
        endOfPreobTime_[idx] = endOfIdleTime_[idx] + preob;
        endOfObservingTime_[idx] = endOfPreobTime_[idx] + obs;
    }
}

void ScanTimes::alignStartTimes() noexcept {
    auto nsta = static_cast<int>(endOfSlewTime_.size());
    removeIdleTime();

    switch(anchor){
        case AlignmentAnchor::start:{
            unsigned int latestObservingStart = *max_element(endOfPreobTime_.begin(),endOfPreobTime_.end());

            for (int i = 0; i < nsta; ++i) {
                unsigned int preob = getPreobDuration(i);
                unsigned int obs = getObservingDuration(i);
                endOfIdleTime_[i] = latestObservingStart-preob;
                endOfPreobTime_[i] = latestObservingStart;
                endOfObservingTime_[i] = endOfPreobTime_[i] + obs;
            }
            break;
        }
        case AlignmentAnchor::end:{
            unsigned int latestObservingEnd = *max_element(endOfObservingTime_.begin(),endOfObservingTime_.end());

            for (int i = 0; i < nsta; ++i) {
                unsigned int preob = getPreobDuration(i);
                unsigned int obs = getObservingDuration(i);
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
            unsigned int minObsStart = getObservingTime(distance(endOfObservingTime_.begin(), it), Timestamp::start);


            for(int i=0; i < nsta; ++i){
                int idx = idxs[i];
//                unsigned int obs = getObservingDuration(i);

                unsigned int thisObsTime = getObservingDuration(idx);
                unsigned int thisPreobTime = getPreobDuration(idx);
                if(maxObsEnd-minObsStart<=thisObsTime){
                    // anchor: end of observation is end of max observation time of all antennas
                    minObsStart = maxObsEnd-thisObsTime;
                    endOfIdleTime_[idx] = minObsStart-thisPreobTime;
                    endOfPreobTime_[idx] = minObsStart;
                    endOfObservingTime_[idx] = maxObsEnd;

                }else{
                    unsigned int thisObsStart = endOfSlewTime_[idx];
                    if(thisObsStart < maxSlewEnd){
                        if(maxSlewEnd + thisObsTime > maxObsEnd){
                            // anchor: end of observation is end of max observation of all stations... maybe this is never true!
                            endOfIdleTime_[idx] = maxObsEnd-thisObsTime-thisPreobTime;
                            endOfPreobTime_[idx] = maxObsEnd-thisObsTime;
                            endOfObservingTime_[idx] = maxObsEnd;

                        }else{
                            // anchor: start of observing time is maximum slew time of all antennas
                            endOfIdleTime_[idx] = maxSlewEnd;
                            endOfPreobTime_[idx] = maxSlewEnd+thisPreobTime;
                            endOfObservingTime_[idx] = maxSlewEnd+thisObsTime;
                        }
                    }else{
                        // leave everything as it is (observation time is already correct)

                        endOfIdleTime_[idx] = getSlewTime(idx, Timestamp::end);
                        endOfPreobTime_[idx] = endOfIdleTime_[idx]+thisPreobTime;
                        endOfObservingTime_[idx] = getObservingTime(idx, Timestamp::end);
                    }
                }
            }
            break;
        }
    }
}

void ScanTimes::setObservingStarts(unsigned int scanStart) noexcept {
    for (unsigned int &i : endOfPreobTime_) {
        i = scanStart;
    }
}

void ScanTimes::setObservingTimes(const vector<unsigned int> &scanTimes) noexcept {
    for (int i = 0; i < endOfSlewTime_.size(); ++i) {
        endOfObservingTime_[i] = endOfPreobTime_[i]+scanTimes[i];
    }
    alignStartTimes();
}

void ScanTimes::setObservingTimes(unsigned int scanTimes) noexcept {
    for (int i = 0; i < endOfSlewTime_.size(); ++i) {
        endOfObservingTime_[i] = endOfPreobTime_[i]+scanTimes;
    }
    alignStartTimes();
}

void ScanTimes::addTagalongStationTime(const VieVS::PointingVector &pv_start, const VieVS::PointingVector &pv_end,
                                       unsigned int slewtime, unsigned int currentTime, unsigned int fieldSystem,
                                       unsigned int preob) {
    endOfLastScan_.push_back(currentTime);
    endOfFieldSystemTime_.push_back(currentTime+fieldSystem);
    endOfSlewTime_.push_back(currentTime+fieldSystem+slewtime);
    endOfIdleTime_.push_back(pv_start.getTime()-preob);
    endOfPreobTime_.push_back(pv_start.getTime());
    endOfObservingTime_.push_back(pv_end.getTime());
}

bool ScanTimes::setPreobTime(const vector<unsigned int> &preob) {
    bool valid = true;
    for(int i=0; i<endOfObservingTime_.size(); ++i){
        endOfIdleTime_[i] = endOfPreobTime_[i]-preob[i];

        if(endOfIdleTime_[i] < endOfSlewTime_[i]){
            valid = false;
            endOfSlewTime_[i] = endOfIdleTime_[i];

            if(endOfSlewTime_[i] < endOfFieldSystemTime_[i]){
                endOfFieldSystemTime_[i] = endOfSlewTime_[i];

                if(endOfFieldSystemTime_[i] < endOfLastScan_[i]){
                    endOfLastScan_[i] = endOfFieldSystemTime_[i];
                }
            }
        }
    }
    return valid;
}

bool ScanTimes::setPreobTime(int i, unsigned int preob) {
    bool valid = true;
    endOfIdleTime_[i] = endOfPreobTime_[i]-preob;

    if(endOfIdleTime_[i] < endOfSlewTime_[i]){
        valid = false;
        endOfSlewTime_[i] = endOfIdleTime_[i];

        if(endOfSlewTime_[i] < endOfFieldSystemTime_[i]){
            endOfFieldSystemTime_[i] = endOfSlewTime_[i];

            if(endOfFieldSystemTime_[i] < endOfLastScan_[i]){
                endOfLastScan_[i] = endOfFieldSystemTime_[i];
            }
        }
    }
    return valid;
}


void ScanTimes::setObservingTime(int idx, unsigned int time, Timestamp ts) {
    switch (ts){
        case Timestamp::start: {
            unsigned int preobTime = getPreobDuration(idx);
            endOfPreobTime_[idx] = time;
            endOfIdleTime_[idx] = time-preobTime;
            break;
        }
        case Timestamp::end:{
            endOfObservingTime_[idx] = time;
            break;
        }
    }
}

int ScanTimes::removeUnnecessaryObservingTime(Timestamp ts) {

    switch (ts){

        case Timestamp::start:{
            auto minElement = min_element(endOfPreobTime_.begin(),endOfPreobTime_.end());
            auto idx = static_cast<int>(distance(endOfPreobTime_.begin(), minElement));
            unsigned int preob = getPreobDuration(idx);

            endOfPreobTime_[idx] = numeric_limits<unsigned int>::max();
            unsigned int secondMax = *min_element(endOfPreobTime_.begin(),endOfPreobTime_.end());
            endOfPreobTime_[idx] = secondMax;
            endOfIdleTime_[idx] = endOfPreobTime_[idx]-preob;

            return idx;
        }
        case Timestamp::end:{

            auto maxElement = max_element(endOfObservingTime_.begin(),endOfObservingTime_.end());
            auto idx = static_cast<int>(distance(endOfObservingTime_.begin(), maxElement));
            endOfObservingTime_[idx] = 0;
            unsigned int secondMax = *max_element(endOfObservingTime_.begin(),endOfObservingTime_.end());
            endOfObservingTime_[idx] = secondMax;

            return idx;
        }
    }
}

bool ScanTimes::reduceObservingTime(int idx, unsigned int time, Timestamp ts) {

    switch (ts){

        case Timestamp::start:{
            bool reduced = false;
            if(endOfObservingTime_[idx] > time){
                endOfObservingTime_[idx] = time;
                reduced = true;
            }
            return reduced;
        }

        case Timestamp::end: {
            unsigned int preob = getPreobDuration(idx);
            bool reduced = false;
            if (endOfPreobTime_[idx] < time) {
                endOfPreobTime_[idx] = time;
                endOfIdleTime_[idx] = time - preob;

                reduced = true;
            }
            return reduced;
        }

    }
}




