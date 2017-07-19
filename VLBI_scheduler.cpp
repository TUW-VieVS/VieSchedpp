/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_scheduler.cpp
 * Author: mschartn
 * 
 * Created on June 29, 2017, 2:29 PM
 */

#include "VLBI_scheduler.h"
namespace VieVS{
    VLBI_scheduler::VLBI_scheduler() {
    }

    VLBI_scheduler::VLBI_scheduler(VLBI_initializer init): 
    stations{init.getStations()}, sources{init.getSources()}, skyCoverages{init.getSkyCoverages()}{
        
        VLBI_initializer::PARAMETERS IPARA = init.getPARA();
        PARA.endTime = IPARA.endTime;
        PARA.startTime = IPARA.startTime;
        PARA.currentTime = 0;
        PARA.mjdStart = IPARA.mjdStart;
    }
    
    VLBI_scheduler::~VLBI_scheduler() {
    }
    
    void VLBI_scheduler::start(){

        cout << "start constructing all Visible Scans\n";
        VLBI_subcon subcon = allVisibleScans();
        cout << "subcon created\n";
        cout << "    1 scan subcons" << subcon.getNumberSingleScans() << "\n\n";

        cout << "calc start times\n";
        subcon.calcStartTimes(stations, sources);
        cout << "start times calculated\n\n";

        cout << "update az el times\n";
        subcon.updateAzEl(stations,sources);
        cout << "az el updated!\n\n";

        cout << "construct all baselines\n";
        subcon.constructAllBaselines();
        cout << "baselines constructed\n\n";

        cout << "calc baseline Duration\n";
        subcon.calcAllBaselineDurations(stations, sources, PARA.mjdStart);
        cout << "baseline Duration calculated\n\n";

        cout << "calc all scan Duration\n";
        subcon.calcAllScanDurations(stations,sources);
        cout << "scan Durations calculated\n\n";


        cout << "create subnetting subcons\n";
        subcon.createSubcon2(PRE.subnettingSrcIds);
        cout << "subcon2 created!\n";
        cout << "    1 scan subcons" << subcon.getNumberSingleScans() << "\n";
        cout << "    2 scan subcons" << subcon.getNumberSubnettingScans() << "\n\n";

/*        cout << "calculate score for each subcon\n";
        subcon.score();
        cout << "scores calculated\n";

        cout << "pick best scan\n";
        int idx1;
        int idx2;
        subcon.best(idx1, idx2);
        cout << "best scan found\n";

        if(idx1 == 1){
            VLBI_scan scan = subcon.getScan(idx2);
            update(scan);
        } else {
            VLBI_scan scan1 = subcon.getFirstScan(idx2);
            VLBI_scan scan2 = subcon.getSecondScan(idx2);

            update(scan1);
            update(scan2);
        }
*/
    }
    /*
    void VLBI_scheduler::update(VLBI_scan scan){
        stations.update(scan);
        sources[scan.getSourceId()].update(scan);
        scans.push_back(scan);
    }
    */
    VLBI_subcon VLBI_scheduler::allVisibleScans(){
        int nsta = stations.size();
        int nsrc = sources.size();


        vector< unsigned int> lastScanLookup;
        for (int i = 0; i<stations.size(); ++i){
            lastScanLookup.push_back(stations[i].getCurrentTime());
        }
        
        VLBI_subcon subcon;

        for (int isrc=0; isrc<nsrc; ++isrc){
            unsigned int visibleSta = 0;
            vector<VLBI_pointingVector> pointingVectors;
            vector<unsigned int> endOfLastScans;
            for (int ista=0; ista<nsta; ++ista){
                VLBI_pointingVector p(ista,isrc);
                bool flag = stations[ista].isVisible(sources[isrc],p,true);
                if (flag){
                    visibleSta++;
                    endOfLastScans.push_back(lastScanLookup[ista]);
                    pointingVectors.push_back(p);
                }
            }
            if (visibleSta>=sources[isrc].getMinNumberOfStations()){
                subcon.addScan(VLBI_scan(pointingVectors,endOfLastScans,sources[isrc].getMinNumberOfStations()));
            }
        }
        
        return subcon;
    }
    
//    void VLBI_scheduler::subconStartTime(VLBI_subcon subcon){
//        
//    }
    
    void VLBI_scheduler::precalcSubnettingSrcIds(){
        int nsrc = sources.size();
        vector<vector<int>> subnettingSrcIds(nsrc);
        for (int i=0; i<nsrc; ++i){
            for (int j=i+1; j<nsrc; ++j){
                double dist = sources[i].angleDistance(sources[j]);
                if (dist>PARA.minAngleBetweenSubnettingSources){
                    subnettingSrcIds[i].push_back(j);
                }
            }
        }
        PRE.subnettingSrcIds = subnettingSrcIds;
    }

}
