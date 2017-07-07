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
 * File:   station.h
 * Author: mschartn
 *
 * Created on June 21, 2017, 1:43 PM
 */

#ifndef VLBI_STATION_H
#define VLBI_STATION_H
#include <map>
#include <iostream>
#include <fstream>

#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "VLBI_position.h"
#include "VLBI_antenna.h"
#include "VLBI_cableWrap.h"
#include "VLBI_equip.h"
#include "VLBI_mask.h"
#include "VLBI_source.h"
#include "VLBI_pointingVector.h"
#include "VieVS_constants.h"

#include <sofa.h>

using namespace std;

namespace VieVS{
    
    class VLBI_station {
    public:        
        enum class axisType {AZEL, HADC, XYNS, XYEW, RICH, SEST, ALGO, undefined};
        enum class azelModel {simple, rigoros};
        
        struct PARAMETERS{
            double lim1_low_offset = 5*deg2rad;
            double lim1_up_offset = 5*deg2rad;
            double lim2_low_offset = 5*deg2rad;
            double lim2_up_offset = 5*deg2rad;
            
            vector<string> minSNR_band;
            vector<double> minSNR_value;
            
            int wait_setup = 10;
            int wait_source = 5;
            int wait_tape = 1;
            int wait_idle = 0;
            int wait_calibration = 10;
            int wait_corsynch = 3;
            int maxSlewtime = 9999;
            int maxWait = 9999;
            int maxScan = 600;
            int minScan = 30;
        };
        
        
        
        VLBI_station();
                
        VLBI_station(string sta_name, 
                int id,
                VLBI_antenna sta_antenna, 
                VLBI_cableWrap sta_cableWrap, 
                VLBI_position sta_position, 
                VLBI_equip sta_equip, 
                VLBI_mask sta_mask,
                string sta_axis);
                
        virtual ~VLBI_station(){};
        
        double distance(VLBI_station other);
        
        void setSkyCoverageId(int id){skyCoverageID = id;}
        
        bool isVisible(VLBI_source source, boost::posix_time::ptime time, VLBI_pointingVector& p);
        
        void slewTo(VLBI_pointingVector& pointingVector);
        
        VLBI_pointingVector getAzEl(VLBI_source source, boost::posix_time::ptime time);
        
        friend ostream& operator<<(ostream& out, const VLBI_station& sta);
                        
    private:
        string name;
        int id;
        VLBI_antenna antenna;
        VLBI_cableWrap cableWrap;
        VLBI_position position;
        VLBI_equip equip;
        VLBI_mask mask;
        axisType axis;
        int skyCoverageID;
        
        PARAMETERS PARA;
        
        VLBI_pointingVector current;
        
        int nscans;
        int nbls;
    };
}
#endif /* VLBI_STATION_H */

