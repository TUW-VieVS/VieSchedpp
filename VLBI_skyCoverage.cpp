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
    }

    VLBI_skyCoverage::~VLBI_skyCoverage() {
    }

    double VLBI_skyCoverage::calcScore(vector<VLBI_pointingVector> &pvs) {

        unsigned long n = pvs.size();

        vector<double> az_old_all;
        vector<double> el_old_all;
        vector<unsigned int> time_old_all;
        for (VLBI_pointingVector &pv: pv_end) {
            az_old_all.push_back(pv.getAz());
            el_old_all.push_back(pv.getEl());
            time_old_all.push_back(pv.getTime());
        }

        normal ndistTime(0, 1200);
        normal ndistDistance(0, 20);

        double preFactorTime = pdf(ndistTime, 0);
        double preFactorDistance = pdf(ndistDistance, 0);


        vector<double> min_score(n, 1);
        for (int idx_newObs = 0; idx_newObs < n; ++idx_newObs) {
            double newAzv = pvs[idx_newObs].getAz();
            double newElv = pvs[idx_newObs].getEl();
            unsigned int newTimev = pvs[idx_newObs].getTime();

            for (int idx_oldObs = 0; idx_oldObs < az_old_all.size(); ++idx_oldObs) {
                double thisTime = time_old_all[idx_oldObs];
                double deltaTime = (long) newTimev - (long) thisTime;
                if (deltaTime > 3600) {
                    continue;
                }

                double thisAz = az_old_all[idx_oldObs];
                double thisEl = el_old_all[idx_oldObs];

                double factorTime = 1 / preFactorTime * pdf(ndistTime, deltaTime);


                double tmp = sin(thisEl) * sin(newElv) + cos(thisEl) * cos(newElv) * cos(thisAz - newAzv);
                double distance = acos(tmp);
                if (tmp > 1) {
                    cout << "numerical problems \n";
                    distance = 0;
                }
                double thisScore = 1 - factorTime * (1 / preFactorDistance * pdf(ndistDistance, distance));

                if (thisScore < min_score[idx_newObs]) {
                    min_score[idx_newObs] = thisScore;
                }
            }
            az_old_all.push_back(newAzv);
            el_old_all.push_back(newElv);
            time_old_all.push_back(newTimev);
        }

        return accumulate(min_score.begin(), min_score.end(), 0.0);
    }

    void VLBI_skyCoverage::update(VLBI_pointingVector &start, VLBI_pointingVector &end) {
        pv_start.push_back(start);
        pv_end.push_back(end);
    }

}
