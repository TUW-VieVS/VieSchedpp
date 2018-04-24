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
int VieVS::Station::nextId = 0;
int VieVS::Station::Parameters::nextId = 0;

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

Station::Station(std::string sta_name, std::shared_ptr<Antenna> sta_antenna, std::shared_ptr<CableWrap> sta_cableWrap,
                 std::shared_ptr<Position> sta_position, std::shared_ptr<Equipment> sta_equip,
                 std::shared_ptr<HorizonMask> sta_mask):
        VieVS_NamedObject(std::move(sta_name),nextId++), antenna_{move(sta_antenna)}, cableWrap_{move(sta_cableWrap)},
        position_{move(sta_position)}, equip_{move(sta_equip)}, mask_{move(sta_mask)}, skyCoverageId_{-1},
        nScans_{0}, nBaselines_{0}, currentPositionVector_{PointingVector(nextId-1,-1)}, parameters_{Parameters("empty")}{
}

void Station::setCurrentPointingVector(const PointingVector &pointingVector) noexcept {
    currentPositionVector_ = pointingVector;
}

namespace VieVS {
    ostream &operator<<(ostream &out, const Station &sta) noexcept {
        cout << boost::format("%=36s\n") % sta.getName();
        cout << sta.position_;
        cout << "uses sky coverage id: " << sta.skyCoverageId_ << "\n";
        cout << "------------------------------------\n";
        return out;
    }
}

bool Station::isVisible(const PointingVector &p, double minElevationSource = 0) const noexcept {

    if(p.getEl()<parameters_.minElevation){
        return false;
    }
    if(mask_ != nullptr && !mask_->visible(p)){
        return false;
    }
    return cableWrap_->anglesInside(p);

}

void Station::calcAzEl(const Source &source, PointingVector &p, AzelModel model) const noexcept {


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
        while (Nutation::nutTime[nut_precalc_idx + 1] < time) {
            ++nut_precalc_idx;
        }
        int delta = Nutation::nutTime[1] - Nutation::nutTime[0];

        unsigned int deltaTime = time - Nutation::nutTime[nut_precalc_idx];

        double x = Nutation::nutX[nut_precalc_idx] +
                   (Nutation::nutX[nut_precalc_idx + 1] - Nutation::nutX[nut_precalc_idx]) / delta *
                   deltaTime;
        double y = Nutation::nutY[nut_precalc_idx] +
                   (Nutation::nutY[nut_precalc_idx + 1] - Nutation::nutY[nut_precalc_idx]) / delta *
                   deltaTime;
        double s = Nutation::nutS[nut_precalc_idx] +
                   (Nutation::nutS[nut_precalc_idx + 1] - Nutation::nutS[nut_precalc_idx]) / delta *
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
        k1a_t1[0] = (Earth::velocity[0] + v1[0]) / CMPS;
        k1a_t1[1] = (Earth::velocity[1] + v1[1]) / CMPS;
        k1a_t1[2] = (Earth::velocity[2] + v1[2]) / CMPS;
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



    //  source in local system
    double g2l[3][3] = {{preCalculated_->g2l[0][0],preCalculated_->g2l[0][1],preCalculated_->g2l[0][2]},
                        {preCalculated_->g2l[1][0],preCalculated_->g2l[1][1],preCalculated_->g2l[1][2]},
                        {preCalculated_->g2l[2][0],preCalculated_->g2l[2][1],preCalculated_->g2l[2][2]},};

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

void Station::preCalc(const vector<double> &distance, const vector<double> &dx, const vector<double> &dy,
                           const vector<double> &dz) noexcept {

    preCalculated_ = make_shared<PreCalculated>(distance,dx,dy,dz);

    double lat = position_->getLat();
    double lon = position_->getLon();

    double theta = DPI/2-lat;

    const double cosTheta = cos(theta);
    const double sinTheta = sin(theta);
    double roty[3][3] = {{cosTheta, 0,  -sinTheta},
                         {0,        -1, 0},
                         {sinTheta, 0,  cosTheta} };

    const double cosLon = cos(lon);
    const double sinLon = sin(lon);
    double rotz[3][3] = {{cosLon,  sinLon, 0},
                         {-sinLon, cosLon, 0},
                         {0,       0,      1}};

    double g2l[3][3] = {};

    iauRxr(roty,rotz,g2l);

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            preCalculated_->g2l[i][j] = g2l[i][j];
        }
    }
}

double Station::distance(const Station &other) const noexcept {
    return position_->getDistance(*other.position_);
}

boost::optional<unsigned int> Station::slewTime(const PointingVector &pointingVector) const noexcept {
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

void Station::update(unsigned long nbl, const PointingVector &start, const PointingVector &end, bool addToStatistics) noexcept {
    if(addToStatistics){
        ++nScans_;
        nBaselines_ += nbl;
    }
    ++nTotalScans_;
    pointingVectorsStart_.push_back(start);
    pointingVectorsEnd_.push_back(end);
    currentPositionVector_ = end;

    if (parameters_.firstScan) {
        parameters_.firstScan = false;
    }
}

void Station::checkForNewEvent() noexcept {

    while (nextEvent_ < events_->size() && events_->at(nextEvent_).time == 0) {
        parameters_ = events_->at(nextEvent_).PARA;
        nextEvent_++;
    }
}

void Station::checkForNewEvent(unsigned int time, bool &hardBreak, bool output, std::ofstream &out) noexcept {

    while (nextEvent_ < events_->size() && events_->at(nextEvent_).time <= time) {
        bool oldAvailable = parameters_.available;

        parameters_ = events_->at(nextEvent_).PARA;

        hardBreak = hardBreak || !events_->at(nextEvent_).softTransition;
        bool newAvailable = parameters_.available;

        if (!oldAvailable && newAvailable) {
            currentPositionVector_.setTime(events_->at(nextEvent_).time);
            parameters_.firstScan = true;
        }

        if(output && time < TimeSystem::duration){
            out << "###############################################\n";
            out << "## changing parameters for station: " << boost::format("%8s") % getName() << " ##\n";
            out << "###############################################\n";
        }
        nextEvent_++;
    }
}

bool Station::checkForTagalongMode(unsigned int time) noexcept{
    bool tagalong = parameters_.tagalong;
    if(tagalong){
        if(nextEvent_ < events_->size() && events_->at(nextEvent_).time <= time){
            return true;
        }
    }
    return false;
}

void Station::applyNextEvent(std::ofstream &out) noexcept{
    unsigned int nextEventTimes = events_->at(nextEvent_).time;
    while (nextEvent_ < events_->size() && events_->at(nextEvent_).time <= nextEventTimes) {
        parameters_ = events_->at(nextEvent_).PARA;

        out << "###############################################\n";
        out << "## changing parameters for station: " << boost::format("%8s") % getName() << " ##\n";
        out << "###############################################\n";
        nextEvent_++;
    }

}


void Station::clearObservations() {
    nextEvent_ = 0;
    checkForNewEvent();

    currentPositionVector_ = PointingVector(getId(),-1);
    currentPositionVector_.setTime(0);

    pointingVectorsStart_.clear();
    pointingVectorsEnd_.clear();

    nScans_ = 0;
    nTotalScans_ = 0;
    nBaselines_ = 0;

    parameters_.firstScan=true;
}

void Station::sortPointingVectors() {

    sort(pointingVectorsStart_.begin(),pointingVectorsStart_.end(), [](const PointingVector &pv1, const PointingVector &pv2){
        return pv1.getTime() < pv2.getTime();
    });

    sort(pointingVectorsEnd_.begin(),pointingVectorsEnd_.end(), [](const PointingVector &pv1, const PointingVector &pv2){
        return pv1.getTime() < pv2.getTime();
    });

}


