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
#include <iostream>
#include <fstream>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <utility>
#include <boost/container/flat_map.hpp>

#include "VLBI_position.h"
#include "VLBI_antenna.h"
#include "VLBI_cableWrap.h"
#include "VLBI_equip.h"
#include "VLBI_mask.h"
#include "VLBI_source.h"
#include "VLBI_pointingVector.h"
#include "VieVS_constants.h"
#include "VieVS_nutation.h"
#include "VieVS_earth.h"

#include "sofa.h"

using namespace std;

namespace VieVS{
    
    class VLBI_station {
    public:        
        enum class axisType {AZEL, HADC, XYNS, XYEW, RICH, SEST, ALGO, undefined};
        enum class azelModel {
            simple, rigorous
        };


        struct PARAMETERS{
            vector<string> parameterGroups;

            double axis1_low_offset = 5;
            double axis1_up_offset = 5;
            double axis2_low_offset = 1;
            double axis2_up_offset = 1;

            vector<pair<string, double> > minSNR;

            unsigned int wait_setup = 10;
            unsigned int wait_source = 5;
            unsigned int wait_tape = 1;
            unsigned int wait_calibration = 10;
            unsigned int wait_corsynch = 3;
            unsigned int maxSlewtime = 9999;
            double maxSlewDistance = 170;
            unsigned int maxWait = 9999;
            unsigned int maxScan = 600;
            unsigned int minScan = 30;
        };
        
        struct PRECALCULATED{
            double mjdStart;
            vector<double> distance;
            vector<double> dx;
            vector<double> dy;
            vector<double> dz;

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

        unsigned int getMaxSlewtime(){
            return PARA.maxSlewtime;
        }

        unsigned int getMaxIdleTime(){
            return PARA.maxWait;
        }

        unsigned int getMinScanTime(){
            return PARA.minScan;
        }

        unsigned int getMaxScanTime(){
            return PARA.maxScan;
        }

        const VLBI_cableWrap &getCableWrap() const {
            return cableWrap;
        }

        string getName(){
            return name;
        }

        unsigned int getCurrentTime(){
            return current.getTime();
        }

        double getSEFD(string band){
            return equip.getSEFD(band);
        }

        double getMaxSEFT(){
            return equip.getMaxSEFD();
        }

        double getMinSNR(string band){
            for (auto &any:PARA.minSNR) {
                if (any.first == band) {
                    return any.second;
                }
            }
        }

        double getDistance(int other_staid){
            return PRECALC.distance[other_staid];
        }

        double getX() const{
            return position.getX();
        }

        double getY() const{
            return position.getY();
        }

        double getZ() const{
            return position.getZ();
        }

        double dx(int id) const{
            return PRECALC.dx[id];
        }
        double dy(int id) const{
            return PRECALC.dy[id];
        }
        double dz(int id) const{
            return PRECALC.dz[id];
        }

        int getNbls() const {
            return nbls;
        }

        int getSkyCoverageID() const {
            return skyCoverageID;
        }

        void setSkyCoverageId(int id) {
            skyCoverageID = id;
        }

        void setId(int id) {
            VLBI_station::id = id;
        }

        double distance(VLBI_station other);

        bool isVisible(VLBI_source source, VLBI_pointingVector& p, bool useTimeFromStation = false);

        void unwrapAz(VLBI_pointingVector &pointingVector);

        bool unwrapAzNearNeutralPoint(VLBI_pointingVector &pointingVector);

        void unwrapAzNearAz(VLBI_pointingVector &pointingVector, double az);

        unsigned int slewTime(VLBI_pointingVector &pointingVector);

        void
        getAzEl(VLBI_source source, VLBI_pointingVector &p, unsigned int time, azelModel model = azelModel::simple);
        
        void pushPointingVector(VLBI_pointingVector pointingVector);

        double getCableWrapNeutralPoint(int axis);

        void setParameters(const string& group, boost::property_tree::ptree& PARA_station);

        friend ostream& operator<<(ostream& out, const VLBI_station& sta);

        void preCalc(double mjd, vector<double> distance, vector<double> dx, vector<double> dy, vector<double> dz);

        void update(unsigned long nbl, VLBI_pointingVector start, VLBI_pointingVector end, vector<unsigned int> times,
                    string srcName);

        void setCableWrapMinimumOffsets();

        unsigned int getWaitSetup() {
            return PARA.wait_setup;
        }

        unsigned int getWaitSource() {
            return PARA.wait_source;
        }

        unsigned int getWaitTape() {
            return PARA.wait_tape;
        }

        unsigned int getWaitCalibration() {
            return PARA.wait_calibration;
        }

        unsigned int getWaitCorsynch() {
            return PARA.wait_corsynch;
        }

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
        PRECALCULATED PRECALC;
        
        VLBI_pointingVector current;
        
        vector<unsigned int> history_time;
        vector<string> history_events;
        vector<VLBI_pointingVector> pv_starScan;
        vector<VLBI_pointingVector> pv_endScan;

        int nscans;
        int nbls;
    };
}
#endif /* VLBI_STATION_H */

