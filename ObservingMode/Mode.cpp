#include <utility>

/*
 *  VieSched++ Very Long Baseline Interferometry (VLBI) Scheduling Software
 *  Copyright (C) 2018  Matthias Schartner
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Mode.h"

using namespace VieVS;
using namespace std;

unsigned long VieVS::Mode::nextId = 0;


Mode::Mode(std::string name, unsigned long nsta): VieVS_NamedObject{std::move(name), nextId++}, nsta_{nsta} {

}

boost::property_tree::ptree Mode::toPropertytree(const std::vector<std::string> &stations) const {
    boost::property_tree::ptree p;
    p.add("<xmlattr>.name",getName());
    for(const auto &any : freqs_){
        if(!any.second.empty()){
            p.add_child("FREQ",staids2propertyTree(any.first->getName(), any.second, stations));
        }
    }
    for(const auto &any : bbcs_){
        if(!any.second.empty()){
            p.add_child("BBC",staids2propertyTree(any.first->getName(), any.second, stations));
        }
    }
    for(const auto &any : ifs_){
        if(!any.second.empty()){
            p.add_child("IF",staids2propertyTree(any.first->getName(), any.second, stations));
        }
    }
    for(const auto &any : tracks_){
        if(!any.second.empty()){
            p.add_child("TRACKS",staids2propertyTree(any.first->getName(), any.second, stations));
        }
    }
    for(const auto &any : track_frame_formats_){
        if(!any.second.empty()){
            p.add_child("track_frame_formats",staids2propertyTree(*any.first, any.second, stations));
        }
    }
    return p;
}

boost::property_tree::ptree Mode::staids2propertyTree(const std::string &name, const std::vector<unsigned long> &ids,
                                                      const std::vector<std::string> &staNames) const {
    boost::property_tree::ptree t;
    t.add("<xmlattr>.name", name);
    for(unsigned long staid : ids){
        boost::property_tree::ptree s;
        s.add("station",staNames[staid]);
        t.add_child("station",s.get_child("station"));
    }
    return t;
}


void Mode::calcRecordingRates() {

    for(unsigned long staid1 = 0; staid1 < nsta_; ++staid1){
        const auto &freq1 = getFreq(staid1);
        const auto &tracks1 = getTracks(staid1);
        // check if station 1 is part of this observing mode
        if(!freq1.is_initialized() || !tracks1.is_initialized()){
            continue;
        }

        for(unsigned long staid2 = staid1+1; staid2 < nsta_; ++staid2){
            const auto &freq2 = getFreq(staid2);
            const auto &tracks2 = getTracks(staid2);
            // check if station 2 is part of this observing mode
            if(!freq2.is_initialized() || !tracks2.is_initialized()){
                continue;
            }

            auto bitsPerChannel = tracks1.get()->numberOfBitsPerChannel(tracks2.get());
            auto overlappingFrequencies = freq1.get()->observingRate(freq2.get(), bitsPerChannel);

            staids2recordingRate_[{staid1, staid2}] = overlappingFrequencies;
        }
    }
}

void Mode::setRecordingRates(const std::string &band, double recRate) {
    for(unsigned long staid1 = 0; staid1 < nsta_; ++staid1){
        for(unsigned long staid2 = staid1+1; staid2 < nsta_; ++staid2){
            staids2recordingRate_[{staid1, staid2}][band] = recRate;
        }
    }
}



boost::optional<const std::shared_ptr<const If> &> Mode::getIf(unsigned long staid) const {
    for(const auto &any: ifs_){
        if(find(any.second.begin(), any.second.end(), staid) != any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}

boost::optional<const std::shared_ptr<const Bbc> &> Mode::getBbc(unsigned long staid) const {
    for(const auto &any: bbcs_){
        if(find(any.second.begin(), any.second.end(), staid) != any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}

boost::optional<const std::shared_ptr<const Freq> &> Mode::getFreq(unsigned long staid) const {
    for(const auto &any: freqs_){
        if(find(any.second.begin(), any.second.end(), staid) != any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}

boost::optional<const std::shared_ptr<const Track> &> Mode::getTracks(unsigned long staid) const {
    for(const auto &any: tracks_){
        if(find(any.second.begin(), any.second.end(), staid) != any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}

boost::optional<const std::shared_ptr<const std::string> &> Mode::getTrackFrameFormat(unsigned long staid) const {
    for(const auto &any: track_frame_formats_){
        if(find(any.second.begin(), any.second.end(), staid) != any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}

void Mode::summary(std::ofstream &of, const std::vector<std::string> &stations) const{

    of << "    observing mode: " << getName() << ":\n";

    for(const auto &band : bands_){

        std::map<double, vector<string>> rate2baseline;
        for(unsigned long staid1=0; staid1<nsta_; ++staid1){
            for(unsigned long staid2=staid1+1; staid2<nsta_; ++staid2){

                double rate = recordingRate(staid1,staid2,band);
                if(rate == 0){
                    continue;
                }

                string name = (boost::format("%s-%s") % stations[staid1] % stations[staid2]).str();

                auto it2 = rate2baseline.find(rate);
                if(it2 == rate2baseline.end()){
                    rate2baseline[rate] = {name};
                }else{
                    rate2baseline[rate].push_back(name);
                }
            }
        }

        for(const auto &any : rate2baseline){
            string title = (boost::format("        band: %2s recording rate: %7.2f [Mbit/s]") % band % (any.first *1e-6)).str();
            util::outputObjectList(title, any.second, of, 12);
        }
    }
}

double Mode::recordingRate(unsigned long staid1, unsigned long staid2, const std::string &band) const{

    auto it = staids2recordingRate_.find({staid1, staid2});
    // if station id combination is not saved in map return 0
    if(it == staids2recordingRate_.end()){
        return 0;
    }

    // check if band exists
    auto it2 = it->second.find(band);
    if(it2 == it->second.end()){
        return 0;
    }

    return it2->second;
}

boost::optional<const std::vector<unsigned long> &> Mode::getAllStationsWithBlock(
        const std::shared_ptr<const If> &this_if) const {
    for(const auto &any : ifs_) {
        if (any.first == this_if) {
            return any.second;
        }
    }
    return boost::optional<const vector<unsigned long> &>();
}

boost::optional<const std::vector<unsigned long> &> Mode::getAllStationsWithBlock(const std::shared_ptr<const Bbc> &bbc) const {
    for(const auto &any : bbcs_) {
        if (any.first == bbc) {
            return any.second;
        }
    }
    return boost::optional<const vector<unsigned long> &>();
}

boost::optional<const std::vector<unsigned long> &> Mode::getAllStationsWithBlock(
        const std::shared_ptr<const Freq> &freq) const {
    for(const auto &any : freqs_) {
        if (any.first == freq) {
            return any.second;
        }
    }
    return boost::optional<const vector<unsigned long> &>();
}

boost::optional<const std::vector<unsigned long> &> Mode::getAllStationsWithBlock(
        const std::shared_ptr<const Track> &track) const {
    for(const auto &any : tracks_) {
        if (any.first == track) {
            return any.second;
        }
    }
    return boost::optional<const vector<unsigned long> &>();
}

boost::optional<const std::vector<unsigned long> &> Mode::getAllStationsWithBlock(
        const std::shared_ptr<const std::string> &trackFrameFormat) const {
    for(const auto &any : track_frame_formats_) {
        if (*any.first == *trackFrameFormat) {
            return any.second;
        }
    }
    return boost::optional<const vector<unsigned long> &>();
}


void Mode::toVexModeDefiniton(std::ofstream &of, const std::vector<std::string> &stations) const{
    string eol = ";\n";

    // get number of characters in longest name
    unsigned long longestName = 9;
    for(const auto &any : freqs_){
        if(any.first->getName().length() > longestName){
            longestName = any.first->getName().length();
        }
    }
    for(const auto &any : bbcs_){
        if(any.first->getName().length() > longestName){
            longestName = any.first->getName().length();
        }
    }
    for(const auto &any : ifs_){
        if(any.first->getName().length() > longestName){
            longestName = any.first->getName().length();
        }
    }
    for(const auto &any : tracks_){
        if(any.first->getName().length() > longestName){
            longestName = any.first->getName().length();
        }
    }
    for(const auto &any : track_frame_formats_){
        if(any.first->length()+7 > longestName){
            longestName = any.first->length()+7;
        }
    }
    string fmt = (boost::format("%%-%ds") %longestName).str();


    // output
    of << "    def " << getName() << eol;

    for(const auto &any : freqs_){
        of << "        ref $FREQ =             " << boost::format(fmt) % any.first->getName();
        for(int i=0; i<nsta_; ++i){
            if(find(any.second.begin(), any.second.end(), i) == any.second.end()){
                of << "     ";
            }else{
                of << " : " << stations.at(i);
            }
        }
        of << " " << eol;
    }

    for(const auto &any : bbcs_){
        of << "        ref $BBC =              " << boost::format(fmt) % any.first->getName();
        for(int i=0; i<nsta_; ++i){
            if(find(any.second.begin(), any.second.end(), i) == any.second.end()){
                of << "     ";
            }else{
                of << " : " << stations.at(i);
            }
        }
        of << " " << eol;
    }

    for(const auto &any : ifs_){
        of << "        ref $IF =               " << boost::format(fmt) % any.first->getName();
        for(int i=0; i<nsta_; ++i){
            if(find(any.second.begin(), any.second.end(), i) == any.second.end()){
                of << "     ";
            }else{
                of << " : " << stations.at(i);
            }
        }
        of << " " << eol;
    }

    for(const auto &any : tracks_){
        of << "        ref $TRACKS =           " << boost::format(fmt) % any.first->getName();
        for(int i=0; i<nsta_; ++i){
            if(find(any.second.begin(), any.second.end(), i) == any.second.end()){
                of << "     ";
            }else{
                of << " : " << stations.at(i);
            }
        }
        of << " " << eol;
    }

    for(const auto &any : track_frame_formats_){
        of << "        ref $TRACKS =           " << boost::format(fmt) % (*any.first+"_format");
        for(int i=0; i<nsta_; ++i){
            if(find(any.second.begin(), any.second.end(), i) == any.second.end()){
                of << "     ";
            }else{
                of << " : " << stations.at(i);
            }
        }
        of << " " << eol;
    }


    of << "        ref $PASS_ORDER =       " << boost::format(fmt) % "passOrder";
    for(const auto &any : stations){
        of << " : " << any;
    }
    of << " " << eol;

    of << "        ref $ROLL =             " << boost::format(fmt) % "NO_ROLL";;
    for(const auto &any : stations){
        of << " : " << any;
    }
    of << " " << eol;

    of << "        ref $PHASE_CAL_DETECT = " << boost::format(fmt) % "Standard";;
    for(const auto &any : stations){
        of << " : " << any;
    }
    of << " " << eol;

    of << "    enddef;\n";
}

void Mode::changeIf(int idx, unsigned long staid) {
    int c = 0;
    for(auto &any: ifs_){
        if(c == idx) {
            auto it = find(any.second.begin(), any.second.end(), staid);
            if( it == any.second.end()){
                any.second.push_back(staid);
            }

        }else{
            auto it = find(any.second.begin(), any.second.end(), staid);
            if( it != any.second.end()){
                any.second.erase(it);
            }
        }
        ++c;
    }
}

void Mode::changeBbc(int idx, unsigned long staid) {
    int c = 0;
    for(auto &any: bbcs_){
        if(c == idx) {
            auto it = find(any.second.begin(), any.second.end(), staid);
            if( it == any.second.end()){
                any.second.push_back(staid);
            }

        }else{
            auto it = find(any.second.begin(), any.second.end(), staid);
            if( it != any.second.end()){
                any.second.erase(it);
            }
        }
        ++c;
    }
}

void Mode::changeTracks(int idx, unsigned long staid) {
    int c = 0;
    for(auto &any: tracks_){
        if(c == idx) {
            auto it = find(any.second.begin(), any.second.end(), staid);
            if( it == any.second.end()){
                any.second.push_back(staid);
            }

        }else{
            auto it = find(any.second.begin(), any.second.end(), staid);
            if( it != any.second.end()){
                any.second.erase(it);
            }
        }
        ++c;
    }
}

void Mode::changeTrackFrameFormat(int idx, unsigned long staid) {
    int c = 0;
    for(auto &any: track_frame_formats_){
        if(c == idx) {
            auto it = find(any.second.begin(), any.second.end(), staid);
            if( it == any.second.end()){
                any.second.push_back(staid);
            }

        }else{
            auto it = find(any.second.begin(), any.second.end(), staid);
            if( it != any.second.end()){
                any.second.erase(it);
            }
        }
        ++c;
    }
}

void Mode::changeFreq(int idx, unsigned long staid) {
    int c = 0;
    for(auto &any: freqs_){
        if(c == idx) {
            auto it = find(any.second.begin(), any.second.end(), staid);
            if( it == any.second.end()){
                any.second.push_back(staid);
            }

        }else{
            auto it = find(any.second.begin(), any.second.end(), staid);
            if( it != any.second.end()){
                any.second.erase(it);
            }
        }
        ++c;
    }
}


