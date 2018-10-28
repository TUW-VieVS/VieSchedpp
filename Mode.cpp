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

            auto bits = tracks1.get()->numberOfBits(tracks2.get());
            auto overlappingFrequencies = freq1.get()->observingRate(freq2.get(), bits);

            staids2recordingRate_[{staid1, staid2}] = overlappingFrequencies;
        }
    }
}

boost::optional<std::shared_ptr<const If>> Mode::getIf(unsigned long staid) const {
    for(const auto &any: ifs_){
        if(find(any.second.begin(), any.second.end(), staid) != any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}

boost::optional<const std::shared_ptr<const If> &> Mode::getBbc(unsigned long staid) const {
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

    of << "observing mode: " << getName() << "\n";

    for(const auto &band : bands_){

        double meanFrequency = 0;
        auto it = band2meanWavelength_.find(band);
        if(it == band2meanWavelength_.end()){
            continue;
        }else{
            meanFrequency = util::wavelength2frequency(it->second) * 1e-6;
        }

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
            string title = (boost::format("band: %2s (%8.2f [MHz]) recording rate: %7.2f [MHz/s]") % band % meanFrequency % (any.first *1e-6)).str();
            util::outputObjectList(title, any.second, of);
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

boost::optional<const std::vector<unsigned long> &> Mode::getAllStationsWithIf(const std::shared_ptr<const If> &this_if) const {
    for(const auto &any : ifs_) {
        if (any.first == this_if) {
            return any.second;
        }
    }
    return boost::optional<const vector<unsigned long> &>();
}

boost::optional<const std::vector<unsigned long> &> Mode::getAllStationsWithBbc(const std::shared_ptr<const Bbc> &bbc) const {
    for(const auto &any : bbcs_) {
        if (any.first == bbc) {
            return any.second;
        }
    }
    return boost::optional<const vector<unsigned long> &>();
}

boost::optional<const std::vector<unsigned long> &> Mode::getAllStationsWithFreq(const std::shared_ptr<const Freq> &freq) const {
    for(const auto &any : freqs_) {
        if (any.first == freq) {
            return any.second;
        }
    }
    return boost::optional<const vector<unsigned long> &>();
}

boost::optional<const std::vector<unsigned long> &> Mode::getAllStationsWithTrack(const std::shared_ptr<const Track> &track) const {
    for(const auto &any : tracks_) {
        if (any.first == track) {
            return any.second;
        }
    }
    return boost::optional<const vector<unsigned long> &>();
}

boost::optional<const std::vector<unsigned long> &> Mode::getAllStationsWithTrackFrameFormat(const std::shared_ptr<const std::string> &trackFrameFormat) const {
    for(const auto &any : track_frame_formats_) {
        if (any.first == trackFrameFormat) {
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
        if(any.first->length() > longestName){
            longestName = any.first->length();
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
        of << "        ref $TRACKS =           " << boost::format(fmt) % any.first.get();
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


void Mode::calcMeanWavelength() {

    for(const auto &band : bands_){
        vector<double> frequencies;
        for(const auto &freq : freqs_){
            auto tmp = freq.first->getFrequencies(band);
            frequencies.insert(frequencies.end(), tmp.begin(), tmp.end());
        }

        double meanFrequency = std::accumulate(frequencies.begin(),frequencies.end(),0.0)/frequencies.size();
        double meanWavelength = util::freqency2wavelenth(meanFrequency*1e6);
        band2meanWavelength_[band] = meanWavelength;
    }
}
