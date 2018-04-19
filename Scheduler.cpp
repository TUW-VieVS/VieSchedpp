/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Scheduler.cpp
 * Author: mschartn
 * 
 * Created on June 29, 2017, 2:29 PM
 */

#include "Scheduler.h"
using namespace std;
using namespace VieVS;
int Scheduler::nextId = 0;

Scheduler::Scheduler(Initializer &init, string name, string path): VieVS_NamedObject(move(name),nextId++),
                                                                   path_{std::move(path)},
                                                                   stations_{std::move(init.stations_)},
                                                                   sources_{std::move(init.sources_)},
                                                                   skyCoverages_{std::move(init.skyCoverages_)},
                                                                   xml_{init.xml_} {

    parameters_.subnetting = init.parameters_.subnetting;
    parameters_.fillinmode = init.parameters_.fillinmode;
    parameters_.fillinmodeInfluenceOnSchedule = init.parameters_.fillinmodeInfluenceOnSchedule;

    parameters_.andAsConditionCombination = init.parameters_.andAsConditionCombination;
    parameters_.minNumberOfSourcesToReduce = init.parameters_.minNumberOfSourcesToReduce;
    parameters_.maxNumberOfIterations = init.parameters_.maxNumberOfIterations;
    parameters_.numberOfGentleSourceReductions = init.parameters_.numberOfGentleSourceReductions;

    parameters_.writeSkyCoverageData = false;

    preCalculated_.subnettingSrcIds = std::move(init.preCalculated_.subnettingSrcIds);

    nSingleScansConsidered = 0;
    nSubnettingScansConsidered = 0;
    nFillinScansConsidered = 0;
}

void Scheduler::startScanSelection(unsigned int endTime, std::ofstream &bodyLog, Scan::ScanType type,
                                   boost::optional<FillinmodeEndposition> &endposition, int depth) {

    // Check if there is a required endposition. If yes change station availability with respect to endposition
    if(endposition.is_initialized()){
        changeStationAvailability(endposition,FillinmodeEndposition::change::start);
    }

    while (true) {
        // look if station is possible with respect to endposition
        if(endposition.is_initialized()){
            endposition->checkStationPossibility(stations_);
        }

        // create a subcon with all possible next scans
        Subcon subcon = createSubcon(parameters_.subnetting, type, endposition);
        consideredUpdate(subcon.getNumberSingleScans(), subcon.getNumberSubnettingScans(), depth, bodyLog);

        // select the best possible next scan(s) and save them under 'bestScans'
        boost::optional<unsigned long> bestIdx_opt = subcon.selectBest(stations_, sources_, skyCoverages_, endposition);

        // check if you have possible next scan
        if (!bestIdx_opt.is_initialized()) {
            if(depth == 0){
                // if there is no more possible scan at the outer most iteration, check 1minute later
                bodyLog << "ERROR! no valid scan found! Checking 1 minute later\n";
                for(auto &any:stations_){
                    PointingVector pv = any.getCurrentPointingVector();
                    pv.setTime(pv.getTime()+60);
                    any.setCurrentPointingVector(pv);
                }
                continue;
            }else{
                // if there is no more possible scan an inner iteration, break this iteration
                break;
            }
        }

        // update selected next possible scans
        unsigned long bestIdx = *bestIdx_opt;
        vector<Scan> bestScans;
        if (bestIdx < subcon.getNumberSingleScans()) {
            Scan& bestScan = subcon.referenceSingleSourceScan(bestIdx);
            bestScans.push_back(bestScan);
        } else {
            unsigned long thisIdx = bestIdx - subcon.getNumberSingleScans();
            pair<Scan, Scan>& bestScan_pair = subcon.referenceSubnettingScans(thisIdx);
            Scan &bestScan1 = bestScan_pair.first;
            Scan &bestScan2 = bestScan_pair.second;

            if (bestScan1.getTimes().getScanStart() > bestScan2.getTimes().getScanStart()) {
                swap(bestScan1, bestScan2);
            }
            bestScans.push_back(bestScan1);
            bestScans.push_back(bestScan2);
        }

        // check end time of best possible next scan
        unsigned int maxScanEnd = 0;
        for (const auto &any:bestScans) {
            if (any.getTimes().getScanEnd() > maxScanEnd) {
                maxScanEnd = any.getTimes().getScanEnd();
            }
        }

        // if end time of best possible next scans is greate than end time of scan selection stop
        if( maxScanEnd > endTime){
            break;
        }

        // check if end time triggers a new event
        bool hardBreak = checkForNewEvent(maxScanEnd, true, bodyLog);
        if (hardBreak) {
            continue;
        }

        // check if it is possible to start a fillin mode block, otherwise put best scans to schedule
        if (parameters_.fillinmode && !scans_.empty()) {
            boost::optional<FillinmodeEndposition> newEndposition(static_cast<int>(stations_.size()));
            if(endposition.is_initialized()){
                for(int i=0; i<stations_.size();++i){
                    if(endposition->hasEndposition(i)){
                        newEndposition->addPointingVectorAsEndposition(endposition->getFinalPosition(i).get());
                    }
                }
            }

            for(const auto&any:bestScans){
                for(int i=0; i<any.getNSta(); ++i){
                    int staid = any.getPointingVector(i).getStaid();
                    newEndposition->addPointingVectorAsEndposition(any.getPointingVector(i));
                }
            }

            // start recursion for fillin mode scans
            startScanSelection(endTime, bodyLog, Scan::ScanType::fillin, newEndposition, depth+1);
        }

        // update best possible scans
        for (const auto &bestScan : bestScans) {
            update(bestScan, bodyLog);
        }

        // update number of scan selections if it is a standard scan
        if(type == Scan::ScanType::standard){
            ++Scan::nScanSelections;
            if(Scan::scanSequence.customScanSequence){
                Scan::scanSequence.newScan();
            }
        }

        // check if you need to schedule a calibration block
        if(CalibratorBlock::scheduleCalibrationBlocks){
            switch(CalibratorBlock::cadenceUnit){
                case CalibratorBlock::CadenceUnit::scans:{
                    if(Scan::nScanSelections == CalibratorBlock::nextBlock){
                        startCalibrationBlock(bodyLog);
                        CalibratorBlock::nextBlock += CalibratorBlock::cadence;
                    }
                    break;
                }
                case CalibratorBlock::CadenceUnit::seconds:{
                    if(maxScanEnd >= CalibratorBlock::nextBlock){
                        startCalibrationBlock(bodyLog);
                        CalibratorBlock::nextBlock += CalibratorBlock::cadence;
                    }
                    break;
                }
            }
        }
    }

    // scan selection block is over. Change station availability back to start value
    if(endposition.is_initialized()){
        changeStationAvailability(endposition,FillinmodeEndposition::change::end);
    }

}

void Scheduler::start(ofstream &bodyLog) noexcept {

    unsigned int nsrc = countAvailableSources();
    if(parameters_.currentIteration>0){
        bodyLog << "Iteration number: " << parameters_.currentIteration << "\n";
    }
    bodyLog << "number of available sources: " << nsrc << "\n";

    outputHeader(stations_, bodyLog);

    boost::optional<FillinmodeEndposition> endposition = boost::none;
    startScanSelection(TimeSystem::duration,bodyLog,Scan::ScanType::standard, endposition, 0);

    sort(scans_.begin(),scans_.end(), [](const Scan &scan1, const Scan &scan2){
        return scan1.getTimes().getScanStart() < scan2.getTimes().getScanStart();
    });

    if (!check(bodyLog)) {
        cout << "ERROR: there was an error while checking the schedule (see log file)\n";
    }
    statistics(bodyLog);
    bodyLog.close();

    if(checkOptimizationConditions(bodyLog)){
        ++parameters_.currentIteration;
        ofstream newBodyLog(path_+getName()+"_iteration_"+to_string(parameters_.currentIteration)+".log");
        start(newBodyLog);
    }

//        if(PARA.writeSkyCoverageData){
//            saveSkyCoverageMain();
//            for (unsigned int i = 0; i < TimeSystem::duration; i+=30) {
//                saveSkyCoverageData(i);
//            }
//        }

}

void Scheduler::statistics(ofstream &log) {
    log << "summary:\n";
    log << "number of scheduled scans       " << scans_.size() << "\n";
    log << "considered single source scans: " << nSingleScansConsidered << "\n";
    log << "considered subnetting scans:    " << nSubnettingScansConsidered << "\n";
    log << "considered fillin mode scans:   " << nFillinScansConsidered << "\n";
    log << "total scans considered:         " << nSingleScansConsidered + 2 * nSubnettingScansConsidered + nFillinScansConsidered << "\n";
}

Subcon Scheduler::createSubcon(bool subnetting, Scan::ScanType type,
                               const boost::optional<FillinmodeEndposition> & endposition) noexcept {
    Subcon subcon = allVisibleScans(type);
    subcon.calcStartTimes(stations_, sources_, endposition);
    subcon.updateAzEl(stations_, sources_);
    subcon.constructAllBaselines(sources_);
    subcon.calcAllBaselineDurations(stations_, sources_);
    subcon.calcAllScanDurations(stations_, sources_, endposition);
    subcon.checkIfEnoughTimeToReachEndposition(stations_, sources_, endposition);

    if (subnetting) {
        subcon.createSubnettingScans(preCalculated_.subnettingSrcIds, static_cast<int>(stations_.size() * 0.66),
                                     sources_);
    }
    subcon.generateScore(stations_,sources_,skyCoverages_);
    return subcon;
}


Subcon Scheduler::allVisibleScans(Scan::ScanType type) noexcept {
    unsigned long nsta = stations_.size();
    unsigned long nsrc = sources_.size();

    unsigned int currentTime = 0;
    vector< unsigned int> lastScanLookup;
    for (auto &station : stations_) {
        unsigned int t = station.getCurrentTime();
        lastScanLookup.push_back(t);
        if (t > currentTime) {
            currentTime = t;
        }
    }

    Subcon subcon;

    for (int isrc=0; isrc<nsrc; ++isrc){
        Source &thisSource = sources_[isrc];

        if (!thisSource.getPARA().available || !thisSource.getPARA().globalAvailable) {
            continue;
        }

        if (type == Scan::ScanType::fillin && !thisSource.getPARA().availableForFillinmode) {
            continue;
        }

        if(type == Scan::ScanType::calibrator &&
                find(CalibratorBlock::calibratorSourceIds.begin(),CalibratorBlock::calibratorSourceIds.end(),thisSource.getId()) == CalibratorBlock::calibratorSourceIds.end()){
            continue;
        }

        if (thisSource.getNscans() > 0 &&
            currentTime - thisSource.lastScanTime() < thisSource.getPARA().minRepeat) {
            continue;
        }

        if (thisSource.getNscans() >= thisSource.getPARA().maxNumberOfScans) {
            continue;
        }

        unsigned int visibleSta = 0;
        vector<PointingVector> pointingVectors;
        vector<unsigned int> endOfLastScans;
        for (int ista=0; ista<nsta; ++ista){
            Station &thisSta = stations_[ista];

            if (!thisSta.getPARA().available || thisSta.getPARA().tagalong) {
                continue;
            }

            if (thisSta.getNScans() >= thisSta.getPARA().maxNumberOfScans){
                continue;
            }

            if (!thisSta.getPARA().ignoreSources.empty()) {
                auto &PARA = thisSta.getPARA();
                if (find(PARA.ignoreSources.begin(), PARA.ignoreSources.end(), isrc) != PARA.ignoreSources.end()) {
                    continue;
                }
            }

            if (!thisSource.getPARA().ignoreStations.empty()) {
                const auto &PARA = thisSource.getPARA();
                if (find(PARA.ignoreStations.begin(), PARA.ignoreStations.end(), ista) !=
                    PARA.ignoreStations.end()) {
                    continue;
                }
            }

            PointingVector p(ista,isrc);

            const Station::WaitTimes &wtimes = thisSta.getWaittimes();
            unsigned int time = lastScanLookup[ista] + wtimes.fieldSystem + wtimes.preob;

            p.setTime(time);

            thisSta.calcAzEl(thisSource, p);

            bool flag = thisSta.isVisible(p, thisSource.getPARA().minElevation);
            if (flag){
                visibleSta++;
                endOfLastScans.push_back(lastScanLookup[ista]);
                pointingVectors.push_back(p);
            }
        }
        if (visibleSta >= thisSource.getPARA().minNumberOfStations) {
            subcon.addScan(Scan(pointingVectors, endOfLastScans, type));
        }
    }

    return subcon;
}


void Scheduler::update(const Scan &scan, ofstream &bodyLog) noexcept {

    bool scanHasInfluence;
    scanHasInfluence = !(scan.getType() == Scan::ScanType::fillin && !parameters_.fillinmodeInfluenceOnSchedule);


    int srcid = scan.getSourceId();
    unsigned long nbl = scan.getNBl();

    for (int i = 0; i < scan.getNSta(); ++i) {
        const PointingVector &pv = scan.getPointingVector(i);
        int staid = pv.getStaid();
        const PointingVector &pv_end = scan.getPointingVectors_endtime(i);
        stations_[staid].update(nbl, pv, pv_end, scanHasInfluence);

        if(scanHasInfluence){
            int skyCoverageId = stations_[staid].getSkyCoverageID();
            skyCoverages_[skyCoverageId].update(pv, pv_end);
        }
    }

    unsigned int latestTime = scan.getTimes().getScanStart();
    Source &thisSource = sources_[srcid];
    thisSource.update(nbl, latestTime, scanHasInfluence);

    scans_.push_back(scan);
    scan.output(scans_.size(), stations_, thisSource, bodyLog);
}

void Scheduler::outputHeader(const vector<Station> &stations, ofstream &bodyLog) noexcept {
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

void Scheduler::consideredUpdate(unsigned long n1scans, unsigned long n2scans, int depth, ofstream &bodyLog) noexcept {

    bodyLog << "|-------------";
    for (int i = 0; i < stations_.size() - 1; ++i) {
        bodyLog << "-----------";
    }
    bodyLog << "----------| \n";

    bodyLog << "| depth: " << depth << " considered single Scans " << n1scans << " subnetting scans " << n2scans << ":\n";
    nSingleScansConsidered += n1scans;
    nSubnettingScansConsidered += n2scans;
}

bool Scheduler::check(ofstream &bodyLog) noexcept {
    bool everythingOk = true;

    bodyLog << "\n=======================   starting check routine   =======================\n";
    bodyLog << "starting check routine!\n";

    for (const auto& any:stations_){
        bodyLog << "    checking station " << any.getName() << ":\n";
        auto tmp = any.getAllScans();
        const vector<PointingVector> & start = tmp.first;
        const vector<PointingVector> & end = tmp.second;

        const Station::WaitTimes wtimes = any.getWaittimes();
        unsigned int constTimes = wtimes.fieldSystem + wtimes.preob;

        if (end.empty()) {
            continue;
            bodyLog << "    no Scans so far!\n";
        }

        for (int i = 0; i < end.size()-1; ++i) {
            const PointingVector &thisEnd = end[i];
            const PointingVector &nextStart = start[i+1];

            unsigned int slewtime = any.getAntenna().slewTime(thisEnd,nextStart);
            unsigned int min_neededTime = slewtime + constTimes;

            unsigned int thisEndTime = thisEnd.getTime();
            unsigned int nextStartTime = nextStart.getTime();

            if(nextStartTime<thisEndTime){
                bodyLog << "    ERROR: somthing went wrong!\n";
                bodyLog << "           start time of next scan is before end time of previouse scan!\n";
                boost::posix_time::ptime thisEndTime_ = TimeSystem::internalTime2PosixTime(thisEndTime);
                boost::posix_time::ptime nextStartTime_ = TimeSystem::internalTime2PosixTime(nextStartTime);

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
                boost::posix_time::ptime thisEndTime_ = TimeSystem::internalTime2PosixTime(thisEndTime);
                boost::posix_time::ptime nextStartTime_ = TimeSystem::internalTime2PosixTime(nextStartTime);
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

bool Scheduler::checkForNewEvent(unsigned int time, bool output, ofstream &bodyLog) noexcept {
    bool hard_break = false;

    for (auto &any:stations_){
        bool tagalong = any.checkForTagalongMode(time);
        if(tagalong){
            bodyLog << "TAGALONG for station " << any.getName() << " required!\n";
            startTagelongMode(any, bodyLog);
        }
    }

    for (auto &any:stations_) {
        any.checkForNewEvent(time, hard_break, bodyLog);
    }

    bool flag = false;
    for (auto &any:sources_) {
        bool flag2 = any.checkForNewEvent(time, hard_break, output, bodyLog);
        flag = flag || flag2;
    }
    if (flag) {
        unsigned int nsrc = countAvailableSources();
        bodyLog << "number of available sources: " << nsrc << "\n";

    }

    Baseline::checkForNewEvent(time, hard_break, output, bodyLog);
    return hard_break;
}

unsigned int Scheduler::countAvailableSources() noexcept {
    unsigned int counter = 0;
    for (const auto &any:sources_) {
        if (any.getPARA().available && any.getPARA().globalAvailable) {
            ++counter;
        }
    }
    return counter;
}

void Scheduler::saveSkyCoverageData(unsigned int time) noexcept {

    for (int i = 0; i < stations_.size(); ++i) {
        Station &thisStation = stations_[i];
        PointingVector pv(i,0);
        std::ofstream log("skyCoverageData/"+thisStation.getName()+".bin", std::ofstream::app | std::ofstream::out);
        unsigned int c=0;
        for (double el = 0; el < 90; el+=5) {
            double deltaAz = 5/cos(el*deg2rad);
            for (int az = 0; az < 360; az+=deltaAz) {
                pv.setAz(az*deg2rad);
                pv.setEl(el*deg2rad);
                pv.setTime(thisStation.getCurrentTime());

                int skyCoverageId = thisStation.getSkyCoverageID();
                SkyCoverage &thisSkyCoverage = skyCoverages_[skyCoverageId];
                double score = thisSkyCoverage.calcScore(vector<PointingVector>{pv},stations_);
                log << score << endl;
                ++c;
            }
        }
        log.close();
    }
}

void Scheduler::saveSkyCoverageMain() noexcept {
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

void Scheduler::displaySummaryOfStaticMembersForDebugging(ofstream &log) {

    WeightFactors::summary(log);

    log << "############################### STATIONS ###############################\n";
    for (auto &station : stations_) {
        log << station.getName() << ":\n";
        station.getPARA().output(log);
    }
    log << "############################### SOURCES ###############################\n";
    for (auto &source : sources_) {
        log << source.getName() << ":\n";
        source.getPARA().output(log);
    }

    Baseline::displaySummaryOfStaticMembersForDebugging(log);


}

void Scheduler::printHorizonMasksForDebugging() {
    for (const auto &any: stations_) {
        ofstream o("hmask_" + any.getName() + ".txt");
        PointingVector p(-1,-1);
        p.setTime(0);
        for (int az = -360; az < 0; ++az) {
            for (int el = 0; el < 20; ++el) {
                double thisAz = static_cast<double>(az) * deg2rad;
                double thisEl = static_cast<double>(el) * deg2rad;
                p.setAz(thisAz);
                p.setEl(thisEl);

                bool flag = true;
                if(any.hasHorizonMask()){
                    flag = any.getMask().visible(p);
                }
                o << flag << endl;
            }
        }
        o.close();
    }
}

void Scheduler::startCalibrationBlock(std::ofstream &bodyLog) {
    unsigned long nsta = stations_.size();
    vector<double> prevLowElevationScores(nsta,0);
    vector<double> prevHighElevationScores(nsta,0);

    vector<double> highestElevations(nsta,numeric_limits<double>::min());
    vector<double> lowestElevations(nsta,numeric_limits<double>::max());



    for (int i = 0; i < CalibratorBlock::nmaxScans; ++i) {

        Subcon subcon = createSubcon(parameters_.subnetting, Scan::ScanType::calibrator);
        consideredUpdate(subcon.getNumberSingleScans(), subcon.getNumberSubnettingScans(), 0, bodyLog);
        subcon.generateScore(prevLowElevationScores, prevHighElevationScores, static_cast<unsigned int>(nsta),
                             stations_, sources_);

        boost::optional<unsigned long> bestIdx_opt = subcon.rigorousScore(stations_,sources_,skyCoverages_, prevLowElevationScores, prevHighElevationScores );
        if (!bestIdx_opt) {
            bodyLog << "ERROR! no valid scan found! End of calibrator block!\n";
            break;
        }
        unsigned long bestIdx = *bestIdx_opt;
        vector<Scan> bestScans;
        if (bestIdx < subcon.getNumberSingleScans()) {
            Scan& bestScan = subcon.referenceSingleSourceScan(bestIdx);
            bestScans.push_back(bestScan);
        } else {
            unsigned long thisIdx = bestIdx - subcon.getNumberSingleScans();
            pair<Scan, Scan> &bestScan_pair = subcon.referenceSubnettingScans(thisIdx);
            Scan &bestScan1 = bestScan_pair.first;
            Scan &bestScan2 = bestScan_pair.second;

            if (bestScan1.getTimes().getScanStart() > bestScan2.getTimes().getScanStart()) {
                swap(bestScan1, bestScan2);
            }
            bestScans.push_back(bestScan1);
            bestScans.push_back(bestScan2);
        }

        // update prev low elevation scores
        for(const auto &scan:bestScans){
            double lowElevationSlopeStart = CalibratorBlock::lowElevationStartWeight;
            double lowElevationSlopeEnd = CalibratorBlock::lowElevationFullWeight;

            double highElevationSlopeStart = CalibratorBlock::highElevationStartWeight;
            double highElevationSlopeEnd = CalibratorBlock::highElevationFullWeight;

//            cout << "new Scan\n";
            for(int j = 0; j<scan.getNSta(); ++j){
                const PointingVector &pv = scan.getPointingVector(j);
                int staid = pv.getStaid();

                double el = pv.getEl();
//                cout << "el: " << el << "\n";
                double lowElScore;
                if(el>lowElevationSlopeStart) {
                    lowElScore = 0;
                }else if(el<lowElevationSlopeEnd) {
                    lowElScore = 1;
                } else {
                    lowElScore = (lowElevationSlopeStart-el)/(lowElevationSlopeStart-lowElevationSlopeEnd);
                }
//                cout << "lowElScore: " << lowElScore << " before " << prevLowElevationScores[staid] <<"\n";
                if(lowElScore>prevLowElevationScores[staid]){
                    prevLowElevationScores[staid] = lowElScore;
                }
                if(el<lowestElevations[staid]){
                    lowestElevations[staid] = el;
                }

                double highElScore;
                if(el<highElevationSlopeStart) {
                    highElScore = 0;
                }else if(el>highElevationSlopeEnd) {
                    highElScore = 1;
                } else {
                    highElScore = (el-highElevationSlopeStart)/(highElevationSlopeEnd-lowElevationSlopeStart);
                }
//                cout << "highElScore: " << highElScore << " before " << prevHighElevationScores[staid] << "\n";
                if(highElScore>prevHighElevationScores[staid]){
                    prevHighElevationScores[staid] = highElScore;
                }
                if(el>highestElevations[staid]){
                    highestElevations[staid] = el;
                }

            }

        }

        unsigned int all_maxTime = 0;
        for (const auto &any:bestScans) {
            if (any.getTimes().getScanEnd() > all_maxTime) {
                all_maxTime = any.getTimes().getScanEnd();
            }
        }


        bool hardBreak = checkForNewEvent(all_maxTime, true, bodyLog);
        if (hardBreak) {
            i -=1;
            continue;
        }

        if( all_maxTime > TimeSystem::duration){
            break;
        }


        if (parameters_.fillinmode && !scans_.empty()) {
//            start_fillinMode(bestScans, bodyLog);
        } else {
            for (const auto &bestScan : bestScans) {
                update(bestScan, bodyLog);
            }
        }


        bool moreScansNecessary = false;
        for(const auto &any:prevLowElevationScores){
            if(any<.5){
                moreScansNecessary = true;
                break;
            }
        }
        if(!moreScansNecessary){
            for(const auto &any:prevHighElevationScores){
                if(any<.5){
                    moreScansNecessary = true;
                    break;
                }
            }
        }

        if(!moreScansNecessary){
            break;
        }
    }

    bodyLog << "|=============";
    for (int i = 0; i < stations_.size() - 1; ++i) {
        bodyLog << "===========";
    }
    bodyLog << "==========| \n";
    bodyLog << "| CALIBRATOR BLOCK SUMMARY:\n";
    bodyLog << "|=============";
    for (int i = 0; i < stations_.size() - 1; ++i) {
        bodyLog << "===========";
    }
    bodyLog << "==========| \n";

    bodyLog << "| low el     |";
    for (const auto &any:lowestElevations) {
        if(any != numeric_limits<double>::max()){
            bodyLog << boost::format(" %8.2f |")% (any*rad2deg);
        }else {
            bodyLog << boost::format(" %8s |")% "";
        }
    }
    bodyLog << "\n";
    bodyLog << "| high el    |";
    for (const auto &any:highestElevations) {
        if(any != numeric_limits<double>::min()){
            bodyLog << boost::format(" %8.2f |")% (any*rad2deg);
        }else {
            bodyLog << boost::format(" %8s |")% "";
        }
    }
    bodyLog << "\n";

    bodyLog << "|=============";
    for (int i = 0; i < stations_.size() - 1; ++i) {
        bodyLog << "===========";
    }
    bodyLog << "==========| \n";



}

void Scheduler::startTagelongMode(Station &station, std::ofstream &bodyLog) {

    int staid = station.getId();

    bodyLog << "Start tagalong mode for station " << station.getName() << ": \n";

    // get wait times
    unsigned int stationConstTimes = station.getWaittimes().fieldSystem + station.getWaittimes().preob;

    // loop through all scans
    unsigned long counter = 0;
    for(auto & scan:scans_){
        ++counter;
        unsigned int scanStartTime = scan.getTimes().getScanStart();
        unsigned int currentStationTime = station.getCurrentTime();

        if(scan.getType() == Scan::ScanType::fillin){
            continue;
        }

        // look if this scan is possible for tagalong mode
        if (scanStartTime > currentStationTime){
            int srcid = scan.getSourceId();
            Source &source = sources_[scan.getSourceId()];

            PointingVector pv_new_start(staid,srcid);

            pv_new_start.setTime(scanStartTime);

            station.calcAzEl(source, pv_new_start);

            // check if source is up from station
            bool flag = station.isVisible(pv_new_start, source.getPARA().minElevation);
            if(!flag){
                continue;
            }

            station.getCableWrap().calcUnwrappedAz(station.getCurrentPointingVector(),pv_new_start);

            auto slewtime = station.slewTime(pv_new_start);
            if (!slewtime.is_initialized()) {
                continue;
            }

            // check if there is enough time to slew to source before scan starts
            if (scanStartTime < currentStationTime + *slewtime + stationConstTimes) {
                continue;
            }

            // loop through all other participating stations and prepare baselines
            vector<Baseline> bls;
            for (int i = 0; i < scan.getNSta(); ++i) {
                // create baseline
                const PointingVector & otherPv = scan.getPointingVector(i);

                int staid1 = staid;
                int staid2 = otherPv.getStaid();

                bool swapped = false;
                if(staid1>staid2){
                    swap(staid1,staid2);
                    swapped = true;
                }

                if(Baseline::PARA.ignore[staid1][staid2]){
                    continue;
                }
                if (!source.getPARA().ignoreBaselines.empty()) {
                    const auto &PARA = source.getPARA();
                    if (find(PARA.ignoreBaselines.begin(), PARA.ignoreBaselines.end(), make_pair(staid1, staid2)) !=
                        PARA.ignoreBaselines.end()) {
                        continue;
                    }

                }
                Baseline bl(srcid, staid1, staid2, scanStartTime);

                // calc baseline scan length
                double date1 = 2400000.5;
                double date2 = TimeSystem::mjdStart + static_cast<double>(scanStartTime) / 86400.0;
                double gmst = iauGmst82(date1, date2);

                unsigned int maxScanDuration = 0;
                if(source.getPARA().fixedScanDuration.is_initialized()){
                    maxScanDuration = *source.getPARA().fixedScanDuration;
                }else {

                    for (auto &band:ObservationMode::bands) {

                        double SEFD_src = source.observedFlux(band, gmst, stations_[staid1].dx(staid2),
                                                              stations_[staid1].dy(staid2),
                                                              stations_[staid1].dz(staid2));

                        double el1;
                        if (!swapped) {
                            el1 = pv_new_start.getEl();
                        } else {
                            el1 = otherPv.getEl();
                        }
                        double SEFD_sta1 = stations_[staid1].getEquip().getSEFD(band, el1);

                        double el2;
                        if (!swapped) {
                            el2 = otherPv.getEl();
                        } else {
                            el2 = pv_new_start.getEl();
                        }
                        double SEFD_sta2 = stations_[staid2].getEquip().getSEFD(band, el2);

                        double minSNR_sta1 = stations_[staid1].getPARA().minSNR.at(band);
                        double minSNR_sta2 = stations_[staid2].getPARA().minSNR.at(band);

                        double minSNR_bl = Baseline::PARA.minSNR[band][staid1][staid2];

                        double minSNR_src = source.getPARA().minSNR.at(band);

                        double maxminSNR = minSNR_src;
                        if (minSNR_sta1 > maxminSNR) {
                            maxminSNR = minSNR_sta1;
                        }
                        if (minSNR_sta2 > maxminSNR) {
                            maxminSNR = minSNR_sta2;
                        }
                        if (minSNR_bl > maxminSNR) {
                            maxminSNR = minSNR_bl;
                        }

                        double maxCorSynch1 = stations_[staid1].getWaittimes().midob;
                        double maxCorSynch = maxCorSynch1;
                        double maxCorSynch2 = stations_[staid2].getWaittimes().midob;
                        if (maxCorSynch2 > maxCorSynch) {
                            maxCorSynch = maxCorSynch2;
                        }

                        double anum = (1.75 * maxminSNR / SEFD_src);
                        double anu1 = SEFD_sta1 * SEFD_sta2;
                        double anu2 = ObservationMode::sampleRate * 1.0e6 * ObservationMode::nChannels[band] *
                                      ObservationMode::bits;

                        double new_duration = anum * anum * anu1 / anu2 + maxCorSynch;
                        new_duration = ceil(new_duration);
                        auto new_duration_uint = static_cast<unsigned int>(new_duration);

                        unsigned int minScanBl = Baseline::PARA.minScan[staid1][staid2];
                        if (new_duration_uint < minScanBl) {
                            new_duration_uint = minScanBl;
                        }
                        unsigned int maxScanBl = Baseline::PARA.maxScan[staid1][staid2];

                        if (new_duration_uint > maxScanBl) {
                            continue;
                        }

                        if (new_duration_uint > maxScanDuration) {
                            maxScanDuration = new_duration_uint;
                        }

                        unsigned int maxScanTime =
                                scan.getPointingVectors_endtime(i).getTime() - scan.getPointingVector(i).getTime();

                        if (maxScanDuration > maxScanTime) {
                            maxScanDuration = maxScanTime;
                        }
                    }
                }
                bl.setScanDuration(maxScanDuration);
                bls.push_back(bl);
            }
            if(bls.empty()){
                continue;
            }

            unsigned int maxBl = 0;
            for (const auto &any:bls){
                if(any.getScanDuration()>maxBl){
                    maxBl = any.getScanDuration();
                }
            }

            // check if source is still visible at end of scan... with same cable wrap
            PointingVector pv_new_end(staid,srcid);

            pv_new_end.setTime(scanStartTime+maxBl);

            station.calcAzEl(source, pv_new_end);

            // check if source is up from station
            flag = station.isVisible(pv_new_end, source.getPARA().minElevation);
            if(!flag){
                continue;
            }

            station.getCableWrap().calcUnwrappedAz(pv_new_start,pv_new_end);
            if(abs(pv_new_end.getAz() - pv_new_start.getAz()) > halfpi){
                continue;
            }

            scan.addTagalongStation(pv_new_start, pv_new_end, bls, *slewtime, station);
            bodyLog << boost::format("possible to observe source: %-8s (scan: %4d) scan start: %5d scan end: %5d \n")
                       %source.getName() %counter %pv_new_start.getTime() %pv_new_end.getTime();
            if(station.referencePARA().firstScan){
                station.referencePARA().firstScan = false;
            }
            station.setCurrentPointingVector(pv_new_end);
            station.addPointingVectorStart(pv_new_start);
            station.addPointingVectorEnd(pv_new_end);
        }
    }

//    station.applyNextEvent(bodyLog);
}

bool Scheduler::checkOptimizationConditions(ofstream &of) {
    bool newScheduleNecessary = false;
    vector<int> excludedSources;
    int excludedScans = 0;
    int excludedBaselines = 0;
    of << "checking optimization conditions... ";
    int consideredSources = 0;
    bool lastExcluded = false;
    for(int i=0; i<sources_.size(); ++i){
        auto &thisSource = sources_[i];

        if(!thisSource.getPARA().globalAvailable){
            continue;
        }
        ++consideredSources;
        bool scansValid = true;
        if(thisSource.getNTotalScans()<thisSource.getOptimization().minNumScans){
            scansValid = false;
        }

        bool baselinesValid = true;
        if(thisSource.getNbls()<thisSource.getOptimization().minNumBaselines) {
            baselinesValid = false;
        }

        bool exclude;
        if(parameters_.andAsConditionCombination){
            exclude = !(scansValid && baselinesValid);
        }else{
            exclude = !(scansValid || baselinesValid);
        }

        if(exclude){
            if(parameters_.currentIteration <= parameters_.numberOfGentleSourceReductions){
                if(lastExcluded){
                    lastExcluded = false;
                    continue;
                }else {
                    lastExcluded = true;
                }
            }
            excludedScans += thisSource.getNTotalScans();
            excludedBaselines += thisSource.getNbls();
            excludedSources.push_back(i);
            newScheduleNecessary = true;
            thisSource.referencePARA().setGlobalAvailable(false);
        }
    }
    if(parameters_.currentIteration>parameters_.maxNumberOfIterations){
        newScheduleNecessary = false;
        of << "max number of iterations reached ";
    }
    if(excludedSources.size() < parameters_.minNumberOfSourcesToReduce){
        newScheduleNecessary = false;
        of << "only " << excludedSources.size() <<
           " sources have to be excluded (minimum = " << parameters_.minNumberOfSourcesToReduce << ") ";
    }

    if(newScheduleNecessary && excludedScans>0){
        of << "new schedule with reduced source list necessary\n";
        CalibratorBlock::nextBlock = 0;
        unsigned long sourcesLeft = consideredSources - excludedSources.size();
        of << "==========================================================================================\n";
        if(sourcesLeft<50){
            of << boost::format("Abortion: only %d sources left!\n")%sourcesLeft;
            return false;
        }
        of << boost::format("creating new schedule with %d sources\n")%sourcesLeft;
        of << "==========================================================================================\n";
        of << "List of removed sources:\n";
        for(int i=0; i<excludedSources.size(); ++i){
            of << boost::format("%8s ")%sources_[excludedSources[i]].getName();
            if(i !=0 && i%8==0 && i!=excludedSources.size()-1){
                of << "\n";
            }
        }
        of << "\n";

        scans_.clear();
        for(auto &any:stations_){
            any.clearObservations();
        }
        for(auto &any:sources_){
            any.clearObservations();
        }
        for(auto &any:skyCoverages_){
            any.clearObservations();
        }
        bool dummy = false;
        vector<vector<unsigned int> > nextEvent(stations_.size(), vector<unsigned int>(stations_.size(), 0));
        Baseline::nextEvent = nextEvent;
        Baseline::checkForNewEvent(0,dummy,0,of);


    }else{
        of << "no new schedule needed!\n";
        newScheduleNecessary = false;
    }
    return newScheduleNecessary;
}

void Scheduler::changeStationAvailability(const boost::optional<FillinmodeEndposition> &endposition,
                                          FillinmodeEndposition::change change) {
    switch (change){
        case FillinmodeEndposition::change::start:{
            for(int i=0; i<stations_.size(); ++i){
                stations_[i].referencePARA().available = endposition->getStationPossible(i);
            }
            break;
        }
        case FillinmodeEndposition::change::end:{
            for(int i=0; i<stations_.size(); ++i){
                stations_[i].referencePARA().available = endposition->getStationAvailable(i);
            }
            break;
        }
    }
}


