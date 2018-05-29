//
// Created by mschartn on 26.04.18.
//

#include "HighImpactScanDescriptor.h"

#include <utility>

using namespace std;
using namespace VieVS;

int HighImpactScanDescriptor::nextId = 0;
int HighImpactScanDescriptor::AzElDescriptor::nextId = 0;

HighImpactScanDescriptor::AzElDescriptor::AzElDescriptor(double az, double el, double margin, std::vector<int> staids):
        VieVS_Object(nextId++), az_{az}, el_{el}, margin_{margin}, staids_{std::move(staids)}{
}

HighImpactScanDescriptor::HighImpactScanDescriptor(unsigned int interval, unsigned int minTimeBetweenScans):
        VieVS_Object(nextId++), interval_{interval}, minTimeBetweenScans_{minTimeBetweenScans}{
}

void HighImpactScanDescriptor::addAzElDescriptor(double az, double el, double margin, const std::vector<int> &staids) {
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

std::vector<int> HighImpactScanDescriptor::getStationIds() const{
    vector<int> ids;
    for(const auto &any:azElDescritors_){
        ids.insert(ids.end(), any.getStaids().begin(), any.getStaids().end());
    }
    return ids;
}

void HighImpactScanDescriptor::possibleHighImpactScans(unsigned int idxTime,
                                                       const std::vector<Station> &stations,
                                                       const std::vector<Source> &sources) {

    // map: key = srcid, value:weight
    std::map<int,double> thisMap;
    unsigned int time = idxTime*interval_;

    // find possible highImpactScans
    // loop over himp pv
    for (const auto &any:azElDescritors_) {
        for(int staid: any.getStaids()){
            PointingVector pv(staid,-1);
            pv.setTime(time);
            const auto &thisSta = stations[staid];

            // loop over all sources
            for (const auto &source:sources){

                thisSta.calcAzEl(source,pv);
                if(thisSta.isVisible(pv,source.getPARA().minElevation)){
                    double score = any.highImpactScore(pv);

                    // if you have possible high impact score add score to map
                    if(score >0){
                        int srcid = source.getId();
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
        int srcid = itm.first;
        const Source &thisSource = sources[srcid];
        highImpactScans_.visibleScan(time,Scan::ScanType::highImpact,stations,thisSource);
    }

    scores_.push_back(move(thisMap));
}


double HighImpactScanDescriptor::AzElDescriptor::highImpactScore(const PointingVector &pv) const {
    int staid = pv.getStaid();
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

const vector<int> &HighImpactScanDescriptor::AzElDescriptor::getStaids() const {
    return staids_;
}


void HighImpactScanDescriptor::updateHighImpactScans(const std::vector<Station> &stations,
                                                     const std::vector<Source> &sources,
                                                     const boost::optional<Subnetting> &subnetting) {
    highImpactScans_.calcStartTimes(stations, sources);
    highImpactScans_.updateAzEl(stations, sources);
    highImpactScans_.constructAllBaselines(sources);
    highImpactScans_.calcAllBaselineDurations(stations, sources);
    highImpactScans_.calcAllScanDurations(stations, sources);
    highImpactScans_.checkIfEnoughTimeToReachEndposition(stations, sources);

    if (subnetting.is_initialized()) {
        highImpactScans_.createSubnettingScans(*subnetting, sources);
    }

    highImpactScans_.generateScore(stations,sources,scores_,interval_);
}


vector<Scan> HighImpactScanDescriptor::highestImpactScans(const std::vector<Station> &stations, const std::vector<Source> &sources) {
    return highImpactScans_.selectBest(stations, sources);
}

bool HighImpactScanDescriptor::isCorrectHighImpactScan(const Scan &target, const std::vector<Scan> &scans, const Source &source) {
    unsigned int minTimeBetweenSource = source.getPARA().minRepeat;

    return std::all_of(scans.begin(),scans.end(),[target, minTimeBetweenScans=minTimeBetweenScans_, minTimeBetweenSource](const Scan &scan){
        unsigned int targetStart = target.getTimes().getObservingStart();
        unsigned int targetEnd = target.getTimes().getObservingEnd();
        unsigned int scanStart = scan.getTimes().getObservingStart();
        unsigned int scanEnd = scan.getTimes().getObservingEnd();

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
