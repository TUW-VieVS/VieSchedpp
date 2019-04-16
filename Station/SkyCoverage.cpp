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

void SkyCoverage::calculateSkyCoverageScores() {

    std::sort(pointingVectors_.begin(), pointingVectors_.end(), [](const PointingVector &left,const PointingVector &right) {
        return left.getTime() < right.getTime();
    });

    a13m30_ = skyCoverageScore_13(1800);
    a25m30_ = skyCoverageScore_25(1800);
    a37m30_ = skyCoverageScore_37(1800);
    a13m60_ = skyCoverageScore_13(3600);
    a25m60_ = skyCoverageScore_25(3600);
    a37m60_ = skyCoverageScore_37(3600);

}


double SkyCoverage::skyCoverageScore_13(unsigned int deltaTime) const {
    double total_score = 0;

    auto thisPv = pointingVectors_.begin();
    int c = 0;
    for(unsigned int startTime = 0; startTime < TimeSystem::duration; startTime+=deltaTime/2){
        unsigned int endTime = startTime += deltaTime;

        set<int> areas1;
        set<int> areas2;
        while( thisPv->getTime() < endTime && thisPv != pointingVectors_.end()){

            areas1.insert(areaIndex13_v1(*thisPv));
            areas2.insert(areaIndex13_v2(*thisPv));

            ++thisPv;
        }

        double score = static_cast<double>(areas1.size())/2.0 + static_cast<double>(areas2.size())/2.0; // average of both distributions
        total_score += score / 13.0; // normalize score
        ++c;
    }
    total_score /= c;

    return total_score;
}

double SkyCoverage::skyCoverageScore_25(unsigned int deltaTime) const{

    double total_score = 0;

    auto thisPv = pointingVectors_.begin();
    int c = 0;
    for(unsigned int startTime = 0; startTime < TimeSystem::duration; startTime+=deltaTime/2){
        unsigned int endTime = startTime += deltaTime;

        set<int> areas1;
        set<int> areas2;
        while( thisPv->getTime() < endTime && thisPv != pointingVectors_.end()){

            areas1.insert(areaIndex25_v1(*thisPv));
            areas2.insert(areaIndex25_v2(*thisPv));

            ++thisPv;
        }

        double score = static_cast<double>(areas1.size())/2.0 + static_cast<double>(areas2.size())/2.0; // average of both distributions
        total_score += score / 25.0; // normalize score
        ++c;
    }
    total_score /= c;

    return total_score;
}

double SkyCoverage::skyCoverageScore_37(unsigned int deltaTime) const {
    double total_score = 0;

    auto thisPv = pointingVectors_.begin();
    int c = 0;
    for(unsigned int startTime = 0; startTime < TimeSystem::duration; startTime+=deltaTime/2){
        unsigned int endTime = startTime += deltaTime;

        set<int> areas1;
        set<int> areas2;
        while( thisPv->getTime() < endTime && thisPv != pointingVectors_.end()){

            areas1.insert(areaIndex37_v1(*thisPv));
            areas2.insert(areaIndex37_v2(*thisPv));

            ++thisPv;
        }

        double score = static_cast<double>(areas1.size())/2.0 + static_cast<double>(areas2.size())/2.0; // average of both distributions
        total_score += score / 37.0; // normalize score
        ++c;
    }
    total_score /= c;

    return total_score;
}

int SkyCoverage::areaIndex13_v1(const PointingVector &pv) noexcept {

    constexpr double el_space = halfpi / 2.;

    int row = static_cast<int>(floorl(pv.getEl() / el_space));
    int idx;
    switch (row){
        case 0:{
            double n = 9;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx = col;
            break;
        }

        default:{
            double n = 4;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx =  9 + col;
            break;
        }
    }

    return idx;
}

int SkyCoverage::areaIndex13_v2(const PointingVector &pv) noexcept {

    constexpr double el_space = halfpi / 2.75;

    int row = static_cast<int>(floorl(pv.getEl() / el_space));
    int idx;
    switch (row){
        case 0:{
            double n = 8;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx = col;
            break;
        }

        case 1:{
            double n = 4;
            double az_space = twopi / n;
            int col = static_cast<int>(floorl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx = 8 + col;
            break;
        }

        default:{
            idx =  12;
            break;
        }
    }

    return idx;
}

int SkyCoverage::areaIndex25_v1(const PointingVector &pv) noexcept {

    constexpr double el_space = halfpi / 3.;

    int row = static_cast<int>(floorl(pv.getEl() / el_space));

    int idx;
    switch (row){
        case 0:{
            double n = 13;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx = col;
            break;
        }

        case 1:{
            double n = 9;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx = 13 + col;
            break;
        }

        default:{
            double n = 3;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx =  22 + col;
            break;
        }
    }

    return idx;
}

int SkyCoverage::areaIndex25_v2(const PointingVector &pv) noexcept {

    constexpr double el_space = halfpi / 3.75;

    int row = static_cast<int>(floorl(pv.getEl() / el_space));
    int idx;
    switch (row){
        case 0:{
            double n = 12;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx = col;
            break;
        }

        case 1:{
            double n = 8;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx = 12 + col;
            break;
        }

        case 2:{
            double n = 4;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx = 20 + col;
            break;
        }

        default:{
            idx =  24;
            break;
        }
    }

    return idx;
}

int SkyCoverage::areaIndex37_v1(const PointingVector &pv) noexcept {

    constexpr double el_space = halfpi / 4.;

    int row = static_cast<int>(floorl(pv.getEl() / el_space));

    int idx;
    switch (row){
        case 0:{
            double n = 14;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx = col;
            break;
        }

        case 1:{
            double n = 12;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx = 14 + col;
            break;
        }

        case 2:{
            double n = 8;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx = 26 + col;
            break;
        }

        default:{
            double n = 3;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx =  34 + col;
            break;
        }
    }

    return idx;
}

int SkyCoverage::areaIndex37_v2(const PointingVector &pv) noexcept {

    constexpr double el_space = halfpi / 4.75;

    int row = static_cast<int>(floorl(pv.getEl() / el_space));
    int idx;
    switch (row){
        case 0:{
            double n = 13;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx = col;
            break;
        }

        case 1:{
            double n = 12;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx = 13 + col;
            break;
        }

        case 2:{
            double n = 7;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx = 25 + col;
            break;
        }

        case 3:{
            double n = 4;
            double az_space = twopi / n;
            int col = static_cast<int>(roundl(util::wrap2twoPi(pv.getAz()) / az_space));
            if(col > n-1) {
                col = 0;
            }
            idx = 32 + col;
            break;
        }

        default:{
            idx =  36;
            break;
        }
    }

    return idx;
}





