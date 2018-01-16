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

vector<vector<vector<float> > > VieVS::SkyCoverage::angularDistanceLookup = {};
double SkyCoverage::maxInfluenceTime = 3600;
double SkyCoverage::maxInfluenceDistance = 30*deg2rad;
double SkyCoverage::maxTwinTelecopeDistance = 0;

SkyCoverage::SkyCoverage() = default;

SkyCoverage::SkyCoverage(const vector<int> &staids, int id)
        : nStations_{staids.size()}, staids_{staids}, id_{id} {
}

double SkyCoverage::calcScore(const vector<PointingVector> &pvs,
                                   const vector<Station> &stations) const noexcept {

    double score = 0;

    for (const auto &pv_new : pvs) {
        if (stations[pv_new.getStaid()].getSkyCoverageID() != id_) {
            continue;
        }

        double min_score = 1;

        for (const auto &pv_old : pointingVectorsEnd_) {
            double thisScore = scorePerPointingVector(pv_new, pv_old);
            if (thisScore < min_score) {
                min_score = thisScore;
            }
        }
        score = min_score;
    }
    return score;
}


double SkyCoverage::calcScore(const vector<PointingVector> &pvs, const vector<Station> &stations,
                                   vector<double> &firstScorePerPv) const noexcept {

    double score = 0;

    for (int idx_newObs = 0; idx_newObs < pvs.size(); ++idx_newObs) {
        const PointingVector &pv_new = pvs[idx_newObs];
        if (stations[pv_new.getStaid()].getSkyCoverageID() != id_) {
            continue;
        }

        double min_score = 1;

        for (const auto &pv_old : pointingVectorsEnd_) {
            double thisScore = scorePerPointingVector(pv_new, pv_old);
            if (thisScore < min_score) {
                min_score = thisScore;
            }
        }

        firstScorePerPv[idx_newObs] = min_score;
        score = min_score;
    }
    return score;
}

double SkyCoverage::calcScore_subcon(const vector<PointingVector> &pvs,
                                          const vector<Station> &stations,
                                          const vector<double> &firstScorePerPv) const noexcept {

    double score = 0;

    for (int idx_newObs = 0; idx_newObs < pvs.size(); ++idx_newObs) {
        const PointingVector &pv_new = pvs[idx_newObs];
        if (stations[pv_new.getStaid()].getSkyCoverageID() != id_) {
            continue;
        }
        score = firstScorePerPv[idx_newObs];
        break;
    }
    return score;
}

void SkyCoverage::update(const PointingVector &start, const PointingVector &end) noexcept {
    pointingVectorsStart_.push_back(start);
    pointingVectorsEnd_.push_back(end);
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

    double pv_1_el = pv_old.getEl();
    double pv_2_el = pv_new.getEl();

    double pv_1_az = pv_old.getAz();
    double pv_2_az = pv_new.getAz();
    if(pv_2_az>pv_1_az){
        swap(pv_1_az,pv_2_az);
    }
    double delta_az = (pv_1_az - pv_2_az)*rad2deg;
    while (delta_az>180){
        delta_az = delta_az-360;
    }


    int pv_delta_az = abs(static_cast<int>(delta_az + .5)); // +.5 for rounding
    if(pv_1_el>pv_2_el){
        swap(pv_1_el,pv_2_el);
    }
    int thisEl = static_cast<int>(pv_1_el * rad2deg + .5); // +.5 for rounding
    int pv_delta_el = static_cast<int>((pv_2_el - pv_1_el) * rad2deg + .5); // +.5 for rounding
    float distance = SkyCoverage::angularDistanceLookup[thisEl][pv_delta_az][pv_delta_el];

    if (distance > SkyCoverage::maxInfluenceDistance) {
        return 1;
    }

//        double scoreDistance = .5 + .5 * cos(distance * pi / maxDistDistance);
//        double scoreTime = .5 + .5 * cos(deltaTime * pi / maxDistTime );
    double scoreDistance =
            .5 + .5 * (LookupTable::cosLookup[static_cast<int>(distance * pi / SkyCoverage::maxInfluenceDistance * 1000)]);
    double scoreTime = .5 + .5 * (LookupTable::cosLookup[static_cast<int>(deltaTime * pi / SkyCoverage::maxInfluenceTime * 1000)]);
    double thisScore = 1 - (scoreDistance * scoreTime);

    return thisScore;
}

void SkyCoverage::clearObservations() {
    pointingVectorsStart_.clear();
    pointingVectorsEnd_.clear();
}


