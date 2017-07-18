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
        cout << "    1 scan subcons" << subcon.getNumberSingleScans() << "\n";
        cout << "    2 scan subcons" << subcon.getNumberSubnettingScans() << "\n";

        cout << "calc start times\n";
        subcon.calcStartTimes(stations, sources);
        cout << "start times calculated\n";

        cout << "update az el times\n";
        subcon.updateAzEl(stations,sources);
        cout << "az el updated!\n";

        cout << "construct all baselines\n";
        subcon.constructAllBaselines();
        cout << "baselines constructed\n";

        cout << "calc baseline Duration\n";
        subcon.calcAllBaselineDurations(stations, sources, PARA.mjdStart);
        cout << "baseline Duration calculated\n";

        cout << "calc all scan Duration\n";
        subcon.calcAllScanDurations(stations,sources);
        cout << "scan Durations calculated\n";


/*        cout << "create subnetting subcons\n";
        subcon.createSubcon2();
        cout << "scheduling finished!\n";

        cout << "calculate score for each subcon\n";
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
        
//        if (PARA.subnetting){
//            for (int isrc1=0; isrc1<nsrc; ++isrc1){
//                vector<int> sta1 = visible[isrc1];
//                if (sta1.size()<PARA.minStaPerScan){
//                    continue;
//                }
//
//                for(size_t nsrc2 = 0; nsrc2<PRE.subnettingSrcIds[isrc1].size(); ++nsrc2){
//                    int isrc2 = PRE.subnettingSrcIds[isrc1][nsrc2];
//                    vector<int> sta2 = visible[isrc2];
//                    if (sta2.size()<PARA.minStaPerScan){
//                        continue;
//                    }
//                    vector<int> uniqueSta1;
//                    vector<int> uniqueSta2;
//                    vector<int> intersection;
//                    for (int any: sta1){
//                        if(find(sta2.begin(), sta2.end(), any) == sta2.end()){
//                            uniqueSta1.push_back(any);
//                        } else {
//                            intersection.push_back(any);
//                        }
//                    }
//                    for (int any: sta2){
//                        if(find(sta1.begin(), sta1.end(), any) == sta1.end()){
//                            uniqueSta2.push_back(any);
//                        }
//                    }
//
//                    int nint = intersection.size();
//                    for (int igroup = 0; igroup<=nint; ++igroup){
//
//                        vector<int> data(nint,1);
//                        for(int i=nint-igroup; i<nint; ++i)
//                            data.at(i) = 2;
//
//                        do {
//                            vector<int> scan1sta{uniqueSta1};
//                            vector<int> scan2sta{uniqueSta2};
//                            for(int i=0; i<nint; ++i){
//                                if (data.at(i)==1){
//                                    scan1sta.push_back(intersection.at(i));
//                                } else{
//                                    scan2sta.push_back(intersection.at(i));
//                                }
//                            }
//                            if (scan1sta.size()>=PARA.minStaPerScan && scan2sta.size()>=PARA.minStaPerScan){
//                                vector<VLBI_pointingVector> p1;
//
//                                for(size_t i=0; i<scan1sta.size(); ++i){
//                                    vector<int>::iterator it = find(visible.at(isrc1).begin(), visible.at(isrc1).end(), scan1sta.at(i));
//                                    if (it != visible[isrc1].end()){
//                                        p1.push_back(pointingVectors[isrc1][it-visible[isrc1].begin()]);
//                                    }
//                                }
//                                vector<VLBI_pointingVector> p2;
//
//                                for(size_t i=0; i<scan2sta.size(); ++i){
//                                    vector<int>::iterator it = find(visible.at(isrc2).begin(), visible.at(isrc2).end(), scan2sta.at(i));
//                                    if (it != visible[isrc2].end()){
//                                        p2.push_back(pointingVectors[isrc2][it-visible[isrc2].begin()]);
//                                    }
//                                }
//                                vector<unsigned int> eols1;
//                                for (int i=0; i<p1.size(); ++i){
//                                    int ista = p1[i].getStaid();
//                                    eols1.push_back(lastScanLookup[ista]);
//                                }
//                                vector<unsigned int> eols2;
//                                for (int i=0; i<p2.size(); ++i){
//                                    int ista = p2[i].getStaid();
//                                    eols2.push_back(lastScanLookup[ista]);
//                                }
//                                subcon.addScan(VLBI_scan(p1,eols1),VLBI_scan(p2,eols2));
//                            }
//
//                        }while(next_permutation(std::begin(data), std::end(data)));
//                    }
//                }
//            }
//        }
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
