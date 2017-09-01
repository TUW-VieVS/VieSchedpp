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
vector<vector<vector<float> > > VieVS::VLBI_skyCoverage::angularDistanceLookup = {};

namespace VieVS{
    VLBI_skyCoverage::VLBI_skyCoverage() {
    }

    VLBI_skyCoverage::VLBI_skyCoverage(const vector<int> &staids, double skyCoverageDistance,
                                       double skyCoverageInterval, int id)
            : nStations{staids.size()}, staids{staids}, maxDistTime{skyCoverageInterval},
              maxDistDistance{skyCoverageDistance}, id{id} {
    }

    VLBI_skyCoverage::~VLBI_skyCoverage() {
    }

    double VLBI_skyCoverage::calcScore(const vector<VLBI_pointingVector> &pvs,
                                       const vector<VLBI_station> &stations) const noexcept {

        double score = 0;

        for (int idx_newObs = 0; idx_newObs < pvs.size(); ++idx_newObs) {
            const VLBI_pointingVector &pv_new = pvs[idx_newObs];
            if (stations[pv_new.getStaid()].getSkyCoverageID() != id) {
                continue;
            }

            double min_score = 1;

            for (int idx_oldObs = 0; idx_oldObs < pv_end.size(); ++idx_oldObs) {
                const VLBI_pointingVector &pv_old = pv_end[idx_oldObs];

                double thisScore = scorePerPointingVector(pv_new, pv_old);
                if (thisScore < min_score) {
                    min_score = thisScore;
                }
            }
            score = min_score;

//            for (int idx_oldObs = 0; idx_oldObs < idx_newObs; ++idx_oldObs) {
//                VLBI_pointingVector &pv_old = pvs[idx_oldObs];
//                if (stations[pv_old.getStaid()].getSkyCoverageID() != id) {
//                    continue;
//                }
//                double thisScore = scorePerPointingVector(pv_new, pv_old);
//                if (thisScore < min_score) {
//                    min_score = thisScore;
//                }
//            }
//            score += min_score;
        }
        return score;
    }


    double VLBI_skyCoverage::calcScore(const vector<VLBI_pointingVector> &pvs, const vector<VLBI_station> &stations,
                                       vector<double> &firstScorePerPv) const noexcept {

        double score = 0;

        for (int idx_newObs = 0; idx_newObs < pvs.size(); ++idx_newObs) {
            const VLBI_pointingVector &pv_new = pvs[idx_newObs];
            if (stations[pv_new.getStaid()].getSkyCoverageID() != id) {
                continue;
            }

            double min_score = 1;

            for (int idx_oldObs = 0; idx_oldObs < pv_end.size(); ++idx_oldObs) {
                const VLBI_pointingVector &pv_old = pv_end[idx_oldObs];

                double thisScore = scorePerPointingVector(pv_new, pv_old);
                if (thisScore < min_score) {
                    min_score = thisScore;
                }
            }

            firstScorePerPv[idx_newObs] = min_score;
            score = min_score;
//            for (int idx_oldObs = 0; idx_oldObs < idx_newObs; ++idx_oldObs) {
//                VLBI_pointingVector &pv_old = pvs[idx_oldObs];
//                if (stations[pv_old.getStaid()].getSkyCoverageID() != id) {
//                    continue;
//                }
//                double thisScore = scorePerPointingVector(pv_new, pv_old);
//                if (thisScore < min_score) {
//                    min_score = thisScore;
//                }
//            }
//            score += min_score;
        }
        return score;
    }

    double VLBI_skyCoverage::calcScore_subcon(const vector<VLBI_pointingVector> &pvs,
                                              const vector<VLBI_station> &stations,
                                              const vector<double> &firstScorePerPv) const noexcept {

        double score = 0;

        for (int idx_newObs = 0; idx_newObs < pvs.size(); ++idx_newObs) {
            const VLBI_pointingVector &pv_new = pvs[idx_newObs];
            if (stations[pv_new.getStaid()].getSkyCoverageID() != id) {
                continue;
            }
            score = firstScorePerPv[idx_newObs];
            break;
//            double min_score = firstScorePerPv[idx_newObs];
//
//            for (int idx_oldObs = 0; idx_oldObs < idx_newObs; ++idx_oldObs) {
//                VLBI_pointingVector &pv_old = pvs[idx_oldObs];
//                if (stations[pv_old.getStaid()].getSkyCoverageID() != id) {
//                    continue;
//                }
//                if (pv_old.getSrcid() == pv_new.getSrcid()){
//                    min_score = 0;
//                    break;
//                }
//                double thisScore = scorePerPointingVector(pv_new, pv_old);
//                if (thisScore < min_score) {
//                    min_score = thisScore;
//                }
//            }
//            score += min_score;
        }
        return score;
    }

    void VLBI_skyCoverage::update(const VLBI_pointingVector &start, const VLBI_pointingVector &end) noexcept {
        pv_start.push_back(start);
        pv_end.push_back(end);
    }

    double
    VLBI_skyCoverage::scorePerPointingVector(const VLBI_pointingVector &pv_new,
                                             const VLBI_pointingVector &pv_old) const noexcept {
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

//         **** THIS IS THE SECOND FASTEST VERSION SO FAR ****
//        double sin_el_old = VieVS_lookup::sinLookup[(int) (pv_old.getEl() * 1000)];
//        double sin_el_new = VieVS_lookup::sinLookup[(int) (pv_new.getEl() * 1000)];
//        double cos_el_old = VieVS_lookup::cosLookup[(int) (pv_old.getEl() * 1000)];
//        double cos_el_new = VieVS_lookup::cosLookup[(int) (pv_new.getEl() * 1000)];
//        double cos_el_daz = VieVS_lookup::cosLookup[(int) (fmod(pv_new.getAz() - pv_old.getAz(), 2 * pi) * 1000)];
//        double tmp = sin_el_old * sin_el_new + cos_el_old * cos_el_new * cos_el_daz;
//        double distance = VieVS_lookup::acosLookup[(int) (tmp * 100)];


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

        // **** THIS IS THE FASTEST VERSION SO FAR ****
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


        int pv_delta_az = abs((int) (delta_az +.5)); // +.5 for rounding
        if(pv_1_el>pv_2_el){
            swap(pv_1_el,pv_2_el);
        }
        int thisEl = (int) (pv_1_el*rad2deg+.5); // +.5 for rounding
        int pv_delta_el = (int) ((pv_2_el-pv_1_el)*rad2deg+.5); // +.5 for rounding
        float distance = VLBI_skyCoverage::angularDistanceLookup[thisEl][pv_delta_az][pv_delta_el];

        if (distance > maxDistDistance) {
            return 1;
        }

//        double scoreDistance = .5 + .5 * cos(distance * pi / maxDistDistance);
//        double scoreTime = .5 + .5 * cos(deltaTime * pi / maxDistTime );
        double scoreDistance = .5 + .5 * (VieVS_lookup::cosLookup[(int) (distance * pi / maxDistDistance * 1000)]);
        double scoreTime = .5 + .5 * (VieVS_lookup::cosLookup[(int) (deltaTime * pi / maxDistTime * 1000)]);
        double thisScore = 1 - (scoreDistance * scoreTime);

        return thisScore;
    }

}
