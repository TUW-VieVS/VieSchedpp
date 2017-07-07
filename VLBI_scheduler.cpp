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
        PARA.currentTime = IPARA.startTime;
    }
    
    VLBI_scheduler::~VLBI_scheduler() {
    }
    
    void VLBI_scheduler::start(){
        VLBI_subcon subcon = allVisibleScans();
        
        
        cout << "scheduling finished!\n";
    }
    
    VLBI_subcon VLBI_scheduler::allVisibleScans(){
        int nsta = stations.size();
        int nsrc = sources.size();
        vector< vector<int> > visible (nsrc);
        vector< vector<VLBI_pointingVector> > pointingVectors (nsrc);
        
        VLBI_subcon subcon;
        
        cout << "start constructing all Visible Scans\n";
        for (int isrc=0; isrc<nsrc; ++isrc){
            int visibleSta = 0;
            for (int ista=0; ista<nsta; ++ista){
                VLBI_pointingVector p;
                bool flag = stations[ista].isVisible(sources[isrc],PARA.currentTime,p);
                if (flag){
                    visibleSta++;
                    visible[isrc].push_back(ista);
                    pointingVectors[isrc].push_back(p);
                }
            }
            if (visibleSta>=PARA.minStaPerSubnet){
                subcon.addScan(VLBI_scan(pointingVectors[isrc]));
            }
        }
        
        if (PARA.subnetting){
            for (int isrc1=0; isrc1<nsrc; ++isrc1){
                vector<int> sta1 = visible[isrc1];
                if (sta1.size()<PARA.minStaPerScan){
                    continue;
                }
                
                for(int nsrc2 = 0; nsrc2<PRE.subnettingSrcIds[isrc1].size(); ++nsrc2){
                    int isrc2 = PRE.subnettingSrcIds[isrc1][nsrc2];
                    vector<int> sta2 = visible[isrc2];
                    if (sta2.size()<PARA.minStaPerScan){
                        continue;
                    }
                    vector<int> uniqueSta1;
                    vector<int> uniqueSta2;
                    vector<int> intersection;
                    for (int any: sta1){
                        if(find(sta2.begin(), sta2.end(), any) == sta2.end()){
                            uniqueSta1.push_back(any);
                        } else {
                            intersection.push_back(any);
                        }
                    }
                    for (int any: sta2){
                        if(find(sta1.begin(), sta1.end(), any) == sta1.end()){
                            uniqueSta2.push_back(any);
                        } 
                    }
                    
                    int nint = intersection.size();
                    for (int igroup = 0; igroup<=nint; ++igroup){
                        
                        vector<int> data(nint,1);
                        for(int i=nint-igroup; i<nint; ++i)
                            data.at(i) = 2;

                        do {
                            vector<int> scan1sta{uniqueSta1};
                            vector<int> scan2sta{uniqueSta2};
                            for(int i=0; i<nint; ++i){
                                if (data.at(i)==1){
                                    scan1sta.push_back(intersection.at(i));
                                } else{
                                    scan2sta.push_back(intersection.at(i));
                                }
                            }
                            if (scan1sta.size()>=PARA.minStaPerScan && scan2sta.size()>=PARA.minStaPerScan && scan1sta.size()+scan2sta.size()>=PARA.minStaPerSubnet){
                                vector<VLBI_pointingVector> p1;
                                for(int i=0; i<scan1sta.size(); ++i){
                                    vector<int>::iterator it = find(visible.at(isrc1).begin(), visible.at(isrc1).end(), scan1sta.at(i));
                                    if (it != visible[isrc1].end()){
                                        p1.push_back(pointingVectors[isrc1][it-visible[isrc1].begin()]);
                                    }
                                    
                                }
                                vector<VLBI_pointingVector> p2;
                                for(int i=0; i<scan2sta.size(); ++i){
                                    vector<int>::iterator it = find(visible.at(isrc2).begin(), visible.at(isrc2).end(), scan2sta.at(i));
                                    if (it != visible[isrc2].end()){
                                        p2.push_back(pointingVectors[isrc2][it-visible[isrc2].begin()]);
                                    }
                                }
                                
                                subcon.addScan(VLBI_scan(p1),VLBI_scan(p2));
                            }
                            
                        }while(next_permutation(std::begin(data), std::end(data)));
                    }
                }
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
