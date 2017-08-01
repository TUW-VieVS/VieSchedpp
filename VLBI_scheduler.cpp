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

    VLBI_scheduler::VLBI_scheduler(VLBI_initializer &init) :
    stations{init.getStations()}, sources{init.getSources()}, skyCoverages{init.getSkyCoverages()}{
        
        VLBI_initializer::PARAMETERS IPARA = init.getPARA();
        PARA.endTime = IPARA.endTime;
        PARA.startTime = IPARA.startTime;
        auto duration = (PARA.endTime - PARA.startTime);

        PARA.duration = duration.total_seconds();
        PARA.mjdStart = IPARA.mjdStart;

        considered_n1scans = 0;
        considered_n2scans = 0;
    }
    
    VLBI_scheduler::~VLBI_scheduler() {
    }
    
    void VLBI_scheduler::start(){
        bool endOfScheduleReached = false;

        outputHeader(stations);

        while (true) {
//            cout << "############################ new scan ############################\n";
            VLBI_subcon subcon = allVisibleScans();

            subcon.calcStartTimes(stations, sources);

            subcon.updateAzEl(stations, sources);

            subcon.constructAllBaselines();

            subcon.calcAllBaselineDurations(stations, sources, PARA.mjdStart);

            subcon.calcAllScanDurations(stations, sources);

            subcon.createSubcon2(PRE.subnettingSrcIds, stations.size() * 0.66);
            subcon.precalcScore(stations, sources);

            subcon.generateScore(stations, skyCoverages);

            subcon.calcScores();

            unsigned long bestIdx = subcon.rigorousScore(stations, sources, skyCoverages, PARA.mjdStart);

            if (bestIdx < subcon.getNumberSingleScans()) {
                VLBI_scan bestScan = subcon.getSingleSourceScan(bestIdx);

                endOfScheduleReached = update(bestScan);
                consideredUpdate(subcon.getNumberSingleScans(), subcon.getNumberSubnettingScans());

            } else {
                unsigned long thisIdx = bestIdx - subcon.getNumberSingleScans();
                pair<VLBI_scan, VLBI_scan> bestScans = subcon.getDoubleSourceScan(thisIdx);
                VLBI_scan bestScan1 = bestScans.first;
                VLBI_scan bestScan2 = bestScans.second;

                if (bestScan1.maxTime() > bestScan2.maxTime()) {
                    swap(bestScan1, bestScan2);
                }

                bool endReached1 = update(bestScan1);
                bool endReached2 = update(bestScan2);
                consideredUpdate(subcon.getNumberSingleScans(), subcon.getNumberSubnettingScans());

                endOfScheduleReached = endReached1 || endReached2;
            }
            if (endOfScheduleReached) {
                break;
            }

        }

        cout << "TOTAL SUMMARY:\n";
        cout << "considered single:      " << considered_n1scans << "\n";
        cout << "considered subnetting:  " << considered_n2scans << "\n";
        cout << "total scans considered: " << considered_n1scans + 2 * considered_n2scans << "\n";

    }

    VLBI_subcon VLBI_scheduler::allVisibleScans(){
        unsigned long nsta = stations.size();
        unsigned long nsrc = sources.size();

        unsigned int currentTime = 0;
        vector< unsigned int> lastScanLookup;
        for (int i = 0; i<stations.size(); ++i){
            unsigned int t = stations[i].getCurrentTime();
            lastScanLookup.push_back(t);
            if (t > currentTime) {
                currentTime = t;
            }
        }
        
        VLBI_subcon subcon;

        for (int isrc=0; isrc<nsrc; ++isrc){
            VLBI_source &thisSource = sources[isrc];
            if (thisSource.lastScanTime() != 0 &&
                currentTime - thisSource.lastScanTime() < thisSource.minRepeatTime()) {
                continue;
            }

            unsigned int visibleSta = 0;
            vector<VLBI_pointingVector> pointingVectors;
            vector<unsigned int> endOfLastScans;
            for (int ista=0; ista<nsta; ++ista){
                VLBI_pointingVector p(ista,isrc);
                bool flag = stations[ista].isVisible(thisSource, p, true);
                if (flag){
                    visibleSta++;
                    endOfLastScans.push_back(lastScanLookup[ista]);
                    pointingVectors.push_back(p);
                }
            }
            if (visibleSta >= thisSource.getMinNumberOfStations()) {
                subcon.addScan(VLBI_scan(pointingVectors, endOfLastScans, thisSource.getMinNumberOfStations()));
            }
        }
        
        return subcon;
    }

    void VLBI_scheduler::precalcSubnettingSrcIds(){
        unsigned long nsrc = sources.size();
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

    bool VLBI_scheduler::update(VLBI_scan &scan) {

        unsigned int latestTime = scan.maxTime();

        if (latestTime > PARA.duration) {
            return true;
        }

        int srcid = scan.getSourceId();
        string sourceName = sources[srcid].getName();
        unsigned long nsta = scan.getNSta();
        unsigned long nbl = scan.getNBl();


        for (int i = 0; i < scan.getNSta(); ++i) {
            VLBI_pointingVector &pv = scan.getPointingVector(i);
            int staid = pv.getStaid();
            VLBI_pointingVector &pv_end = scan.getPointingVectors_endtime(i);
            vector<unsigned int> times = scan.getTimes().stationTimes(i);
            stations[staid].update(nbl, pv, pv_end, times, sourceName);


            int skyCoverageId = stations[staid].getSkyCoverageID();
            skyCoverages[skyCoverageId].update(pv, pv_end);

        }

        VLBI_source &thisSource = sources[srcid];
        thisSource.update(nbl, latestTime);

        scans.push_back(scan);
        scan.output(scans.size(), stations, thisSource, PARA.startTime);

        return false;
    }

    void VLBI_scheduler::outputHeader(vector<VLBI_station> &stations) {
        cout << ".------------.";
        for (auto &t:stations) {
            cout << "----------.";
        }
        cout << "\n";
        cout << "| stations   | ";
        for (auto &t:stations) {
            cout << boost::format("%8s | ") % t.getName();
        }
        cout << "\n";
    }

    void VLBI_scheduler::consideredUpdate(unsigned long n1scans, unsigned long n2scans) {
        cout << "| considered single Scans: " << n1scans << " subnetting scans: " << n2scans << "\n";
        considered_n1scans += n1scans;
        considered_n2scans += n2scans;
    }

}
