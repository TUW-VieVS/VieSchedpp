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
    
    VLBI_station::VLBI_station(string sta_name, int id, VLBI_antenna sta_antenna, VLBI_cableWrap sta_cableWrap,
                               VLBI_position sta_position, VLBI_equip sta_equip, VLBI_mask sta_mask, string sta_axis):
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
        
    void VLBI_station::pushPointingVector(VLBI_pointingVector pointingVector){
        current = pointingVector;
    }

    double VLBI_station::getCableWrapNeutralPoint(int axis){
        return cableWrap.neutralPoint(axis);
    }

    void VLBI_station::setParameters(const string& group, boost::property_tree::ptree& PARA_station){
        PARA.parameterGroups.push_back(group);
        for (auto it: PARA_station){
            string name = it.first;
            if ( name == "<xmlattr>")
                continue;
            else if ( name == "axis1_low_offset")
                PARA.axis1_low_offset = PARA_station.get<double>("axis1_low_offset");
            else if ( name == "axis1_up_offset")
                PARA.axis1_up_offset = PARA_station.get<double>("axis1_up_offset");
            else if ( name == "axis2_low_offset")
                PARA.axis2_low_offset = PARA_station.get<double>("axis2_low_offset");
            else if ( name == "axis2_up_offset")
                PARA.axis2_up_offset = PARA_station.get<double>("axis2_up_offset");
            else if ( name == "wait_setup")
                PARA.wait_setup = PARA_station.get<unsigned int>("wait_setup");
            else if ( name == "wait_source")
                PARA.wait_source = PARA_station.get<unsigned int>("wait_source");
            else if ( name == "wait_tape")
                PARA.wait_tape = PARA_station.get<unsigned int>("wait_tape");
            else if ( name == "wait_calibration")
                PARA.wait_calibration = PARA_station.get<unsigned int>("wait_calibration");
            else if ( name == "wait_corsynch")
                PARA.wait_corsynch = PARA_station.get<unsigned int>("wait_corsynch");
            else if ( name == "maxSlewtime")
                PARA.maxSlewtime = PARA_station.get<unsigned int>("maxSlewtime");
            else if ( name == "maxWait")
                PARA.maxWait = PARA_station.get<unsigned int>("maxWait");
            else if ( name == "maxScan")
                PARA.maxScan = PARA_station.get<unsigned int>("maxScan");
            else if ( name == "minScan")
                PARA.minScan = PARA_station.get<unsigned int>("minScan");
            else if ( name == "minSNR"){
                string bandName = it.second.get_child("<xmlattr>.band").data();
                double value = it.second.get_value<double>();
                PARA.minSNR.push_back(make_pair(bandName, value));
            } else
                cerr << "Station " << this->name << ": parameter <" << name << "> not understood! (Ignored)\n";
        }
    }

    ostream& operator<<(ostream& out, const VLBI_station& sta){
        cout << boost::format("%=36s\n") %sta.name; 
        cout << sta.position ;
        cout << "uses sky coverage id: " << sta.skyCoverageID << "\n";
        cout << sta.antenna << sta.cableWrap << sta.equip;
        cout << "------------------------------------\n";
        return out;
    }
    
    bool VLBI_station::isVisible(VLBI_source source, VLBI_pointingVector& p, bool useTimeFromStation){
        if (useTimeFromStation){
            unsigned int time = current.getTime() + PARA.wait_setup + PARA.wait_calibration + PARA.wait_source +
                                PARA.wait_tape;
            getAzEl(source, p, time);
        }else {
            unsigned int time = p.getTime();
            getAzEl(source, p, time);
        }
        
        return cableWrap.anglesInside(p);
    }

    void VLBI_station::getAzEl(VLBI_source source, VLBI_pointingVector &p, unsigned int time, azelModel model) {


        double ra = source.getRa();
        double de = source.getDe();
        double lat = position.getLat();
        double lon = position.getLon();
        double omega = 7.2921151467069805e-05; //1.00273781191135448*D2PI/86400;

        //  TIME 
        double date1 = 2400000.5;
        double date2 = PRECALC.mjdStart + (double) time / 86400;

        // Earth Rotation
        double ERA = iauEra00(date1, date2);

        // Source vector in CRF
        double rqu[3] = {cos(de) * cos(ra),
                         cos(de) * sin(ra),
                         sin(de)};

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
        double theta = DPI/2-lat;
        double roty[3][3] = { {cos(theta),0,-sin(theta)}, 
                              {0, -1, 0}, 
                              {sin(theta), 0, cos(theta)} };
        
        double rotz[3][3] = { {cos(lon), sin(lon), 0}, 
                              {-sin(lon), cos(lon), 0},
                              {0, 0, 1}};

        double g2l[3][3] = {};
        iauRxr(roty,rotz,g2l);

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

    void VLBI_station::preCalc(double mjd, vector<double> distance, vector<double> dx, vector<double> dy,
                               vector<double> dz) {
        PRECALC.mjdStart = mjd;
        PRECALC.distance = distance;
        PRECALC.dx = dx;
        PRECALC.dy = dy;
        PRECALC.dz = dz;

    }
    
    double VLBI_station::distance(VLBI_station other){
        return position.getDistance(other.position);
    }

    void VLBI_station::unwrapAz(VLBI_pointingVector &pointingVector) {
        cableWrap.calcUnwrappedAz(current,pointingVector);
    }

    unsigned int VLBI_station::slewTime(VLBI_pointingVector &pointingVector) {
        return antenna.slewTime(current, pointingVector);
    }

    void VLBI_station::update(unsigned long nbl, VLBI_pointingVector start, VLBI_pointingVector end,
                              vector<unsigned int> times, string srcName) {
        ++nscans;
        nbls += nbl;
        pv_starScan.push_back(start);
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


    }

    void VLBI_station::setCableWrapMinimumOffsets() {
        cableWrap.setMinimumOffsets(PARA.axis1_low_offset, PARA.axis1_up_offset, PARA.axis2_low_offset,
                                    PARA.axis2_up_offset);
    }

    bool VLBI_station::unwrapAzNearNeutralPoint(VLBI_pointingVector &pointingVector) {
        return cableWrap.unwrapAzNearNeutralPoint(pointingVector);
    }

    void VLBI_station::unwrapAzNearAz(VLBI_pointingVector &pointingVector, double az) {
        cableWrap.unwrapAzNearAz(pointingVector, az);
    }


}
