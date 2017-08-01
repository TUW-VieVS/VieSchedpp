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

#include "VLBI_skyCoverage.h"

vector<int> VieVS::VLBI_skyCoverage::sta2sky = {};

namespace VieVS{
    VLBI_skyCoverage::VLBI_skyCoverage() {
    }

    VLBI_skyCoverage::VLBI_skyCoverage(vector<int> &staids) : nStations{staids.size()}, staids{staids} {
        maxDistTime = 3600;
        maxDistDistance = 60 * deg2rad;
    }

    VLBI_skyCoverage::~VLBI_skyCoverage() {
    }

    double VLBI_skyCoverage::calcScore(vector<VLBI_pointingVector> &pvs, vector<int> pvIds) {

        unsigned long n = pvIds.size();

        vector<double> min_score(n, 1);

        for (int idx_newObs = 0; idx_newObs < n; ++idx_newObs) {
            VLBI_pointingVector &pv_new = pvs[pvIds[idx_newObs]];

            for (int idx_oldObs = 0; idx_oldObs < pv_end.size(); ++idx_oldObs) {
                VLBI_pointingVector &pv_old = pv_end[idx_oldObs];
                double thisScore = scorePerPointingVector(pv_new, pv_old);
                if (thisScore < min_score[idx_newObs]) {
                    min_score[idx_newObs] = thisScore;
                }
            }
            for (int idx_oldObs = 0; idx_oldObs < idx_newObs; ++idx_oldObs) {
                VLBI_pointingVector &pv_old = pvs[pvIds[idx_oldObs]];
                double thisScore = scorePerPointingVector(pv_new, pv_old);
                if (thisScore < min_score[idx_newObs]) {
                    min_score[idx_newObs] = thisScore;
                }
            }
        }
        return accumulate(min_score.begin(), min_score.end(), 0.0);
    }

    void VLBI_skyCoverage::update(VLBI_pointingVector &start, VLBI_pointingVector &end) {
        pv_start.push_back(start);
        pv_end.push_back(end);
    }

    double VLBI_skyCoverage::scorePerPointingVector(VLBI_pointingVector &pv_new, VLBI_pointingVector &pv_old) {
        double deltaTime = (long) pv_new.getTime() - (long) pv_old.getTime();
        if (deltaTime > maxDistDistance) {
            return 1;
        }

        double tmp = sin(pv_old.getEl()) * sin(pv_new.getEl()) + cos(pv_old.getEl()) * cos(pv_new.getEl()) *
                                                                 cos(pv_new.getAz() - pv_old.getAz());
        double distance = acos(tmp);
        if (tmp > 1) {
            cout << "numerical problems \n";
            distance = 0;
        }

        if (distance > maxDistTime) {
            return 1;
        }
        double scoreDistance = .5 + .5 * (cos(distance * pi / maxDistDistance));
        double scoreTime = .5 + .5 * (cos(deltaTime * pi / maxDistTime));
        double thisScore = 1 - (scoreDistance * scoreTime);

        return thisScore;
    }

}
