/*
 * Copyright (C) 2017 mschartn
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * File:   station.cpp
 * Author: mschartn
 * 
 * Created on June 21, 2017, 1:43 PM
 */

#include "Station.h"
#include "LookupTable.h"

using namespace std;
using namespace VieVS;
unsigned long VieVS::Station::nextId = 0;
unsigned long VieVS::Station::Parameters::nextId = 0;

void Station::Parameters::setParameters(const Station::Parameters &other) {
    firstScan = other.firstScan;
    available = other.available;
    tagalong = other.tagalong;
    availableForFillinmode = other.availableForFillinmode;

    weight = other.weight;
    minElevation = other.minElevation;

    minSNR = other.minSNR;

    maxSlewtime = other.maxSlewtime;
    maxSlewDistance = other.maxSlewDistance;
    minSlewDistance = other.minSlewDistance;
    maxWait = other.maxWait;
    maxScan = other.maxScan;
    minScan = other.minScan;
    maxNumberOfScans = other.maxNumberOfScans;

    ignoreSources = other.ignoreSources;
}

Station::Station(std::string sta_name, std::string tlc, std::shared_ptr<Antenna> sta_antenna,
                 std::shared_ptr<CableWrap> sta_cableWrap, std::shared_ptr<Position> sta_position,
                 std::shared_ptr<Equipment> sta_equip, std::shared_ptr<HorizonMask> sta_mask):
        VieVS_NamedObject(std::move(sta_name), std::move(tlc), nextId++),
        antenna_{move(sta_antenna)},
        cableWrap_{move(sta_cableWrap)},
        position_{move(sta_position)},
        equip_{move(sta_equip)},
        mask_{move(sta_mask)},
        currentPositionVector_{PointingVector(nextId-1,numeric_limits<unsigned long>::max())},
        parameters_{Parameters("empty")}{
    parameters_.firstScan = true;
}

void Station::setCurrentPointingVector(const PointingVector &pointingVector) noexcept {
    currentPositionVector_ = pointingVector;
}

bool Station::isVisible(const PointingVector &p, double minElevationSource) const noexcept {
    #ifdef VIESCHEDPP_LOG
    if(Flags::logTrace) BOOST_LOG_TRIVIAL(trace) << "station " << this->getName() << " check if source " << p.getSrcid() << " is visible";
    #endif

    if(p.getEl()<parameters_.minElevation){
        return false;
    }
    if(p.getEl()<minElevationSource){
        return false;
    }
    if(mask_ != nullptr && !mask_->visible(p)){
        return false;
    }
    return cableWrap_->anglesInside(p);

}

void Station::calcAzEl(const Source &source, PointingVector &p, AzelModel model) const noexcept {

    #ifdef VIESCHEDPP_LOG
    if(Flags::logTrace) BOOST_LOG_TRIVIAL(trace) << "station " << this->getName() << " calculate azimuth and elevation to source " << p.getSrcid();
    #endif

    double omega = 7.2921151467069805e-05; //1.00273781191135448*D2PI/86400;

    unsigned int time = p.getTime();
    //  TIME
    double date1 = 2400000.5;
    double mjd = TimeSystem::mjdStart + static_cast<double>(time) / 86400.0;

    // Earth Rotation
    double ERA = iauEra00(date1, mjd);

    // precession nutation
    double C[3][3] = {{1, 0, 0},
                      {0, 1, 0},
                      {0, 0, 1}};

    if (model == AzelModel::rigorous) {
        unsigned int nut_precalc_idx = 0;
        while (AstronomicalParameters::earth_nutTime[nut_precalc_idx + 1] < time) {
            ++nut_precalc_idx;
        }
        int delta = AstronomicalParameters::earth_nutTime[1] - AstronomicalParameters::earth_nutTime[0];

        unsigned int deltaTime = time - AstronomicalParameters::earth_nutTime[nut_precalc_idx];

        double x = AstronomicalParameters::earth_nutX[nut_precalc_idx] +
                   (AstronomicalParameters::earth_nutX[nut_precalc_idx + 1] - AstronomicalParameters::earth_nutX[nut_precalc_idx]) / delta *
                   deltaTime;
        double y = AstronomicalParameters::earth_nutY[nut_precalc_idx] +
                   (AstronomicalParameters::earth_nutY[nut_precalc_idx + 1] - AstronomicalParameters::earth_nutY[nut_precalc_idx]) / delta *
                   deltaTime;
        double s = AstronomicalParameters::earth_nutS[nut_precalc_idx] +
                   (AstronomicalParameters::earth_nutS[nut_precalc_idx + 1] - AstronomicalParameters::earth_nutS[nut_precalc_idx]) / delta *
                   deltaTime;


        iauC2ixys(x, y, s, C);
    }

    //  Polar Motion
    double W[3][3] = {{1,0,0},
                      {0,1,0},
                      {0,0,1}};
    //  GCRS to ITRS
    double c2t[3][3];
    iauC2tcio(C, ERA, W, c2t);
    double t2c[3][3] = {};
    iauTr(c2t, t2c);


    //  Transformation
    double v1[3] = {-omega*position_->getX(),
                     omega*position_->getY(),
                     0};

    double v1R[3] = {};
    iauRxp(t2c, v1, v1R);



    double k1a[3] = {};
    double k1a_t1[3];
    if (model == AzelModel::rigorous) {
        k1a_t1[0] = (AstronomicalParameters::earth_velocity[0] + v1[0]) / CMPS;
        k1a_t1[1] = (AstronomicalParameters::earth_velocity[1] + v1[1]) / CMPS;
        k1a_t1[2] = (AstronomicalParameters::earth_velocity[2] + v1[2]) / CMPS;
    }else{
        k1a_t1[0] = 0;
        k1a_t1[1] = 0;
        k1a_t1[2] = 0;
    }

    // Source vector in CRF
    const vector<double> & scrs_ = source.getSourceInCrs();
    double rqu[3] = {scrs_[0],scrs_[1],scrs_[2]};

    double k1a_t2[3] = {};
    iauSxp(iauPdp(rqu,k1a_t1),rqu,k1a_t2);
    k1a_t2[0] = -k1a_t2[0];
    k1a_t2[1] = -k1a_t2[1];
    k1a_t2[2] = -k1a_t2[2];

    double k1a_temp[3] ={};
    iauPpp ( rqu, k1a_t1, k1a_temp );
    iauPpp ( k1a_temp,k1a_t2,k1a);

    //  source in TRS
    double rq[3] = {};
    iauRxp(c2t, k1a, rq);



    //  station in local system
    const auto & g2l2 = position_->getGeodetic2Local();

    double g2l[3][3] = {{g2l2[0][0], g2l2[0][1], g2l2[0][2]},
                        {g2l2[1][0], g2l2[1][1], g2l2[1][2]},
                        {g2l2[2][0], g2l2[2][1], g2l2[2][2]}};
//    position_->geodetic2Local(g2l);

    double lq[3] = {};
    iauRxp(g2l,rq,lq);

    double zd = acos(lq[2]);
    double el = DPI/2 - zd;

    double saz = atan2(lq[1],lq[0]);
    if (lq[1]<0){
        saz = DPI*2+saz;
    }
    double az = fmod(saz+DPI,DPI*2);

    p.setAz(az);
    p.setEl(el);

    // only for hadc antennas
    double gmst = TimeSystem::mjd2gmst(mjd);

    double ha = gmst + position_->getLon() - source.getRa();
    while(ha>pi){
        ha = ha - twopi;
    }
    while(ha< -pi){
        ha = ha + twopi;
    }
    p.setHa(ha);
    p.setDc(source.getDe());
    // end of hadc part

    p.setTime(time);
}

double Station::distance(const Station &other) const noexcept {
    return position_->getDistance(*other.position_);
}

boost::optional<unsigned int> Station::slewTime(const PointingVector &pointingVector) const noexcept {
    #ifdef VIESCHEDPP_LOG
    if(Flags::logTrace) BOOST_LOG_TRIVIAL(trace) << "station " << this->getName() << " calculate slew time to source " << pointingVector.getSrcid();
    #endif
    if (parameters_.firstScan) {
        return 0;
    } else {
        unsigned int slewTime = antenna_->slewTime(currentPositionVector_, pointingVector);

        float distance = LookupTable::angularDistance(currentPositionVector_, pointingVector);

        if (slewTime > parameters_.maxSlewtime || distance < parameters_.minSlewDistance ||
            distance > parameters_.maxSlewDistance) {
            return boost::none;
        } else {
            return slewTime;
        }
    }
}

void Station::update(unsigned long nbl, const PointingVector &end, bool addToStatistics) noexcept {
    if(addToStatistics){
        ++nScans_;
        nObs_ += nbl;
    }
    ++nTotalScans_;
    currentPositionVector_ = end;

    if (parameters_.firstScan) {
        parameters_.firstScan = false;
    }
}

bool Station::checkForNewEvent(unsigned int time, bool &hardBreak) noexcept {

    bool flag = false;
    while (nextEvent_ < events_->size() && events_->at(nextEvent_).time <= time) {
        bool oldAvailable = parameters_.available;

        parameters_ = events_->at(nextEvent_).PARA;

        hardBreak = hardBreak || !events_->at(nextEvent_).softTransition;
        bool newAvailable = parameters_.available;

        if (!oldAvailable && newAvailable) {
            currentPositionVector_.setTime(events_->at(nextEvent_).time);
            parameters_.firstScan = true;
        }
        nextEvent_++;
        flag = true;
    }
    return flag;
}

unsigned int Station::maximumAllowedObservingTime(Timestamp ts) const noexcept {
    switch (ts){
        case Timestamp::start:{
            int tmp = static_cast<int>(nextEvent_)-2; // -1 would be current parameters and -2 are previous parameters
            while (tmp >= 0) {
                if(!events_->at(tmp).PARA.available){
                    return events_->at(tmp).time;
                }
                --tmp;
            }
            return 0;
        }
        case Timestamp::end:{
            unsigned int tmp = nextEvent_;
            while (tmp < events_->size()) {
                if(!events_->at(tmp).PARA.available){
                    return events_->at(tmp).time;
                }
                ++tmp;
            }
            return TimeSystem::duration;
        }
    }
}

bool Station::checkForTagalongMode(unsigned int time) const noexcept{
    bool tagalong = parameters_.tagalong;
    if(tagalong){
        if(nextEvent_ < events_->size() && events_->at(nextEvent_).time <= time){
            return true;
        }
    }
    return false;
}

void Station::applyNextEvent(std::ofstream &of) noexcept{
    unsigned int nextEventTimes = events_->at(nextEvent_).time;
    while (nextEvent_ < events_->size() && events_->at(nextEvent_).time <= nextEventTimes) {
        parameters_ = events_->at(nextEvent_).PARA;

        of << "###############################################\n";
        of << "## changing parameters for station: " << boost::format("%8s") % getName() << " ##\n";
        of << "###############################################\n";
        nextEvent_++;
    }
}


void Station::clearObservations() {
    nextEvent_ = 0;
    bool hardBreak;
    checkForNewEvent(0,hardBreak);

    currentPositionVector_ = PointingVector(getId(),-1);
    currentPositionVector_.setTime(0);
    currentPositionVector_.setAz((cableWrap_->getNLow() + cableWrap_->getNUp())/2);

    nScans_ = 0;
    nTotalScans_ = 0;
    nObs_ = 0;

    parameters_.firstScan=true;
}

std::pair<std::vector<double>, std::vector<double>> Station::getHorizonMask() const noexcept{
    if(mask_ != nullptr){
        return mask_->getHorizonMask();
    }else{
        return std::pair<std::vector<double>, std::vector<double>>();
    }
}



