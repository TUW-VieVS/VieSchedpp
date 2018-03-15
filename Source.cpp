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
int VieVS::Source::nextId = 0;

Source::Source(const string &src_name, const string &src_name2, double src_ra_deg, double src_de_deg,
               unordered_map<string, Flux> src_flux): VieVS_NamedObject(src_name,src_name2,nextId++),
                                                      ra_{src_ra_deg * deg2rad}, de_{src_de_deg * deg2rad},
                                                      flux_{std::move(src_flux)}, lastScan_{0}, nScans_{0},
                                                      nTotalScans_{0}, nBaselines_{0}{

    preCalculated_.sourceInCrs.resize(3);
    preCalculated_.sourceInCrs[0] = cos(de_)*cos(ra_);
    preCalculated_.sourceInCrs[1] = cos(de_)*sin(ra_);
    preCalculated_.sourceInCrs[2] = sin(de_);
}

double Source::angleDistance(const Source &other) const noexcept {
    return acos(cos(de_)*cos(other.de_) * cos(ra_-other.ra_) + sin(de_)*sin(other.de_));
}

bool Source::isStrongEnough(double &maxFlux) const noexcept {
    maxFlux = 0;

    for (auto& any: flux_){
        double thisFlux = any.second.getMaximumFlux();
        if (thisFlux > maxFlux){
            maxFlux = thisFlux;
        }
    }
    return maxFlux > parameters_.minFlux;
}

namespace VieVS {
    ostream &operator<<(ostream &out, const Source &src) noexcept {
        cout << boost::format("%=36s\n") % src.name_;
        double ra_deg = src.ra_ * rad2deg;
        double de_deg = src.de_ * rad2deg;
        cout << "position:\n";
        cout << boost::format("  RA: %10.6f [deg]\n") % ra_deg;
        cout << boost::format("  DE: %10.6f [deg]\n") % de_deg;
//        cout << src.flux;
        cout << "------------------------------------\n";
        return out;
    }
}

double Source::observedFlux(const string &band, double gmst, double dx, double dy, double dz) const noexcept {

    double ha = gmst - ra_;

    double u = dx * sin(ha) + dy * cos(ha);
    double v = dz*cos(de_) + sin(de_) * (-dx * cos(ha) + dy * sin(ha));

    double flux = flux_.at(band).observedFlux(u, v);
    return flux;
}

void Source::update(unsigned long nbl, unsigned int time, bool addToStatistics) noexcept {
    if(addToStatistics){
        ++nScans_;
        nBaselines_ += nbl;
        lastScan_ = time;
    }
    ++nTotalScans_;
}

bool Source::checkForNewEvent(unsigned int time, bool &hardBreak, bool output, ofstream &bodyLog) noexcept {
    bool flag = false;
    while (nextEvent_ < events_.size() && events_[nextEvent_].time <= time) {
        double oldMinFlux = parameters_.minFlux;
        bool oldGlobalAvailable = parameters_.globalAvailable;
        parameters_ = events_[nextEvent_].PARA;
        parameters_.globalAvailable = oldGlobalAvailable;
        double newMinFlux = parameters_.minFlux;
        hardBreak = hardBreak || !events_[nextEvent_].softTransition;

        if (output && time < TimeSystem::duration) {
            bodyLog << "###############################################\n";
            bodyLog << "## changing parameters for source: " << boost::format("%8s") % name_ << "  ##\n";
            bodyLog << "###############################################\n";
        }

        nextEvent_++;
        if (oldMinFlux != newMinFlux) {
            double maxFlux = 0;
            bool strongEnough = isStrongEnough(maxFlux);
            if (!strongEnough) {
                referencePARA().setAvailable(false);
                bodyLog << "source: " << boost::format("%8s") % name_ << " not strong enough! (max flux = "
                        << boost::format("%4.2f") % maxFlux << " min required flux = "
                        << boost::format("%4.2f") % parameters_.minFlux << ")\n";;
            }
            flag = true;
        }
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
    nBaselines_ = 0;

    bool hardBreak = false;
    ofstream dummy;
    nextEvent_ = 0;
    checkForNewEvent(0, hardBreak, false, dummy);
}
