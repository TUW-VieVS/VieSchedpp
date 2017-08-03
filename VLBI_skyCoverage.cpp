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

    VLBI_skyCoverage::VLBI_skyCoverage(vector<int> &staids, double skyCoverageDistance, double skyCoverageInterval)
            : nStations{staids.size()}, staids{staids}, maxDistTime{skyCoverageInterval},
              maxDistDistance{skyCoverageDistance} {
    }

    VLBI_skyCoverage::~VLBI_skyCoverage() {
    }

    double VLBI_skyCoverage::calcScore(vector<VLBI_pointingVector> &pvs) {

        double score = 0;
        vector<bool> isSky(pvs.size());

        for (int i = 0; i < pvs.size(); ++i) {
            VLBI_pointingVector &thisPV = pvs[i];
            int staid = thisPV.getStaid();
            isSky[i] = find(staids.begin(), staids.end(), staid) != staids.end();
        }


        for (int idx_newObs = 0; idx_newObs < pvs.size(); ++idx_newObs) {
            if (!isSky[idx_newObs]) {
                continue;
            }
            VLBI_pointingVector &pv_new = pvs[idx_newObs];

            double min_score = 1;

            for (int idx_oldObs = 0; idx_oldObs < pv_end.size(); ++idx_oldObs) {
                VLBI_pointingVector &pv_old = pv_end[idx_oldObs];

                double thisScore = scorePerPointingVector(pv_new, pv_old);
                if (thisScore < min_score) {
                    min_score = thisScore;
                }
            }
            for (int idx_oldObs = 0; idx_oldObs < idx_newObs; ++idx_oldObs) {
                if (!isSky[idx_oldObs]) {
                    continue;
                }
                VLBI_pointingVector &pv_old = pvs[idx_newObs];
                double thisScore = scorePerPointingVector(pv_new, pv_old);
                if (thisScore < min_score) {
                    min_score = thisScore;
                }
            }
            score += min_score;
        }
        return score;
    }

    void VLBI_skyCoverage::update(VLBI_pointingVector &start, VLBI_pointingVector &end) {
        pv_start.push_back(start);
        pv_end.push_back(end);
    }

    double
    VLBI_skyCoverage::scorePerPointingVector(VLBI_pointingVector &pv_new, VLBI_pointingVector &pv_old) {
        long deltaTime = (long) pv_new.getTime() - (long) pv_old.getTime();
        if (deltaTime > maxDistTime) {
            return 1;
        }

        if (abs(pv_new.getEl() - pv_old.getEl()) > maxDistDistance) {
            return 1;
        }

//        double sin_el_old = sin(pv_old.getEl());
//        double sin_el_new = sin(pv_new.getEl());
//        double cos_el_old = cos(pv_old.getEl());
//        double cos_el_new = cos(pv_new.getEl());
//        double cos_el_daz = cos(pv_new.getAz() - pv_old.getAz());
//        double tmp = sin_el_old * sin_el_new + cos_el_old * cos_el_new * cos_el_daz;
//        double distance = acos(tmp);

        double sin_el_old = VieVS_lookup::sinLookup[(int) pv_old.getEl() * 1000];
        double sin_el_new = VieVS_lookup::sinLookup[(int) pv_new.getEl() * 1000];
        double cos_el_old = VieVS_lookup::cosLookup[(int) pv_old.getEl() * 1000];
        double cos_el_new = VieVS_lookup::cosLookup[(int) pv_new.getEl() * 1000];
        double cos_el_daz = VieVS_lookup::cosLookup[(int) fmod(pv_new.getAz() - pv_old.getAz(), 2 * pi) * 1000];
        double tmp = sin_el_old * sin_el_new + cos_el_old * cos_el_new * cos_el_daz;
        double distance = VieVS_lookup::acosLookup[(int) tmp * 100];


//        double daz_half = (pv_new.getAz() - pv_old.getAz())/2;
//        double del_half = (pv_new.getEl() - pv_old.getEl())/2;
//        double tel = sin(del_half);
//        double taz = sin(daz_half);
//        double a = tel*tel + cos(pv_new.getEl()) * cos(pv_old.getEl()) * taz * taz ;
//        double distance = 2*atan2(sqrt(a),sqrt(1-a));

//        double daz_half = (pv_new.getAz() - pv_old.getAz())/2;
//        double del_half = (pv_new.getEl() - pv_old.getEl())/2;
//        double tel = VieVS_lookup::sinLookup[(int) del_half *1000];
//        double taz = VieVS_lookup::sinLookup[(int) daz_half *1000];
//        double a = tel*tel + VieVS_lookup::cosLookup[(int) pv_new.getEl() *1000] * VieVS_lookup::cosLookup[(int) pv_old.getEl()*1000] * taz * taz ;
//        double distance = 2*atan2(sqrt(a),sqrt(1-a));


        if (distance > maxDistDistance) {
            return 1;
        }

//        double scoreDistance = .5 + .5 * cos(distance * pi / maxDistDistance);
//        double scoreTime = .5 + .5 * cos(deltaTime * pi / maxDistTime );
        double scoreDistance = .5 + .5 * (VieVS_lookup::cosLookup[(int) distance * pi / maxDistDistance * 1000]);
        double scoreTime = .5 + .5 * (VieVS_lookup::cosLookup[(int) deltaTime * pi / maxDistTime * 1000]);
        double thisScore = 1 - (scoreDistance * scoreTime);

        return thisScore;
    }

}
