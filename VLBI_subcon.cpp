/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_subcon.cpp
 * Author: mschartn
 * 
 * Created on June 29, 2017, 5:51 PM
 */

#include "VLBI_subcon.h"
namespace VieVS{
    VLBI_subcon::VLBI_subcon(): n1scans{0}, n2scans{0} {
    }

    VLBI_subcon::~VLBI_subcon() {
    }
    
    void VLBI_subcon::addScan(VLBI_scan scan){
        subnet1.push_back(scan);
        n1scans++;
    }
    
    void VLBI_subcon::addScan(VLBI_scan scan1, VLBI_scan scan2){
        subnet2.push_back(make_pair(scan1,scan2));
        n2scans++;
    }

    void VLBI_subcon::calcStartTimes(vector<VLBI_station> &stations, vector<VLBI_source> &sources) {

        int i=0;
        while(i<n1scans){
            bool scanValid_slew = true;
            bool scanValid_idle = true;
            vector<unsigned  int> maxIdleTimes;

            int j=0;
            while(j<subnet1[i].getNSta()){
                int staid = subnet1[i].getStationId(j);

                unsigned int slewtime = stations[staid].unwrapAzGetSlewTime(subnet1[i].getPointingVector(j));
                if (slewtime > stations[staid].getMaxSlewtime()){
                    cout << "scan: " << i << "deleting station" << stations[staid].getName() << "\n";
                    scanValid_slew = subnet1[i].removeElement(j);
                    if(!scanValid_slew){
                        break;
                    }
                } else {
                    maxIdleTimes.push_back(stations[staid].getMaxIdleTime());
                    subnet1[i].addTimes(j, stations[staid].getWaitSetup(), stations[staid].getWaitSource(), slewtime, stations[staid].getWaitTape(), stations[staid].getWaitCalibration());
                    ++j;
                }
            }

            if (scanValid_slew) {
                scanValid_idle = subnet1[i].checkIdleTimes(maxIdleTimes);
            }

            if (!scanValid_slew || !scanValid_idle){
                cout << "scan: " << i << " not valid anymore\n";
                subnet1.erase(subnet1.begin()+i);
                --n1scans;
            }else{
                ++i;
            }
        }
    }
    
    void VLBI_subcon::constructAllBaselines(){
        for (auto& any: subnet1){
            any.constructBaselines();
        }
    }

    void VLBI_subcon::updateAzEl(vector<VLBI_station> &stations, vector<VLBI_source> &sources) {
        int i = 0;
        while (i < n1scans) {

            VLBI_source thisSource = sources[subnet1[i].getSourceId()];
            bool scanValid_slew = true;
            bool scanValid_idle = true;
            vector<unsigned  int> maxIdleTimes;

            int j = 0;
            while (j < subnet1[i].getNSta()) {
                int staid = subnet1[i].getPointingVector(j).getStaid();
                VLBI_pointingVector thisPointingVector = subnet1[i].getPointingVector(j);
                thisPointingVector.setTime(subnet1[i].getTimes().getEndOfIdleTime(j));
                bool visible = stations[staid].isVisible(thisSource,thisPointingVector,false);
                unsigned int slewtime;
                if (visible){
                     slewtime = stations[staid].unwrapAzGetSlewTime(thisPointingVector);
                }

                if (!visible || slewtime > stations[staid].getMaxSlewtime()){
                    cout << "scan: " << i << "deleting station" << stations[staid].getName() << "\n";
                    scanValid_slew = subnet1[i].removeElement(j);
                    if(!scanValid_slew){
                        break;
                    }
                } else {
                    maxIdleTimes.push_back(stations[staid].getMaxIdleTime());
                    subnet1[i].updateSlewtime(j, slewtime);
                    subnet1[i].setPointingVector(j,thisPointingVector);
                    ++j;
                }
            }


            if (scanValid_slew) {
                scanValid_idle = subnet1[i].checkIdleTimes(maxIdleTimes);
            }

            if (!scanValid_slew || !scanValid_idle){
                cout << "scan: " << i << " not valid anymore\n";
                subnet1.erase(subnet1.begin()+i);
                --n1scans;
            }else{
                ++i;
            }
        }
    }

    void
    VLBI_subcon::calcAllBaselineDurations(vector<VLBI_station> &stations, vector<VLBI_source> &sources, double mjdStart) {
        for (int i = 0; i < n1scans; ++i) {
            VLBI_scan& thisScan = subnet1[i];
            thisScan.calcBaselineScanDuration(stations, sources[thisScan.getSourceId()], mjdStart);
        }
    }

    void VLBI_subcon::calcAllScanDurations(vector<VLBI_station>& stations, vector<VLBI_source>& sources) {
        int i=0;
        while (i<n1scans){
            VLBI_scan& thisScan = subnet1[i];
            int srcid = thisScan.getSourceId();

            bool scanValid = thisScan.scanDuration(stations, sources[srcid]);
            if (scanValid){
                ++i;
            } else {
                --n1scans;
                subnet1.erase(subnet1.begin()+i);
            }
        }
    }


}
