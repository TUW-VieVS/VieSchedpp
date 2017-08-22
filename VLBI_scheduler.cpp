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

        subnetting = IPARA.subnetting;
        fillinmode = IPARA.fillinmode;

        considered_n1scans = 0;
        considered_n2scans = 0;
        considered_fillin = 0;
    }
    
    VLBI_scheduler::~VLBI_scheduler() {
    }
    
    void VLBI_scheduler::start(){

        outputHeader(stations);

        while (true) {
            VLBI_subcon subcon = createSubcon(subnetting);

            boost::optional<unsigned long> bestIdx_opt = subcon.rigorousScore(stations, sources, skyCoverages,
                                                                              PARA.mjdStart);
            if (!bestIdx_opt) {
                cout << "ERROR! no valid scan found!\n";
                continue;
            }
            unsigned long bestIdx = *bestIdx_opt;
            vector<VLBI_scan> bestScans;
            if (bestIdx < subcon.getNumberSingleScans()) {
                VLBI_scan &bestScan = subcon.getSingleSourceScan(bestIdx);
                bestScans.push_back(bestScan);
            } else {
                unsigned long thisIdx = bestIdx - subcon.getNumberSingleScans();
                pair<VLBI_scan, VLBI_scan> &bestScan_pair = subcon.getDoubleSourceScan(thisIdx);
                VLBI_scan &bestScan1 = bestScan_pair.first;
                VLBI_scan &bestScan2 = bestScan_pair.second;

                if (bestScan1.maxTime() > bestScan2.maxTime()) {
                    swap(bestScan1, bestScan2);
                }
                bestScans.push_back(bestScan1);
                bestScans.push_back(bestScan2);
            }

            bool finished = endOfSessionReached(bestScans);
            if (finished) {
                break;
            }
            consideredUpdate(subcon.getNumberSingleScans(), subcon.getNumberSubnettingScans());

            if (fillinmode) {
                start_fillinMode(subcon, bestScans);
            } else {
                for (int i = 0; i < bestScans.size(); ++i) {
                    update(bestScans[i]);
                }
            }
        }

        cout << "TOTAL SUMMARY:\n";
        cout << "created single source scans:      " << considered_n1scans << "\n";
        cout << "created subnetting scans:  " << considered_n2scans << "\n";
        cout << "created fillin mode scans:  " << considered_fillin << "\n";
        cout << "total scans considered: " << considered_n1scans + 2 * considered_n2scans + considered_fillin << "\n";

        check(PARA.startTime);
        displayStatistics();
    }

    VLBI_subcon VLBI_scheduler::createSubcon(bool subnetting) {
        VLBI_subcon subcon = allVisibleScans();
        subcon.calcStartTimes(stations, sources);
        subcon.updateAzEl(stations, sources);
        subcon.constructAllBaselines();
        subcon.calcAllBaselineDurations(stations, sources, PARA.mjdStart);
        subcon.calcAllScanDurations(stations, sources);
        if (subnetting) {
            subcon.createSubcon2(PRE.subnettingSrcIds, (int) (stations.size() * 0.66));
        }
        subcon.precalcScore(stations, sources);
        subcon.generateScore(stations, skyCoverages, sources.size());
        return subcon;
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
                VLBI_station &thisSta = stations[ista];
                if (!thisSta.available()) {
                    continue;
                }

                VLBI_pointingVector p(ista,isrc);

                unsigned int time = lastScanLookup[ista] + thisSta.getWaitSetup() + thisSta.getWaitSource() +
                                    thisSta.getWaitTape() + thisSta.getWaitCalibration();
                p.setTime(time);

                thisSta.updateAzEl(thisSource, p);

                bool flag = thisSta.isVisible(p);
                if (flag){
                    visibleSta++;
                    endOfLastScans.push_back(lastScanLookup[ista]);
                    pointingVectors.push_back(p);
                }
            }
            if (visibleSta >= thisSource.getMinNumberOfStations()) {
                subcon.addScan(VLBI_scan(pointingVectors, endOfLastScans, thisSource.getMinNumberOfStations(),
                                         VLBI_scan::scanType::single));
            }
        }
        
        return subcon;
    }

    void VLBI_scheduler::precalcSubnettingSrcIds(){
        unsigned long nsrc = sources.size();
        vector<vector<int> > subnettingSrcIds(nsrc);
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

    void VLBI_scheduler::update(VLBI_scan &scan) {

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

        unsigned int latestTime = scan.maxTime();
        VLBI_source &thisSource = sources[srcid];
        thisSource.update(nbl, latestTime);

        scans.push_back(scan);
        scan.output(scans.size(), stations, thisSource, PARA.startTime);
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

    void VLBI_scheduler::consideredUpdate(unsigned long n1scans, bool created) {
        if(created){
            cout << "|     created new fillin Scans " << n1scans <<" \n";
        }else{
            cout << "|     considered possible fillin Scans " << n1scans <<": \n";
        }
        considered_fillin += n1scans;
    }

    void VLBI_scheduler::consideredUpdate(unsigned long n1scans, unsigned long n2scans) {

        cout << "|-------------";
        for (int i = 0; i < stations.size() - 1; ++i) {
            cout << "-----------";
        }
        cout << "----------| \n";

        cout << "| considered single Scans " << n1scans << " subnetting scans " << n2scans << ":\n";
        considered_n1scans += n1scans;
        considered_n2scans += n2scans;
    }


    boost::optional<VLBI_scan>
    VLBI_scheduler::fillin_scan(VLBI_subcon &subcon, VLBI_fillin_endpositions &fi_endp,
                                vector<int> &sourceWillBeScanned) {
        VLBI_subcon fillin_subcon;

        for (unsigned long i = 0; i < subcon.getNumberSingleScans(); ++i) {
            VLBI_scan &thisScan = subcon.getSingleSourceScan(i);
            int srcid = thisScan.getSourceId();
            if (std::find(sourceWillBeScanned.begin(), sourceWillBeScanned.end(), srcid) != sourceWillBeScanned.end()) {
                continue;
            }
            VLBI_source &thisSource = sources[srcid];
            boost::optional<VLBI_scan> this_fillinScan = thisScan.possibleFillinScan(stations, thisSource,
                                                                                     fi_endp.getStationPossible(),
                                                                                     fi_endp.getStationUnused(),
                                                                                     fi_endp.getFinalPosition());
            if (this_fillinScan) {
                this_fillinScan->setType(VLBI_scan::scanType::fillin);
                fillin_subcon.addScan(move(*this_fillinScan));
            }
        }
        if (fillin_subcon.getNumberSingleScans() == 0) {
            return boost::none;
        } else {
            consideredUpdate(fillin_subcon.getNumberSingleScans());

            fillin_subcon.precalcScore(stations, sources);
            fillin_subcon.generateScore(stations, skyCoverages, sources.size());

            while (true) {
                boost::optional<unsigned long> bestIdx = fillin_subcon.rigorousScore(stations, sources, skyCoverages,
                                                                                     PARA.mjdStart);
                if (bestIdx) {
                    VLBI_scan &fillinScan = fillin_subcon.getSingleSourceScan(*bestIdx);

                    VLBI_source &thisSource = sources[fillinScan.getSourceId()];
                    boost::optional<VLBI_scan> updatedFillinScan = fillinScan.possibleFillinScan(stations, thisSource,
                                                                                                 fi_endp.getStationPossible(),
                                                                                                 fi_endp.getStationUnused(),
                                                                                                 fi_endp.getFinalPosition());
                    if (updatedFillinScan && fillinScan.getNSta() == updatedFillinScan->getNSta()) {
                        return fillinScan;
                    } else {
                        fillin_subcon.removeScan(*bestIdx);
                    }
                } else {
                    return boost::none;
                }
            }
        }
    }

   void VLBI_scheduler::start_fillinMode(VLBI_subcon &subcon, vector<VLBI_scan> &bestScans) {

        VLBI_fillin_endpositions fi_endp(bestScans, stations);
        if (fi_endp.getNumberOfPossibleStations() < 2) {
            for (int i = 0; i < bestScans.size(); ++i) {
                update(bestScans[i]);
            }
            return;
        }

        std::vector<char> stationAvailable(stations.size());
        unsigned long nsta = stations.size();

        for (int i = 0; i < nsta; ++i) {
            bool flag = stations[i].available();
            stationAvailable[i] = flag;
        }

        vector<int> sourceWillBeScanned;
        for (int i = 0; i < bestScans.size(); ++i) {
            sourceWillBeScanned.push_back(bestScans[i].getSourceId());
        }

        while (bestScans.size() > 0) {
            while (true) {
                boost::optional<VLBI_scan> fillinScan = fillin_scan(subcon, fi_endp, sourceWillBeScanned);
                if (fillinScan) {
                    bestScans.push_back(*fillinScan);
                    sourceWillBeScanned.push_back(fillinScan->getSourceId());
                } else {
                    break;
                }

                fi_endp = VLBI_fillin_endpositions(bestScans, stations);
                if (fi_endp.getNumberOfPossibleStations() < 2) {
                    break;
                }
            }

            VLBI_scan &thisBestScan = bestScans.back();
            update(thisBestScan);
            bestScans.pop_back();

            fi_endp = VLBI_fillin_endpositions(bestScans, stations);
            if (fi_endp.getNumberOfPossibleStations() >= 2) {

                const std::vector<char> &stationPossible = fi_endp.getStationPossible();
                for (int i = 0; i < stations.size(); ++i) {
                    stations[i].setAvailable(stationPossible[i]);
                }

                subcon = createSubcon(false);
                consideredUpdate(subcon.getNumberSingleScans(),true);
            }
        }

        for (int i = 0; i < stations.size(); ++i) {
            stations[i].setAvailable(stationAvailable[i]);
        }


    }

    bool VLBI_scheduler::endOfSessionReached(vector<VLBI_scan> bestScans) {
        bool finished = false;
        for (int i = 0; i < bestScans.size(); ++i) {
            VLBI_scan &thisScan = bestScans[i];
            if (thisScan.maxTime() > PARA.duration) {
                finished = true;
            }
        }
        return finished;
    }

    void VLBI_scheduler::check(boost::posix_time::ptime &sessionStart) {

        cout << "\n=======================   starting check routine   =======================\n";
        cout << "starting check routine!\n";

        for (const auto& any:stations){
            cout << "    checking station " << any.getName() << ":\n";
            auto tmp = any.getAllScans();
            const vector<VLBI_pointingVector> & start = tmp.first;
            const vector<VLBI_pointingVector> & end = tmp.second;

            unsigned int constTimes = any.getWaitSetup() + any.getWaitSource() + any.getWaitCalibration() + any.getWaitTape();

            for (int i = 0; i < end.size()-1; ++i) {
                const VLBI_pointingVector &thisEnd = end[i];
                const VLBI_pointingVector &nextStart = start[i+1];

                unsigned int slewtime = any.getAntenna().slewTime(thisEnd,nextStart);
                unsigned int min_neededTime = slewtime + constTimes;

                unsigned int thisEndTime = thisEnd.getTime();
                unsigned int nextStartTime = nextStart.getTime();

                if(nextStartTime<thisEndTime){
                    cout << "    ERROR: somthing went wrong!\n";
                    cout << "           start time of next scan is before end time of previouse scan!\n";
                    boost::posix_time::ptime thisEndTime_ = sessionStart + boost::posix_time::seconds(thisEndTime);
                    boost::posix_time::ptime nextStartTime_ = sessionStart + boost::posix_time::seconds(nextStartTime);
                    cout << "           end time of previouse scan: " << thisEndTime_.time_of_day() << "(" << thisEndTime << ")\n";
                    cout << "           start time of next scan:    " << nextStartTime_.time_of_day() << "(" << nextStartTime << "\n)";
                    continue;
                }
                unsigned int availableTime = nextStartTime-thisEndTime;

                if(availableTime+1<min_neededTime){
                    cout << "    ERROR: somthing went wrong!\n";
                    cout << "           not enough available time for slewing!\n";
                    boost::posix_time::ptime thisEndTime_ = sessionStart + boost::posix_time::seconds(thisEndTime);
                    boost::posix_time::ptime nextStartTime_ = sessionStart + boost::posix_time::seconds(nextStartTime);
                    cout << "               end time of previouse scan: " << thisEndTime_.time_of_day() << " (" << thisEndTime << ")\n";
                    cout << "               start time of next scan:    " << nextStartTime_.time_of_day() << " (" << nextStartTime << ")\n";
                    cout << "           available time: " << availableTime << "\n";
                    cout << "               needed slew time:           " << slewtime << "\n";
                    cout << "               needed constant times:      " << constTimes << "\n";
                    cout << "           needed time:    " << min_neededTime << "\n";
                    cout << "           difference:     " << (long)availableTime-(long)min_neededTime << "\n";
                }

            }
            cout << "    finished!\n";
        }
        cout << "=========================   end check routine    =========================\n";
    }

    void VLBI_scheduler::displayStatistics() {
        cout << "\n=======================   starting statistics   =======================\n";

        unsigned long n_scans = scans.size();
        unsigned long n_single = 0;
        unsigned long n_subnetting = 0;
        unsigned long n_fillin = 0;
        unsigned long n_bl = 0;

        unsigned long nsta = stations.size();
        vector< vector <unsigned long> > bl_storage(nsta,vector<unsigned long>(nsta,0));

        for (const auto&any:scans){
            VLBI_scan::scanType thisType = any.getType();
            switch (thisType){
                case VLBI_scan::scanType::single:{
                    ++n_single;
                    break;
                }
                case VLBI_scan::scanType::subnetting:{
                    ++n_subnetting;
                    break;
                }
                case VLBI_scan::scanType::fillin:{
                    ++n_fillin;
                    break;
                }
            }
            unsigned long this_n_bl = any.getNBl();
            n_bl += this_n_bl;

            for (int i = 0; i < this_n_bl; ++i) {
                int staid1 = any.getBaseline(i).getStaid1();
                int staid2 = any.getBaseline(i).getStaid2();
                if(staid1>staid2){
                    swap(staid1,staid2);
                }
                ++bl_storage[staid1][staid2];
            }

        }

        cout << "number of total scans:         " << n_scans << "\n";
        cout << "number of single source scans: " << n_single << "\n";
        cout << "number of subnetting scans:    " << n_subnetting << "\n";
        cout << "number of fillin mode scans:   " << n_fillin << "\n\n";

        cout << "number of scheduled baselines: " << n_bl << "\n";
        cout << ".–––––––––––";
        for (int i = 0; i < nsta-1; ++i) {
            cout << "––––––––––";
        }
        cout << "–––––––––––";
        cout << "––––––––––.\n";


        cout << boost::format("| %8s |") % "STATIONS";
        for (int i = 0; i < nsta; ++i) {
            cout << boost::format(" %8s ") % stations[i].getName();
        }
        cout << "|";
        cout << boost::format(" %8s ") % "TOTAL";
        cout << "|\n";

        cout << "|––––––––––|";
        for (int i = 0; i < nsta-1; ++i) {
            cout << "––––––––––";
        }
        cout << "––––––––––|";
        cout << "––––––––––|\n";

        vector<unsigned long> nbl_sta(nsta);
        for (int i = 0; i < nsta; ++i) {
            unsigned long counter = 0;
            cout << boost::format("| %8s |") % stations[i].getName();
            for (int j = 0; j < nsta; ++j) {
                if (j<i+1){
                    cout << "          ";
                    counter += bl_storage[j][i];
                }else{
                    cout << boost::format(" %8d ") % bl_storage[i][j];
                    counter += bl_storage[i][j];
                }
            }
            cout << "|";
            cout << boost::format(" %8d ") % counter;
            nbl_sta[i] = counter;
            cout << "|\n";
        }

        cout << "'–––––––––––";
        for (int i = 0; i < nsta-1; ++i) {
            cout << "––––––––––";
        }
        cout << "–––––––––––";
        cout << "––––––––––'\n\n";

        vector<unsigned int> nscan_sta(stations.size());
        vector< vector<unsigned int> > time_sta(stations.size());
        for (auto& any:scans){
            for (int ista = 0; ista < any.getNSta(); ++ista) {
                VLBI_pointingVector& pv =  any.getPointingVector(ista);
                int id = pv.getStaid();
                ++nscan_sta[id];
                time_sta[id].push_back(any.maxTime());
            }
        }

        cout << ".––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––.\n";
        cout << "|          time since session start (1 char equals 15 minutes)                                                           |\n";
        cout << "| STATION |0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23  | #SCANS #OBS |\n";
        cout << "|–––––––––|––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––|–––––––––––––|\n";
        for (int i = 0; i<stations.size(); ++i){
            cout << boost::format("| %8s|") % stations[i].getName();
            unsigned int timeStart = 0;
            unsigned int timeEnd = 900;
            for (int j = 0; j < 96; ++j) {
                if(any_of(time_sta[i].begin(), time_sta[i].end(), [timeEnd,timeStart](unsigned int k){return k<timeEnd && k>=timeStart;})){
                    cout << "x";
                }else{
                    cout << " ";
                }
                timeEnd += 900;
                timeStart += 900;
            }
            cout << boost::format("| %6d %4d |\n") %nscan_sta[i] %nbl_sta[i];
        }

        cout << "'––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––'\n\n";



        cout << "number of available sources:   " << sources.size() << "\n";
        vector<unsigned int> nscan_src(sources.size(),0);
        vector<unsigned int> nbl_src(sources.size(),0);
        vector< vector<unsigned int> > time_src(sources.size());

        for (const auto& any:scans){
            int id = any.getSourceId();
            ++nscan_src[id];
            nbl_src[id] += any.getNBl();
            time_src[id].push_back(any.maxTime());
        }
        long number = count_if(nscan_src.begin(), nscan_src.end(), [](int i) {return i > 0;});
        cout << "number of scheduled sources:   " << number << "\n";
        cout << ".––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––.\n";
        cout << "|          time since session start (1 char equals 15 minutes)                                                           |\n";
        cout << "|  SOURCE |0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23  | #SCANS #OBS |\n";
        cout << "|–––––––––|––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––|–––––––––––––|\n";
        for (int i = 0; i<sources.size(); ++i){
            cout << boost::format("| %8s|") % sources[i].getName();
            unsigned int timeStart = 0;
            unsigned int timeEnd = 900;
            for (int j = 0; j < 96; ++j) {
                if(any_of(time_src[i].begin(), time_src[i].end(), [timeEnd,timeStart](unsigned int k){return k<timeEnd && k>=timeStart;})){
                    cout << "x";
                }else{
                    cout << " ";
                }
                timeEnd += 900;
                timeStart += 900;
            }
            cout << boost::format("| %6d %4d |\n") %nscan_src[i] %nbl_src[i];
        }
        cout << "'––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––'\n";

        cout << "=========================   end statistics    =========================\n";
    }

}
