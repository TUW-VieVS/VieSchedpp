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

Scheduler::Scheduler(Initializer &init, string path, string name, int version): VieVS_NamedObject(move(name),nextId++),
                                                                   path_{std::move(path)},
                                                                   version_{version},
                                                                   stations_{std::move(init.stations_)},
                                                                   sources_{std::move(init.sources_)},
                                                                   skyCoverages_{std::move(init.skyCoverages_)},
                                                                   xml_{init.xml_} {

    if(init.parameters_.subnetting){
        Subnetting subnetting;
        subnetting.subnettingMinNSta = static_cast<unsigned int>(stations_.size() * init.parameters_.subnettingMinNSta);
        subnetting.subnettingSrcIds = std::move(init.preCalculated_.subnettingSrcIds);
        parameters_.subnetting = move(subnetting);
    }

    parameters_.fillinmodeDuringScanSelection = init.parameters_.fillinmodeDuringScanSelection;
    parameters_.fillinmodeInfluenceOnSchedule = init.parameters_.fillinmodeInfluenceOnSchedule;
    parameters_.fillinmodeAPosteriori = init.parameters_.fillinmodeAPosteriori;

    parameters_.andAsConditionCombination = init.parameters_.andAsConditionCombination;
    parameters_.minNumberOfSourcesToReduce = init.parameters_.minNumberOfSourcesToReduce;
    parameters_.maxNumberOfIterations = init.parameters_.maxNumberOfIterations;
    parameters_.numberOfGentleSourceReductions = init.parameters_.numberOfGentleSourceReductions;

    parameters_.writeSkyCoverageData = false;
}

Scheduler::Scheduler(std::string name, std::vector<Station> stations, std::vector<Source> sources,
                     std::vector<SkyCoverage> skyCoverages, std::vector<Scan> scans, boost::property_tree::ptree xml):
                                                                            VieVS_NamedObject(move(name),nextId++),
                                                                            stations_{std::move(stations)},
                                                                            sources_{std::move(sources)},
                                                                            skyCoverages_{std::move(skyCoverages)},
                                                                            scans_{std::move(scans)},
                                                                            xml_{std::move(xml)}{

}

void Scheduler::startScanSelection(unsigned int endTime, std::ofstream &bodyLog, Scan::ScanType type,
                                   boost::optional<StationEndposition> &opt_endposition,
                                   boost::optional<Subcon> &opt_subcon, int depth) {

    // Check if there is a required opt_endposition. If yes change station availability with respect to opt_endposition
    if(opt_endposition.is_initialized()){
        changeStationAvailability(opt_endposition,StationEndposition::change::start);
    }

    while (true) {
        // look if station is possible with respect to opt_endposition
        if(opt_endposition.is_initialized()){
            if (!opt_endposition->checkStationPossibility(stations_)){
                break;
            }
        }

        // create a subcon with all possible next scans
        Subcon subcon;
        if(opt_subcon.is_initialized()){
            // if you already have a subcon, check the endposition of each scan and generate subnetting scans and new score.
            subcon = std::move(*opt_subcon);
            opt_subcon = boost::none;
            subcon.changeType(Scan::ScanType::fillin);
            subcon.checkIfEnoughTimeToReachEndposition(stations_, sources_, opt_endposition);
            if (parameters_.subnetting) {
                subcon.createSubnettingScans(*parameters_.subnetting, sources_);
            }
            subcon.generateScore(stations_,sources_,skyCoverages_);
        }else{
            // otherwise calculate new subcon
            subcon = createSubcon(parameters_.subnetting, type, opt_endposition);
            subcon.generateScore(stations_,sources_,skyCoverages_);
        }

        // select the best possible next scan(s) and save them under 'bestScans'
        vector<Scan> bestScans = subcon.selectBest(stations_, sources_, skyCoverages_, opt_endposition);

        // check if you have possible next scan
        if (bestScans.empty()) {
            if(depth == 0){
                // if there is no more possible scan at the outer most iteration, check 1minute later
                bodyLog << "ERROR! no valid scan found! Checking 1 minute later\n";
                unsigned int maxScanEnd = 0;
                for(auto &any:stations_){
                    PointingVector pv = any.getCurrentPointingVector();
                    pv.setTime(pv.getTime()+60);
                    any.setCurrentPointingVector(pv);
                    if(pv.getTime()>maxScanEnd){
                        maxScanEnd = pv.getTime();
                    }
                }
                checkForNewEvents(maxScanEnd,true,bodyLog);
                if( maxScanEnd > endTime){
                    break;
                }

                continue;
            }else{
                // if there is no more possible scan in an deep recursion, break this recursion
                break;
            }
        }

        // check end time of best possible next scan
        unsigned int maxScanEnd = 0;
        for (const auto &any:bestScans) {
            if (any.getTimes().getObservingEnd() > maxScanEnd) {
                maxScanEnd = any.getTimes().getObservingEnd();
            }
        }

        // check if end time triggers a new event
        bool hardBreak = checkForNewEvents(maxScanEnd, true, bodyLog);
        if (hardBreak) {
            continue;
        }

        // if end time of best possible next scans is greate than end time of scan selection stop
        if( maxScanEnd > endTime){
            break;
        }

        unsigned long nSingleScans = subcon.getNumberSingleScans();
        unsigned long  nSubnettingScans = subcon.getNumberSubnettingScans();

        // check if it is possible to start a fillin mode block, otherwise put best scans to schedule
        if (parameters_.fillinmodeDuringScanSelection && !scans_.empty()) {
            boost::optional<StationEndposition> newEndposition(static_cast<int>(stations_.size()));
            if(opt_endposition.is_initialized()){
                for(int i=0; i<stations_.size();++i){
                    if(opt_endposition->hasEndposition(i)){
                        newEndposition->addPointingVectorAsEndposition(opt_endposition->getFinalPosition(i).get());
                    }
                }
            }

            for(const auto&any:bestScans){
                for(int i=0; i<any.getNSta(); ++i){
                    newEndposition->addPointingVectorAsEndposition(any.getPointingVector(i));
                }
            }

            newEndposition->setStationAvailable(stations_);
            newEndposition->checkStationPossibility(stations_);

            boost::optional<Subcon> new_opt_subcon(std::move(subcon));
            // start recursion for fillin mode scans
            startScanSelection(newEndposition->getEarliestScanStart(), bodyLog, Scan::ScanType::fillin, newEndposition, new_opt_subcon, depth+1);
        }

        // update best possible scans
        for (const auto &bestScan : bestScans) {
            consideredUpdate(nSingleScans, nSubnettingScans, depth, bodyLog);
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
    if(opt_endposition.is_initialized()){
        changeStationAvailability(opt_endposition,StationEndposition::change::end);
    }

}

void Scheduler::start(ofstream &bodyLog) noexcept {

    if(parameters_.currentIteration>0){
        bodyLog << "Iteration number: " << parameters_.currentIteration << "\n";
    }
    listSourceOverview(bodyLog);

    boost::optional<StationEndposition> endposition = boost::none;
    boost::optional<Subcon> subcon = boost::none;


    // check if you have some fixed high impact scans
    if(scans_.empty()){
        // no fixed scans: start creating a schedule
        startScanSelection(TimeSystem::duration,bodyLog,Scan::ScanType::standard, endposition, subcon, 0);

        // sort scans
        sort(scans_.begin(),scans_.end(), [](const Scan &scan1, const Scan &scan2){
            return scan1.getTimes().getObservingStart() < scan2.getTimes().getObservingStart();
        });
    } else {
        startScanSelectionBetweenScans(TimeSystem::duration, bodyLog, Scan::ScanType::standard, true);
    }

    // start fillinmode a posterior
    if(parameters_.fillinmodeAPosteriori){
        bodyLog << "* --- start fillin mode a posteriori ---\n";
        startScanSelectionBetweenScans(TimeSystem::duration, bodyLog, Scan::ScanType::fillin);
    }

    // check if there was an error during the session
    if (!check(bodyLog)) {
        if(version_ == 0){
            cout << boost::format("iteration %d ERROR: there was an error while checking the schedule (see log file);\n")
                    % (parameters_.currentIteration);
        }else{
            cout << boost::format("version %d iteration %d ERROR: there was an error while checking the schedule (see log file);\n")
                    % version_ %(parameters_.currentIteration);
        }
    }

    // output some statistics
    statistics(bodyLog);

    bool newScheduleNecessary = checkOptimizationConditions(bodyLog);
    bodyLog.close();

    // check if new iteration is necessary
    if(newScheduleNecessary){
        ++parameters_.currentIteration;
        ofstream newBodyLog(path_+getName()+"_iteration_"+to_string(parameters_.currentIteration)+".log");
        // restart schedule
        start(newBodyLog);
    }

}

void Scheduler::statistics(ofstream &log) {
    log << "summary:\n";
    log << "number of scheduled scans       " << scans_.size() << "\n";
    log << "considered single source scans: " << nSingleScansConsidered << "\n";
    log << "considered subnetting scans:    " << nSubnettingScansConsidered << "\n";
    log << "total scans considered:         " << nSingleScansConsidered + 2 * nSubnettingScansConsidered << "\n";
    int nbl = std::accumulate(scans_.begin(), scans_.end(), 0, [](int sum, const Scan &any){ return sum + any.getNBl(); });
    log << "number of observations:         " << nbl << "\n";
    log << "considered observations:        " << Baseline::numberOfCreatedBaselines() << "\n\n";

}

Subcon Scheduler::createSubcon(const boost::optional<Subnetting> &subnetting, Scan::ScanType type,
                               const boost::optional<StationEndposition> & endposition) noexcept {
    Subcon subcon = allVisibleScans(type, endposition);
    subcon.calcStartTimes(stations_, sources_, endposition);
    subcon.updateAzEl(stations_, sources_);
    subcon.constructAllBaselines(sources_);
    subcon.calcAllBaselineDurations(stations_, sources_);
    subcon.calcAllScanDurations(stations_, sources_, endposition);
    subcon.checkIfEnoughTimeToReachEndposition(stations_, sources_, endposition);

    if (subnetting.is_initialized()) {
        subcon.createSubnettingScans(*subnetting, sources_);
    }
    return subcon;
}


Subcon Scheduler::allVisibleScans(Scan::ScanType type, const boost::optional<StationEndposition> &endposition) noexcept {
    unsigned long nsta = stations_.size();
    unsigned long nsrc = sources_.size();

    unsigned int currentTime = 0;
    for (auto &station : stations_) {
        if (station.getCurrentTime() > currentTime) {
            currentTime = station.getCurrentTime();
        }
    }

    set<int> observedSources;
    if(endposition.is_initialized()){
        observedSources = endposition->getObservedSources();
    }

    Subcon subcon;

    for (int isrc=0; isrc<nsrc; ++isrc){
        const Source &thisSource = sources_[isrc];
        subcon.visibleScan(currentTime, type, stations_, thisSource, observedSources);
    }

    return subcon;
}


void Scheduler::update(const Scan &scan, ofstream &bodyLog) noexcept {

    bool scanHasInfluence;
    scanHasInfluence = !(scan.getType() == Scan::ScanType::fillin && !parameters_.fillinmodeInfluenceOnSchedule);

    int srcid = scan.getSourceId();

    for (int i = 0; i < scan.getNSta(); ++i) {
        const PointingVector &pv = scan.getPointingVector(i);
        int staid = pv.getStaid();
        const PointingVector &pv_end = scan.getPointingVectors_endtime(i);
        unsigned long nbl = scan.getNBl(staid);
        stations_[staid].update(nbl, pv_end, scanHasInfluence);

        if(scanHasInfluence){
            int skyCoverageId = stations_[staid].getSkyCoverageID();
            skyCoverages_[skyCoverageId].update(pv);
        }
    }

    unsigned long nbl = scan.getNBl();
    unsigned int latestTime = scan.getTimes().getObservingStart();
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

    if(n1scans+n2scans>0){
//        bodyLog << "|-------------";
//        for (int i = 0; i < stations_.size() - 1; ++i) {
//            bodyLog << "-----------";
//        }
//        bodyLog << "----------| \n";
        bodyLog << "*   depth "<< depth << " considered: single Scans " << n1scans << " subnetting scans " << n2scans << "\n";
        nSingleScansConsidered += n1scans;
        nSubnettingScansConsidered += n2scans;
    }
}

bool Scheduler::check(ofstream &bodyLog) noexcept {
    bool everythingOk = true;

    bodyLog << "\n=======================   starting check routine   =======================\n";
    bodyLog << "starting check routine!\n";

    int countErrors = 0;
    int countWarnings = 0;

    for (auto& thisStation:stations_){
        bodyLog << "    checking station " << thisStation.getName() << ":\n";
        int staid = thisStation.getId();
        unsigned int constTimes = thisStation.getWaittimes().fieldSystem + thisStation.getWaittimes().preob;


        // get first scan with this station and initialize idx
        int i_thisEnd = 0;
        int idx_thisEnd = -1;
        while( i_thisEnd < scans_.size()) {
            const Scan &scan_thisEnd = scans_[i_thisEnd];
            // look for index of station in scan
            boost::optional<int> oidx_thisEnd = scan_thisEnd.findIdxOfStationId(staid);
            if (!oidx_thisEnd.is_initialized()) {
                ++i_thisEnd;
                continue; // if you do not find one continue
            }
            idx_thisEnd = *oidx_thisEnd;
            break;
        }

        // save statistics
        Station::Statistics statistics;

        while( i_thisEnd < scans_.size()){
            // get scan and pointing vector at start
            const Scan &scan_thisEnd = scans_[i_thisEnd];
            const PointingVector &thisEnd = scan_thisEnd.getPointingVectors_endtime(idx_thisEnd);

            // update statistics
            statistics.scanStartTimes.push_back(scan_thisEnd.getPointingVector(idx_thisEnd).getTime());
            statistics.totalObservingTime += scan_thisEnd.getTimes().getObservingTime(idx_thisEnd);
            statistics.totalFieldSystemTime += scan_thisEnd.getTimes().getFieldSystemTime(idx_thisEnd);
            statistics.totalPreobTime += scan_thisEnd.getTimes().getPreobTime(idx_thisEnd);

            int i_nextStart = i_thisEnd+1;
            while( i_nextStart < scans_.size()){
                // get scan and pointing vector at end
                const Scan &scan_nextStart = scans_[i_nextStart];
                // look for index of station in scan
                boost::optional<int> oidx_nextStart = scan_nextStart.findIdxOfStationId(staid);
                if(!oidx_nextStart.is_initialized()){
                    ++i_nextStart;
                    continue; // if you do not find one continue
                }
                int idx_nextStart = *oidx_nextStart;
                const PointingVector &nextStart = scan_nextStart.getPointingVector(idx_nextStart);

                // check if scan times are in correct order
                unsigned int thisEndTime = thisEnd.getTime();
                unsigned int nextStartTime = nextStart.getTime();

                if(nextStartTime<thisEndTime){
                    ++countErrors;
                    bodyLog << "    ERROR #" << countErrors << ": start time of next scan is before end time of previouse scan! scans: "
                            << scan_thisEnd.printId() << " and " << scan_nextStart.printId() << "\n";
                    boost::posix_time::ptime thisEndTime_ = TimeSystem::internalTime2PosixTime(thisEndTime);
                    boost::posix_time::ptime nextStartTime_ = TimeSystem::internalTime2PosixTime(nextStartTime);

                    bodyLog << "           end time of previouse scan: " << thisEndTime_.time_of_day()
                            << " " << thisEnd.printId() << "\n";
                    bodyLog << "           start time of next scan:    " << nextStartTime_.time_of_day()
                            << " " << nextStart.printId() << "\n";
                    bodyLog << "*\n";
                    everythingOk = false;
                }else{
                    // check slew time
                    unsigned int slewtime = thisStation.getAntenna().slewTime(thisEnd,nextStart);
                    unsigned int min_neededTime = slewtime + constTimes;
                    unsigned int availableTime = nextStartTime-thisEndTime;
                    unsigned int idleTime;

                    // update statistics
                    statistics.totalSlewTime += slewtime;
                    if(availableTime>=min_neededTime){
                        idleTime = availableTime-min_neededTime;
                    }else{
                        idleTime = 0;
                    }

                    statistics.totalIdleTime += idleTime;

                    if(availableTime+1<min_neededTime){
                        ++countErrors;
                        bodyLog << "    ERROR #" << countErrors << ": not enough available time for slewing! scans: "
                                << scan_thisEnd.printId() << " and " << scan_nextStart.printId() << "\n";
                        boost::posix_time::ptime thisEndTime_ = TimeSystem::internalTime2PosixTime(thisEndTime);
                        boost::posix_time::ptime nextStartTime_ = TimeSystem::internalTime2PosixTime(nextStartTime);
                        bodyLog << "               end time of previouse scan: " << thisEndTime_.time_of_day()
                                << " " << thisEnd.printId() << "\n";
                        bodyLog << "               start time of next scan:    " << nextStartTime_.time_of_day()
                                << " " << nextStart.printId() << "\n";
                        bodyLog << "           available time: " << availableTime << "\n";
                        bodyLog << "               needed slew time:           " << slewtime << "\n";
                        bodyLog << "               needed constant times:      " << constTimes << "\n";
                        bodyLog << "           needed time:    " << min_neededTime << "\n";
                        bodyLog << "           difference:     " << (long) availableTime - (long) min_neededTime << "\n";
                        bodyLog << "*\n";
                        everythingOk = false;
                    }else{
                        if(idleTime > 1200){
                            ++countWarnings;
                            bodyLog << "    WARNING #" << countWarnings << ": long idle time! scans: "
                                    << scan_thisEnd.printId() << " and " << scan_nextStart.printId() << "\n";
                            bodyLog << "        idle time: " << idleTime << "[s]\n";
                            boost::posix_time::ptime thisEndTime_ = TimeSystem::internalTime2PosixTime(thisEndTime);
                            boost::posix_time::ptime nextStartTime_ = TimeSystem::internalTime2PosixTime(nextStartTime);
                            bodyLog << "               end time of previouse scan: " << thisEndTime_.time_of_day()
                                    << " " << thisEnd.printId() << "\n";
                            bodyLog << "               start time of next scan:    " << nextStartTime_.time_of_day()
                                    << " " << nextStart.printId() << "\n";
                            bodyLog << "*\n";
                        }
                    }
                }
                // change index of pointing vector and scan
                idx_thisEnd = idx_nextStart;
                break;
            }
            // change index of pointing vector and scan
            i_thisEnd = i_nextStart;

        }
        bodyLog << "    finished!\n";
        thisStation.setStatistics(statistics);
    }
    bodyLog << "Total: " << countErrors << " errors and " << countWarnings << " warnings\n";
    bodyLog << "=========================   end check routine    =========================\n";

    std::vector<Source::Statistics> statistics(sources_.size());
    for(auto &any:scans_){
        int srcid = any.getSourceId();
        auto &thisStatistics = statistics[srcid];
        thisStatistics.scanStartTimes.push_back(any.getTimes().getObservingStart());
        thisStatistics.totalObservingTime += any.getTimes().getObservingTime();
    }
    for(int i=0; i<sources_.size(); ++i){
        Source &source = sources_[i];
        source.setStatistics(statistics[i]);
    }

    return everythingOk;
}

bool Scheduler::checkForNewEvents(unsigned int time, bool output, ofstream &bodyLog) noexcept {
    bool hard_break = false;

    for (auto &any:stations_){
        bool tagalong = any.checkForTagalongMode(time);
        if(tagalong){
            bodyLog << "TAGALONG for station " << any.getName() << " required!\n";
            startTagelongMode(any, bodyLog);
        }
    }

    vector<string> stationChanged;
    for (auto &any:stations_) {
        bool changed = any.checkForNewEvent(time, hard_break);
        if(changed){
            stationChanged.push_back(any.getName());
        }
    }
    if(!stationChanged.empty() && output && time<TimeSystem::duration){
        util::outputObjectList("station parameter changed",stationChanged,bodyLog);
    }

    vector<string> sourcesChanged;
    for (auto &any:sources_) {
        bool changed = any.checkForNewEvent(time, hard_break);
        if(changed){
            sourcesChanged.push_back(any.getName());
        }
    }
    if(!sourcesChanged.empty() &&output && time<TimeSystem::duration){
        util::outputObjectList("source parameter changed",sourcesChanged,bodyLog);
        listSourceOverview(bodyLog);
    }

    Baseline::checkForNewEvent(time, hard_break, output, bodyLog);
    return hard_break;
}

void Scheduler::listSourceOverview(ofstream &log) noexcept {
    unsigned int counter = 0;
    vector<string> available;
    vector<string> notAvailable;
    vector<string> notAvailable_optimization;
    vector<string> notAvailable_tooWeak;
    vector<string> notAvailable_tooCloseToSun;

    for (const auto &any:sources_) {
        if (any.getPARA().available && any.getPARA().globalAvailable) {

            available.push_back(any.getName());

        }else if(!any.getPARA().globalAvailable){

            notAvailable_optimization.push_back(any.getName());

        }else if(any.getMaxFlux() < any.getPARA().minFlux){

            string message = (boost::format("%8s (%4.2f/%4.2f)")
                              % any.getName()
                              % any.getMaxFlux()
                              % any.getPARA().minFlux).str();
            notAvailable_tooWeak.push_back(message);

        }else if(any.getSunDistance() < any.getPARA().minSunDistance){

            string message = (boost::format("%8s (%4.2f/%4.2f)")
                              % any.getName()
                              % (any.getSunDistance()*rad2deg)
                              % (any.getPARA().minSunDistance*rad2deg)).str();
            notAvailable_tooCloseToSun.push_back(message);

        }else{

            notAvailable.push_back(any.getName());

        }
    }

    util::outputObjectList("available source",available,log);
    util::outputObjectList("not available",notAvailable,log);
    util::outputObjectList("not available because of optimization",notAvailable_optimization,log);
    util::outputObjectList("not available because too weak",notAvailable_tooWeak,log);
    util::outputObjectList("not available because of sun distance",notAvailable_tooCloseToSun,log);

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
        subcon.generateScore(prevLowElevationScores, prevHighElevationScores, stations_, sources_);

        boost::optional<unsigned long> bestIdx_opt = subcon.rigorousScore(stations_,sources_,skyCoverages_, prevLowElevationScores, prevHighElevationScores );
        if (!bestIdx_opt) {
            bodyLog << "ERROR! no valid scan found! End of calibrator block!\n";
            break;
        }
        unsigned long bestIdx = *bestIdx_opt;
        vector<Scan> bestScans;
        if (bestIdx < subcon.getNumberSingleScans()) {
            Scan bestScan = subcon.takeSingleSourceScan(bestIdx);
            bestScans.push_back(std::move(bestScan));
        } else {
            unsigned long thisIdx = bestIdx - subcon.getNumberSingleScans();
            pair<Scan, Scan> bestScan_pair = subcon.takeSubnettingScans(thisIdx);
            Scan &bestScan1 = bestScan_pair.first;
            Scan &bestScan2 = bestScan_pair.second;

            bestScans.push_back(std::move(bestScan1));
            bestScans.push_back(std::move(bestScan2));
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
            if (any.getTimes().getObservingEnd() > all_maxTime) {
                all_maxTime = any.getTimes().getObservingEnd();
            }
        }


        bool hardBreak = checkForNewEvents(all_maxTime, true, bodyLog);
        if (hardBreak) {
            i -=1;
            continue;
        }

        if( all_maxTime > TimeSystem::duration){
            break;
        }


        if (parameters_.fillinmodeDuringScanSelection && !scans_.empty()) {
//            start_fillinMode(bestScans, bodyLog);
        } else {
            for (const auto &bestScan : bestScans) {
                consideredUpdate(subcon.getNumberSingleScans(), subcon.getNumberSubnettingScans(), 0, bodyLog);
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
        unsigned int scanStartTime = scan.getTimes().getObservingStart();
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

                        unsigned int minScanBl = std::max({Baseline::PARA.minScan[staid1][staid2],
                                                           stations_[staid1].getPARA().minScan,
                                                           stations_[staid2].getPARA().minScan,
                                                           source.getPARA().minScan});

                        if (new_duration_uint < minScanBl) {
                            new_duration_uint = minScanBl;
                        }
                        unsigned int maxScanBl = std::min({Baseline::PARA.maxScan[staid1][staid2],
                                                           stations_[staid1].getPARA().maxScan,
                                                           stations_[staid2].getPARA().maxScan,
                                                           source.getPARA().maxScan});

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
        }
    }

//    station.applyNextEvent(bodyLog);
}

bool Scheduler::checkOptimizationConditions(ofstream &of) {
    bool newScheduleNecessary = false;
    vector<string> excludedSources;
    int excludedScans = 0;
    int excludedBaselines = 0;
    of << "checking optimization conditions... ";
    int consideredSources = 0;
    bool lastExcluded = false;
    for (auto &thisSource : sources_) {
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
            if(parameters_.currentIteration < parameters_.numberOfGentleSourceReductions){
                if(lastExcluded){
                    lastExcluded = false;
                    continue;
                }else {
                    lastExcluded = true;
                }
            }
            excludedScans += thisSource.getNTotalScans();
            excludedBaselines += thisSource.getNbls();
            excludedSources.push_back(thisSource.getName());
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

        util::outputObjectList("List of removed sources",excludedSources,of);

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

void Scheduler::changeStationAvailability(const boost::optional<StationEndposition> &endposition,
                                          StationEndposition::change change) {
    switch (change){
        case StationEndposition::change::start:{
            for(int i=0; i<stations_.size(); ++i){
                stations_[i].referencePARA().available = endposition->getStationPossible(i);
            }
            break;
        }
        case StationEndposition::change::end:{
            for(int i=0; i<stations_.size(); ++i){
                stations_[i].referencePARA().available = endposition->getStationAvailable(i);
            }
            break;
        }
    }
}

void Scheduler::startScanSelectionBetweenScans(unsigned int duration, std::ofstream &bodyLog, Scan::ScanType type, bool output) {

    // save number of predefined scans (new scans will be added at end of those)
    auto nMainScans = static_cast<int>(scans_.size());

    // reset all events
    resetAllEvents(bodyLog);


    // loop through all predefined scans
    for(int i=0; i<nMainScans-1; ++i){

        if(output){
            bodyLog << "* --- start new scan selection ---\n";
        }
        // look through all stations of last scan and set current pointing vector to last scan
        Scan &lastScan = scans_[i];
        for(int k=0; k<lastScan.getNSta(); ++k){
            const auto &pv = lastScan.getPointingVectors_endtime(k);
            int staid = pv.getStaid();
            unsigned int time = pv.getTime();
            Station &thisSta = stations_[staid];
            if(time >= thisSta.getCurrentTime()){
                thisSta.setCurrentPointingVector(pv);
            }
        }

        // loop through all upcoming scans and set endposition
        boost::optional<StationEndposition> endposition(static_cast<int>(stations_.size()));
        for(int j=i+1; j<nMainScans; ++j){
            const Scan &nextScan = scans_[j];
            bool nextRequired = true;
            for(int k=0; k<nextScan.getNSta(); ++k){
                endposition->addPointingVectorAsEndposition(nextScan.getPointingVector(k));
                if (endposition->everyStationInitialized()){
                    nextRequired = false;
                    break;
                }
            }
            if(!nextRequired){
                break;
            }
        }

        // check if there was an new upcoming event in the meantime
        unsigned int startTime = lastScan.getTimes().getObservingEnd();
        checkForNewEvents(startTime, true, bodyLog);

        // recursively start scan selection
        boost::optional<Subcon> subcon = boost::none;
        startScanSelection(scans_[i + 1].getTimes().getObservingStart(), bodyLog, type, endposition, subcon, 1);
    }

    // do the same between time at from last scan until duration with no endposition
    if(output){
        bodyLog << "* --- start final scan selection ---\n";
    }

    // get last predefined scan and set current position of station
    Scan &lastScan = scans_[nMainScans-1];
    for(int k=0; k<lastScan.getNSta(); ++k){
        const auto &pv = lastScan.getPointingVectors_endtime(k);
        int staid = pv.getStaid();
        unsigned int time = pv.getTime();
        Station &thisSta = stations_[staid];
        if(time >= thisSta.getCurrentTime()){
            thisSta.setCurrentPointingVector(pv);
        }
    }
    // check if there was an new upcoming event in the meantime
    unsigned int startTime = lastScan.getTimes().getObservingEnd();
    checkForNewEvents(startTime, true, bodyLog);

    // recursively start scan selection
    boost::optional<Subcon> subcon = boost::none;
    boost::optional<StationEndposition> endposition = boost::none;
    startScanSelection(duration, bodyLog, type, endposition, subcon, 1);

    // sort scans at the end
    sort(scans_.begin(),scans_.end(), [](const Scan &scan1, const Scan &scan2){
        return scan1.getTimes().getObservingStart() < scan2.getTimes().getObservingStart();
    });

}

void Scheduler::highImpactScans(HighImpactScanDescriptor &himp, ofstream &bodyLog) {



    bodyLog << "###############################################################################################\n";
    bodyLog << "##                                 fixing high impact scans                                  ##\n";
    bodyLog << "###############################################################################################\n";
    unsigned int interval = himp.getInterval();
    int n = TimeSystem::duration/interval;

    // look for possible high impact scans
    for(unsigned int iTime=0; iTime<n; ++iTime){

        // check for new event and changes current pointing vector as well as first scan
        unsigned int time = iTime*interval;
        checkForNewEvents(time, true, bodyLog);
        for(auto &thisStation:stations_){
            PointingVector pv(thisStation.getId(),-1);
            pv.setTime(time);
            thisStation.setCurrentPointingVector(pv);
            thisStation.referencePARA().firstScan = true;
        }

        // create all possible high impact scan pointing vectors for this time
        himp.possibleHighImpactScans(iTime,stations_,sources_);
    }

    // create the actual scans
    himp.updateHighImpactScans(stations_,sources_,parameters_.subnetting);

    himp.updateLogfile(bodyLog);

    // select bestScans
    vector<Scan> bestScans;
    do{
        bestScans = himp.highestImpactScans(stations_, sources_);
        for(const auto& scan:bestScans){
            if(himp.isCorrectHighImpactScan(scan,scans_)){
                update(scan, bodyLog);

                for(auto &thisStation:stations_){
                    thisStation.referencePARA().firstScan = true;
                }

            }
        }
    }while(himp.hasMoreScans());

    sort(scans_.begin(),scans_.end(), [](const Scan &scan1, const Scan &scan2){
        return scan1.getTimes().getObservingStart() < scan2.getTimes().getObservingStart();
    });


    bodyLog << "###############################################################################################\n";
    bodyLog << "##                                 start with normal scans                                   ##\n";
    bodyLog << "###############################################################################################\n";


    // reset all events
    resetAllEvents(bodyLog);

    for(auto &thisStation:stations_){
        PointingVector pv(thisStation.getId(),-1);
        pv.setTime(0);
        thisStation.setCurrentPointingVector(pv);
        thisStation.referencePARA().firstScan = true;
    }

}

void Scheduler::resetAllEvents(std::ofstream &bodyLog) {

    // reset all events
    for(auto &any:stations_){
        PointingVector pv(any.getId(),-1);
        pv.setTime(0);
        any.setCurrentPointingVector(pv);
        any.setNextEvent(0);
    }
    for(auto &any:sources_){
        any.setNextEvent(0);
    }
    for (int i = 0; i < stations_.size(); ++i) {
        for (int j = i + 1; j < stations_.size(); ++j) {
            Baseline::nextEvent[i][j]=0;
        }
    }
    checkForNewEvents(0, false, bodyLog);

}


