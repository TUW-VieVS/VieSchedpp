//
// Created by mschartn on 26.04.18.
//

#include "HighImpactScanDescriptor.h"

#include <utility>

using namespace std;
using namespace VieVS;

unsigned long HighImpactScanDescriptor::nextId = 0;
unsigned long HighImpactScanDescriptor::AzElDescriptor::nextId = 0;

HighImpactScanDescriptor::AzElDescriptor::AzElDescriptor(double az, double el, double margin, std::vector<unsigned long> staids):
        VieVS_Object(nextId++), az_{az}, el_{el}, margin_{margin}, staids_{std::move(staids)}{
}

HighImpactScanDescriptor::HighImpactScanDescriptor(unsigned int interval, unsigned int minTimeBetweenScans):
        VieVS_Object(nextId++), interval_{interval}, minTimeBetweenScans_{minTimeBetweenScans}{
}

void HighImpactScanDescriptor::addAzElDescriptor(double az, double el, double margin, const std::vector<unsigned long> &staids) {
    azElDescritors_.emplace_back(az,el,margin,staids);
}

double HighImpactScanDescriptor::highImpactScore(const Scan &scan) const {
    double score = 0;
    for(const auto &thisAzElDesc:azElDescritors_){
        for(int i=0; i<scan.getNSta(); ++i){
            const auto &pv = scan.getPointingVector(i);
            score += thisAzElDesc.highImpactScore(pv);
        }
    }
    return score;
}

double HighImpactScanDescriptor::highImpactScore(const PointingVector &pv) const{
    double score = 0;
    for(const auto &thisAzElDesc:azElDescritors_){
        score += thisAzElDesc.highImpactScore(pv);
    }
    return score;
}

std::vector<unsigned long> HighImpactScanDescriptor::getStationIds() const{
    vector<unsigned long> ids;
    for(const auto &any:azElDescritors_){
        ids.insert(ids.end(), any.getStaids().begin(), any.getStaids().end());
    }
    return ids;
}

void HighImpactScanDescriptor::possibleHighImpactScans(unsigned int idxTime,
                                                       const Network &network,
                                                       const std::vector<Source> &sources) {

    // map: key = srcid, value:weight
    std::map<unsigned long,double> thisMap;
    unsigned int time = idxTime*interval_;

    // find possible highImpactScans
    // loop over himp pv
    for (const auto &any:azElDescritors_) {
        for(unsigned long staid: any.getStaids()){
            PointingVector pv(staid,numeric_limits<unsigned long>::max());
            pv.setTime(time);
            const auto &thisSta = network.getStation(staid);

            // loop over all sources
            for (const auto &source:sources){

                thisSta.calcAzEl(source,pv);
                if(thisSta.isVisible(pv,source.getPARA().minElevation)){
                    double score = any.highImpactScore(pv);

                    // if you have possible high impact score add score to map
                    if(score >0){
                        unsigned long srcid = source.getId();
                        auto i = thisMap.find(srcid);
                        if(i==thisMap.end()){
                            thisMap[srcid] = score;
                        }else{
                            thisMap[srcid] = thisMap[srcid]+score;
                        }
                    }
                }
            }
        }
    }

    // create high impact scans
    for(const auto& itm: thisMap){
        unsigned long srcid = itm.first;
        const Source &thisSource = sources[srcid];
        highImpactScans_.visibleScan(time,Scan::ScanType::highImpact,network,thisSource);
    }

    scores_.push_back(move(thisMap));
}


double HighImpactScanDescriptor::AzElDescriptor::highImpactScore(const PointingVector &pv) const {
    unsigned long staid = pv.getStaid();
    double score = 0;
    if(std::find(staids_.begin(), staids_.end(), staid) != staids_.end()){
        double thisAz = util::wrapToPi(pv.getAz());
        double thisEl = pv.getEl();
        double dist = LookupTable::angularDistance(thisAz,thisEl,az_,el_);
        if(dist<margin_){
            score = (margin_-dist)/margin_;
        }
    }
    return score;
}

const vector<unsigned long> &HighImpactScanDescriptor::AzElDescriptor::getStaids() const {
    return staids_;
}


void HighImpactScanDescriptor::updateHighImpactScans(const Network &network,
                                                     const std::vector<Source> &sources,
                                                     const boost::optional<Subnetting> &subnetting) {
    highImpactScans_.calcStartTimes(network, sources);
    highImpactScans_.updateAzEl(network, sources);
    highImpactScans_.constructAllBaselines(network, sources);
    highImpactScans_.calcAllBaselineDurations(network, sources);
    highImpactScans_.calcAllScanDurations(network, sources);
    highImpactScans_.checkIfEnoughTimeToReachEndposition(network, sources);

    if (subnetting.is_initialized()) {
        highImpactScans_.createSubnettingScans(*subnetting, sources);
    }

    highImpactScans_.generateScore(network,sources,scores_,interval_);
}


vector<Scan> HighImpactScanDescriptor::highestImpactScans(const Network &network, const std::vector<Source> &sources) {
    return highImpactScans_.selectBest(network, sources);
}

bool HighImpactScanDescriptor::isCorrectHighImpactScan(const Scan &target, const std::vector<Scan> &scans, const Source &source) {
    unsigned int minTimeBetweenSource = source.getPARA().minRepeat;

    return std::all_of(scans.begin(),scans.end(),[target, minTimeBetweenScans=minTimeBetweenScans_, minTimeBetweenSource](const Scan &scan){
        unsigned int targetStart = target.getTimes().getObservingTime(Timestamp::start);
        unsigned int targetEnd = target.getTimes().getObservingTime(Timestamp::end);
        unsigned int scanStart = scan.getTimes().getObservingTime(Timestamp::start);
        unsigned int scanEnd = scan.getTimes().getObservingTime(Timestamp::end);

        bool valid = true;

        unsigned int diff1 = util::absDiff(targetEnd,scanStart);
        unsigned int diff2 = util::absDiff(scanEnd,targetStart);

        // enough time in between
        if(diff1<minTimeBetweenScans){
            valid = false;
        }
        if(diff2<minTimeBetweenScans){
            valid = false;
        }

        // check if scan does not start/end during other scan
        if(targetStart>scanStart && targetStart<scanEnd){
            valid = false;
        }
        if(targetEnd>scanStart && targetEnd<scanEnd){
            valid = false;
        }

        // check if one of those is inside the other
        if(targetStart>=scanStart && targetEnd<=scanEnd){
            valid = false;
        }
        if(scanStart>=targetStart && scanEnd<=targetEnd){
            valid = false;
        }


        unsigned int diff_src = util::absDiff(targetStart,scanStart);
        if(diff_src < minTimeBetweenSource){
            valid = false;
        }

        return valid;
    });
}

void HighImpactScanDescriptor::updateLogfile(std::ofstream &bodyLog) {
    bodyLog << "*   depth 0 considered: single Scans " << highImpactScans_.getNumberSingleScans() << " subnetting scans " << highImpactScans_.getNumberSubnettingScans() << "\n*\n";
}
