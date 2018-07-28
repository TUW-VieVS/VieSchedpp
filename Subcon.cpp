/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Subcon.cpp
 * Author: mschartn
 * 
 * Created on June 29, 2017, 5:51 PM
 */

#include "Subcon.h"

using namespace std;
using namespace VieVS;
unsigned long Subcon::nextId = 0;

Subcon::Subcon(): VieVS_Object(nextId++), nSingleScans_{0}, nSubnettingScans_{0} {
}

void Subcon::addScan(Scan &&scan) noexcept {
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " add scan " << scan.printId() << " to source " << scan.getSourceId();
    #endif

    singleScans_.push_back(std::move(scan));
    nSingleScans_++;
}

void
Subcon::calcStartTimes(const Network &network, const vector<Source> &sources,
                       const boost::optional<StationEndposition> & endposition) noexcept {
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " calc scan start times";
    #endif

    int i=0;
    // loop through all scans
    while(i<nSingleScans_){
        bool scanValid_slew = true;
        bool scanValid_idle = true;
        bool scanValid_endposition = true;

        // save maximum idle times
        vector<unsigned  int> maxIdleTimes;

        // current scan
        Scan &thisScan = singleScans_[i];

        const Source &thisSource = sources[thisScan.getSourceId()];

        // loop through all stations
        int j=0;
        while(j<thisScan.getNSta()){
            unsigned long staid = thisScan.getStationId(j);

            // current station
            const Station &thisSta = network.getStation(staid);

            // first scan means no field system, slew and preob time
            if (thisSta.getPARA().firstScan) {
                thisScan.addTimes(j, 0, 0, 0);
                ++j;
                maxIdleTimes.push_back(thisSta.getPARA().maxWait);
                continue;
            }

            // unwrap azimuth and calculate slewtime
            thisSta.getCableWrap().calcUnwrappedAz(thisSta.getCurrentPointingVector(),
                                                   thisScan.referencePointingVector(j));
            auto slewtime = thisSta.slewTime(thisScan.getPointingVector(j));

            // look if slewtime is valid, if yes add field system, slew and preob times
            if (slewtime.is_initialized()) {
                maxIdleTimes.push_back(thisSta.getPARA().maxWait);
                const Station::WaitTimes wtimes = thisSta.getWaittimes();
                thisScan.addTimes(j, wtimes.fieldSystem, *slewtime, wtimes.preob);
            } else {
                scanValid_slew = thisScan.removeStation(j, thisSource);
                if(!scanValid_slew){
                    break;      // scan is no longer valid
                } else {
                    continue;   // station was removed, continue with next station (do not increment counter!)
                }
            }

            // look if there is enough time to reach endposition (if there is any) under perfect circonstance
            if(endposition.is_initialized()){
                const auto &times = thisScan.getTimes();

                const auto &waitTimes = thisSta.getWaittimes();
                unsigned int minimumScanTime = max(thisSta.getPARA().minScan, thisSource.getPARA().minScan);


                // calc possible endposition time. Assumtion: 5sec slew time, no idle time and minimum scan time
                int possibleEndpositionTime =
                        times.getObservingTime(j, Timestamp::start) + minimumScanTime +5+ waitTimes.fieldSystem + waitTimes.preob;

                // get minimum required endpositon time
                int requiredEndpositionTime = endposition->requiredEndpositionTime(staid);

                // check if there is enough time left
                if(possibleEndpositionTime-5 > requiredEndpositionTime){
                    scanValid_endposition = thisScan.removeStation(j, thisSource);
                    if(!scanValid_endposition){
                        break;      // scan is no longer valid
                    } else {
                        continue;   // station was removed, continue with next station (do not increment counter!)
                    }
                }
            }
            ++j;
        }

        if (scanValid_slew && scanValid_endposition) {
            scanValid_idle = thisScan.checkIdleTimes(maxIdleTimes, thisSource);
        }


        if (scanValid_slew && scanValid_endposition && scanValid_idle){
            ++i;
        }else{
            #ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL(debug) << "scan " << thisScan.printId() << " no longer valid -> removed";
            #endif
            singleScans_.erase(next(singleScans_.begin(),i));
            --nSingleScans_;
        }
    }
}

void Subcon::constructAllBaselines(const Network &network, const vector<Source> &sources) noexcept {
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " construct all observations";
    #endif
    int i = 0;
    while (i < nSingleScans_) {
        Scan &thisScan = singleScans_[i];
        const Source &thisSource = sources[thisScan.getSourceId()];
        bool scanValid = thisScan.constructObservations(network, thisSource);
        if (scanValid) {
            ++i;
        } else {
            #ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " scan " << thisScan.printId() << " no longer valid -> removed";
            #endif
            --nSingleScans_;
            singleScans_.erase(next(singleScans_.begin(),i));
        }
    }
}

void Subcon::updateAzEl(const Network &network, const vector<Source> &sources) noexcept {
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " update azimuth and elevation";
    #endif

    int i = 0;
    while (i < nSingleScans_) {

        auto &thisScan = singleScans_[i];

        const Source &thisSource = sources[thisScan.getSourceId()];
        bool scanValid_slew = true;
        bool scanValid_idle = true;
        vector<unsigned  int> maxIdleTimes;

        int staidx = 0;
        while (staidx < thisScan.getNSta()) {
            unsigned long staid = thisScan.getStationId(staidx);
            PointingVector &thisPointingVector = thisScan.referencePointingVector(staidx);
            const Station &thisStation = network.getStation(staid);
            thisPointingVector.setTime(thisScan.getTimes().getObservingTime(staidx, Timestamp::start));
            thisStation.calcAzEl(thisSource, thisPointingVector);
            bool visible = thisStation.isVisible(thisPointingVector, sources[thisScan.getSourceId()].getPARA().minElevation);

            boost::optional<unsigned int> slewtime;

            if (visible){
                thisStation.getCableWrap().calcUnwrappedAz(thisStation.getCurrentPointingVector(),
                                                               thisPointingVector);
                slewtime = thisStation.slewTime(thisPointingVector);
            }

            if (!visible || !slewtime.is_initialized()) {
                scanValid_slew = thisScan.removeStation(staidx, sources[thisScan.getSourceId()]);
                if(!scanValid_slew){
                    break;
                }
            } else {
                maxIdleTimes.push_back(thisStation.getPARA().maxWait);
                thisScan.referenceTime().setSlewTime(staidx, *slewtime);
                ++staidx;
            }
        }


        if (scanValid_slew) {
            scanValid_idle = thisScan.checkIdleTimes(maxIdleTimes, sources[thisScan.getSourceId()]);
        }

        if (!scanValid_slew || !scanValid_idle){
            #ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " scan " << thisScan.printId() << " no longer valid -> removed";
            #endif
            singleScans_.erase(next(singleScans_.begin(),i));
            --nSingleScans_;
        }else{
            ++i;
        }
    }
}

void
Subcon::calcAllBaselineDurations(const Network &network, const vector<Source> &sources) noexcept {
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " calc observing durations";
    #endif

    int i = 0;
    while ( i < nSingleScans_ ) {
        Scan& thisScan = singleScans_[i];
        bool scanValid = thisScan.calcObservationDuration(network, sources[thisScan.getSourceId()]);
        if (scanValid){
            ++i;
        } else {
            #ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " scan " << thisScan.printId() << " no longer valid -> removed";
            #endif
            --nSingleScans_;
            singleScans_.erase(next(singleScans_.begin(),i));
        }
    }
}

void Subcon::calcAllScanDurations(const Network &network, const vector<Source> &sources,
                                  const boost::optional<StationEndposition> & endposition) noexcept {
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " calc scan durations";
    #endif
    // loop through all scans
    int i=0;
    while (i<nSingleScans_){

        // current scan and source
        Scan& thisScan = singleScans_[i];
        const Source &thisSource = sources[thisScan.getSourceId()];

        // calculate scan durations and check if they are valid
        bool scanValid_scanDuration = thisScan.scanDuration(network, thisSource);

        // check if there is enough time to slew to endposition under perfect circumstances
        bool scanValid_endposition = true;
        if(endposition.is_initialized()){
            const auto &times = thisScan.getTimes();

            // loop through all stations
            int staidx=0;
            while(staidx<thisScan.getNSta()){
                unsigned long staid = thisScan.getStationId(staidx);
                const Station &thisSta = network.getStation(staid);

                const auto &waitTimes = thisSta.getWaittimes();

                // calc possible endposition time. Assumtion: 5sec slew time, no idle time
                int possibleEndpositionTime = times.getObservingTime(staidx, Timestamp::end) + waitTimes.fieldSystem + waitTimes.preob + 5;

                // get minimum required endpositon time
                int requiredEndpositionTime = endposition->requiredEndpositionTime(staid);

                // check if there is enough time left
                if(possibleEndpositionTime-5 > requiredEndpositionTime){
                    scanValid_endposition = thisScan.removeStation(staidx, thisSource);
                    if(!scanValid_endposition){
                        break;      // scan is no longer valid
                    } else {
                        continue;   // station was removed, continue with next station (do not increment counter!)
                    }
                }
                ++staidx;
            }
        }

        if (scanValid_scanDuration && scanValid_endposition){
            ++i;
        } else {
            #ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " scan " << thisScan.printId() << " no longer valid -> removed";
            #endif
            --nSingleScans_;
            singleScans_.erase(next(singleScans_.begin(),i));
        }
    }
}

void Subcon::calcCalibratorScanDuration(const vector<Station> &stations, const vector<Source> &sources) {
    for(auto &thisScan:singleScans_) {
        thisScan.setFixedScanDuration(CalibratorBlock::scanLength);
    }
}


void Subcon::createSubnettingScans(const Subnetting &subnetting, const vector<Source> &sources) noexcept {
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " create subnetting scans";
    #endif

    subnettingScans_.clear();
    vector<unsigned long> sourceIds(nSingleScans_);
    for (int i = 0; i < nSingleScans_; ++i) {
        sourceIds[i] = singleScans_[i].getSourceId();
    }

    for (int i = 0; i < nSingleScans_; ++i) {
        unsigned long firstSrcId = sourceIds[i];
        Scan &first = singleScans_[i];
        const vector<unsigned long> &secondSrcIds = subnetting.subnettingSrcIds[firstSrcId];
        for (unsigned long secondSrcId : secondSrcIds) {

            auto it = find(sourceIds.begin(), sourceIds.end(), secondSrcId);
            if (it != sourceIds.end()) {
                long srcid = distance(sourceIds.begin(), it);
                Scan &second = singleScans_[srcid];

                vector<unsigned long> uniqueSta1;
                vector<unsigned long> uniqueSta2;
                vector<unsigned long> intersection;

                for (int idx=0; idx<first.getNSta(); ++idx) {
                    const PointingVector &pv = first.getPointingVector(idx);
                    unsigned long staid = pv.getStaid();

                    if (!second.findIdxOfStationId(staid)) {
                        uniqueSta1.push_back(staid);
                    } else {
                        intersection.push_back(staid);
                    }
                }
                for (int idx=0; idx<second.getNSta(); ++idx) {
                    const PointingVector &pv = second.getPointingVector(idx);
                    unsigned long staid = pv.getStaid();

                    if (!first.findIdxOfStationId(staid)) {
                        uniqueSta2.push_back(staid);
                    }
                }

                if (uniqueSta1.size() + uniqueSta2.size() + intersection.size() < subnetting.subnettingMinNSta) {
                    continue;
                }

                unsigned long nint = intersection.size();


                for (int igroup = 0; igroup <= nint; ++igroup) {

                    vector<int> data(nint, 1);
                    for (unsigned long ii = nint - igroup; ii < nint; ++ii) {
                        data.at(ii) = 2;
                    }


                    do {
                        vector<unsigned long> scan1sta{uniqueSta1};
                        vector<unsigned long> scan2sta{uniqueSta2};
                        for (unsigned long ii = 0; ii < nint; ++ii) {
                            if (data.at(ii) == 1) {
                                scan1sta.push_back(intersection[ii]);
                            } else {
                                scan2sta.push_back(intersection[ii]);
                            }
                        }
                        if (scan1sta.size() >= sources[firstSrcId].getPARA().minNumberOfStations &&
                            scan2sta.size() >= sources[secondSrcId].getPARA().minNumberOfStations) {

                            unsigned int firstTime = first.getTimes().getScanTime(Timestamp::end);
                            unsigned int secondTime = second.getTimes().getScanTime(Timestamp::end);

                            if( util::absDiff(firstTime,secondTime) > 600) {
                                continue;
                            }

                            boost::optional<Scan> new_first = first.copyScan(scan1sta, sources[firstSrcId]);
                            if (!new_first) {
                                continue;
                            }

                            boost::optional<Scan> new_second = second.copyScan(scan2sta, sources[secondSrcId]);
                            if (!new_second) {
                                continue;
                            }

                            #ifdef VIESCHEDPP_LOG
                            BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " add subnetting scans with " << new_first->printId() << " and " << new_second->printId();
                            #endif

                            ++nSubnettingScans_;
                            pair<Scan, Scan>tmp = make_pair(move(*new_first),move(*new_second));
//                            tmp.first = ;
//                            tmp.second = ;
                            subnettingScans_.push_back(move(tmp));
                        }
                    } while (next_permutation(std::begin(data), std::end(data)));
                }
            }
        }
    }
}

void Subcon::generateScore(const Network &network, const vector<Source> &sources) noexcept {
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " generate scores ";
    #endif

    precalcScore(network, sources);
//    unsigned long nmaxsta = stations.size();
    vector< unordered_map<unsigned long, double> > staids2skyCoverageScores(sources.size());
    for (auto &thisScan: singleScans_) {
        unsigned long srcid = thisScan.getSourceId();
        unordered_map<unsigned long, double> & staids2skyCoverageScore = staids2skyCoverageScores[srcid];
        const Source &thisSource = sources[srcid];
        thisScan.calcScore(astas_, asrcs_, abls_, minRequiredTime_, maxRequiredTime_, network, thisSource,
                           staids2skyCoverageScore);
    }


    for (auto &thisScans:subnettingScans_) {
        Scan &thisScan1 = thisScans.first;
        unsigned long srcid1 = thisScan1.getSourceId();
        const Source &thisSource1 = sources[srcid1];
        const unordered_map<unsigned long, double> & staids2skyCoverageScore1 = staids2skyCoverageScores[srcid1];
        thisScan1.calcScore_subnetting(astas_, asrcs_, abls_, minRequiredTime_, maxRequiredTime_,
                                       network, thisSource1, staids2skyCoverageScore1);
//        double score1 = thisScan1.getScore();

        Scan &thisScan2 = thisScans.second;
        unsigned long srcid2 = thisScan2.getSourceId();
        const Source &thisSource2 = sources[srcid2];
        const unordered_map<unsigned long, double> & staids2skyCoverageScore2 = staids2skyCoverageScores[srcid2];
        thisScan2.calcScore_subnetting(astas_, asrcs_, abls_, minRequiredTime_, maxRequiredTime_,
                                       network, thisSource2, staids2skyCoverageScore2);
//        double score2 = thisScan2.getScore();
    }
}

void Subcon::generateScore(const Network &network, const std::vector<Source> &sources,
                           const std::vector<std::map<unsigned long, double>> &hiscores, unsigned int interval) {
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " generate scores ";
    #endif

    precalcScore(network, sources);
//    unsigned long nmaxsta = stations.size();
    for (auto &thisScan: singleScans_) {
        vector<double> firstScorePerPv(thisScan.getNSta(),0);
        const Source &thisSource = sources[thisScan.getSourceId()];
        unsigned int iTime = thisScan.getTimes().getObservingTime(Timestamp::start)/interval;
        const map<unsigned long, double> &thisMap = hiscores[iTime];
        double hiscore = thisMap.at(thisSource.getId());
        thisScan.calcScore(minRequiredTime_, maxRequiredTime_, network, thisSource, hiscore);
    }


    for (auto &thisScans:subnettingScans_) {
        Scan &thisScan1 = thisScans.first;
        unsigned long srcid1 = thisScan1.getSourceId();
        const Source &thisSource1 = sources[srcid1];
        unsigned int iTime1 = thisScan1.getTimes().getObservingTime(Timestamp::start)/interval;
        const map<unsigned long, double> &thisMap1 = hiscores[iTime1];
        double hiscore1 = thisMap1.at(thisSource1.getId());
        thisScan1.calcScore(minRequiredTime_, maxRequiredTime_, network, thisSource1, hiscore1);
//        double score1 = thisScan1.getScore();

        Scan &thisScan2 = thisScans.second;
        unsigned long srcid2 = thisScan2.getSourceId();
        const Source &thisSource2 = sources[srcid2];
        unsigned int iTime2 = thisScan2.getTimes().getObservingTime(Timestamp::start)/interval;
        const map<unsigned long, double> &thisMap2 = hiscores[iTime2];
        double hiscore2 = thisMap2.at(thisSource2.getId());
        thisScan2.calcScore(minRequiredTime_, maxRequiredTime_, network, thisSource2, hiscore2);
//        double score2 = thisScan2.getScore();
    }

}


void Subcon::generateScore(const std::vector<double> &lowElevatrionScore, const std::vector<double> &highElevationScore,
                           const Network &network, const std::vector<Source> &sources) {
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " generate scores ";
    #endif

    minMaxTime();
//    auto nsta = static_cast<unsigned int>(stations.size());
//    unsigned int nMaxBl = (nsta*(nsta-1))/2;

    int i=0;
    while (i<nSingleScans_){
        Scan& thisScan = singleScans_[i];

        bool valid = thisScan.calcScore(lowElevatrionScore, highElevationScore, network, minRequiredTime_,
                                        maxRequiredTime_, sources[thisScan.getSourceId()]);
        if (valid){
            ++i;
        } else {
            --nSingleScans_;
            singleScans_.erase(next(singleScans_.begin(),i));
        }
    }

    i=0;
    while (i<subnettingScans_.size()){
        Scan &thisScan1 = subnettingScans_[i].first;

        bool valid1 = thisScan1.calcScore(lowElevatrionScore, highElevationScore, network, minRequiredTime_,
                                          maxRequiredTime_, sources[thisScan1.getSourceId()]);
//        double score1 = thisScan1.getScore();

        Scan &thisScan2 = subnettingScans_[i].first;

        bool valid2 = thisScan2.calcScore(lowElevatrionScore, highElevationScore, network, minRequiredTime_,
                                          maxRequiredTime_, sources[thisScan2.getSourceId()]);
//        double score2 = thisScan2.getScore();

        if (valid1 && valid2){
            ++i;
        } else {
            --nSubnettingScans_;
            subnettingScans_.erase(next(subnettingScans_.begin(),i));
        }
    }
}


void Subcon::minMaxTime() noexcept {
    unsigned int maxTime = 0;
    unsigned int minTime = numeric_limits<unsigned int>::max();
    for (auto &thisScan: singleScans_) {
        unsigned int thisTime = thisScan.getTimes().getScanTime(Timestamp::end) - thisScan.getTimes().getScanTime(Timestamp::start);
        if (thisTime < minTime) {
            minTime = thisTime;
        }
        if (thisTime > maxTime) {
            maxTime = thisTime;
        }
    }
    for (auto &thisScan: subnettingScans_) {
        unsigned int thisTime1 =
                thisScan.first.getTimes().getScanDuration();
        if (thisTime1 < minTime) {
            minTime = thisTime1;
        }
        if (thisTime1 > maxTime) {
            maxTime = thisTime1;
        }
        unsigned int thisTime2 =
                thisScan.first.getTimes().getScanDuration();
        if (thisTime2 < minTime) {
            minTime = thisTime2;
        }
        if (thisTime2 > maxTime) {
            maxTime = thisTime2;
        }
    }
    minRequiredTime_ = minTime;
    maxRequiredTime_ = maxTime;
}


void Subcon::prepareAverageScore(const vector<Station> &objects) noexcept {
    vector<unsigned long> nobs;
    for (auto &thisObject:objects) {
        nobs.push_back(thisObject.getNObs());
    }
    astas_ =  prepareAverageScore_base(nobs);
}

void Subcon::prepareAverageScore(const vector<Baseline> &objects) noexcept{
    vector<unsigned long> nobs;
    for (auto &thisObject:objects) {
        nobs.push_back(thisObject.getNObs());
    }

    abls_ = prepareAverageScore_base(nobs);
}

void Subcon::prepareAverageScore(const vector<Source> &objects) noexcept {
    vector<unsigned long> nobs;
    for (auto &thisObject:objects) {
        nobs.push_back(thisObject.getNObs());
    }
    asrcs_ = prepareAverageScore_base(nobs);
}

std::vector<double> Subcon::prepareAverageScore_base(const std::vector<unsigned long> &nobs) noexcept{
    double mean = std::accumulate(std::begin(nobs), std::end(nobs), 0.0) / nobs.size();

    double maxDiff = 0;
    vector<double> score;
    for (auto &thisObs : nobs){
        double diff = mean - thisObs;
        if (diff > 0) {
            score.push_back(diff);
            if (diff > maxDiff){
                maxDiff = diff;
            }
        } else {
            score.push_back(0);
        }
    }
    if (maxDiff != 0) {
        for (auto &thisScore : score){
            if(thisScore != 0){
                thisScore = thisScore/maxDiff;
            }
        }
    }
    return score;
}

void Subcon::precalcScore(const Network &network, const vector<Source> &sources) noexcept {

    if (WeightFactors::weightDuration != 0) {
        minMaxTime();
    }
    if (WeightFactors::weightAverageStations != 0) {
        prepareAverageScore(network.getStations());
    }
    if (WeightFactors::weightAverageBaselines != 0) {
        prepareAverageScore(network.getBaselines());
    }
    if (WeightFactors::weightAverageSources != 0) {
        prepareAverageScore(sources);
    }
}


//std::vector<Scan> Subcon::selectBest(const Network &network, const std::vector<Source> &sources) {
//    vector<Scan> bestScans;
//
//    if(nSingleScans_ == 0){
//        return bestScans;
//    }
//    auto it_single = max_element(singleScans_.begin(),singleScans_.end(),[](const Scan &a, const Scan &b){
//        return a.getScore() < b.getScore();
//    });
//
//    long idxSingle = distance(singleScans_.begin(), it_single);
//
//    long idxSubnetting = -1;
//    if(!subnettingScans_.empty()){
//
//        auto it_subnetting = max_element(subnettingScans_.begin(),subnettingScans_.end(),[]
//                (const std::pair<Scan,Scan> &a, const std::pair<Scan,Scan> &b){
//            return (a.first.getScore() + a.second.getScore()) < (b.first.getScore() + b.second.getScore());
//        });
//
//        idxSubnetting = distance(subnettingScans_.begin(), it_subnetting);
//    }
//
//
//    if (idxSubnetting == -1 || singleScans_[idxSingle].getScore() >
//                               subnettingScans_[idxSubnetting].first.getScore() +
//                               subnettingScans_[idxSubnetting].second.getScore()) {
//
//        Scan bestScan = takeSingleSourceScan(static_cast<unsigned long>(idxSingle));
//        bool valid = bestScan.rigorousUpdate(network,sources[bestScan.getSourceId()]);
//        if(valid){
//            bestScans.push_back(std::move(bestScan));
//        }
//
//    } else {
//
//        pair<Scan, Scan> bestScan_pair = takeSubnettingScans(static_cast<unsigned long>(idxSubnetting));
//
//        Scan bestScan1 = bestScan_pair.first;
//        bool valid1 = bestScan1.rigorousUpdate(network,sources[bestScan1.getSourceId()]);
//        Scan bestScan2 = bestScan_pair.second;
//        bool valid2 = bestScan2.rigorousUpdate(network,sources[bestScan2.getSourceId()]);
//
//        if(valid1 && valid2) {
//            bestScans.push_back(std::move(bestScan1));
//            bestScans.push_back(std::move(bestScan2));
//        }
//
//    }
//
//    return bestScans;
//}


vector<Scan> Subcon::selectBest(const Network &network, const vector<Source> &sources,
                                const boost::optional<StationEndposition> &endposition) noexcept {
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " select best scan ";
    #endif

    vector<Scan> bestScans;

    // merge single scan scores and subnetting scores
    vector<double> scores;
    for(const auto&any: singleScans_){
        scores.push_back(any.getScore());
    }
    for(const auto&any: subnettingScans_){
        scores.push_back(any.first.getScore() + any.second.getScore());
    }

    // push data into queue
    std::priority_queue<std::pair<double, unsigned long> > q;
    for (unsigned long i = 0; i < scores.size(); ++i) {
        q.push(std::pair<double, unsigned long>(scores[i], i));
    }
    vector<unsigned long> scansToRemove;

    // loop through queue
    unsigned long idx;
    while (true) {
        if (q.empty()) {
            return bestScans;
        }

        // get index of scan(s) with highest score and remove it from list
        idx = q.top().second;
        q.pop();

        // distinguish between single source scan and subnetting scans
        if (idx < nSingleScans_) {
            unsigned long thisIdx = idx;

            // get scan with highest score
            Scan &thisScan = singleScans_[thisIdx];
            #ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " highest score for scan " << thisScan.printId();
            #endif

            const Source &thisSource = sources[thisScan.getSourceId()];
            // make rigorous update
            bool flag = thisScan.rigorousUpdate(network, sources[thisScan.getSourceId()], endposition);
            if (!flag) {
                scansToRemove.push_back(idx);
                #ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " scan " << thisScan.printId() << " no longer valid -> removed";
                #endif
                continue;
            }

            // calculate score again
            thisScan.calcScore(astas_, asrcs_, abls_, minRequiredTime_, maxRequiredTime_,
                               network, thisSource);

            // push score in queue
            q.push(make_pair(thisScan.getScore(), idx));

        } else {
            unsigned long thisIdx = idx - nSingleScans_;
            auto &thisScans = subnettingScans_[thisIdx];

            // get scans with highest score
            Scan &thisScan1 = thisScans.first;
            const Source &thisSource1 = sources[thisScan1.getSourceId()];
            Scan &thisScan2 = thisScans.second;
            const Source &thisSource2 = sources[thisScan2.getSourceId()];

            #ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " highest score for scan " << thisScan1.printId() << " and " << thisScan2.printId();
            #endif

            // make rigorous update
            bool flag1 = thisScan1.rigorousUpdate(network, sources[thisScan1.getSourceId()], endposition);
            if (!flag1) {
                scansToRemove.push_back(idx);
                #ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " scan " << thisScan1.printId() << " no longer valid -> removed";
                #endif

                continue;
            }
            bool flag2 = thisScan2.rigorousUpdate(network, sources[thisScan2.getSourceId()], endposition);
            if (!flag2) {
                scansToRemove.push_back(idx);
                #ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " scan " << thisScan2.printId() << " no longer valid -> removed";
                #endif
                continue;
            }

            // check time differences between subnetting scans
            unsigned int maxTime1 = thisScan1.getTimes().getScanTime(Timestamp::end);
            unsigned int maxTime2 = thisScan2.getTimes().getScanTime(Timestamp::end);
            unsigned int deltaTime = util::absDiff(maxTime1,maxTime2);
            if (deltaTime > 600) {
                #ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " too much time between subnetting scans -> removed";
                #endif
                continue;
            }

            // calculate score again
            thisScan1.calcScore(astas_, asrcs_, abls_, minRequiredTime_, maxRequiredTime_,
                                network, thisSource1);
            thisScan2.calcScore(astas_, asrcs_, abls_, minRequiredTime_, maxRequiredTime_,
                                network, thisSource2);

            // push score in queue
            q.push(make_pair(thisScan1.getScore() + thisScan2.getScore(), idx));
        }

        // check if newly added score is again the highest score in the queue. If yes this is/are our selected scan/scans
        unsigned long newIdx = q.top().second;
        if (newIdx == idx) {
            break;
        }
    }

    if (idx < nSingleScans_) {
        Scan bestScan = takeSingleSourceScan(idx);
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " scan " << bestScan.printId() << " is valid best scan";
        #endif
        bestScans.push_back(std::move(bestScan));
    } else {
        unsigned long thisIdx = idx - nSingleScans_;
        pair<Scan, Scan> bestScan_pair = takeSubnettingScans(thisIdx);
        Scan bestScan1 = bestScan_pair.first;
        Scan bestScan2 = bestScan_pair.second;
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " scan " << bestScan1.printId() << " and " << bestScan2.printId() << " are valid best scans";
        #endif

        bestScans.push_back(std::move(bestScan1));
        bestScans.push_back(std::move(bestScan2));
    }

    sort(scansToRemove.begin(), scansToRemove.end(), [](const int a, const int b) {
        return a > b;
    });

    // remove all scans which are invalid
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " remove invalid scan(s) ";
    #endif
    for(auto invalidIdx:scansToRemove){
        // if invalid index is larger as idx decrement it (source(s) with idx are already removed!)
        if(invalidIdx>idx){
            --invalidIdx;
        }

        removeScan(invalidIdx);
    }


    return bestScans;


}

boost::optional<unsigned long> Subcon::rigorousScore(const Network &network, const vector<Source> &sources,
                                                     const vector<double> &prevLowElevationScores,
                                                     const vector<double> &prevHighElevationScores) {
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " calculate rigorous score ";
    #endif

    vector<double> scores;
    for(const auto&any: singleScans_){
        scores.push_back(any.getScore());
    }
    for(const auto&any: subnettingScans_){
        scores.push_back(any.first.getScore() + any.second.getScore());
    }

    std::priority_queue<std::pair<double, unsigned long> > q;
    for (unsigned long i = 0; i < scores.size(); ++i) {
        q.push(std::pair<double, unsigned long>(scores[i], i));
    }

    while (true) {
        if (q.empty()) {
            return boost::none;
        }
        unsigned long idx = q.top().second;
        q.pop();

        if (idx < nSingleScans_) {
            unsigned long thisIdx = idx;
            Scan &thisScan = singleScans_[thisIdx];
            const Source &thisSource = sources[thisScan.getSourceId()];
            bool flag = thisScan.rigorousUpdate(network, sources[thisScan.getSourceId()]);
            if (!flag) {
                continue;
            }
            flag = thisScan.calcScore(prevLowElevationScores, prevHighElevationScores, network,
                                      minRequiredTime_, maxRequiredTime_, sources[thisScan.getSourceId()]);
            if (!flag) {
                continue;
            }
            double newScore = thisScan.getScore();

            q.push(make_pair(newScore, idx));

        } else {
            unsigned long thisIdx = idx - nSingleScans_;
            auto &thisScans = subnettingScans_[thisIdx];

            Scan &thisScan1 = thisScans.first;
            const Source &thisSource1 = sources[thisScan1.getSourceId()];
            bool flag1 = thisScan1.rigorousUpdate(network, sources[thisScan1.getSourceId()]);
            if (!flag1) {
                continue;
            }
            flag1 = thisScan1.calcScore(prevLowElevationScores, prevHighElevationScores, network,
                                        minRequiredTime_, maxRequiredTime_, sources[thisScan1.getSourceId()]);
            if (!flag1) {
                continue;
            }
            double newScore1 = thisScan1.getScore();

            Scan &thisScan2 = thisScans.second;
            const Source &thisSource2 = sources[thisScan2.getSourceId()];
            bool flag2 = thisScan2.rigorousUpdate(network, sources[thisScan2.getSourceId()]);
            if (!flag2) {
                continue;
            }

            unsigned int maxTime1 = thisScan1.getTimes().getScanTime(Timestamp::end);
            unsigned int maxTime2 = thisScan2.getTimes().getScanTime(Timestamp::end);
            unsigned int deltaTime;
            if (maxTime1 > maxTime2) {
                deltaTime = maxTime1 - maxTime2;
            } else {
                deltaTime = maxTime2 - maxTime1;
            }
            if (deltaTime > 600) {
                continue;
            }

            flag2 = thisScan2.calcScore(prevLowElevationScores, prevHighElevationScores, network,
                                        minRequiredTime_, maxRequiredTime_, sources[thisScan2.getSourceId()]);
            if (!flag2) {
                continue;
            }

            double newScore2 = thisScan2.getScore();
            double newScore = newScore1 + newScore2;

            q.push(make_pair(newScore, idx));
        }
        unsigned long newIdx = q.top().second;
        if (newIdx == idx) {
            return idx;
        }
    }
}


void Subcon::removeScan(unsigned long idx) noexcept {

    if (idx < nSingleScans_) {
        unsigned long thisIdx = idx;
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " remove scan " << singleScans_[idx].printId();
        #endif
        singleScans_.erase(next(singleScans_.begin(), static_cast<int>(thisIdx)));
        --nSingleScans_;

    } else {
        unsigned long thisIdx = idx - nSingleScans_;
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " remove scans " << subnettingScans_[thisIdx].first.printId() << " and " << subnettingScans_[thisIdx].second.printId();
        #endif

        subnettingScans_.erase(next(subnettingScans_.begin(), static_cast<int>(thisIdx)));
        --nSubnettingScans_;
    }
}

void Subcon::clearSubnettingScans() {
    nSubnettingScans_ = 0;
    subnettingScans_.clear();
}

void Subcon::checkIfEnoughTimeToReachEndposition(const Network &network, const std::vector<Source> &sources,
                                            const boost::optional<StationEndposition> &endposition) {

    // if there is no required endposition do nothing
    if(!endposition.is_initialized()) {
        return;
    }

    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " check if required endposition can be reached ";
    #endif

    // loop through all scans
    int iscan = 0;
    while (iscan < nSingleScans_) {
        bool scanValid = true;

        // current scan and source
        auto &thisScan = singleScans_[iscan];
        const ScanTimes &times = thisScan.getTimes();
        const Source &thisSource = sources[thisScan.getSourceId()];

        // loop through all stations
        int istation = 0;
        while (istation < thisScan.getNSta()) {

            // current station id
            unsigned long staid = thisScan.getStationId(istation);
            const Station &thisSta = network.getStation(staid);
            const Station::WaitTimes &waitTimes = thisSta.getWaittimes();

            int possibleEndpositionTime;
            if(endposition->hasEndposition(staid)){
                // required endposition
                const PointingVector &thisEndposition = endposition->getFinalPosition(staid).get();

                // assume that pointing vector at scan end is same as pointing vector at scan start
                const PointingVector &assumedSlewStart = thisScan.getPointingVector(istation);

                // calculate slew time between pointing vectors
                unsigned int slewtime = thisSta.getAntenna().slewTime(assumedSlewStart,thisEndposition);

                // check if there is enough time
                possibleEndpositionTime =
                        times.getObservingTime(istation, Timestamp::end) + waitTimes.fieldSystem + slewtime + waitTimes.preob;
            }else{
                possibleEndpositionTime = times.getObservingTime(istation, Timestamp::end);
            }

            // get minimum required endpositon time
            int requiredEndpositionTime = endposition->requiredEndpositionTime(staid);

            if(possibleEndpositionTime-5 > requiredEndpositionTime){
                scanValid = thisScan.removeStation(istation, thisSource);
                if(!scanValid){
                    break;      // scan is no longer valid
                } else {
                    continue;   // station was removed, continue with next station (do not increment counter!)
                }
            }
            ++istation;
        }

        // if scan is valid increment scan counter, otherwise remove scan.
        if (scanValid){
            ++iscan;
        }else{
            #ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " scan " << thisScan.printId() << " no longer valid -> removed";
            #endif

            singleScans_.erase(next(singleScans_.begin(),iscan));
            --nSingleScans_;
        }
    }
}

void Subcon::changeType(Scan::ScanType type) {
    for(auto &any:singleScans_){
        any.setType(type);
    }
}

void Subcon::visibleScan(unsigned int currentTime, Scan::ScanType type, const Network &network,
                         const Source &thisSource, std::set<unsigned long>observedSources) {
    unsigned long srcid = thisSource.getId();

    if (!thisSource.getPARA().available || !thisSource.getPARA().globalAvailable) {
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " source " << thisSource.getName() << " not available";
        #endif
        return;
    }

    if (type == Scan::ScanType::fillin && !thisSource.getPARA().availableForFillinmode) {
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " source " << thisSource.getName() << " not available for fillin mode";
        #endif
        return;
    }

    if(type == Scan::ScanType::calibrator && find(CalibratorBlock::calibratorSourceIds.begin(),CalibratorBlock::calibratorSourceIds.end(),thisSource.getId()) == CalibratorBlock::calibratorSourceIds.end()) {
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " source " << thisSource.getName() << " not available as calibrator";
        #endif
        return;
    }

    if(observedSources.find(srcid) != observedSources.end()){
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " source " << thisSource.getName() << " not available - already observed in next scans";
        #endif
        return;
    }

    if (thisSource.getNscans() > 0 && currentTime - thisSource.lastScanTime() < thisSource.getPARA().minRepeat) {
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " source " << thisSource.getName() << " not available - observed recently";
        #endif
        return;
    }

    if (thisSource.getNscans() >= thisSource.getPARA().maxNumberOfScans) {
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " source " << thisSource.getName() << " not available - max number of scans reached";
        #endif
        return;
    }

    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "subcon " << this->printId() << " create scan for source " << thisSource.getName();
    #endif

    unsigned int visibleSta = 0;
    vector<PointingVector> pointingVectors;
    vector<unsigned int> endOfLastScans;
    for (const auto &thisSta: network.getStations()){
        unsigned long staid = thisSta.getId();

        if (!thisSta.getPARA().available || thisSta.getPARA().tagalong) {
            #ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL(trace) << "subcon " << this->printId() << " source " << thisSource.getName() << " ignore station " << thisSta.getName() << " (not available)";
            #endif
            continue;
        }

        if (thisSta.getNTotalScans() >= thisSta.getPARA().maxNumberOfScans){
            #ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL(trace) << "subcon " << this->printId()<< " source " << thisSource.getName() << " ignore station " << thisSta.getName() << " (not available - max number of allowed scans reached)";
            #endif
            continue;
        }

        if (!thisSta.getPARA().ignoreSources.empty()) {
            auto &PARA = thisSta.getPARA();
            if (find(PARA.ignoreSources.begin(), PARA.ignoreSources.end(), srcid) != PARA.ignoreSources.end()) {
                #ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL(trace) << "subcon " << this->printId()<< " source " << thisSource.getName() << " ignore station " << thisSta.getName() << " (source should be ignored for this station)";
                #endif
                continue;
            }
        }

        if (!thisSource.getPARA().ignoreStations.empty()) {
            const auto &PARA = thisSource.getPARA();
            if (find(PARA.ignoreStations.begin(), PARA.ignoreStations.end(), staid) !=
                PARA.ignoreStations.end()) {
                #ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL(trace) << "subcon " << this->printId() << " source " << thisSource.getName() << " ignore station " << thisSta.getName() << " (station should be ignored for this source)";
                #endif
                continue;
            }
        }

        PointingVector p(staid,srcid);

        const Station::WaitTimes &wtimes = thisSta.getWaittimes();
        unsigned int time = thisSta.getCurrentTime() + wtimes.fieldSystem + wtimes.preob;

        p.setTime(time);

        thisSta.calcAzEl(thisSource, p);

        bool flag = thisSta.isVisible(p, thisSource.getPARA().minElevation);
        if (flag){
            visibleSta++;
            endOfLastScans.push_back(thisSta.getCurrentTime());
            pointingVectors.push_back(std::move(p));
            #ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL(trace) << "subcon " << this->printId() << " source " << thisSource.getName() << " add station " << thisSta.getName();
            #endif
        }else{
            #ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL(trace) << "subcon " << this->printId() << " source " << thisSource.getName() << " ignore station " << thisSta.getName() << " (source not visible)";
            #endif
        }
    }
    if (visibleSta >= thisSource.getPARA().minNumberOfStations) {
        addScan(Scan(pointingVectors, endOfLastScans, type));
    }

}







