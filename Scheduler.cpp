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
unsigned long Scheduler::nextId = 0;

Scheduler::Scheduler(Initializer &init, string path, string fname): VieVS_NamedObject(move(fname),nextId++),
                                                                    path_{std::move(path)},
                                                                    network_{std::move(init.network_)},
                                                                    sources_{std::move(init.sources_)},
                                                                    himp_{std::move(init.himp_)},
                                                                    multiSchedulingParameters_{std::move(init.multiSchedulingParameters_)},
                                                                    xml_{init.xml_} {

    if(init.parameters_.subnetting){
        Subnetting subnetting;
        subnetting.subnettingMinNSta = static_cast<unsigned int>(network_.getNSta() * init.parameters_.subnettingMinNSta);
        subnetting.subnettingSrcIds = std::move(init.preCalculated_.subnettingSrcIds);
        parameters_.subnetting = move(subnetting);
    }

    parameters_.fillinmodeDuringScanSelection = init.parameters_.fillinmodeDuringScanSelection;
    parameters_.fillinmodeInfluenceOnSchedule = init.parameters_.fillinmodeInfluenceOnSchedule;
    parameters_.fillinmodeAPosteriori = init.parameters_.fillinmodeAPosteriori;

    parameters_.idleToObservingTime = init.parameters_.idleToObservingTime;

    parameters_.andAsConditionCombination = init.parameters_.andAsConditionCombination;
    parameters_.minNumberOfSourcesToReduce = init.parameters_.minNumberOfSourcesToReduce;
    parameters_.maxNumberOfIterations = init.parameters_.maxNumberOfIterations;
    parameters_.numberOfGentleSourceReductions = init.parameters_.numberOfGentleSourceReductions;

    parameters_.writeSkyCoverageData = false;
}

Scheduler::Scheduler(std::string name, Network network, std::vector<Source> sources, std::vector<Scan> scans,
                     boost::property_tree::ptree xml): VieVS_NamedObject(move(name),nextId++),
                                                       network_{std::move(network)},
                                                       sources_{std::move(sources)},
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
            if (!opt_endposition->checkStationPossibility(network_.getStations())){
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
            subcon.checkIfEnoughTimeToReachEndposition(network_, sources_, opt_endposition);
            subcon.clearSubnettingScans();
            if (parameters_.subnetting) {
                subcon.createSubnettingScans(*parameters_.subnetting, sources_);
            }
            subcon.generateScore(network_,sources_);
        }else{
            // otherwise calculate new subcon
            subcon = createSubcon(parameters_.subnetting, type, opt_endposition);
            subcon.generateScore(network_,sources_);
        }
        unsigned long nSingleScans = subcon.getNumberSingleScans();
        unsigned long  nSubnettingScans = subcon.getNumberSubnettingScans();

        // select the best possible next scan(s) and save them under 'bestScans'
        vector<Scan> bestScans = subcon.selectBest(network_, sources_, opt_endposition);

        // check if you have possible next scan
        if (bestScans.empty()) {
            if(depth == 0){
                // if there is no more possible scan at the outer most iteration, check 1minute later
                unsigned int maxScanEnd = 0;
                for(auto &any:network_.refStations()){
                    PointingVector pv = any.getCurrentPointingVector();
                    pv.setTime(pv.getTime()+60);
                    any.setCurrentPointingVector(pv);
                    if(pv.getTime()>maxScanEnd){
                        maxScanEnd = pv.getTime();
                    }
                }
                bodyLog << (boost::format("ERROR! no valid scan found! Checking 1 minute later: %s\n")
                                          % TimeSystem::ptime2string(TimeSystem::internalTime2PosixTime(maxScanEnd))).str();
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
            if (any.getTimes().getScanEnd() > maxScanEnd) {
                maxScanEnd = any.getTimes().getScanEnd();
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


        // check if it is possible to start a fillin mode block, otherwise put best scans to schedule
        if (parameters_.fillinmodeDuringScanSelection && !scans_.empty()) {
            boost::optional<StationEndposition> newEndposition(network_.getNSta());
            if(opt_endposition.is_initialized()){
                for(unsigned long i=0; i<network_.getNSta();++i){
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

            newEndposition->setStationAvailable(network_.getStations());
            newEndposition->checkStationPossibility(network_.getStations());

            boost::optional<Subcon> new_opt_subcon(std::move(subcon));
            // start recursion for fillin mode scans
            startScanSelection(newEndposition->getEarliestScanStart(), bodyLog, Scan::ScanType::fillin, newEndposition, new_opt_subcon, depth+1);
        }

        // update best possible scans
        consideredUpdate(nSingleScans, nSubnettingScans, depth, bodyLog);
        for (auto &bestScan : bestScans) {
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

        if(depth == 0){
            bodyLog << "* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n*\n";
        }
    }

    // scan selection block is over. Change station availability back to start value
    if(opt_endposition.is_initialized()){
        changeStationAvailability(opt_endposition,StationEndposition::change::end);
    }

}

void Scheduler::start() noexcept {

    string fileName = getName()+"_iteration_"+to_string(parameters_.currentIteration)+".log";
    ofstream bodyLog(path_ + fileName);

    if(network_.getNSta() == 0 || sources_.empty() || network_.getNBls() == 0){
        string e = (boost::format("ERROR: number of stations: %d number of baselines: %d number of sources: %d;\n")
        % network_.getNSta() %network_.getNBls() %sources_.size()).str();
        bodyLog << e;
        return;
    }

    cout << (boost::format("writing scheduling log file: %s;\n") % fileName).str();
    if(parameters_.currentIteration>0){
        bodyLog << "Iteration number: " << parameters_.currentIteration << "\n";
    }
    if(multiSchedulingParameters_.is_initialized()){
        bodyLog << "multi scheduling parameters:\n";
        multiSchedulingParameters_->output(bodyLog);
    }

    listSourceOverview(bodyLog);

    boost::optional<StationEndposition> endposition = boost::none;
    boost::optional<Subcon> subcon = boost::none;

    if(himp_.is_initialized()){
        highImpactScans(himp_.get(), bodyLog);
    }


    // check if you have some fixed high impact scans
    if(scans_.empty()){
        // no fixed scans: start creating a schedule
        startScanSelection(TimeSystem::duration,bodyLog,Scan::ScanType::standard, endposition, subcon, 0);

        // sort scans
        sortSchedule(Timestamp::start);

    } else {
        startScanSelectionBetweenScans(TimeSystem::duration, bodyLog, Scan::ScanType::standard, true, false);
    }

    // start fillinmode a posterior
    if(parameters_.fillinmodeAPosteriori){
        bodyLog << "* --- start fillin mode a posteriori ---\n";
        startScanSelectionBetweenScans(TimeSystem::duration, bodyLog, Scan::ScanType::fillin, false, true);
    }

    if(parameters_.idleToObservingTime){
        switch (ScanTimes::getAlignmentAnchor()){

            case ScanTimes::AlignmentAnchor::start:{
                idleToScanTime(Timestamp::end,bodyLog);
                break;
            }
            case ScanTimes::AlignmentAnchor::end:{
                idleToScanTime(Timestamp::start,bodyLog);
                break;
            }
            case ScanTimes::AlignmentAnchor::individual:{
                idleToScanTime(Timestamp::end,bodyLog);
                idleToScanTime(Timestamp::start,bodyLog);
                break;
            }
        }
    }

    // check if there was an error during the session
    if (!checkAndStatistics(bodyLog)) {
        cout << boost::format("ERROR: %s iteration %d while checking the schedule (see log file);\n")
                % getName() %(parameters_.currentIteration);
    }

    // output some statistics
    statistics(bodyLog);

    bool newScheduleNecessary = checkOptimizationConditions(bodyLog);
    bodyLog.close();

    // check if new iteration is necessary
    if(newScheduleNecessary){
        ++parameters_.currentIteration;
        // restart schedule
        start();
    }

}

void Scheduler::statistics(ofstream &log) {
    log << "*\n";
    log << "* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n";
    log << "*\n";
    log << "summary:\n";
    log << "number of scheduled scans         " << scans_.size() << "\n";
    log << "considered single source scans:   " << nSingleScansConsidered << "\n";
    log << "considered subnetting combiation: " << nSubnettingScansConsidered << "\n";
    log << "total scans considered:           " << nSingleScansConsidered + 2 * nSubnettingScansConsidered << "\n";
    int nbl = std::accumulate(scans_.begin(), scans_.end(), 0, [](int sum, const Scan &any){ return sum + any.getNObs(); });
    log << "number of observations:           " << nbl << "\n";
}

Subcon Scheduler::createSubcon(const boost::optional<Subnetting> &subnetting, Scan::ScanType type,
                               const boost::optional<StationEndposition> & endposition) noexcept {
    Subcon subcon = allVisibleScans(type, endposition);
    subcon.calcStartTimes(network_, sources_, endposition);
    subcon.updateAzEl(network_, sources_);
    subcon.constructAllBaselines(network_, sources_);
    subcon.calcAllBaselineDurations(network_, sources_);
    subcon.calcAllScanDurations(network_, sources_, endposition);
    subcon.checkIfEnoughTimeToReachEndposition(network_, sources_, endposition);

    if (subnetting.is_initialized()) {
        subcon.createSubnettingScans(*subnetting, sources_);
    }
    return subcon;
}


Subcon Scheduler::allVisibleScans(Scan::ScanType type, const boost::optional<StationEndposition> &endposition) noexcept {

    // get latest start time of new scan
    unsigned int currentTime = 0;
    for (auto &station : network_.getStations()) {
        if (station.getCurrentTime() > currentTime) {
            currentTime = station.getCurrentTime();
        }
    }

    // save all ids of the next observed sources (if there is a required endposition)
    set<unsigned long> observedSources;
    if(endposition.is_initialized()){
        observedSources = endposition->getObservedSources();
    }

    // create subcon with all visible scans
    Subcon subcon;
    for (const auto &thisSource : sources_) {
        subcon.visibleScan(currentTime, type, network_, thisSource, observedSources);
    }

    return subcon;
}


void Scheduler::update(Scan &scan, ofstream &bodyLog) noexcept {

    // check if scan has influence (only required for fillin mode scans)
    bool influence;
    influence = !(scan.getType() == Scan::ScanType::fillin && !parameters_.fillinmodeInfluenceOnSchedule);


    unsigned long srcid = scan.getSourceId();

    for (int i = 0; i < scan.getNSta(); ++i) {
        const PointingVector &pv = scan.getPointingVector(i);
        unsigned long staid = pv.getStaid();
        const PointingVector &pv_end = scan.getPointingVector(i, Timestamp::end);
        unsigned long nObs = scan.getNObs(staid);

        network_.update(nObs, pv_end, influence);
    }
    for (int i=0; i< scan.getNObs(); ++i){
        const Observation &obs = scan.getObservation(i);
        network_.update(obs.getBlid(), influence);
    }

    unsigned long nbl = scan.getNObs();
    unsigned int latestTime = scan.getTimes().getObservingStart();
    Source &thisSource = sources_[srcid];
    thisSource.update(nbl, latestTime, influence);

    scan.output(scans_.size(), network_, thisSource, bodyLog);
    scans_.push_back(std::move(scan));
}

void Scheduler::consideredUpdate(unsigned long n1scans, unsigned long n2scans, int depth, ofstream &bodyLog) noexcept {

    if(n1scans+n2scans>0){
        bodyLog << "*   depth "<< depth << " considered: single Scans " << n1scans << " subnetting scans " << n2scans << "\n";
        nSingleScansConsidered += n1scans;
        nSubnettingScansConsidered += n2scans;
    }
}

bool Scheduler::checkAndStatistics(ofstream &bodyLog) noexcept {
    bool everythingOk = true;

    bodyLog << "starting check routine!\n";

    int countErrors = 0;
    int countWarnings = 0;

    for (auto& thisStation : network_.refStations()){
        bodyLog << "    checking station " << thisStation.getName() << ":\n";
        unsigned long staid = thisStation.getId();
        unsigned int constTimes = thisStation.getWaittimes().fieldSystem + thisStation.getWaittimes().preob;

        // sort scans based on observation start of this station (can be different if you align scans individual or at end)
        sortSchedule(staid);

        // get first scan with this station and initialize idx
        int i_thisEnd = 0;
        int idx_thisEnd = -1;
        while( i_thisEnd < scans_.size()) {
            const Scan &scan_thisEnd = scans_[i_thisEnd];
            // look for index of station in scan
            boost::optional<unsigned long> oidx_thisEnd = scan_thisEnd.findIdxOfStationId(staid);
            if (!oidx_thisEnd.is_initialized()) {
                ++i_thisEnd;
                continue; // if you do not find one continue
            }
            idx_thisEnd = static_cast<int>(*oidx_thisEnd);
            break;
        }

        // save staStatistics
        Station::Statistics staStatistics;

        while( i_thisEnd < scans_.size()){
            // get scan and pointing vector at start
            const Scan &scan_thisEnd = scans_[i_thisEnd];
            const PointingVector &thisEnd = scan_thisEnd.getPointingVector(idx_thisEnd, Timestamp::end);

            // update staStatistics
            staStatistics.scanStartTimes.push_back(scan_thisEnd.getPointingVector(idx_thisEnd).getTime());
            staStatistics.totalObservingTime += scan_thisEnd.getTimes().getObservingTime(idx_thisEnd);
            staStatistics.totalFieldSystemTime += scan_thisEnd.getTimes().getFieldSystemTime(idx_thisEnd);
            staStatistics.totalPreobTime += scan_thisEnd.getTimes().getPreobTime(idx_thisEnd);

            int i_nextStart = i_thisEnd+1;
            while( i_nextStart < scans_.size()){
                // get scan and pointing vector at end
                const Scan &scan_nextStart = scans_[i_nextStart];
                // look for index of station in scan
                boost::optional<unsigned long> oidx_nextStart = scan_nextStart.findIdxOfStationId(staid);
                if(!oidx_nextStart.is_initialized()){
                    ++i_nextStart;
                    continue; // if you do not find one continue
                }
                auto idx_nextStart = static_cast<int>(*oidx_nextStart);
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

                    // update staStatistics
                    staStatistics.totalSlewTime += slewtime;
                    if(availableTime>=min_neededTime){
                        idleTime = availableTime-min_neededTime;
                    }else{
                        idleTime = 0;
                    }

                    staStatistics.totalIdleTime += idleTime;

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
        thisStation.setStatistics(staStatistics);
    }
    bodyLog << "Total: " << countErrors << " errors and " << countWarnings << " warnings\n";

    // sort scans again based on their observation start
    sortSchedule(Timestamp::start);

    // add source srcStatistics
    std::vector<Source::Statistics> srcStatistics(sources_.size());
    std::vector<Baseline::Statistics> blsStatistics(network_.getNBls());
    for(const auto &any:scans_){
        unsigned long srcid = any.getSourceId();
        auto &thisSrcStatistics = srcStatistics[srcid];
        thisSrcStatistics.scanStartTimes.push_back(any.getTimes().getObservingStart());
        thisSrcStatistics.totalObservingTime += any.getTimes().getObservingTime();

        for(const auto &obs: any.getObservations()){
            unsigned long blid = obs.getBlid();
            auto &thisBlsStatistics = blsStatistics[blid];
            thisBlsStatistics.scanStartTimes.push_back(any.getTimes().getObservingStart());
            thisBlsStatistics.totalObservingTime += any.getTimes().getObservingTime();
        }
    }
    for(unsigned long isrc=0; isrc<sources_.size(); ++isrc){
        Source &source = sources_[isrc];
        source.setStatistics(srcStatistics[isrc]);
    }
    for(unsigned long ibl=0; ibl<network_.getNBls(); ++ibl){
        Baseline &baseline = network_.refBaseline(ibl);
        baseline.setStatistics(blsStatistics[ibl]);
    }
    return everythingOk;
}

bool Scheduler::checkForNewEvents(unsigned int time, bool output, ofstream &bodyLog) noexcept {
    bool hard_break = false;

    // check if it is required to tagalong a station
    for (auto &any:network_.refStations()){
        bool tagalong = any.checkForTagalongMode(time);
        if(tagalong){
            bodyLog << "TAGALONG for station " << any.getName() << " required!\n";
            startTagelongMode(any, bodyLog);
        }
    }

    // check if a station has to be changed
    vector<string> stationChanged;
    for (auto &any : network_.refStations()) {
        bool changed = any.checkForNewEvent(time, hard_break);
        if(changed){
            stationChanged.push_back(any.getName());
        }
    }
    if(!stationChanged.empty() && output && time<TimeSystem::duration){
        util::outputObjectList("station parameter changed",stationChanged,bodyLog);
    }

    // check if a source has to be changed
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

    // check if a baseline has to be changed
    vector<string> baselineChanged;
    for (auto &any : network_.refBaselines()) {
        bool changed = any.checkForNewEvent(time, hard_break);
        if(changed){
            baselineChanged.push_back(any.getName());
        }
    }
    if(!baselineChanged.empty() && output && time<TimeSystem::duration){
        util::outputObjectList("baseline parameter changed",baselineChanged,bodyLog);
    }
    return hard_break;
}

void Scheduler::ignoreTagalongParameter() {
    // ignore the tagalong mode for each station
    for (auto &any : network_.refStations()) {
        any.referencePARA().tagalong = false;
    }
}

void Scheduler::listSourceOverview(ofstream &log) noexcept {
//    unsigned int counter = 0;
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

    log << "Total number of sources: " << sources_.size() << "\n";
    util::outputObjectList("available source",available,log);
    util::outputObjectList("not available",notAvailable,log);
    util::outputObjectList("not available because of optimization",notAvailable_optimization,log);
    util::outputObjectList("not available because too weak",notAvailable_tooWeak,log);
    util::outputObjectList("not available because of sun distance",notAvailable_tooCloseToSun,log);

}

void Scheduler::startCalibrationBlock(std::ofstream &bodyLog) {
    unsigned long nsta = network_.getNSta();
    vector<double> prevLowElevationScores(nsta,0);
    vector<double> prevHighElevationScores(nsta,0);

    vector<double> highestElevations(nsta,numeric_limits<double>::min());
    vector<double> lowestElevations(nsta,numeric_limits<double>::max());



    for (int i = 0; i < CalibratorBlock::nmaxScans; ++i) {

        Subcon subcon = createSubcon(parameters_.subnetting, Scan::ScanType::calibrator);
        subcon.generateScore(prevLowElevationScores, prevHighElevationScores, network_, sources_);

        boost::optional<unsigned long> bestIdx_opt = subcon.rigorousScore(network_, sources_, prevLowElevationScores, prevHighElevationScores );
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
                unsigned long staid = pv.getStaid();

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
            for (auto &bestScan : bestScans) {
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
    for (int i = 0; i < network_.getNSta() - 1; ++i) {
        bodyLog << "===========";
    }
    bodyLog << "==========| \n";
    bodyLog << "| CALIBRATOR BLOCK SUMMARY:\n";
    bodyLog << "|=============";
    for (int i = 0; i < network_.getNSta() - 1; ++i) {
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
    for (int i = 0; i < network_.getNSta() - 1; ++i) {
        bodyLog << "===========";
    }
    bodyLog << "==========| \n";

}

void Scheduler::startTagelongMode(Station &station, std::ofstream &bodyLog) {

    unsigned long staid = station.getId();

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
            unsigned long srcid = scan.getSourceId();
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
            vector<Observation> newObs;
            for (int i = 0; i < scan.getNSta(); ++i) {
                // create baseline
                const PointingVector & otherPv = scan.getPointingVector(i);

                const Station &sta1 = network_.getStation(staid);
                const Station &sta2 = network_.getStation(otherPv.getStaid());
                const Baseline &bl = network_.getBaseline(sta1.getId(),sta2.getId());

                if(bl.getParameters().ignore){
                    continue;
                }
                if (!source.getPARA().ignoreBaselines.empty()) {
                    const auto &PARA = source.getPARA();
                    if (find(PARA.ignoreBaselines.begin(), PARA.ignoreBaselines.end(), bl.getId()) !=
                        PARA.ignoreBaselines.end()) {
                        continue;
                    }

                }
                Observation obs(bl.getId(), sta1.getId(), sta2.getId(), srcid, scanStartTime);

                // calc baseline scan length
                double date1 = 2400000.5;
                double date2 = TimeSystem::mjdStart + static_cast<double>(scanStartTime) / 86400.0;
                double gmst = iauGmst82(date1, date2);

                unsigned int maxScanDuration = 0;
                if(source.getPARA().fixedScanDuration.is_initialized()){
                    maxScanDuration = *source.getPARA().fixedScanDuration;
                }else {

                    for (auto &band:ObservationMode::bands) {

                        double SEFD_src = source.observedFlux(band, gmst, network_.getDxyz(sta1.getId(), sta2.getId()));

                        double el1 = pv_new_start.getEl();
                        double SEFD_sta1 = sta1.getEquip().getSEFD(band, el1);

                        double el2 = otherPv.getEl();
                        double SEFD_sta2 = sta2.getEquip().getSEFD(band, el2);

                        double minSNR_sta1 = sta1.getPARA().minSNR.at(band);
                        double minSNR_sta2 = sta2.getPARA().minSNR.at(band);

                        double minSNR_bl = bl.getParameters().minSNR.at(band);

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

                        double maxCorSynch1 = sta1.getWaittimes().midob;
                        double maxCorSynch = maxCorSynch1;
                        double maxCorSynch2 = sta2.getWaittimes().midob;
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

                        unsigned int minScanBl = std::max({bl.getParameters().minScan,
                                                           sta1.getPARA().minScan,
                                                           sta2.getPARA().minScan,
                                                           source.getPARA().minScan});

                        if (new_duration_uint < minScanBl) {
                            new_duration_uint = minScanBl;
                        }
                        unsigned int maxScanBl = std::min({bl.getParameters().maxScan,
                                                           sta1.getPARA().maxScan,
                                                           sta2.getPARA().maxScan,
                                                           source.getPARA().maxScan});

                        if (new_duration_uint > maxScanBl) {
                            continue;
                        }

                        if (new_duration_uint > maxScanDuration) {
                            maxScanDuration = new_duration_uint;
                        }

                        unsigned int maxScanTime =
                                scan.getPointingVector(i, Timestamp::end).getTime() - scan.getPointingVector(i).getTime();

                        if (maxScanDuration > maxScanTime) {
                            maxScanDuration = maxScanTime;
                        }
                    }
                }
                obs.setObservingTime(maxScanDuration);
                newObs.push_back(obs);
            }
            if(newObs.empty()){
                continue;
            }

            unsigned int maxBl = 0;
            for (const auto &any:newObs){
                if(any.getObservingTime()>maxBl){
                    maxBl = any.getObservingTime();
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

            scan.addTagalongStation(pv_new_start, pv_new_end, newObs, *slewtime, station);
            bodyLog << boost::format("    possible to observe source: %-8s (scan: %4d) scan start: %s scan end: %s \n")
                       %source.getName()
                       %counter
                       %TimeSystem::internalTime2timeString(pv_new_start.getTime())
                       %TimeSystem::internalTime2timeString(pv_new_end.getTime());
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
        if(thisSource.getNObs()<thisSource.getOptimization().minNumBaselines) {
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
            excludedBaselines += thisSource.getNObs();
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
        for(auto &any:network_.refStations()){
            any.clearObservations();
        }
        for(auto &any:sources_){
            any.clearObservations();
        }
        for(auto &any:network_.refSkyCoverages()){
            any.clearObservations();
        }

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
            for(auto &sta : network_.refStations()){
                sta.referencePARA().available = endposition->getStationPossible(sta.getId());
            }
            break;
        }
        case StationEndposition::change::end:{
            for(auto &sta : network_.refStations()){
                sta.referencePARA().available = endposition->getStationAvailable(sta.getId());
            }
            break;
        }
    }
}

void Scheduler::startScanSelectionBetweenScans(unsigned int duration, std::ofstream &bodyLog, Scan::ScanType type,
                                               bool output, bool ignoreTagalong) {

    // save number of predefined scans (new scans will be added at end of those)
    auto nMainScans = static_cast<int>(scans_.size());
    if(nMainScans == 0){
        return;
    }

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
            const auto &pv = lastScan.getPointingVector(k, Timestamp::end);
            unsigned long staid = pv.getStaid();
            unsigned int time = pv.getTime();
            Station &thisSta = network_.refStation(staid);
            if(time >= thisSta.getCurrentTime()){
                thisSta.setCurrentPointingVector(pv);
            }
        }

        // loop through all upcoming scans and set endposition
        boost::optional<StationEndposition> endposition(network_.getNSta());
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
        endposition->setStationAvailable(network_.getStations());
        endposition->checkStationPossibility(network_.getStations());


        // check if there was an new upcoming event in the meantime
        unsigned int startTime = lastScan.getTimes().getScanEnd();
        checkForNewEvents(startTime, true, bodyLog);
        if(ignoreTagalong ){
            ignoreTagalongParameter();
        }

        // recursively start scan selection
        boost::optional<Subcon> subcon = boost::none;
        startScanSelection(scans_[i + 1].getTimes().getScanEnd(), bodyLog, type, endposition, subcon, 1);
    }

    // do the same between time at from last scan until duration with no endposition
    if(output){
        bodyLog << "* --- start final scan selection ---\n";
    }

    // get last predefined scan and set current position of station
    Scan &lastScan = scans_[nMainScans-1];
    for(int k=0; k<lastScan.getNSta(); ++k){
        const auto &pv = lastScan.getPointingVector(k, Timestamp::end);
        unsigned long staid = pv.getStaid();
        unsigned int time = pv.getTime();
        Station &thisSta = network_.refStation(staid);
        if(time >= thisSta.getCurrentTime()){
            thisSta.setCurrentPointingVector(pv);
        }
    }
    // check if there was an new upcoming event in the meantime
    unsigned int startTime = lastScan.getTimes().getScanEnd();
    checkForNewEvents(startTime, true, bodyLog);
    if(ignoreTagalong ){
        ignoreTagalongParameter();
    }

    // recursively start scan selection
    boost::optional<Subcon> subcon = boost::none;
    boost::optional<StationEndposition> endposition = boost::none;
    startScanSelection(duration, bodyLog, type, endposition, subcon, 1);

    // sort scans at the end
    sortSchedule(Timestamp::start);
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
        for(auto &thisStation : network_.refStations()){
            PointingVector pv(thisStation.getId(), numeric_limits<unsigned long>::max());
            pv.setTime(time);
            thisStation.setCurrentPointingVector(pv);
            thisStation.referencePARA().firstScan = true;
        }

        // create all possible high impact scan pointing vectors for this time
        himp.possibleHighImpactScans(iTime, network_, sources_);
    }

    // create the actual scans
    himp.updateHighImpactScans(network_, sources_, parameters_.subnetting);

    himp.updateLogfile(bodyLog);

    // select bestScans
    vector<Scan> bestScans;
    do{
        bestScans = himp.highestImpactScans(network_, sources_);
        for(auto& scan:bestScans){
            const Source &source = sources_[scan.getSourceId()];
            if(himp.isCorrectHighImpactScan(scan, scans_, source)){
                update(scan, bodyLog);

                for(auto &thisStation : network_.refStations()){
                    thisStation.referencePARA().firstScan = true;
                }

            }
        }
    }while(himp.hasMoreScans());

    sortSchedule(Timestamp::start);


    bodyLog << "###############################################################################################\n";
    bodyLog << "##                              start with normal scan selection                             ##\n";
    bodyLog << "###############################################################################################\n";


    // reset all events
    resetAllEvents(bodyLog);

    for(auto &thisStation : network_.refStations()){
        PointingVector pv(thisStation.getId(),numeric_limits<unsigned long>::max());
        pv.setTime(0);
        thisStation.setCurrentPointingVector(pv);
        thisStation.referencePARA().firstScan = true;
    }

}

void Scheduler::resetAllEvents(std::ofstream &bodyLog) {

    // reset all events
    for(auto &any:network_.refStations()){
        PointingVector pv(any.getId(),numeric_limits<unsigned long>::max());
        pv.setTime(0);
        any.setCurrentPointingVector(pv);
        any.setNextEvent(0);
    }
    for(auto &any:sources_){
        any.setNextEvent(0);
    }
    for(auto &any:network_.refBaselines()){
        any.setNextEvent(0);
    }
    checkForNewEvents(0, false, bodyLog);
}

void Scheduler::idleToScanTime(ScanTimes::AlignmentAnchor anchor, std::ofstream &bodyLog) {

    switch (anchor){
        case ScanTimes::AlignmentAnchor::start:{
            bodyLog << "Extend observing time at end of scan:\n\n";
            resetAllEvents(bodyLog);
            checkForNewEvents(0, false, bodyLog);

            // start with each scan
            for(int iscan=0; iscan<scans_.size(); ++iscan){
                Scan &thisScan = scans_[iscan];
                checkForNewEvents(thisScan.getTimes().getScanStart(), true, bodyLog);

                // save station ids of this scan (only these stations can be affected)
                vector<unsigned long> staids= thisScan.getStationIds();
                unsigned long nThisSta = staids.size();

                vector<char> found(nThisSta);
                vector<unsigned int> constantTime(nThisSta,0);
                vector<unsigned int> slewTimes(nThisSta,0);

                StationEndposition endp(network_.getNSta());

                ScanTimes copyOfScanTimes = thisScan.getTimes();

                // look in each following scan when each station is next used. Save the next position in endp
                for(int iNextScan = iscan+1; iNextScan<scans_.size(); ++iNextScan){
                    const Scan &nextScan = scans_[iNextScan];

                    // search each station in this next scan
                    for(int idx = 0; idx<nThisSta; ++idx){
                        if(!found[idx]){
                            unsigned long staid = staids[idx];
                            const Station &thisSta = network_.getStation(staid);
                            boost::optional<unsigned long> oidx = nextScan.findIdxOfStationId(staid);

                            // check if station is part of this next scan
                            if(oidx.is_initialized()){

                                // if yes save endposition
                                auto nidx = static_cast<int>(oidx.get());
                                const PointingVector &endposition = nextScan.getPointingVector(nidx);
                                endp.addPointingVectorAsEndposition(endposition);
                                constantTime[idx] = thisSta.getWaittimes().fieldSystem + thisSta.getWaittimes().preob;
                                slewTimes[idx] = nextScan.getTimes().getSlewTime(nidx);
                                found[idx] = true;
                            }
                        }
                    }
                    // if all stations are found you can stop searching for next scan
                    if( all_of(found.begin(), found.end(), [](bool i){ return i; } ) ){
                        break;
                    }
                }

                unsigned long srcid = thisScan.getSourceId();
                const Source & thisSource = sources_[srcid];

                //extend scan time
                for(int idx = 0; idx<nThisSta; ++idx){
                    unsigned long staid = staids[idx];
                    const Station &thisSta = network_.getStation(staid);
                    const PointingVector &start = thisScan.getPointingVector(idx, Timestamp::end);
                    unsigned int startTime = start.getTime();

                    // get variable pointing vector (it will change time)
                    PointingVector variable(start);
                    variable.setId(start.getId());

                    if(found[idx]){
                        const PointingVector &end = endp.getFinalPosition(staid).get();
                        unsigned int availableTime = end.getTime()-startTime;
                        unsigned int idleTime = availableTime - constantTime[idx] - slewTimes[idx];

                        // update azimuth and elevation of variable
                        variable.setTime(startTime + idleTime);
                        thisSta.calcAzEl(thisSource,variable);
                        thisSta.getCableWrap().calcUnwrappedAz(start, variable);

                        // check if cable wrap changes
                        if( abs(start.getAz() - variable.getAz()) > pi/2){
                            continue;
                        }

                        // check if azimuth and elevation is visible
                        if( !thisSta.isVisible(variable,thisSource.getPARA().minElevation) ){
                            continue;
                        }

                        // calc new slew time
                        unsigned int slewTime = thisSta.getAntenna().slewTime(variable,end);

                        // iteratively adjust observing time
                        int offset = 0;
                        bool valid = true;
                        while(slewTime+offset != slewTimes[idx]){
                            offset = slewTimes[idx]-slewTime;

                            // update azimuth and elevation of variable
                            variable.setTime(startTime + idleTime + offset);
                            thisSta.calcAzEl(sources_[srcid],variable);
                            thisSta.getCableWrap().calcUnwrappedAz(start, variable);

                            // check if cable wrap changes
                            if( abs(start.getAz() - variable.getAz()) > pi/2){
                                valid = false;
                                break;
                            }

                            // check if azimuth and elevation is visible
                            if( !thisSta.isVisible(variable,thisSource.getPARA().minElevation) ){
                                valid = false;
                                break;
                            }

                            // get new slewtime
                            slewTime = thisSta.getAntenna().slewTime(variable,end);

                            // live safer: If you are within one second you are ok
                            if(slewTime+offset == slewTimes[idx]-1){
                                break;
                            }
                        }

                        // continue if azimuth and elevation is not visible
                        if(!valid){
                            continue;
                        }

                        // if scan time would be reduced skip station!
                        if(offset + static_cast<int>(idleTime) < 0){
                            continue;
                        }

                        // adjust observing times
                        thisScan.setPointingVector(idx, move(variable), Timestamp::end);

                    }else{
                        // update azimuth and elevation of variable
                        variable.setTime(TimeSystem::duration);
                        thisSta.calcAzEl(sources_[srcid],variable);
                        thisSta.getCableWrap().calcUnwrappedAz(start, variable);

                        // check if azimuth and elevation is visible and adjust observing times
                        if( thisSta.isVisible(variable,thisSource.getPARA().minElevation) ){
                            thisScan.setPointingVector(idx, move(variable), Timestamp::end);
                        }
                    }
                }

                // get minimum allowed observing times (until station is not available) and change scheduled time if necessary
                for(int idx=0; idx<nThisSta; ++idx){
                    unsigned long staid = staids[idx];
                    const Station &sta = network_.getStation(staid);
                    unsigned int maxObsTime = sta.maximumAllowedObservingTime(Timestamp::end);
                    thisScan.removeAdditionalObservingTime(maxObsTime, sta, thisSource, bodyLog, Timestamp::end);
                }

                thisScan.removeUnnecessaryObservingTime(network_, thisSource, bodyLog, Timestamp::end);


                bool change = false;
                for(int i=0; i<nThisSta; ++i) {
                    unsigned int oldObservingTime = copyOfScanTimes.getObservingTime(i);
                    unsigned int newObservingTime = thisScan.getTimes().getObservingTime(i);
                    if(oldObservingTime != newObservingTime){
                        change = true;
                        break;
                    }
                }

                if(change){
                    bodyLog << boost::format("Scan (id: %d) source %-8s changing idle time to observing time:\n") % thisScan.getId() % thisSource.getName();
                    for(int i=0; i<nThisSta; ++i){
                        unsigned long staid = thisScan.getStationId(i);
                        unsigned int oldObservingTime = copyOfScanTimes.getObservingTime(i);
                        unsigned int newObservingTime = thisScan.getTimes().getObservingTime(i);
                        if(oldObservingTime == newObservingTime){
                            continue;
                        }

                        string id = thisScan.getPointingVector(i, Timestamp::end).printId();

                        bodyLog << (boost::format("    %-8s %+4d seconds: new observing time: %s - %s (%3d sec) old observing time %s - %s (%3d sec) %s\n")
                                    % network_.getStation(staid).getName()
                                    %(static_cast<int>(newObservingTime-oldObservingTime))
                                    % TimeSystem::internalTime2timeString(thisScan.getTimes().getObservingStart(i))
                                    % TimeSystem::internalTime2timeString(thisScan.getTimes().getObservingEnd(i))
                                    % newObservingTime
                                    % TimeSystem::internalTime2timeString(copyOfScanTimes.getObservingStart(i))
                                    % TimeSystem::internalTime2timeString(copyOfScanTimes.getObservingEnd(i))
                                    % oldObservingTime
                                    % id).str();
                    }
                }
            }
            bodyLog << "\n";
            break;
        }
        case ScanTimes::AlignmentAnchor::end:{
            bodyLog << "Extend observing time at start of scan:\n\n";
            resetAllEvents(bodyLog);
            checkForNewEvents(0, false, bodyLog);

            // sort scans per endtime
            sortSchedule(Timestamp::end);

            // loop over each scan
            for(int iscan=0; iscan<scans_.size(); ++iscan) {
                Scan &thisScan = scans_[iscan];
                checkForNewEvents(thisScan.getTimes().getScanStart(), true, bodyLog);

                // save station ids of this scan (only these stations can be affected)
                vector<unsigned long> staids = thisScan.getStationIds();
                unsigned long nThisSta = staids.size();

                vector<char> found(nThisSta);

                StationEndposition endp(network_.getNSta());

                ScanTimes copyOfScanTimes = thisScan.getTimes();

                // look in each previous scan when each station is next used.
                for (int iPrevScan = iscan - 1; iPrevScan >= 0; --iPrevScan) {
                    const Scan &prevScan = scans_[iPrevScan];

                    // search each station in this next scan
                    for (int idx = 0; idx < nThisSta; ++idx) {
                        if (!found[idx]) {
                            unsigned long staid = staids[idx];
                            const Station &thisSta = network_.getStation(staid);
                            boost::optional<unsigned long> oidx = prevScan.findIdxOfStationId(staid);

                            // check if station is part of this next scan
                            if (oidx.is_initialized()) {

                                // if yes save endposition
                                auto nidx = static_cast<int>(oidx.get());
                                const PointingVector &endposition = prevScan.getPointingVector(nidx, Timestamp::end);
                                endp.addPointingVectorAsEndposition(endposition);
                                found[idx] = true;
                            }
                        }
                    }
                    // if all stations are found you can stop searching for next scan
                    if (all_of(found.begin(), found.end(), [](bool i) { return i; })) {
                        break;
                    }
                }

                unsigned long srcid = thisScan.getSourceId();
                const Source &thisSource = sources_[srcid];

                //extend scan time
                for (int idx = 0; idx < nThisSta; ++idx) {
                    unsigned long staid = staids[idx];
                    const Station &thisSta = network_.getStation(staid);

                    const PointingVector &end = thisScan.getPointingVector(idx);
                    unsigned int endTime = end.getTime();

                    // get variable pointing vector (it will change time)
                    PointingVector variable(end);
                    variable.setId(end.getId());

                    if (found[idx]) {
                        const PointingVector &start = endp.getFinalPosition(staid).get();
                        unsigned int prevSlewTime = thisSta.getAntenna().slewTime(start,end);

                        unsigned int availableTime = end.getTime() - start.getTime();
                        unsigned int idleTime = availableTime - thisScan.getTimes().getFieldSystemTime(idx) -
                                thisScan.getTimes().getPreobTime(idx) - prevSlewTime;

                        // update azimuth and elevation of variable
                        variable.setTime(endTime - idleTime);
                        thisSta.calcAzEl(thisSource, variable);
                        thisSta.getCableWrap().calcUnwrappedAz(end, variable);

                        // check if cable wrap changes
                        if (abs(end.getAz() - variable.getAz()) > pi / 2) {
                            continue;
                        }

                        // check if azimuth and elevation is visible
                        if (!thisSta.isVisible(variable, thisSource.getPARA().minElevation)) {
                            continue;
                        }

                        // calc new slew time
                        unsigned int slewTime = thisSta.getAntenna().slewTime(start, variable);

                        // iteratively adjust observing time
                        int offset = 0;
                        bool valid = true;
                        while (slewTime + offset != prevSlewTime) {
                            offset = prevSlewTime - slewTime;

                            // update azimuth and elevation of variable
                            variable.setTime(endTime - idleTime - offset);
                            thisSta.calcAzEl(sources_[srcid], variable);
                            thisSta.getCableWrap().calcUnwrappedAz(end, variable);

                            // check if cable wrap changes
                            if (abs(end.getAz() - variable.getAz()) > pi / 2) {
                                valid = false;
                                break;
                            }

                            // check if azimuth and elevation is visible
                            if (!thisSta.isVisible(variable, thisSource.getPARA().minElevation)) {
                                valid = false;
                                break;
                            }

                            // get new slewtime
                            slewTime = thisSta.getAntenna().slewTime(start, variable);

                            // live safer: If you are within one second you are ok
                            if (slewTime + offset == prevSlewTime - 1) {
                                break;
                            }
                        }

                        // continue if azimuth and elevation is not visible
                        if (!valid) {
                            continue;
                        }

                        // if scan time would be reduced skip station!
                        if (offset + static_cast<int>(idleTime) < 0) {
                            continue;
                        }

                        // adjust observing times
                        thisScan.setPointingVector(idx, move(variable), Timestamp::start);

                    } else {
                        // update azimuth and elevation of variable
                        variable.setTime(0);
                        thisSta.calcAzEl(sources_[srcid], variable);
                        thisSta.getCableWrap().calcUnwrappedAz(end, variable);

                        // check if azimuth and elevation is visible and adjust observing times
                        if (thisSta.isVisible(variable, thisSource.getPARA().minElevation)) {
                            thisScan.setPointingVector(idx, move(variable), Timestamp::start);
                        }
                    }
                }


                // get maximum allowed observing times (until station is not available) and change scheduled time if necessary
                for(int idx=0; idx<nThisSta; ++idx){
                    unsigned long staid = staids[idx];
                    const Station &sta = network_.getStation(staid);
                    unsigned int minObsTime = sta.maximumAllowedObservingTime(Timestamp::start);
                    thisScan.removeAdditionalObservingTime(minObsTime, sta, thisSource, bodyLog, Timestamp::start);
                }

                thisScan.removeUnnecessaryObservingTime(network_, thisSource, bodyLog, Timestamp::start);


                bool change = false;
                for(int i=0; i<nThisSta; ++i) {
                    unsigned int oldObservingTime = copyOfScanTimes.getObservingTime(i);
                    unsigned int newObservingTime = thisScan.getTimes().getObservingTime(i);
                    if(oldObservingTime != newObservingTime){
                        change = true;
                        break;
                    }
                }

                if(change){
                    bodyLog << boost::format("Scan (id: %d) source %-8s changing idle time to observing time:\n") % thisScan.getId() % thisSource.getName();
                    for(int i=0; i<nThisSta; ++i){
                        unsigned long staid = thisScan.getStationId(i);
                        unsigned int oldObservingTime = copyOfScanTimes.getObservingTime(i);
                        unsigned int newObservingTime = thisScan.getTimes().getObservingTime(i);
                        if(oldObservingTime == newObservingTime){
                            continue;
                        }

                        string id = thisScan.getPointingVector(i).printId();

                        bodyLog << (boost::format("    %-8s %+4d seconds: new observing time: %s - %s (%3d sec) old observing time %s - %s (%3d sec) %s\n")
                                    % network_.getStation(staid).getName()
                                    %(static_cast<int>(newObservingTime-oldObservingTime))
                                    % TimeSystem::internalTime2timeString(thisScan.getTimes().getObservingStart(i))
                                    % TimeSystem::internalTime2timeString(thisScan.getTimes().getObservingEnd(i))
                                    % newObservingTime
                                    % TimeSystem::internalTime2timeString(copyOfScanTimes.getObservingStart(i))
                                    % TimeSystem::internalTime2timeString(copyOfScanTimes.getObservingEnd(i))
                                    % oldObservingTime
                                    % id).str();
                    }
                }
            }

            // sort scans back with respect to observing start
            sortSchedule(Timestamp::start);

            break;
        }
        case ScanTimes::AlignmentAnchor::individual:{
            bodyLog << "idle to scan time is not supported for individual scan alignment:\n\n";
//            idleToScanTime(ScanTimes::AlignmentAnchor::start, bodyLog);
//            idleToScanTime(ScanTimes::AlignmentAnchor::end, bodyLog);
            break;
        }
    }
}

void Scheduler::idleToScanTime(Timestamp ts, std::ofstream &bodyLog) {

    switch (ts){

        case Timestamp::start:
            bodyLog << "increasing observing time at start of scan\n*\n";
            break;
        case Timestamp::end:
            bodyLog << "increasing observing time at end of scan\n*\n";
            break;
    }

    // hard copy previouse observing times
    map<unsigned long, ScanTimes> oldScanTimes;

    for(const auto &scan : scans_){
        oldScanTimes.insert({scan.getId(), scan.getTimes()});
    }


    for(const auto &thisSta : network_.getStations()){

        // reset all events
        resetAllEvents(bodyLog);
        unsigned long staid = thisSta.getId();

        // sort schedule based on this station
        sortSchedule(staid);

        bool first = true;

        // loop over all scans
        for(int iscan1 = 0; iscan1<scans_.size(); ++iscan1){

            // get scan1
            auto &scan1 = scans_[iscan1];

            // skip scan if it does not contain this station
            if(!scan1.findIdxOfStationId(staid).is_initialized()){
                continue;
            }

            // increase observing time to session start in case of first scan
            if(first && ts == Timestamp::start){
                first = false;

                int staidx1 = static_cast<int>(*scan1.findIdxOfStationId(staid));
                const Source &thisSource = sources_[scan1.getSourceId()];
                const PointingVector &pv1 = scan1.getPointingVector(staidx1, Timestamp::start);

                if(pv1.getTime() != 0){
                    PointingVector variable(pv1);
                    variable.setId(pv1.getId());
                    variable.setTime(0);
                    thisSta.calcAzEl(thisSource,variable);
                    thisSta.getCableWrap().calcUnwrappedAz(pv1, variable);
                    bool valid = true;
                    // check if cable wrap changes
                    if( abs(pv1.getAz() - variable.getAz()) > pi/2){
                        valid = false;
                    }
                    // check if azimuth and elevation is visible
                    if( !thisSta.isVisible(variable,thisSource.getPARA().minElevation) ){
                        valid = false;
                    }
                    if(valid){
                        scan1.setPointingVector(staidx1, move(variable), Timestamp::start);
                    }
                }
            }

            // check for new events
            checkForNewEvents(scan1.getTimes().getScanStart(), true, bodyLog);

            // look for index of next scan with this station
            int iscan2 = iscan1+1;
            while(iscan2 < scans_.size() && !scans_[iscan2].findIdxOfStationId(staid).is_initialized()){
                ++iscan2;
            }

            // continue with next station if end of schedule is reached
            if(iscan2 == scans_.size()){
                continue;
            }

            // get scan2
            auto &scan2 = scans_[iscan2];

            // get indices
            auto staidx1 = static_cast<int>(*scan1.findIdxOfStationId(staid));
            auto staidx2 = static_cast<int>(*scan2.findIdxOfStationId(staid));

            // get pointing vectors
            const PointingVector &pv1 = scan1.getPointingVector(staidx1, Timestamp::end);
            const PointingVector &pv2 = scan2.getPointingVector(staidx2, Timestamp::start);

            // get times
            unsigned int availableTime = pv2.getTime()-pv1.getTime();
            unsigned int prevSlewTime = thisSta.getAntenna().slewTime(pv1,pv2);
            unsigned int fsTime = thisSta.getWaittimes().fieldSystem;
            unsigned int preobTime = thisSta.getWaittimes().preob;

            // avoid rounding errors
            if( availableTime < prevSlewTime-fsTime-preobTime ){
                continue;
            }

            // calc idle time
            unsigned int idleTime = availableTime-prevSlewTime-fsTime-preobTime;
            if(idleTime == 0){
                continue;
            }

            // if ts == end: time will be added after pv1
            PointingVector ref = scan1.getPointingVector(staidx1, Timestamp::end);
            ref.setId(pv1.getId());
            unsigned int variableStartTime = pv1.getTime() + idleTime;
            // if ts == start: time will be added before pv2
            if(ts == Timestamp::start){
                ref = scan2.getPointingVector(staidx2, Timestamp::start);
                ref.setId(pv2.getId());
                variableStartTime = pv2.getTime() - idleTime;
            }

            const auto &thisSource = sources_[ref.getSrcid()];

            PointingVector variable(ref);
            variable.setId(ref.getId());

            // set new time for variable
            variable.setTime(variableStartTime);

            // calc new azimuth and elevation
            thisSta.calcAzEl(thisSource, variable);

            // unwrap cable wrap near reference pointing vector
            thisSta.getCableWrap().calcUnwrappedAz(ref, variable);

            // check if cable wrap changes
            if( abs(ref.getAz() - variable.getAz()) > pi/2){
                continue;
            }

            // check if azimuth and elevation is visible
            if( !thisSta.isVisible(variable,thisSource.getPARA().minElevation) ){
                continue;
            }

            // calc new slew time
            unsigned int slewTime;
            switch (ts){
                case Timestamp::start:{
                    slewTime = thisSta.getAntenna().slewTime(pv1,variable);
                    break;
                }
                case Timestamp::end:{
                    slewTime = thisSta.getAntenna().slewTime(variable,pv2);
                    break;
                }
            }

            // iteratively adjust new idle time and new slew time until it is equal to previouse slew time
            // we also allow 1 second offset (1 sec additional idle time) as a live saver
            int offset = 0;
            bool valid = true;
            while(slewTime + offset != prevSlewTime && slewTime + offset != prevSlewTime-1){
                offset = prevSlewTime - slewTime;

                switch (ts){
                    case Timestamp::start:{
                        variable.setTime(variableStartTime - offset);
                        break;
                    }
                    case Timestamp::end:{
                        variable.setTime(variableStartTime + offset);
                        break;
                    }
                }

                thisSta.calcAzEl(thisSource,variable);
                thisSta.getCableWrap().calcUnwrappedAz(ref, variable);

                // check if cable wrap changes
                if( abs(ref.getAz() - variable.getAz()) > pi/2){
                    valid = false;
                    break;
                }

                // check if azimuth and elevation is visible
                if( !thisSta.isVisible(variable,thisSource.getPARA().minElevation) ){
                    valid = false;
                    break;
                }

                // get new slewtime
                switch (ts){
                    case Timestamp::start:{
                        slewTime = thisSta.getAntenna().slewTime(pv1,variable);
                        break;
                    }
                    case Timestamp::end:{
                        slewTime = thisSta.getAntenna().slewTime(variable,pv2);
                        break;
                    }
                }
            }

            // continue if source is still visible
            if(!valid){
                continue;
            }

            // if scan time would be reduced skip station!
            if(offset + static_cast<int>(idleTime) < 0){
                continue;
            }

            // adjust observing times
            switch (ts){
                case Timestamp::start:{
                    scan2.setPointingVector(staidx2, move(variable), Timestamp::start);
                    break;
                }
                case Timestamp::end:{
                    scan1.setPointingVector(staidx1, move(variable), Timestamp::end);
                    break;
                }
            }
        }

        // extend observing time until session end in case of last scan
        if(ts == Timestamp::end){
            for(int iscan = scans_.size()-1; iscan >= 0; --iscan){
                // get lastScan
                auto &lastScan = scans_[iscan];
                auto oidx = lastScan.findIdxOfStationId(staid);

                // skip scan if it does not contain this station
                if(oidx.is_initialized()){
                    auto staidx1 = static_cast<int>(*oidx);

                    const Source &thisSource = sources_[lastScan.getSourceId()];
                    const PointingVector &pv1 = lastScan.getPointingVector(staidx1, Timestamp::end);

                    if(pv1.getTime() != 0){
                        PointingVector variable(pv1);
                        variable.setId(pv1.getId());
                        variable.setTime(TimeSystem::duration);
                        thisSta.calcAzEl(thisSource,variable);
                        thisSta.getCableWrap().calcUnwrappedAz(pv1, variable);
                        bool valid = true;
                        // check if cable wrap changes
                        if( abs(pv1.getAz() - variable.getAz()) > pi/2){
                            valid = false;
                        }
                        // check if azimuth and elevation is visible
                        if( !thisSta.isVisible(variable,thisSource.getPARA().minElevation) ){
                            valid = false;
                        }
                        if(valid){
                            lastScan.setPointingVector(staidx1, move(variable), Timestamp::end);
                        }
                    }
                    break;
                }
            }
        }
    }

    sortSchedule(Timestamp::start);

    resetAllEvents(bodyLog);
    // remove unnecessary observing times
    for(auto &thisScan : scans_){
        const Source &thisSource = sources_[thisScan.getSourceId()];

        // loop over all stations and check for down times
        for(int staidx = 0; staidx<thisScan.getNSta(); ++staidx){
            unsigned long staid = thisScan.getStationId(staidx);
            Station &thisSta = network_.refStation(staid);
            bool dummy;

            // change event to time of scan
            switch (ts) {
                case Timestamp::start: {
                    thisSta.checkForNewEvent(thisScan.getPointingVector(staidx, Timestamp::end).getTime(), dummy);
                    break;
                }
                case Timestamp::end: {
                    thisSta.checkForNewEvent(thisScan.getPointingVector(staidx, Timestamp::start).getTime(), dummy);
                    break;
                }
            }

            // look for maximum allowed time before/after current event time
            unsigned int maximum = thisSta.maximumAllowedObservingTime(ts);


            // check if it is necessary to adjust observing time
            const PointingVector &pv1 = thisScan.getPointingVector(staidx, ts);
            bool changeToMaximum = false;
            switch (ts){
                case Timestamp::start:{
                    if(pv1.getTime() < maximum){
                        changeToMaximum = true;
                    }
                    break;
                }
                case Timestamp::end:{
                    if(pv1.getTime() > maximum){
                        changeToMaximum = true;
                    }
                    break;
                }
            }

            // adjust observing time
            if(changeToMaximum){
                PointingVector variable(pv1);
                variable.setId(pv1.getId());
                variable.setTime(maximum);
                thisSta.calcAzEl(thisSource,variable);
                thisSta.getCableWrap().calcUnwrappedAz(pv1, variable);
                bool valid = true;
                // check if cable wrap changes
                if( abs(pv1.getAz() - variable.getAz()) > pi/2){
                    valid = false;
                }
                // check if azimuth and elevation is visible
                if( !thisSta.isVisible(variable,thisSource.getPARA().minElevation) ){
                    valid = false;
                }
                thisScan.setPointingVector(staidx, move(variable), ts);
                if(!valid){
                    bodyLog << (boost::format("ERROR while extending observing time to idle time:\n    "
                                              "source %s might not be visible from %s during %s. ")
                                % thisSource.getName()
                                % thisSta.getName()
                                %TimeSystem::internalTime2timeString(maximum)).str();
                }
            }
        }

        thisScan.removeUnnecessaryObservingTime(network_, thisSource, bodyLog, ts);
    }


    // output
    for (const auto &scan : scans_) {

        const ScanTimes &copyOfScanTimes = oldScanTimes.at(scan.getId());
        const Source &source = sources_[scan.getSourceId()];

        // check if output is required
        bool change = false;
        for(int i=0; i<scan.getNSta(); ++i) {
            unsigned int oldObservingTime = copyOfScanTimes.getObservingTime(i);
            unsigned int newObservingTime = scan.getTimes().getObservingTime(i);
            if(oldObservingTime != newObservingTime){
                change = true;
                break;
            }
        }

        // output if there was a change
        if(change){
            bodyLog << boost::format("Scan (id: %d) source %-8s changing idle time to observing time:\n") % scan.getId() % source.getName();
            for(int i=0; i<scan.getNSta(); ++i){
                unsigned long staid = scan.getStationId(i);
                unsigned int oldObservingTime = copyOfScanTimes.getObservingTime(i);
                unsigned int newObservingTime = scan.getTimes().getObservingTime(i);
                if(oldObservingTime == newObservingTime){
                    continue;
                }

                string id = scan.getPointingVector(i, ts).printId();

                bodyLog << (boost::format("    %-8s %+5d seconds: new observing time: %s - %s (%4d sec) old observing time %s - %s (%4d sec) %s\n")
                            % network_.getStation(staid).getName()
                            %(static_cast<int>(newObservingTime)-static_cast<int>(oldObservingTime))
                            % TimeSystem::internalTime2timeString(scan.getTimes().getObservingStart(i))
                            % TimeSystem::internalTime2timeString(scan.getTimes().getObservingEnd(i))
                            % newObservingTime
                            % TimeSystem::internalTime2timeString(copyOfScanTimes.getObservingStart(i))
                            % TimeSystem::internalTime2timeString(copyOfScanTimes.getObservingEnd(i))
                            % oldObservingTime
                            % id).str();
            }
        }
    }
    bodyLog << "*\n";
    bodyLog << "* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n";
    bodyLog << "*\n";

}


void Scheduler::sortSchedule(Timestamp ts) {
    switch (ts){

        case Timestamp::start:{
            stable_sort(scans_.begin(),scans_.end(), [](const Scan &scan1, const Scan &scan2){
                return scan1.getTimes().getObservingStart() < scan2.getTimes().getObservingStart();
            });

            break;
        }
        case Timestamp::end:{
            stable_sort(scans_.begin(),scans_.end(), [](const Scan &scan1, const Scan &scan2){
                return scan1.getTimes().getObservingEnd() < scan2.getTimes().getObservingEnd();
            });

            break;
        }
    }


}

void Scheduler::sortSchedule(unsigned long staid, Timestamp ts) {

    switch (ts){

        case Timestamp::start:{
            stable_sort(scans_.begin(),scans_.end(), [staid](const Scan &scan1, const Scan &scan2){

                boost::optional<unsigned long> idx1 = scan1.findIdxOfStationId(staid);
                boost::optional<unsigned long> idx2 = scan2.findIdxOfStationId(staid);

                if(!idx1.is_initialized() && !idx2.is_initialized()){

                    return scan1.getTimes().getObservingStart() <
                           scan2.getTimes().getObservingStart();

                } else if(idx1.is_initialized() && !idx2.is_initialized()){

                    return scan1.getTimes().getObservingStart(static_cast<int>(*idx1)) <
                           scan2.getTimes().getObservingStart();

                } else if(!idx1.is_initialized() && idx2.is_initialized()){

                    return scan1.getTimes().getObservingStart() <
                            scan2.getTimes().getObservingStart(static_cast<int>(*idx2));

                } else {

                    return scan1.getTimes().getObservingStart(static_cast<int>(*idx1)) <
                           scan2.getTimes().getObservingStart(static_cast<int>(*idx2));

                }
            });


            break;
        }
        case Timestamp::end:{
            stable_sort(scans_.begin(),scans_.end(), [staid](const Scan &scan1, const Scan &scan2){

                boost::optional<unsigned long> idx1 = scan1.findIdxOfStationId(staid);
                boost::optional<unsigned long> idx2 = scan2.findIdxOfStationId(staid);

                if(!idx1.is_initialized() && !idx2.is_initialized()){

                    return scan1.getTimes().getObservingEnd() <
                           scan2.getTimes().getObservingEnd();

                } else if(idx1.is_initialized() && !idx2.is_initialized()){

                    return scan1.getTimes().getObservingEnd(static_cast<int>(*idx1)) <
                           scan2.getTimes().getObservingEnd();

                } else if(!idx1.is_initialized() && idx2.is_initialized()){

                    return scan1.getTimes().getObservingEnd() <
                           scan2.getTimes().getObservingEnd(static_cast<int>(*idx2));

                } else {

                    return scan1.getTimes().getObservingEnd(static_cast<int>(*idx1)) <
                           scan2.getTimes().getObservingEnd(static_cast<int>(*idx2));

                }
            });


            break;
        }
    }


}


