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
    
    VLBI_station::VLBI_station(string sta_name, int id, VLBI_antenna sta_antenna, VLBI_cableWrap sta_cableWrap, VLBI_position sta_position, VLBI_equip sta_equip, VLBI_mask sta_mask, string sta_axis):
            name{sta_name}, id{id}, antenna{sta_antenna}, cableWrap{sta_cableWrap}, position{sta_position}, equip{sta_equip}, mask{sta_mask}{
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
    }
        
    void VLBI_station::pushPointingVector(VLBI_pointingVector pointingVector){
        current = pointingVector;
    }

    double VLBI_station::getCableWrapNeutralPoint(int axis){
        return cableWrap.neutralPoint(axis);
    }

    ostream& operator<<(ostream& out, const VLBI_station& sta){
        cout << boost::format("%=36s\n") %sta.name; 
        cout << sta.position ;
        cout << "uses sky coverage id: " << sta.skyCoverageID << "\n";
        cout << sta.antenna << sta.cableWrap << sta.equip;
        cout << "------------------------------------\n";
        return out;
    }
    
    bool VLBI_station::isVisible(VLBI_source source, boost::posix_time::ptime time, VLBI_pointingVector& p){
        p = getAzEl(source, time);
        return cableWrap.anglesInside(p);
    }
    
    VLBI_pointingVector VLBI_station::getAzEl(VLBI_source source, boost::posix_time::ptime time){
        double ra = source.getRa();
        double de = source.getDe();
        double lat = position.getLat();
        double lon = position.getLon();
        double omega = 1.00273781191135448*D2PI/86400;

        //  TIME 
        double date1, date2;
        iauCal2jd(time.date().year(), time.date().month(), time.date().day(), &date1, &date2);
        
        date2 = date2 + 
                time.time_of_day().hours()/24 + 
                time.time_of_day().minutes()/3600 + 
                time.time_of_day().seconds()/86400;
        
        //  GCRS to Intermediate 
        double x,y,s;
        iauXys06a(date1, date2, &x, &y, &s);
        
        double C[3][3];
        iauC2ixys(x, y, s, C);

        //  Earth Rotation 
        double ERA = iauEra00(date1, date2);
        
        //  Polar Motion 
        double W[3][3] = {{1,0,0},
                          {0,1,0},
                          {0,0,1}};

        //  GCRS to ITRS 
        double t2ct[3][3];
        iauC2tcio ( C, ERA, W, t2ct );

        double t2c[3][3] = {};
        iauTr(t2ct, t2c);

        //  earth velocity 
        double pvh[2][3];
        double pvb[2][3];
        iauEpv00(date1, date2, pvh, pvb);
        double aud2ms = DAU/DAYSEC;
        double vearth[3] = {aud2ms*pvb[1][0], 
                            aud2ms*pvb[1][1], 
                            aud2ms*pvb[1][2]};

        //  Transformation 
        double v1[3] = {-omega*position.getX(), 
                         omega*position.getY(), 
                         0};
        
        double v1R[3] = {};
        iauRxp(t2c, v1, v1R);

        double k1a[3] = {};
        double k1a_t1[3] = {(vearth[0] + v1[0])/CMPS, 
                            (vearth[1] + v1[1])/CMPS,
                            (vearth[2] + v1[2])/CMPS};
        
        double rqu[3] = {cos(de)*cos(ra), cos(de)*sin(ra), sin(de)};
        
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
        iauRxp(t2ct,k1a,rq);

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

        return VLBI_pointingVector(id,source.getId(),az,el,time);
    }
    
    double VLBI_station::distance(VLBI_station other){
        return position.getDistance(other.position);
    }
    
    void VLBI_station::slewTo(VLBI_pointingVector& pointingVector){
        
        cableWrap.calcUnwrappedAz(current,pointingVector);
        antenna.scanStart(current,pointingVector,PARA.wait_setup, PARA.wait_source, PARA.wait_tape, PARA.wait_idle, PARA.wait_calibration);
        
        
    }
}
