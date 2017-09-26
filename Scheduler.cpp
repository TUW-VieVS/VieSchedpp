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

Scheduler::Scheduler() = default;

Scheduler::Scheduler(Initializer &init) : stations_{std::move(init.stations_)}, sources_{std::move(init.sources_)},
                                          skyCoverages_{std::move(init.skyCoverages_)}, xml_{std::move(init.xml_)} {

    parameters_.subnetting = init.parameters_.subnetting;
    parameters_.fillinmode = init.parameters_.fillinmode;
    parameters_.writeSkyCoverageData = false;

    preCalculated_.subnettingSrcIds = std::move(init.preCalculated_.subnettingSrcIds);

    nSingleScansConsidered = 0;
    nSubnettingScansConsidered = 0;
    nFillinScansConsidered = 0;
}

Scheduler::~Scheduler() = default;

void Scheduler::start(ofstream &bodyLog) noexcept {

    unsigned int nsrc = countAvailableSources();
    bodyLog << "number of available sources: " << nsrc << "\n";

    outputHeader(stations_, bodyLog);

//        displaySummaryOfStaticMembersForDebugging(bodyLog);
//        printHorizonMasksForDebugging();

//    VieVS::Scan::nScanSelections = 0;

    while (true) {
        Subcon subcon = createSubcon(parameters_.subnetting);
        consideredUpdate(subcon.getNumberSingleScans(), subcon.getNumberSubnettingScans(), bodyLog);
        subcon.precalcScore(stations_, sources_);
        subcon.generateScore(stations_, sources_, skyCoverages_, sources_.size());


        boost::optional<unsigned long> bestIdx_opt = subcon.rigorousScore(stations_, sources_, skyCoverages_);
        if (!bestIdx_opt) {
            bodyLog << "ERROR! no valid scan found!\n";
            continue;
        }
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

            if (bestScan1.maxTime() > bestScan2.maxTime()) {
                swap(bestScan1, bestScan2);
            }
            bestScans.push_back(bestScan1);
            bestScans.push_back(bestScan2);
        }


//        cout << setprecision(18) << "mjd=" << TimeSystem::mjdStart + (double)bestScans[0].getPointingVector(0).getTime()/86400.0 << "\n";
//        cout << setprecision(18) << "ra=" << sources_[bestScans[0].getSourceId()].getRa() << "\n";
//        cout << setprecision(18) << "de=" << sources_[bestScans[0].getSourceId()].getDe() << "\n";
//        cout << setprecision(18) << "lon=" << stations_[bestScans[0].getPointingVector(0).getStaid()].getPosition().getLon() << "\n";
//        cout << setprecision(18) << "lat=" << stations_[bestScans[0].getPointingVector(0).getStaid()].getPosition().getLat() << "\n";
//        cout << setprecision(18) << "az_=" << bestScans[0].getPointingVector(0).getAz() << "\n";
//        cout << setprecision(18) << "el_=" << bestScans[0].getPointingVector(0).getEl() << "\n";



        unsigned int all_maxTime = 0;
        for (const auto &any:bestScans) {
            if (any.maxTime() > all_maxTime) {
                all_maxTime = any.maxTime();
            }
        }
        if( all_maxTime > TimeSystem::duration){
            break;
        }

        bool hardBreak = checkForNewEvent(all_maxTime, true, bodyLog);
        if (hardBreak) {
            continue;
        }


        if (parameters_.fillinmode && !scans_.empty()) {
            start_fillinMode(bestScans, bodyLog);
        } else {
            for (const auto &bestScan : bestScans) {
                update(bestScan, bodyLog);
            }
        }

        ++Scan::nScanSelections;
        if(Scan::scanSequence.customScanSequence){
            Scan::scanSequence.newScan();
        }
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
                    if(all_maxTime >= CalibratorBlock::nextBlock){
                        startCalibrationBlock(bodyLog);
                        CalibratorBlock::nextBlock += CalibratorBlock::cadence;
                    }
                    break;
                }
            }
        }

//            bool flag = check(bodyLog);
    }


    bodyLog << "TOTAL SUMMARY:\n";
    bodyLog << "created single source scans: " << nSingleScansConsidered << "\n";
    bodyLog << "created subnetting scans:    " << nSubnettingScansConsidered << "\n";
    bodyLog << "created fillin mode scans:   " << nFillinScansConsidered << "\n";
    bodyLog << "total scans considered:      " << nSingleScansConsidered + 2 * nSubnettingScansConsidered + nFillinScansConsidered
            << "\n";

//        if(PARA.writeSkyCoverageData){
//            saveSkyCoverageMain();
//            for (unsigned int i = 0; i < TimeSystem::duration; i+=30) {
//                saveSkyCoverageData(i);
//            }
//        }

    bool everythingOk = check(bodyLog);
    if (!everythingOk) {
        cout << "########################################################### ERROR ######################################################\n";
    }

}

Subcon Scheduler::createSubcon(bool subnetting, bool calibrator) noexcept {
    Subcon subcon = allVisibleScans();
    subcon.calcStartTimes(stations_, sources_);
    subcon.updateAzEl(stations_, sources_);
    subcon.constructAllBaselines(sources_);
    subcon.calcAllBaselineDurations(stations_, sources_);
    subcon.calcAllScanDurations(stations_, sources_);

    if (subnetting) {
        subcon.createSubnettingScans(preCalculated_.subnettingSrcIds, static_cast<int>(stations_.size() * 0.66),
                                     sources_);
    }

    if(calibrator){
        subcon.changeScanTypes(Scan::ScanType::calibrator);
    }

    return subcon;
}


Subcon Scheduler::allVisibleScans() noexcept {
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

        if (!*thisSource.getPARA().available) {
            continue;
        }

        if (thisSource.lastScanTime() != 0 &&
            currentTime - thisSource.lastScanTime() < *thisSource.getPARA().minRepeat) {
            continue;
        }

        if (thisSource.getNscans() >= *thisSource.getPARA().maxNumberOfScans) {
            continue;
        }

        unsigned int visibleSta = 0;
        vector<PointingVector> pointingVectors;
        vector<unsigned int> endOfLastScans;
        for (int ista=0; ista<nsta; ++ista){
            Station &thisSta = stations_[ista];

            if (!*thisSta.getPARA().available) {
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

            PointingVector p(ista,isrc);

            const Station::WAITTIMES &wtimes = thisSta.getWaittimes();
            unsigned int time = lastScanLookup[ista] + wtimes.setup + wtimes.source + wtimes.tape + wtimes.calibration;

            p.setTime(time);

            thisSta.calcAzEl(thisSource, p);

            bool flag = thisSta.isVisible(p);
            if (flag){
                visibleSta++;
                endOfLastScans.push_back(lastScanLookup[ista]);
                pointingVectors.push_back(std::move(p));
            }
        }
        if (visibleSta >= *thisSource.getPARA().minNumberOfStations) {
            subcon.addScan(Scan(pointingVectors, endOfLastScans, Scan::ScanType::single));
        }
    }

    return subcon;
}


void Scheduler::update(const Scan &scan, ofstream &bodyLog) noexcept {

    int srcid = scan.getSourceId();
    string sourceName = sources_[srcid].getName();
    unsigned long nbl = scan.getNBl();


    for (int i = 0; i < scan.getNSta(); ++i) {
        const PointingVector &pv = scan.getPointingVector(i);
        int staid = pv.getStaid();
        const PointingVector &pv_end = scan.getPointingVectors_endtime(i);
        vector<unsigned int> times = scan.getTimes().stationTimes(i);
        stations_[staid].update(nbl, pv, pv_end, times, sourceName);


        int skyCoverageId = stations_[staid].getSkyCoverageID();
        skyCoverages_[skyCoverageId].update(pv, pv_end);
    }

    unsigned int latestTime = scan.maxTime();
    Source &thisSource = sources_[srcid];
    thisSource.update(nbl, latestTime);

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

void Scheduler::consideredUpdate(unsigned long n1scans, ofstream &bodyLog) noexcept {
    bodyLog << "|     created new fillin Scans " << n1scans << " \n";
    nFillinScansConsidered += n1scans;
}

void Scheduler::consideredUpdate(unsigned long n1scans, unsigned long n2scans, ofstream &bodyLog) noexcept {

    bodyLog << "|-------------";
    for (int i = 0; i < stations_.size() - 1; ++i) {
        bodyLog << "-----------";
    }
    bodyLog << "----------| \n";

    bodyLog << "| considered single Scans " << n1scans << " subnetting scans " << n2scans << ":\n";
    nSingleScansConsidered += n1scans;
    nSubnettingScansConsidered += n2scans;
}


boost::optional<Scan>
Scheduler::fillin_scan(Subcon &subcon, const FillinmodeEndposition &fi_endp,
                            const vector<int> &sourceWillBeScanned, ofstream &bodyLog) noexcept {
    Subcon fillin_subcon;

    unsigned long i = 0;
    while (i < subcon.getNumberSingleScans()) {
        Scan &thisScan = subcon.referenceSingleSourceScan(i);

        int srcid = thisScan.getSourceId();
        if (std::find(sourceWillBeScanned.begin(), sourceWillBeScanned.end(), srcid) != sourceWillBeScanned.end()) {
            subcon.removeScan(i);
            continue;
        }
        const Source &thisSource = sources_[srcid];
        bool flag = thisScan.possibleFillinScan(stations_, thisSource, fi_endp.getStationUnused(),
                                                fi_endp.getFinalPosition());

        if (flag) {
            thisScan.setType(Scan::ScanType::fillin);
            fillin_subcon.addScan(thisScan);
            ++i;
        } else {
            subcon.removeScan(i);
        }


//            boost::optional<Scan> this_fillinScan = thisScan.possibleFillinScan(stations, thisSource,
//                                                                                     fi_endp.getStationPossible(),
//                                                                                     fi_endp.getStationUnused(),
//                                                                                     fi_endp.getFinalPosition());
//            if (this_fillinScan) {
//                this_fillinScan->setType(Scan::SCANTYPE::fillin);
//                fillin_subcon.addScan(move(*this_fillinScan));
//            }
    }
    if (fillin_subcon.getNumberSingleScans() == 0) {
        return boost::none;
    } else {

        fillin_subcon.precalcScore(stations_, sources_);
        fillin_subcon.generateScore(stations_, sources_, skyCoverages_, sources_.size());

        while (true) {
            boost::optional<unsigned long> bestIdx = fillin_subcon.rigorousScore(stations_, sources_, skyCoverages_);
            if (bestIdx) {
                Scan &fillinScan = fillin_subcon.referenceSingleSourceScan(*bestIdx);

                const Source &thisSource = sources_[fillinScan.getSourceId()];

                unsigned long nstaBefore = fillinScan.getNSta();
                bool flag = fillinScan.possibleFillinScan(stations_, thisSource, fi_endp.getStationUnused(),
                                                          fi_endp.getFinalPosition());

                if (flag && nstaBefore == fillinScan.getNSta()) {
                    return fillinScan;
                } else {
                    fillin_subcon.removeScan(*bestIdx);
                }

//                    boost::optional<Scan> updatedFillinScan = fillinScan.possibleFillinScan(stations, thisSource,
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
Scheduler::start_fillinMode(vector<Scan> &bestScans, ofstream &bodyLog) noexcept {

    FillinmodeEndposition fi_endp(bestScans, stations_);
    if (fi_endp.getNumberOfPossibleStations() < 2) {
        for (const auto &bestScan : bestScans) {
            update(bestScan, bodyLog);
        }
        return;
    }

    std::vector<char> stationAvailable(stations_.size());
    unsigned long nsta = stations_.size();

    for (int i = 0; i < nsta; ++i) {
        bool flag = *stations_[i].getPARA().available;
        stationAvailable[i] = flag;
    }

    vector<int> sourceWillBeScanned;
    for (auto &bestScan : bestScans) {
        sourceWillBeScanned.push_back(bestScan.getSourceId());
    }

    const std::vector<char> &stationPossible = fi_endp.getStationPossible();
    for (int i = 0; i < stations_.size(); ++i) {
        stations_[i].referencePARA().setAvailable(stationPossible[i]);
    }
    Subcon subcon = createSubcon(false);
    consideredUpdate(subcon.getNumberSingleScans(), bodyLog);
    subcon.precalcScore(stations_, sources_);
    subcon.generateScore(stations_, sources_, skyCoverages_, sources_.size());


    while (!bestScans.empty()) {
        if (fi_endp.getNumberOfPossibleStations() >= 2) {
            while (true) {
                boost::optional<Scan> fillinScan = fillin_scan(subcon, fi_endp, sourceWillBeScanned,
                                                                    bodyLog);
                if (fillinScan) {
                    sourceWillBeScanned.push_back(fillinScan->getSourceId());
                    bestScans.push_back(*fillinScan);
                } else {
                    break;
                }

                fi_endp = FillinmodeEndposition(bestScans, stations_);
                if (fi_endp.getNumberOfPossibleStations() < 2) {
                    break;
                }
            }
        }

        Scan &thisBestScan = bestScans.back();
        update(thisBestScan, bodyLog);
        bestScans.pop_back();


        fi_endp = FillinmodeEndposition(bestScans, stations_);
        const std::vector<char> &stationPossible = fi_endp.getStationPossible();
        for (int i = 0; i < stations_.size(); ++i) {
            stations_[i].referencePARA().setAvailable(stationPossible[i]);
        }
        subcon = createSubcon(false);
        consideredUpdate(subcon.getNumberSingleScans(), bodyLog);
        subcon.precalcScore(stations_, sources_);
        subcon.generateScore(stations_, sources_, skyCoverages_, sources_.size());
    }

    for (int i = 0; i < stations_.size(); ++i) {
        stations_[i].referencePARA().setAvailable(stationAvailable[i]);
    }
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

        const Station::WAITTIMES wtimes = any.getWaittimes();
        unsigned int constTimes = wtimes.setup + wtimes.source + wtimes.calibration + wtimes.tape;

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
                boost::posix_time::ptime thisEndTime_ =
                        TimeSystem::startTime + boost::posix_time::seconds(thisEndTime);
                boost::posix_time::ptime nextStartTime_ =
                        TimeSystem::startTime + boost::posix_time::seconds(nextStartTime);
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
                        TimeSystem::startTime + boost::posix_time::seconds(thisEndTime);
                boost::posix_time::ptime nextStartTime_ =
                        TimeSystem::startTime + boost::posix_time::seconds(nextStartTime);
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

    for (auto &any:stations_) {
        any.checkForNewEvent(time, hard_break, output, bodyLog);
    }

    bool flag = false;
    for (auto &any:sources_) {
        flag = flag || any.checkForNewEvent(time, hard_break, output, bodyLog);
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
        if (*any.getPARA().available) {
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
        PointingVector p;
        p.setTime(0);
        for (int az = -360; az < 0; ++az) {
            for (int el = 0; el < 20; ++el) {
                double thisAz = static_cast<double>(az) * deg2rad;
                double thisEl = static_cast<double>(el) * deg2rad;
                p.setAz(thisAz);
                p.setEl(thisEl);

                bool flag = any.getMask().visible(p);
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

        Subcon subcon = createSubcon(parameters_.subnetting, true);
        consideredUpdate(subcon.getNumberSingleScans(), subcon.getNumberSubnettingScans(), bodyLog);
        subcon.generateScore(prevLowElevationScores,prevHighElevationScores);

        boost::optional<unsigned long> bestIdx_opt = subcon.rigorousScore(stations_,sources_,skyCoverages_, prevLowElevationScores, prevHighElevationScores );
        if (!bestIdx_opt) {
            bodyLog << "ERROR! no valid scan found!\n";
            continue;
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

            if (bestScan1.maxTime() > bestScan2.maxTime()) {
                swap(bestScan1, bestScan2);
            }
            bestScans.push_back(bestScan1);
            bestScans.push_back(bestScan2);
        }

        // update prev low elevation scores
        for(const auto &scan:bestScans){
            double lowElevationSlopeStart = CalibratorBlock::lowElevationSlopeStart;
            double lowElevationSlopeEnd = CalibratorBlock::lowElevationSlopeEnd;

            double highElevationSlopeStart = CalibratorBlock::highElevationSlopeStart;
            double highElevationSlopeEnd = CalibratorBlock::highElevationSlopeEnd;

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
            if (any.maxTime() > all_maxTime) {
                all_maxTime = any.maxTime();
            }
        }

        if( all_maxTime > TimeSystem::duration){
            break;
        }

        bool hardBreak = checkForNewEvent(all_maxTime, true, bodyLog);
        if (hardBreak) {
            i -=1;
            continue;
        }


        if (parameters_.fillinmode && !scans_.empty()) {
            start_fillinMode(bestScans, bodyLog);
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

    bodyLog << "|-------------";
    for (int i = 0; i < stations_.size() - 1; ++i) {
        bodyLog << "-----------";
    }
    bodyLog << "----------| \n";
    bodyLog << "CALIBRATOR BLOCK SUMMARY:\n";
    bodyLog << "|-------------";
    for (int i = 0; i < stations_.size() - 1; ++i) {
        bodyLog << "-----------";
    }
    bodyLog << "----------| \n";

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

    bodyLog << "|-------------";
    for (int i = 0; i < stations_.size() - 1; ++i) {
        bodyLog << "-----------";
    }
    bodyLog << "----------| \n";



}
