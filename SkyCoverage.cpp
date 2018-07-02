/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
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

    double scoreDistance;
    switch (interpolationDistance) {
        case Interpolation::constant: {
            scoreDistance = 0;
            break;
        }
        case Interpolation::linear: {
            scoreDistance = distance / maxInfluenceDistance;
            break;
        }
        case Interpolation::cosine: {
            scoreDistance = .5 + .5 * (LookupTable::cosLookup(distance * pi / maxInfluenceDistance));
            break;
        }
    }

    double scoreTime;
    switch (interpolationTime) {
        case Interpolation::constant: {
            scoreTime = 0;
            break;
        }
        case Interpolation::linear: {
            scoreTime = deltaTime / maxInfluenceTime;
            break;
        }
        case Interpolation::cosine: {
            scoreTime = .5 + .5 * (LookupTable::cosLookup(deltaTime * pi / maxInfluenceTime));
            break;
        }
    }

    return 1 - (scoreDistance * scoreTime);
}

void SkyCoverage::clearObservations() {
    pointingVectors_.clear();
}



