/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Source.cpp
 * Author: mschartn
 * 
 * Created on June 28, 2017, 11:25 AM
 */

#include "Source.h"

using namespace std;
using namespace VieVS;
unsigned long VieVS::Source::nextId = 0;
unsigned long VieVS::Source::Parameters::nextId = 0;

void Source::Parameters::setParameters(const Source::Parameters &other) {
    available = other.available;
    globalAvailable = other.globalAvailable;
    availableForFillinmode = other.availableForFillinmode;

    weight = other.weight;

    minSNR = other.minSNR;

    minNumberOfStations = other.minNumberOfStations;
    minFlux = other.minFlux;
    minRepeat = other.minRepeat;
    maxScan = other.maxScan;
    minScan = other.minScan;
    maxNumberOfScans = other.maxNumberOfScans;
    minElevation = other.minElevation;

    tryToFocusIfObservedOnce = other.tryToFocusIfObservedOnce;
    tryToFocusFactor = other.tryToFocusFactor;
    tryToFocusOccurrency = other.tryToFocusOccurrency;
    tryToFocusType = other.tryToFocusType;

    tryToObserveXTimesEvenlyDistributed = other.tryToObserveXTimesEvenlyDistributed;
    tryToObserveXTimesMinRepeat = other.tryToObserveXTimesMinRepeat;

    fixedScanDuration = other.fixedScanDuration;

    ignoreStations = other.ignoreStations;
    ignoreBaselines = other.ignoreBaselines;
    requiredStations = other.requiredStations;

}

Source::Source(const string &src_name, const string &src_name2, double src_ra_deg, double src_de_deg,
               unordered_map<string, unique_ptr<Flux> > &src_flux): VieVS_NamedObject(src_name,src_name2,nextId++),
                                                                    ra_{src_ra_deg*deg2rad}, de_{src_de_deg*deg2rad},
                                                                    parameters_{Parameters("empty")}{

    flux_ = std::make_shared<std::unordered_map<std::string, std::unique_ptr<Flux>>>(std::move(src_flux));

    PreCalculated preCalculated = PreCalculated();
    preCalculated.sourceInCrs.resize(3);
    preCalculated.sourceInCrs[0] = cos(de_)*cos(ra_);
    preCalculated.sourceInCrs[1] = cos(de_)*sin(ra_);
    preCalculated.sourceInCrs[2] = sin(de_);

    preCalculated_ = make_shared<PreCalculated>(move(preCalculated));
    condition_ = make_shared<Optimization>(Optimization());
}

double Source::getMaxFlux() const noexcept {
    double maxFlux = 0;

    for (auto& any: *flux_){
        double thisFlux = any.second->getMaximumFlux();
        if (thisFlux > maxFlux){
            maxFlux = thisFlux;
        }
    }
    return maxFlux;
}

double Source::getSunDistance() const noexcept{
    double sunRa = AstronomicalParameters::sun_radc[0];
    double sunDe = AstronomicalParameters::sun_radc[1];

    double tmp = sin(sunDe) * sin(de_) + cos(sunDe) * cos(de_) * cos(sunRa - ra_);
    return acos(tmp);
}

double Source::observedFlux(const string &band, double gmst, const std::vector<double> &dxyz) const noexcept {
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(trace) << "source " << this->getName() << " get observed flux density";
    #endif

    double ha = gmst - ra_;

    double u = dxyz[0] * sin(ha) + dxyz[1] * cos(ha);
    double v = dxyz[2]*cos(de_) + sin(de_) * (-dxyz[0] * cos(ha) + dxyz[1] * sin(ha));

    double flux = flux_->at(band)->observedFlux(u, v);
    return flux;
}

void Source::update(unsigned long nbl, unsigned int time, bool addToStatistics) noexcept {
    if(addToStatistics){
        ++nScans_;
        nObs_ += nbl;
        lastScan_ = time;
    }
    ++nTotalScans_;
}

bool Source::checkForNewEvent(unsigned int time, bool &hardBreak) noexcept {
    bool flag = false;
    while (nextEvent_ < events_->size() && events_->at(nextEvent_).time <= time) {
        double oldMinFlux = parameters_.minFlux;
        bool oldGlobalAvailable = parameters_.globalAvailable;
        parameters_ = events_->at(nextEvent_).PARA;
        parameters_.globalAvailable = oldGlobalAvailable;

        hardBreak = hardBreak || !events_->at(nextEvent_).softTransition;
        nextEvent_++;

        if(getMaxFlux() < parameters_.minFlux){
            parameters_.available = false;
        }
        flag = true;
    }
    return flag;
}

std::string Source::getRaString() const noexcept{
    double h = rad2deg*ra_/15;
    auto m = fmod(h, 1.);
    m *= 60;
    auto s = fmod(m, 1.);
    s *= 60;
    string str = (boost::format("%02dh%02dm%08.5fs") % static_cast<int>(h) %static_cast<int>(m) %s).str();
    return str;
}

std::string Source::getDeString() const noexcept{
    double d = rad2deg*de_;
    auto m = abs(fmod(d, 1.));
    m *= 60;
    auto s = fmod(m, 1.);
    s *= 60;
    string str = (boost::format("%+03dd%02d'%08.5f\"") %static_cast<int>(d) %static_cast<int>(m) %s).str();
    return str;
}

void Source::clearObservations() {
    lastScan_ = 0;
    nScans_ = 0;
    nTotalScans_ = 0;
    nObs_ = 0;

    bool hardBreak = false;
    nextEvent_ = 0;
    checkForNewEvent(0, hardBreak);
}

//Source Source::clone() const{
//    std::unordered_map<string,unique_ptr<Flux> > newFlux;
//
//    for(const auto &any: flux_){
//        newFlux[any.first] = any.second->clone();
//    }
//
//    return Source(name_,alternativeName_,ra_*rad2deg, de_*rad2deg, newFlux);
//}
