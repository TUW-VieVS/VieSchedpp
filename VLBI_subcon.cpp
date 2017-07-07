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
    
    void VLBI_subcon::calcSlewTimes(vector<VLBI_station> stations){
        vector<int> srcIds(n1scans);
        vector<vector<int> > srcStaId(n1scans);
        for(int i=0; i<n1scans; ++i){
            srcIds[i] = subnet1[i].getSourceId();
            for(int j=0; j<subnet1[i].getNSta(); ++j){
                int staid = subnet1[i].getStationId(j);
                srcStaId[i].push_back(staid);
                stations[staid].slewTo(subnet1[i].getPointingVector(j));
            }
        }
        
        for(int i=0; i<n2scans; ++i){
            int srcid1 = subnet2[i].first.getSourceId();
            vector<int>::iterator it1 = find(srcIds.begin(), srcIds.end(), srcid1);
            int idxSrcStaId11 = it1-srcIds.begin();
            for(int j=0; j<subnet2[i].first.getNSta(); ++j){
                int staid = subnet2[i].first.getStationId(j);
                vector<int>::iterator it2 = find(srcStaId[idxSrcStaId11].begin(), srcStaId[idxSrcStaId11].end(), staid);
                int idxSrcStaId12 = it2-srcStaId[idxSrcStaId11].begin();

                subnet2[i].first.setPointingVector(j,subnet1[idxSrcStaId11].getPointingVector(idxSrcStaId12));

            }

            int srcid2 = subnet2[i].second.getSourceId();
            vector<int>::iterator it2 = find(srcIds.begin(), srcIds.end(), srcid2);
            int idxSrcStaId21 = it2-srcIds.begin();
            for(int j=0; j<subnet2[i].second.getNSta(); ++j){
                int staid = subnet2[i].second.getStationId(j);
                vector<int>::iterator it2 = find(srcStaId[idxSrcStaId21].begin(), srcStaId[idxSrcStaId21].end(), staid);
                int idxSrcStaId22 = it2-srcStaId[idxSrcStaId21].begin();

                subnet2[i].second.setPointingVector(j,subnet1[idxSrcStaId21].getPointingVector(idxSrcStaId22));

            }

        }
    }
    
}
