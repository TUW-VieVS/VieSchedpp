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



