/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   baseline.cpp
 * Author: mschartn
 * 
 * Created on June 29, 2017, 3:25 PM
 */

#include "Baseline.h"
using namespace std;
using namespace VieVS;

unsigned long Baseline::nextId = 0;
unsigned long VieVS::Baseline::Parameters::nextId = 0;

void Baseline::Parameters::setParameters(const Baseline::Parameters &other) {
    ignore = other.ignore;
    weight = other.weight;
    minSNR = other.minSNR;
}

Baseline::Baseline(std::string name, std::string alternativeName, unsigned long staid1, unsigned long staid2):
        VieVS_NamedObject(move(name), move(alternativeName), nextId++),
        staid1_{staid1}, staid2_{staid2}, parameters_{Parameters("empty")} {

}

bool Baseline::hasStationIds(unsigned long staid1, unsigned long staid2) const noexcept{
    if(staid1 > staid2){
        swap(staid1, staid2);
    }

    return staid1 == staid1_ && staid2 == staid2_;
}

bool Baseline::hasStationIds(const std::pair<unsigned long, unsigned long> &staids)const noexcept{
    return hasStationIds(staids.first, staids.second);
}


bool Baseline::checkForNewEvent(unsigned int time, bool &hardBreak) noexcept {
    bool flag = false;
    while (nextEvent_ < events_->size() && events_->at(nextEvent_).time <= time) {

        parameters_ = events_->at(nextEvent_).PARA;

        hardBreak = hardBreak || !events_->at(nextEvent_).softTransition;

        nextEvent_++;
        flag = true;
    }
    return flag;

}

void Baseline::update(bool influence) noexcept {
    if(influence){
        ++nObs_;
        ++nTotalObs_;
    }else{
        ++nTotalObs_;
    }
}

