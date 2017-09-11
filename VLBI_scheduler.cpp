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
            stations{std::move(init.getStations())}, sources{std::move(init.getSources())},
            skyCoverages{std::move(init.getSkyCoverages())} {

        const VLBI_initializer::PARAMETERS &IPARA = init.getPARA();

        PARA.subnetting = IPARA.subnetting;
        PARA.fillinmode = IPARA.fillinmode;
        PARA.writeSkyCoverageData = false;

        const VLBI_initializer::PRECALC &IPRE = init.getPRE();
        PRE.subnettingSrcIds = IPRE.subnettingSrcIds;

        considered_n1scans = 0;
        considered_n2scans = 0;
        considered_fillin = 0;
    }
    
    VLBI_scheduler::~VLBI_scheduler() {
    }

    void VLBI_scheduler::start(ofstream &bodyLog) noexcept {

        unsigned int nsrc = countAvailableSources();
        bodyLog << "number of available sources: " << nsrc << "\n";

        outputHeader(stations, bodyLog);

//        displaySummaryOfStaticMembersForDebugging(bodyLog);
//        printHorizonMasksForDebugging();

        while (true) {
            VLBI_subcon subcon = createSubcon(PARA.subnetting);
            consideredUpdate(subcon.getNumberSingleScans(), subcon.getNumberSubnettingScans(), bodyLog);

            boost::optional<unsigned long> bestIdx_opt = subcon.rigorousScore(stations, sources, skyCoverages);
            if (!bestIdx_opt) {
                bodyLog << "ERROR! no valid scan found!\n";
                continue;
            }
            unsigned long bestIdx = *bestIdx_opt;
            vector<VLBI_scan> bestScans;
            if (bestIdx < subcon.getNumberSingleScans()) {
                VLBI_scan& bestScan = subcon.referenceSingleSourceScan(bestIdx);
                bestScans.push_back(bestScan);
            } else {
                unsigned long thisIdx = bestIdx - subcon.getNumberSingleScans();
                pair<VLBI_scan, VLBI_scan> &bestScan_pair = subcon.referenceDoubleSourceScan(thisIdx);
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

            unsigned int all_maxTime = 0;
            for (const auto &any:bestScans) {
                if (any.maxTime() > all_maxTime) {
                    all_maxTime = any.maxTime();
                }
            }


            bool hardBreak = checkForNewEvent(all_maxTime, true, bodyLog);
            if (hardBreak) {
                continue;
            }

            if (PARA.fillinmode && !scans.empty()) {
                start_fillinMode(bestScans, bodyLog);
            } else {
                for (const auto &bestScan : bestScans) {
                    update(bestScan, bodyLog);
                }
            }

//            bool flag = check(bodyLog);
        }


        bodyLog << "TOTAL SUMMARY:\n";
        bodyLog << "created single source scans:      " << considered_n1scans << "\n";
        bodyLog << "created subnetting scans:  " << considered_n2scans << "\n";
        bodyLog << "created fillin mode scans:  " << considered_fillin << "\n";
        bodyLog << "total scans considered: " << considered_n1scans + 2 * considered_n2scans + considered_fillin
                << "\n";

//        if(PARA.writeSkyCoverageData){
//            saveSkyCoverageMain();
//            for (unsigned int i = 0; i < VieVS_time::duration; i+=30) {
//                saveSkyCoverageData(i);
//            }
//        }

        bool everythingOk = check(bodyLog);
        if (!everythingOk) {
            cout
                    << "########################################################### ERROR ######################################################\n";
        }

    }

    VLBI_subcon VLBI_scheduler::createSubcon(bool subnetting) noexcept {
        VLBI_subcon subcon = allVisibleScans();
        subcon.calcStartTimes(stations, sources);
        subcon.updateAzEl(stations, sources);
        subcon.constructAllBaselines(sources);
        subcon.calcAllBaselineDurations(stations, sources);
        subcon.calcAllScanDurations(stations, sources);
        if (subnetting) {
            subcon.createSubcon2(PRE.subnettingSrcIds, (int) (stations.size() * 0.66));
        }
        subcon.precalcScore(stations, sources);
        subcon.generateScore(stations, skyCoverages, sources.size());
        return subcon;
    }


    VLBI_subcon VLBI_scheduler::allVisibleScans() noexcept {
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

            if (!thisSource.isAvailable()) {
                continue;
            }

            if (thisSource.lastScanTime() != 0 &&
                currentTime - thisSource.lastScanTime() < thisSource.minRepeatTime()) {
                continue;
            }

            if (thisSource.getNscans() >= thisSource.getMaxNumberOfScans()) {
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

                if (!thisSta.getPARA().ignoreSources.empty()) {
                    auto &PARA = thisSta.getPARA();
                    if (find(PARA.ignoreSources.begin(), PARA.ignoreSources.end(), isrc) != PARA.ignoreSources.end()) {
                        continue;
                    }
                }

                if (!thisSource.getPARA().ignoreStations.empty()) {
                    auto &PARA = thisSource.getPARA();
                    if (find(PARA.ignoreStations.begin(), PARA.ignoreStations.end(), ista) !=
                        PARA.ignoreStations.end()) {
                        continue;
                    }
                }

                VLBI_pointingVector p(ista,isrc);

                unsigned int time = lastScanLookup[ista] + thisSta.getWaitSetup() + thisSta.getWaitSource() +
                                    thisSta.getWaitTape() + thisSta.getWaitCalibration();
                p.setTime(time);

                thisSta.calcAzEl(thisSource, p);

                bool flag = thisSta.isVisible(p);
                if (flag){
                    visibleSta++;
                    endOfLastScans.push_back(lastScanLookup[ista]);
                    pointingVectors.push_back(std::move(p));
                }
            }
            if (visibleSta >= thisSource.getMinNumberOfStations()) {
                subcon.addScan(VLBI_scan(pointingVectors, endOfLastScans, thisSource.getMinNumberOfStations(),
                                         VLBI_scan::scanType::single));
            }
        }
        
        return subcon;
    }


    void VLBI_scheduler::update(const VLBI_scan &scan, ofstream &bodyLog) noexcept {

        int srcid = scan.getSourceId();
        string sourceName = sources[srcid].getName();
        unsigned long nbl = scan.getNBl();


        for (int i = 0; i < scan.getNSta(); ++i) {
            const VLBI_pointingVector &pv = scan.getPointingVector(i);
            int staid = pv.getStaid();
            const VLBI_pointingVector &pv_end = scan.getPointingVectors_endtime(i);
            vector<unsigned int> times = scan.getTimes().stationTimes(i);
            stations[staid].update(nbl, pv, pv_end, times, sourceName);


            int skyCoverageId = stations[staid].getSkyCoverageID();
            skyCoverages[skyCoverageId].update(pv, pv_end);
        }

        unsigned int latestTime = scan.maxTime();
        VLBI_source &thisSource = sources[srcid];
        thisSource.update(nbl, latestTime);

        scans.push_back(scan);
        scan.output(scans.size(), stations, thisSource, bodyLog);
    }

    void VLBI_scheduler::outputHeader(const vector<VLBI_station> &stations, ofstream &bodyLog) noexcept {
        bodyLog << ".------------.";
        for (auto &t:stations) {
            bodyLog << "----------.";
        }
        bodyLog << "\n";
        bodyLog << "| stations   | ";
        for (auto &t:stations) {
            bodyLog << boost::format("%8s | ") % t.getName();
        }
        bodyLog << "\n";
    }

    void VLBI_scheduler::consideredUpdate(unsigned long n1scans, bool created, ofstream &bodyLog) noexcept {
        if(created){
            bodyLog << "|     created new fillin Scans " << n1scans << " \n";
        }else{
            bodyLog << "|     considered possible fillin Scans " << n1scans << ": \n";
        }
        considered_fillin += n1scans;
    }

    void VLBI_scheduler::consideredUpdate(unsigned long n1scans, unsigned long n2scans, ofstream &bodyLog) noexcept {

        bodyLog << "|-------------";
        for (int i = 0; i < stations.size() - 1; ++i) {
            bodyLog << "-----------";
        }
        bodyLog << "----------| \n";

        bodyLog << "| considered single Scans " << n1scans << " subnetting scans " << n2scans << ":\n";
        considered_n1scans += n1scans;
        considered_n2scans += n2scans;
    }


    boost::optional<VLBI_scan>
    VLBI_scheduler::fillin_scan(VLBI_subcon &subcon, const VLBI_fillin_endpositions &fi_endp,
                                const vector<int> &sourceWillBeScanned, ofstream &bodyLog) noexcept {
        VLBI_subcon fillin_subcon;

        int i = 0;
        while (i < subcon.getNumberSingleScans()) {
            VLBI_scan &thisScan = subcon.referenceSingleSourceScan(i);

            int srcid = thisScan.getSourceId();
            if (std::find(sourceWillBeScanned.begin(), sourceWillBeScanned.end(), srcid) != sourceWillBeScanned.end()) {
                subcon.removeScan(i);
                continue;
            }
            const VLBI_source &thisSource = sources[srcid];
            bool flag = thisScan.possibleFillinScan(stations, thisSource, fi_endp.getStationUnused(),
                                                    fi_endp.getFinalPosition());

            if (flag) {
                thisScan.setType(VLBI_scan::scanType::fillin);
                fillin_subcon.addScan(thisScan);
                ++i;
            } else {
                subcon.removeScan(i);
            }


//            boost::optional<VLBI_scan> this_fillinScan = thisScan.possibleFillinScan(stations, thisSource,
//                                                                                     fi_endp.getStationPossible(),
//                                                                                     fi_endp.getStationUnused(),
//                                                                                     fi_endp.getFinalPosition());
//            if (this_fillinScan) {
//                this_fillinScan->setType(VLBI_scan::scanType::fillin);
//                fillin_subcon.addScan(move(*this_fillinScan));
//            }
        }
        if (fillin_subcon.getNumberSingleScans() == 0) {
            return boost::none;
        } else {
            consideredUpdate(fillin_subcon.getNumberSingleScans(), false, bodyLog);

            fillin_subcon.precalcScore(stations, sources);
            fillin_subcon.generateScore(stations, skyCoverages, sources.size());

            while (true) {
                boost::optional<unsigned long> bestIdx = fillin_subcon.rigorousScore(stations, sources, skyCoverages);
                if (bestIdx) {
                    VLBI_scan &fillinScan = fillin_subcon.referenceSingleSourceScan(*bestIdx);

                    const VLBI_source &thisSource = sources[fillinScan.getSourceId()];

                    unsigned int nstaBefore = fillinScan.getNSta();
                    bool flag = fillinScan.possibleFillinScan(stations, thisSource, fi_endp.getStationUnused(),
                                                              fi_endp.getFinalPosition());

                    if (flag && nstaBefore == fillinScan.getNSta()) {
                        return fillinScan;
                    } else {
                        fillin_subcon.removeScan(*bestIdx);
                    }

//                    boost::optional<VLBI_scan> updatedFillinScan = fillinScan.possibleFillinScan(stations, thisSource,
//                                                                                                 fi_endp.getStationPossible(),
//                                                                                                 fi_endp.getStationUnused(),
//                                                                                                 fi_endp.getFinalPosition());
//                    if (updatedFillinScan && fillinScan.getNSta() == updatedFillinScan->getNSta()) {
//                        return fillinScan;
//                    } else {
//                        fillin_subcon.removeScan(*bestIdx);
//                    }
                } else {
                    return boost::none;
                }
            }
        }
    }

    void
    VLBI_scheduler::start_fillinMode(vector<VLBI_scan> &bestScans, ofstream &bodyLog) noexcept {

        VLBI_fillin_endpositions fi_endp(bestScans, stations);
        if (fi_endp.getNumberOfPossibleStations() < 2) {
            for (int i = 0; i < bestScans.size(); ++i) {
                update(bestScans[i], bodyLog);
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

        const std::vector<char> &stationPossible = fi_endp.getStationPossible();
        for (int i = 0; i < stations.size(); ++i) {
            stations[i].setAvailable(stationPossible[i]);
        }
        VLBI_subcon subcon = createSubcon(false);
        consideredUpdate(subcon.getNumberSingleScans(), true, bodyLog);

        while (bestScans.size() > 0) {
            if (fi_endp.getNumberOfPossibleStations() >= 2) {
                while (true) {
                    boost::optional<VLBI_scan> fillinScan = fillin_scan(subcon, fi_endp, sourceWillBeScanned,
                                                                        bodyLog);
                    if (fillinScan) {
                        sourceWillBeScanned.push_back(fillinScan->getSourceId());
                        bestScans.push_back(*fillinScan);
                    } else {
                        break;
                    }

                    fi_endp = VLBI_fillin_endpositions(bestScans, stations);
                    if (fi_endp.getNumberOfPossibleStations() < 2) {
                        break;
                    }
                }
            }

            VLBI_scan &thisBestScan = bestScans.back();
            update(thisBestScan, bodyLog);
            bestScans.pop_back();


            fi_endp = VLBI_fillin_endpositions(bestScans, stations);
            const std::vector<char> &stationPossible = fi_endp.getStationPossible();
            for (int i = 0; i < stations.size(); ++i) {
                stations[i].setAvailable(stationPossible[i]);
            }
            subcon = createSubcon(false);
            consideredUpdate(subcon.getNumberSingleScans(), true, bodyLog);
        }

        for (int i = 0; i < stations.size(); ++i) {
            stations[i].setAvailable(stationAvailable[i]);
        }
    }

    bool VLBI_scheduler::endOfSessionReached(const vector<VLBI_scan> &bestScans) const noexcept {
        bool finished = false;
        for (int i = 0; i < bestScans.size(); ++i) {
            const VLBI_scan &thisScan = bestScans[i];
            if (thisScan.maxTime() > VieVS_time::duration) {
                finished = true;
            }
        }
        return finished;
    }

    bool VLBI_scheduler::check(ofstream &bodyLog) noexcept {
        bool everythingOk = true;

        bodyLog << "\n=======================   starting check routine   =======================\n";
        bodyLog << "starting check routine!\n";

        for (const auto& any:stations){
            bodyLog << "    checking station " << any.getName() << ":\n";
            auto tmp = any.getAllScans();
            const vector<VLBI_pointingVector> & start = tmp.first;
            const vector<VLBI_pointingVector> & end = tmp.second;

            unsigned int constTimes = any.getWaitSetup() + any.getWaitSource() + any.getWaitCalibration() + any.getWaitTape();

            if (end.size() == 0) {
                continue;
                bodyLog << "    no Scans so far!\n";
            }

            for (int i = 0; i < end.size()-1; ++i) {
                const VLBI_pointingVector &thisEnd = end[i];
                const VLBI_pointingVector &nextStart = start[i+1];

                unsigned int slewtime = any.getAntenna().slewTime(thisEnd,nextStart);
                unsigned int min_neededTime = slewtime + constTimes;

                unsigned int thisEndTime = thisEnd.getTime();
                unsigned int nextStartTime = nextStart.getTime();

                if(nextStartTime<thisEndTime){
                    bodyLog << "    ERROR: somthing went wrong!\n";
                    bodyLog << "           start time of next scan is before end time of previouse scan!\n";
                    boost::posix_time::ptime thisEndTime_ =
                            VieVS_time::startTime + boost::posix_time::seconds(thisEndTime);
                    boost::posix_time::ptime nextStartTime_ =
                            VieVS_time::startTime + boost::posix_time::seconds(nextStartTime);
                    bodyLog << "           end time of previouse scan: " << thisEndTime_.time_of_day() << "("
                            << thisEndTime << ")\n";
                    bodyLog << "           start time of next scan:    " << nextStartTime_.time_of_day() << "("
                            << nextStartTime << "\n)";
                    everythingOk = false;
                    continue;
                }
                unsigned int availableTime = nextStartTime-thisEndTime;

                if(availableTime+1<min_neededTime){
                    bodyLog << "    ERROR: somthing went wrong!\n";
                    bodyLog << "           not enough available time for slewing!\n";
                    boost::posix_time::ptime thisEndTime_ =
                            VieVS_time::startTime + boost::posix_time::seconds(thisEndTime);
                    boost::posix_time::ptime nextStartTime_ =
                            VieVS_time::startTime + boost::posix_time::seconds(nextStartTime);
                    bodyLog << "               end time of previouse scan: " << thisEndTime_.time_of_day() << " ("
                            << thisEndTime << ")\n";
                    bodyLog << "               start time of next scan:    " << nextStartTime_.time_of_day() << " ("
                            << nextStartTime << ")\n";
                    bodyLog << "           available time: " << availableTime << "\n";
                    bodyLog << "               needed slew time:           " << slewtime << "\n";
                    bodyLog << "               needed constant times:      " << constTimes << "\n";
                    bodyLog << "           needed time:    " << min_neededTime << "\n";
                    bodyLog << "           difference:     " << (long) availableTime - (long) min_neededTime << "\n";
                    everythingOk = false;
                }

            }
            bodyLog << "    finished!\n";
        }
        bodyLog << "=========================   end check routine    =========================\n";
        return everythingOk;
    }

    bool VLBI_scheduler::checkForNewEvent(unsigned int time, bool output, ofstream &bodyLog) noexcept {
        bool hard_break = false;

        for (auto &any:stations) {
            any.checkForNewEvent(time, hard_break, output, bodyLog);
        }

        bool flag = false;
        for (auto &any:sources) {
            flag = flag || any.checkForNewEvent(time, hard_break, output, bodyLog);
        }
        if (flag) {
            unsigned int nsrc = countAvailableSources();
            bodyLog << "number of available sources: " << nsrc << "\n";

        }

        VLBI_baseline::checkForNewEvent(time, hard_break, output, bodyLog);
        return hard_break;
    }

    unsigned int VLBI_scheduler::countAvailableSources() noexcept {
        unsigned int counter = 0;
        for (const auto &any:sources) {
            if (any.isAvailable()) {
                ++counter;
            }
        }
        return counter;
    }

    void VLBI_scheduler::saveSkyCoverageData(unsigned int time) noexcept {

        for (int i = 0; i < stations.size(); ++i) {
            VLBI_station &thisStation = stations[i];
            VLBI_pointingVector pv(i,0);
            std::ofstream log("skyCoverageData/"+thisStation.getName()+".bin", std::ofstream::app | std::ofstream::out);
            unsigned int c=0;
            for (double el = 0; el < 90; el+=5) {
                double deltaAz = 5/cos(el*deg2rad);
                for (int az = 0; az < 360; az+=deltaAz) {
                    pv.setAz(az*deg2rad);
                    pv.setEl(el*deg2rad);
                    pv.setTime(thisStation.getCurrentTime());

                    int skyCoverageId = thisStation.getSkyCoverageID();
                    VLBI_skyCoverage &thisSkyCoverage = skyCoverages[skyCoverageId];
                    double score = thisSkyCoverage.calcScore(vector<VLBI_pointingVector>{pv},stations);
                    log << score << endl;
                    ++c;
                }
            }
            log.close();
        }
    }

    void VLBI_scheduler::saveSkyCoverageMain() noexcept {
        std::ofstream az_file("skyCoverageData/az.bin", std::ofstream::out);
        std::ofstream el_file("skyCoverageData/el.bin", std::ofstream::out);


        for (double el = 0; el < 90; el += 5) {
            double deltaAz = 5 / cos(el * deg2rad);
            for (int az = 0; az < 360; az += deltaAz) {
                az_file << az << endl;
                el_file << el << endl;
            }
        }
        az_file.close();
        el_file.close();
    }

    void VLBI_scheduler::displaySummaryOfStaticMembersForDebugging(ofstream &log) {

        VLBI_weightFactors::summary(log);

        log << "############################### STATIONS ###############################\n";
        for (int i = 0; i < stations.size(); ++i) {
            log << stations[i].getName() << ":\n";
            stations[i].getPARA().output(log);
        }
        log << "############################### SOURCES ###############################\n";
        for (int i = 0; i < sources.size(); ++i) {
            log << sources[i].getName() << ":\n";
            sources[i].getPARA().output(log);
        }

        VLBI_baseline::displaySummaryOfStaticMembersForDebugging(log);


    }

    void VLBI_scheduler::printHorizonMasksForDebugging() {
        for (const auto &any: stations) {
            ofstream o("hmask_" + any.getName() + ".txt");
            VLBI_pointingVector p;
            p.setTime(0);
            for (int az = -360; az < 0; ++az) {
                for (int el = 0; el < 20; ++el) {
                    double thisAz = (double) az * deg2rad;
                    double thisEl = (double) el * deg2rad;
                    p.setAz(thisAz);
                    p.setEl(thisEl);

                    bool flag = any.getMask().visible(p);
                    o << flag << endl;
                }
            }
            o.close();
        }
    }

}
