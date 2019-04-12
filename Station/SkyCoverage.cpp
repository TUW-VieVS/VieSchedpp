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


/* 
 * File:   skyCoverage.cpp
 * Author: mschartn
 * 
 * Created on June 29, 2017, 11:28 AM
 */

#include "SkyCoverage.h"
using namespace std;
using namespace VieVS;

unsigned long VieVS::SkyCoverage::nextId = 0;

double thread_local SkyCoverage::maxInfluenceTime = 3600;
double thread_local SkyCoverage::maxInfluenceDistance = 30*deg2rad;
SkyCoverage::Interpolation thread_local SkyCoverage::interpolationDistance = Interpolation::linear;
SkyCoverage::Interpolation thread_local SkyCoverage::interpolationTime = Interpolation::linear;

SkyCoverage::SkyCoverage(): VieVS_Object(nextId++){
}

double SkyCoverage::calcScore(const PointingVector &pv) const{

    double score = 1;
    for (const auto &pv_old : pointingVectors_) {
        if(pv_old.getTime()>pv.getTime()){
            continue;
        }
        double thisScore = scorePerPointingVector(pv, pv_old);
        if (thisScore < score) {
            score = thisScore;
        }
    }

    return score;
}


void SkyCoverage::update(const PointingVector &pv) noexcept {
    pointingVectors_.push_back(pv);
}

double
SkyCoverage::scorePerPointingVector(const PointingVector &pv_new,
                                         const PointingVector &pv_old) const noexcept {
    long deltaTime = (long) pv_new.getTime() - (long) pv_old.getTime();
    if (deltaTime > SkyCoverage::maxInfluenceTime) {
        return 1;
    }

    if (abs(pv_new.getEl() - pv_old.getEl()) > SkyCoverage::maxInfluenceDistance) {
        return 1;
    }

    float distance = LookupTable::angularDistance(pv_new, pv_old);

    if (distance > SkyCoverage::maxInfluenceDistance) {
        return 1;
    }

    double saturationDistance;
    switch (interpolationDistance) {
        case Interpolation::constant: {
            saturationDistance = 1;
            break;
        }
        case Interpolation::linear: {
            saturationDistance = 1 - distance / maxInfluenceDistance;
            break;
        }
        case Interpolation::cosine: {
            saturationDistance = .5 + .5 * (LookupTable::cosLookup(distance * pi / maxInfluenceDistance));
            break;
        }
    }

    double saturationTime;
    switch (interpolationTime) {
        case Interpolation::constant: {
            saturationTime = 1;
            break;
        }
        case Interpolation::linear: {
            saturationTime = 1 - deltaTime / maxInfluenceTime;
            break;
        }
        case Interpolation::cosine: {
            saturationTime = .5 + .5 * (LookupTable::cosLookup(deltaTime * pi / maxInfluenceTime));
            break;
        }
    }

    return 1 - (saturationDistance * saturationTime);
}

void SkyCoverage::clearObservations() {
    pointingVectors_.clear();
}

const double SkyCoverage::skyCoverageScore() const{

    double total_score = 0;

    auto thisPv = pointingVectors_.begin();
    unsigned int deltaTime = 3600;
    int c = 0;
    for(unsigned int startTime = 0; startTime < TimeSystem::duration; startTime+=deltaTime/2){
        unsigned int endTime = startTime += deltaTime;

        set<int> areas1;
        set<int> areas2;
        while( thisPv->getTime() < endTime && thisPv != pointingVectors_.end()){

            areas1.insert(areaIndex1(*thisPv));
            areas2.insert(areaIndex2(*thisPv));

            ++thisPv;
        }

        double score = static_cast<double>(areas1.size())/2.0 + static_cast<double>(areas2.size())/2.0; // average of both distributions
        total_score += score / 25; // normalize score
        ++c;
    }
    total_score /= c;

    return total_score;
}

int SkyCoverage::areaIndex1(const PointingVector &pv) noexcept {

    constexpr double el_space = halfpi / 3.;

    int row = floor(pv.getEl() / el_space);

    double az_space;
    switch(row){
        case 0: az_space = twopi / 13.; break;
        case 1: az_space = twopi / 9.; break;
        case 2: az_space = twopi / 3.; break;
        default: az_space = twopi / 3.; break;
    }

    int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));

    switch(row){
        case 0: if(col > 12) col = 0; break;
        case 1: if(col > 8) col = 0; break;
        case 2: if(col > 2) col = 0; break;
        default: if(col > 2) col = 0; break;
    }

    int offset;
    switch(row){
        case 0: offset = 12; break;
        case 1: offset = 3; break;
        case 2: offset = 0; break;
        default: offset = 0; break;
    }

    return offset + col;
}

int SkyCoverage::areaIndex2(const PointingVector &pv) noexcept {

    constexpr double el_space = halfpi / 3.5;

    int row = floor(pv.getEl() / el_space);

    double az_space;
    switch(row){
        case 0: az_space = twopi / 12.; break;
        case 1: az_space = twopi / 8.; break;
        case 2: az_space = twopi / 4.; break;
        case 3: az_space = twopi; break;
        default: az_space = twopi; break;
    }

    int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));

    switch(row){
        case 0: if(col > 11) col = 0; break;
        case 1: if(col > 7) col = 0; break;
        case 2: if(col > 3) col = 0; break;
        case 3: if(col > 0) col = 0; break;
        default: if(col > 0) col = 0; break;
    }

    int offset;
    switch(row){
        case 0: offset = 13; break;
        case 1: offset = 5; break;
        case 2: offset = 1; break;
        case 3: offset = 0; break;
        default: offset = 0; break;
    }

    return offset + col;
}



