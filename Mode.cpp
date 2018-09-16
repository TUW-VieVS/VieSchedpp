//
// Created by mschartn on 10.09.18.
//

#include "Mode.h"

using namespace VieVS;
using namespace std;

unsigned long VieVS::Mode::nextId = 0;

Mode::Mode(std::string name): VieVS_NamedObject{std::move(name), nextId++} {

}

void Mode::calcRecordingRates() {

}

void Mode::addIf(const If &newIf, const std::vector<unsigned long> &staids) {
    ifs_.emplace_back(newIf, staids);
}

void Mode::addBbc(const Bbc &newBbc, const std::vector<unsigned long> &staids) {
    bbcs_.emplace_back(newBbc, staids);
}

void Mode::addFreq(const Freq &newFreq, const std::vector<unsigned long> &staids) {
    freqs_.emplace_back(newFreq, staids);
}

void Mode::addTrack(const Track &newTrack, const std::vector<unsigned long> &staids) {
    tracks_.emplace_back(newTrack, staids);
}

void Mode::addTrackFrameFormat(const std::string &newTrackFrameFormat, const std::vector<unsigned long> &staids) {
    track_frame_formats_.emplace_back(newTrackFrameFormat, staids);
}

boost::optional<const std::vector<unsigned long> &>Mode::getAllStationsWithIf(std::string name) {
    for(const auto &any: ifs_){
        if(any.first.hasName(name)){
            return any.second;
        }
    }
    return boost::none;
}

boost::optional<const std::vector<unsigned long> &>Mode::getAllStationsWithBbc(std::string name) {
    for(const auto &any: bbcs_){
        if(any.first.hasName(name)){
            return any.second;
        }
    }
    return boost::none;
}

boost::optional<const std::vector<unsigned long> &>Mode::getAllStationsWithFreq(std::string name) {
    for(const auto &any: freqs_){
        if(any.first.hasName(name)){
            return any.second;
        }
    }
    return boost::none;
}

boost::optional<const std::vector<unsigned long> &>Mode::getAllStationsWithTrack(std::string name) {
    for(const auto &any: tracks_){
        if(any.first.hasName(name)){
            return any.second;
        }
    }
    return boost::none;
}

boost::optional<const std::vector<unsigned long> &>Mode::getAllStationsWithTrackFrameFormat(std::string name) {
    for(const auto &any: track_frame_formats_){
        if(any.first == name){
            return any.second;
        }
    }
    return boost::none;
}

boost::optional<const If &> Mode::getIf(unsigned long staid) {
    for(const auto &any: ifs_){
        if(find(any.second.begin(), any.second.end(), staid) == any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}

boost::optional<const Bbc &> Mode::getBbc(unsigned long staid) {
    for(const auto &any: bbcs_){
        if(find(any.second.begin(), any.second.end(), staid) == any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}

boost::optional<const Freq &> Mode::getFreq(unsigned long staid) {
    for(const auto &any: freqs_){
        if(find(any.second.begin(), any.second.end(), staid) == any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}

boost::optional<const Track &> Mode::getTrack(unsigned long staid) {
    for(const auto &any: tracks_){
        if(find(any.second.begin(), any.second.end(), staid) == any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}

boost::optional<const std::string &> Mode::getTrackFrameFormat(unsigned long staid) {
    for(const auto &any: track_frame_formats_){
        if(find(any.second.begin(), any.second.end(), staid) == any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}
