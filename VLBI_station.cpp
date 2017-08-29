/*
 * Copyright (C) 2017 mschartn
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * File:   station.cpp
 * Author: mschartn
 * 
 * Created on June 21, 2017, 1:43 PM
 */

#include "VLBI_station.h"

namespace VieVS{
    
    VLBI_station::VLBI_station(){
        axis = axisType::undefined;
    }
    
    VLBI_station::VLBI_station(const string &sta_name, int id, const VLBI_antenna &sta_antenna,
                               const VLBI_cableWrap &sta_cableWrap, const VLBI_position &sta_position,
                               const VLBI_equip &sta_equip, const VLBI_mask &sta_mask, const string &sta_axis):
            name{sta_name}, id{id}, antenna{sta_antenna}, cableWrap{sta_cableWrap}, position{sta_position},
            equip{sta_equip}, mask{sta_mask}{
        skyCoverageID = -1;
        if (sta_axis.compare("AZEL") == 0)
            axis = axisType::AZEL;
        else if(sta_axis.compare("HADC") == 0)
            axis = axisType::HADC;
        else if(sta_axis.compare("XYNS") == 0)
            axis = axisType::XYNS;
        else if(sta_axis.compare("XYEW") == 0)
            axis = axisType::XYEW;
        else if(sta_axis.compare("RICH") == 0)
            axis = axisType::RICH;
        else if(sta_axis.compare("SEST") == 0)
            axis = axisType::SEST;
        else if(sta_axis.compare("ALGO") == 0)
            axis = axisType::ALGO;
        else 
            axis = axisType::undefined;
        
        history_time.push_back(0);
    }

    void VLBI_station::setCurrentPointingVector(const VLBI_pointingVector &pointingVector) noexcept {
        current = pointingVector;
    }

    ostream &operator<<(ostream &out, const VLBI_station &sta) noexcept {
        cout << boost::format("%=36s\n") %sta.name; 
        cout << sta.position ;
        cout << "uses sky coverage id: " << sta.skyCoverageID << "\n";
        cout << sta.antenna << sta.cableWrap << sta.equip;
        cout << "------------------------------------\n";
        return out;
    }

    bool VLBI_station::isVisible(const VLBI_pointingVector &p) const noexcept {
        return mask.visible(p) && cableWrap.anglesInside(p);
    }

    void VLBI_station::updateAzEl(const VLBI_source &source, VLBI_pointingVector &p, azelModel model) const noexcept {


        double omega = 7.2921151467069805e-05; //1.00273781191135448*D2PI/86400;

        unsigned int time = p.getTime();
        //  TIME 
        double date1 = 2400000.5;
        double date2 = VieVS_time::mjdStart + (double) time / 86400;

        // Earth Rotation
        double ERA = iauEra00(date1, date2);

        // precession nutation
        double C[3][3] = {{1, 0, 0},
                          {0, 1, 0},
                          {0, 0, 1}};

        if (model == azelModel::rigorous) {
            unsigned int nut_precalc_idx = 0;
            while (VieVS_nutation::nut_time[nut_precalc_idx + 1] < time) {
                ++nut_precalc_idx;
            }
            int delta = VieVS_nutation::nut_time[1] - VieVS_nutation::nut_time[0];

            unsigned int deltaTime = time - VieVS_nutation::nut_time[nut_precalc_idx];

            double x = VieVS_nutation::nut_x[nut_precalc_idx] +
                       (VieVS_nutation::nut_x[nut_precalc_idx + 1] - VieVS_nutation::nut_x[nut_precalc_idx]) / delta *
                       deltaTime;
            double y = VieVS_nutation::nut_y[nut_precalc_idx] +
                       (VieVS_nutation::nut_y[nut_precalc_idx + 1] - VieVS_nutation::nut_y[nut_precalc_idx]) / delta *
                       deltaTime;
            double s = VieVS_nutation::nut_s[nut_precalc_idx] +
                       (VieVS_nutation::nut_s[nut_precalc_idx + 1] - VieVS_nutation::nut_s[nut_precalc_idx]) / delta *
                       deltaTime;


            iauC2ixys(x, y, s, C);
        }

        //  Polar Motion 
        double W[3][3] = {{1,0,0},
                          {0,1,0},
                          {0,0,1}};
        //  GCRS to ITRS 
        double c2t[3][3];
        iauC2tcio(C, ERA, W, c2t);
        double t2c[3][3] = {};
        iauTr(c2t, t2c);


        //  Transformation 
        double v1[3] = {-omega*position.getX(), 
                         omega*position.getY(), 
                         0};
        
        double v1R[3] = {};
        iauRxp(t2c, v1, v1R);


        double k1a[3] = {};
        double k1a_t1[3] = {(VieVS_earth::velocity[0] + v1[0]) / CMPS,
                            (VieVS_earth::velocity[1] + v1[1]) / CMPS,
                            (VieVS_earth::velocity[2] + v1[2]) / CMPS};


        // Source vector in CRF
        const vector<double> & scrs_ = source.getSourceInCrs();
        double rqu[3] = {scrs_[0],scrs_[1],scrs_[2]};

        double k1a_t2[3] = {};
        iauSxp(iauPdp(rqu,k1a_t1),rqu,k1a_t2);
        k1a_t2[0] = -k1a_t2[0];
        k1a_t2[1] = -k1a_t2[1];
        k1a_t2[2] = -k1a_t2[2];

        double k1a_temp[3] ={};
        iauPpp ( rqu, k1a_t1, k1a_temp );
        iauPpp ( k1a_temp,k1a_t2,k1a);

        //  source in TRS 
        double rq[3] = {};
        iauRxp(c2t, k1a, rq);



        //  source in local system 
        double g2l[3][3] = {{PRECALC.g2l[0][0],PRECALC.g2l[0][1],PRECALC.g2l[0][2]},
                            {PRECALC.g2l[1][0],PRECALC.g2l[1][1],PRECALC.g2l[1][2]},
                            {PRECALC.g2l[2][0],PRECALC.g2l[2][1],PRECALC.g2l[2][2]},};

        double lq[3] = {};
        iauRxp(g2l,rq,lq);

        double zd = acos(lq[2]);
        double el = DPI/2 - zd;

        double saz = atan2(lq[1],lq[0]);
        if (lq[1]<0){
            saz = DPI*2+saz;
        }
        double az = fmod(saz+DPI,DPI*2);

        p.setAz(az);
        p.setEl(el);
        p.setTime(time);
    }

    void VLBI_station::preCalc(const vector<double> &distance, const vector<double> &dx, const vector<double> &dy,
                               const vector<double> &dz) noexcept {


        PRECALC.distance = distance;
        PRECALC.dx = dx;
        PRECALC.dy = dy;
        PRECALC.dz = dz;

        double lat = position.getLat();
        double lon = position.getLon();

        double theta = DPI/2-lat;

        const double cosTheta = cos(theta);
        const double sinTheta = sin(theta);
        double roty[3][3] = {{cosTheta, 0,  -sinTheta},
                             {0,        -1, 0},
                             {sinTheta, 0,  cosTheta} };

        const double cosLon = cos(lon);
        const double sinLon = sin(lon);
        double rotz[3][3] = {{cosLon,  sinLon, 0},
                             {-sinLon, cosLon, 0},
                             {0,       0,      1}};

        double g2l[3][3] = {};

        iauRxr(roty,rotz,g2l);

        PRECALC.g2l.resize(3);
        PRECALC.g2l[0].resize(3);
        PRECALC.g2l[1].resize(3);
        PRECALC.g2l[2].resize(3);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                PRECALC.g2l[i][j] = g2l[i][j];
            }
        }
    }

    double VLBI_station::distance(const VLBI_station &other) const noexcept {
        return position.getDistance(other.position);
    }

    unsigned int VLBI_station::slewTime(const VLBI_pointingVector &pointingVector) const noexcept {
        if (*PARA.firstScan) {
            return 0;
        } else {
            return antenna.slewTime(current, pointingVector);
        }
    }

    void VLBI_station::update(unsigned long nbl, const VLBI_pointingVector &start, const VLBI_pointingVector &end,
                              const vector<unsigned int> &times, const string &srcName) noexcept {
        ++nscans;
        nbls += nbl;
        pv_startScan.push_back(start);
        pv_endScan.push_back(end);
        current = end;

        history_time.push_back(times[0]);
        history_events.push_back("setup");

        history_time.push_back(times[1]);
        history_events.push_back("source");

        history_time.push_back(times[2]);
        history_events.push_back("slew to " + srcName);

        history_time.push_back(times[3]);
        history_events.push_back("idle");

        history_time.push_back(times[4]);
        history_events.push_back("tape");

        history_time.push_back(times[5]);
        history_events.push_back("calibration");

        history_time.push_back(times[6]);
        history_events.push_back("scan " + srcName);

        if (*PARA.firstScan) {
            PARA.firstScan = false;
        }
    }

    void VLBI_station::setCableWrapMinimumOffsets() noexcept {
        cableWrap.setMinimumOffsets(*PARA.axis1_low_offset, *PARA.axis1_up_offset, *PARA.axis2_low_offset,
                                    *PARA.axis2_up_offset);
    }

    void VLBI_station::checkForNewEvent(unsigned int time, bool output) noexcept {
        while (EVENTS[nextEvent].time <= time) {
            bool oldAvailable = *PARA.available;
            PARA = EVENTS[nextEvent].PARA;
            bool newAvailable = *PARA.available;

            if (!oldAvailable && newAvailable) {
                current.setTime(EVENTS[nextEvent].time);
                PARA.firstScan = true;
            }

            if (output) {
                cout << "###############################################\n";
                cout << "## changing parameters for station: " << boost::format("%8s") % name << " ##\n";
                cout << "###############################################\n";
            }
            nextEvent++;
        }
    }


}
