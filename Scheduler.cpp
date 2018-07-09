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
        sort(scans_.begin(),scans_.end(), [](const Scan &scan1, const Scan &scan2){
            return scan1.getTimes().getObservingStart() < scan2.getTimes().getObservingStart();
        });
    } else {
        startScanSelectionBetweenScans(TimeSystem::duration, bodyLog, Scan::ScanType::standard, true, false);
    }

    // start fillinmode a posterior
    if(parameters_.fillinmodeAPosteriori){
        bodyLog << "* --- start fillin mode a posteriori ---\n";
        startScanSelectionBetweenScans(TimeSystem::duration, bodyLog, Scan::ScanType::fillin, false, true);
    }

    if(parameters_.idleToObservingTime){
        idleToScanTime(bodyLog);
    }

    // check if there was an error during the session
    if (!checkAndStatistics(bodyLog)) {
        cout << boost::format("%s iteration %d ERROR: there was an error while checking the schedule (see log file);\n")
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
        const PointingVector &pv_end = scan.getPointingVectors_endtime(i);
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

    bodyLog << "\n=======================   starting check routine   =======================\n";
    bodyLog << "starting check routine!\n";

    int countErrors = 0;
    int countWarnings = 0;

    for (auto& thisStation : network_.refStations()){
        bodyLog << "    checking station " << thisStation.getName() << ":\n";
        unsigned long staid = thisStation.getId();
        unsigned int constTimes = thisStation.getWaittimes().fieldSystem + thisStation.getWaittimes().preob;

        // sort scans based on observation start of this station (can be different if you align scans individual or at end)
        sort(scans_.begin(),scans_.end(), [staid](const Scan &scan1, const Scan &scan2){
            boost::optional<unsigned long> idx1 = scan1.findIdxOfStationId(staid);
            if(!idx1.is_initialized()){
                return scan1.getTimes().getObservingStart() < scan2.getTimes().getObservingStart();
            }

            boost::optional<unsigned long> idx2 = scan2.findIdxOfStationId(staid);
            if(!idx2.is_initialized()){
                return scan1.getTimes().getObservingStart() < scan2.getTimes().getObservingStart();
            }

            return scan1.getTimes().getObservingStart(static_cast<int>(*idx1)) < scan2.getTimes().getObservingStart(
                    static_cast<int>(*idx2));
        });



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
            const PointingVector &thisEnd = scan_thisEnd.getPointingVectors_endtime(idx_thisEnd);

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
    bodyLog << "=========================   end check routine    =========================\n";

    // sort scans again based on their observation start
    sort(scans_.begin(),scans_.end(), [](const Scan &scan1, const Scan &scan2){
        return scan1.getTimes().getObservingStart() < scan2.getTimes().getObservingStart();
    });

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
                                scan.getPointingVectors_endtime(i).getTime() - scan.getPointingVector(i).getTime();

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
            const auto &pv = lastScan.getPointingVectors_endtime(k);
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
        const auto &pv = lastScan.getPointingVectors_endtime(k);
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

    sort(scans_.begin(),scans_.end(), [](const Scan &scan1, const Scan &scan2){
        return scan1.getTimes().getObservingStart() < scan2.getTimes().getObservingStart();
    });


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

void Scheduler::idleToScanTime(std::ofstream &bodyLog) {

    switch (ScanTimes::getAlignmentAnchor()){
        case ScanTimes::AlignmentAnchor::start:{
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
                    const PointingVector &start = thisScan.getPointingVectors_endtime(idx);
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
                        bool visible = true;
                        while(slewTime+offset != slewTimes[idx]){
                            offset = slewTimes[idx]-slewTime;

                            // update azimuth and elevation of variable
                            variable.setTime(startTime + idleTime + offset);
                            thisSta.calcAzEl(sources_[srcid],variable);
                            thisSta.getCableWrap().calcUnwrappedAz(start, variable);

                            // check if azimuth and elevation is visible
                            if( !thisSta.isVisible(variable,thisSource.getPARA().minElevation) ){
                                visible = false;
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
                        if(!visible){
                            continue;
                        }

                        // adjust observing times
                        thisScan.setPointingVectorEnd(idx, move(variable));

                    }else{
                        // update azimuth and elevation of variable
                        variable.setTime(TimeSystem::duration);
                        thisSta.calcAzEl(sources_[srcid],variable);
                        thisSta.getCableWrap().calcUnwrappedAz(start, variable);

                        // check if azimuth and elevation is visible and adjust observing times
                        if( thisSta.isVisible(variable,thisSource.getPARA().minElevation) ){
                            thisScan.setPointingVectorEnd(idx, move(variable));
                        }
                    }
                }

                thisScan.removeUnnecessaryObservingTime(network_, thisSource, bodyLog);


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

                        bodyLog << (boost::format("    %-8s %+4d seconds: new observing time: %s - %s (%3d sec) old observing time %s - %s (%3d sec)\n")
                                    % network_.getStation(staid).getName()
                                    %(newObservingTime-oldObservingTime)
                                    % TimeSystem::internalTime2timeString(thisScan.getTimes().getObservingStart(i))
                                    % TimeSystem::internalTime2timeString(thisScan.getTimes().getObservingEnd(i))
                                    % newObservingTime
                                    % TimeSystem::internalTime2timeString(copyOfScanTimes.getObservingStart(i))
                                    % TimeSystem::internalTime2timeString(copyOfScanTimes.getObservingEnd(i))
                                    % oldObservingTime).str();
                    }
                }
            }
            break;
        }
        case ScanTimes::AlignmentAnchor::end:{

            bodyLog << "Idle to scan time is not supported for scan alignment end";
            break;
        }
        case ScanTimes::AlignmentAnchor::individual:{

            bodyLog << "Idle to scan time is not supported for scan alignment individual";
            break;
        }
    }
}


