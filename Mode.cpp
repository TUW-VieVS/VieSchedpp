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

bool VieVS::Mode::manual = false;

std::set<std::string> VieVS::Mode::bands;                                            ///< list of bands
std::unordered_map<std::string, double> VieVS::Mode::minSNR;                         ///< backup min SNR

std::unordered_map<std::string, VieVS::Mode::Property> VieVS::Mode::stationProperty; ///< is band required or optional for station
std::unordered_map<std::string, VieVS::Mode::Backup> VieVS::Mode::stationBackup;     ///< backup version for station
std::unordered_map<std::string, double> VieVS::Mode::stationBackupValue;             ///< backup value for station

std::unordered_map<std::string, VieVS::Mode::Property> VieVS::Mode::sourceProperty;  ///< is band required or optional for source
std::unordered_map<std::string, VieVS::Mode::Backup> VieVS::Mode::sourceBackup;      ///< backup version for source
std::unordered_map<std::string, double> VieVS::Mode::sourceBackupValue;              ///< backup value for source


Mode::Mode(): VieVS_NamedObject{"empty", 0}{}

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

            auto bits = tracks1->numberOfBits(tracks2.get());
            auto overlappingFrequencies = freq1->observingRate(freq2.get(), bits);

            staids2recordingRate_[{staid1, staid2}] = overlappingFrequencies;
        }
    }


}

void Mode::addIf(const If &newIf, const std::vector<unsigned long> &staids) {
    ifs_.emplace_back(newIf, staids);
}

void Mode::addBbc(const Bbc &newBbc, const std::vector<unsigned long> &staids) {
    bbcs_.emplace_back(newBbc, staids);
}

void Mode::addFreq(const Freq &newFreq, const std::vector<unsigned long> &staids) {
    freqs_.emplace_back(newFreq, staids);
    const auto &tmp = newFreq.getBands();
    Mode::bands.insert(tmp.begin(), tmp.end());
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
        if(find(any.second.begin(), any.second.end(), staid) != any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}

boost::optional<const Bbc &> Mode::getBbc(unsigned long staid) {
    for(const auto &any: bbcs_){
        if(find(any.second.begin(), any.second.end(), staid) != any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}

boost::optional<const Freq &> Mode::getFreq(unsigned long staid) {
    for(const auto &any: freqs_){
        if(find(any.second.begin(), any.second.end(), staid) != any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}

boost::optional<const Track &> Mode::getTracks(unsigned long staid) {
    for(const auto &any: tracks_){
        if(find(any.second.begin(), any.second.end(), staid) != any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}

boost::optional<const std::string &> Mode::getTrackFrameFormat(unsigned long staid) {
    for(const auto &any: track_frame_formats_){
        if(find(any.second.begin(), any.second.end(), staid) != any.second.end()){
            return any.first;
        }
    }
    return boost::none;
}

void Mode::readFromSkedCatalogs(const SkdCatalogReader &skd) {

    const auto &channelNr2Bbc = readSkdTracks(skd);
    readSkdFreq(skd, channelNr2Bbc);
    readSkdIf(skd);
    readSkdBbc(skd);
    readSkdTrackFrameFormat(skd);

}

void Mode::readSkdFreq(const SkdCatalogReader &skd, const std::map<int,int> &channelNr2Bbc) {

    const auto &staNames = skd.getStaNames();

    const auto &freqName = skd.getFreqName();
    vector<unsigned long> freqIds = vector<unsigned long>(staNames.size());
    std::iota(freqIds.begin(),freqIds.end(), 0);
    Freq thisFreq(freqName);


    thisFreq.setSampleRate(skd.getSampleRate());
    const double bandwidth = skd.getBandWidth();

    const auto &channelNumber2band = skd.getChannelNumber2band();
    const auto &channelNumber2Bbc = skd.getChannelNumber2BBC();
    const auto &channelNumber2skyFreq = skd.getChannelNumber2skyFreq();

    int lastBbcNr = -1;
    for (const auto &any:channelNr2Bbc) {
        int chNr = any.first;
        int bbcNr = any.second;
        string chStr = (boost::format("&CH%02d") % chNr).str();
        string bbcStr = (boost::format("&BBC%02d") % bbcNr).str();
        Freq::Net_sideband sideband = bbcNr == lastBbcNr ? Freq::Net_sideband::U : Freq::Net_sideband::L;
        auto skyFreq = boost::lexical_cast<double>(channelNumber2skyFreq.at(bbcNr));
        thisFreq.addChannel(channelNumber2band.at(bbcNr), skyFreq, sideband,  bandwidth, chStr, bbcStr, "&U_cal");
        lastBbcNr = bbcNr;
    }

    // add freq to mode
    addFreq(thisFreq,freqIds);
}

std::map<int,int> Mode::readSkdTracks(const SkdCatalogReader &skd) {
    const auto &staNames = skd.getStaNames();
    std::map<int,int> channelNr2Bbc;

    // create tracks object
    const auto &staName2tracksMap       = skd.getStaName2tracksMap();
    const auto &tracksIds               = skd.getTracksIds();
    const auto &tracksId2fanout         = skd.getTracksId2fanoutMap();
    const auto &channelNumber2tracksMap = skd.getTracksId2channelNumber2tracksMap();
    const auto &tracksId2bits           = skd.getTracksId2bits();

    for(const auto &tracksId : tracksIds){
        int chn = 1;
        int bbcNr = 1;

        // get corresponding station ids
        vector<unsigned long> ids;
        for(unsigned long i=0; i<staNames.size(); ++i){
            const auto &thisStation = staNames[i];
            if(staName2tracksMap.at(thisStation) == tracksId){
                ids.push_back(i);
            }
        }
        Track track(tracksId);
        track.setBits(tracksId2bits.at(tracksId));

        if(tracksId2fanout.at(tracksId) == 1){
            // 1:1 fanout
            for(const auto &any:channelNumber2tracksMap.at(tracksId)){

                const string &t = any.second;
                string tracks = t.substr(2,t.size()-3);
                vector<string> splitVector;
                boost::split(splitVector, tracks, boost::is_any_of(","), boost::token_compress_off);
                if(splitVector.size()<=2){
                    // 1 bit
                    for(const auto &ch: splitVector){
                        string chStr = (boost::format("&CH%02d") % chn).str();
                        auto nr = boost::lexical_cast<int>(ch);
                        track.addFanout("A", chStr, Track::Bitstream::sign, 1, nr+3);
                        channelNr2Bbc[chn] = bbcNr;
                        ++ chn;
                    }
                }else if(splitVector.size()>2){
                    // 2 bit
                    for(int i=0; i<splitVector.size()/2; ++i){
                        string chStr = (boost::format("&CH%02d") % chn).str();
                        auto nr = boost::lexical_cast<int>(splitVector.at(i));
                        track.addFanout("A", chStr, Track::Bitstream::sign, 1, nr+3);
                        nr = boost::lexical_cast<int>(splitVector.at(i+2));
                        track.addFanout("A", chStr, Track::Bitstream::mag, 1, nr+3);
                        channelNr2Bbc[chn] = bbcNr;
                        ++ chn;
                    }
                }
                bbcNr++;
            }
        }else if(tracksId2fanout.at(tracksId) == 2){
            // 1:2 fanout
            for(const auto &any:channelNumber2tracksMap.at(tracksId)){
                const string &t = any.second;
                unsigned long idx1 = t.find('(');
                unsigned long idx2 = t.find(')');
                string tracks = t.substr(idx1+1,idx2-idx1-1);
                vector<string> splitVector;
                boost::split(splitVector, tracks, boost::is_any_of(","), boost::token_compress_off);
                if(splitVector.size()<=2){
                    // 1 bit
                    for(const auto &ch: splitVector){
                        string chStr = (boost::format("&CH%02d") % chn).str();
                        auto nr = boost::lexical_cast<int>(ch);
                        track.addFanout("A", chStr, Track::Bitstream::sign, 1, nr+3, nr+5);
                        channelNr2Bbc[chn] = bbcNr;
                        ++ chn;
                    }
                }else if(splitVector.size()>2){
                    // 2 bits
                    for(int i=0; i<splitVector.size()/2; ++i){
                        string chStr = (boost::format("&CH%02d") % chn).str();
                        auto nr = boost::lexical_cast<int>(splitVector.at(i));
                        track.addFanout("A", chStr, Track::Bitstream::sign, 1, nr+3, nr+5);
                        nr = boost::lexical_cast<int>(splitVector.at(i+2));
                        track.addFanout("A", chStr, Track::Bitstream::mag, 1, nr+3, nr+5);
                        channelNr2Bbc[chn] = bbcNr;
                        ++ chn;
                    }
                }
                bbcNr++;
            }
        }

        // add track to mode
        addTrack(track, ids);
    }

    return channelNr2Bbc;
}

void Mode::readSkdIf(const SkdCatalogReader &skd) {

    const auto &staNames = skd.getStaNames();
    const auto &staName2loifId = skd.getStaName2loifId();
    const auto &loifId2loifInfo = skd.getLoifId2loifInfo();

    for (const auto &any:loifId2loifInfo) {
        string loifId = any.first;
        vector<string> ifs = any.second;
        string lastId;

        // get corresponding station ids
        vector<unsigned long> ids;
        for(unsigned long i=0; i<staNames.size(); ++i){
            const auto &thisStation = staNames[i];
            if(staName2loifId.at(thisStation) == loifId){
                ids.push_back(i);
            }
        }
        If thisIf(loifId);


        for (const auto &anyIf : ifs) {
            vector<string> splitVector;
            boost::split(splitVector, anyIf, boost::is_space(), boost::token_compress_on);
            if (lastId == splitVector.at(2)) {
                continue;
            }
            string if_id_link = "&IF_" + splitVector.at(2);
            const string &physicalName = splitVector.at(2);
            If::Polarization polarization = If::Polarization::R;
            auto total_IO = boost::lexical_cast<double>(splitVector.at(4));
            If::Net_sidband net_sidband = If::Net_sidband::U;
            if(splitVector.at(5) == "D"){
                net_sidband = If::Net_sidband::D;
            }else if(splitVector.at(5) == "L"){
                net_sidband = If::Net_sidband::L;
            }

            thisIf.addIf(if_id_link, physicalName, polarization, total_IO, net_sidband, 1, 0);

            lastId = if_id_link;
        }

        // add IF to Mode
        addIf(thisIf, ids);
    }
}

void Mode::readSkdBbc(const SkdCatalogReader &skd) {

    const auto &staNames = skd.getStaNames();
    const auto &staName2loifId = skd.getStaName2loifId();
    const auto &loifId2loifInfo = skd.getLoifId2loifInfo();

    for(const auto &any:loifId2loifInfo){
        string bbcId = any.first;
        vector<string> ifs = any.second;

        // get corresponding station ids
        vector<unsigned long> ids;
        for(unsigned long i=0; i<staNames.size(); ++i){
            const auto &thisStation = staNames[i];
            if(staName2loifId.at(thisStation) == bbcId){
                ids.push_back(i);
            }
        }
        Bbc bbc(bbcId);

        for(int i=0; i<ifs.size(); ++i) {
            const string & anyIf = ifs.at(i);
            vector<string> splitVector;
            boost::split(splitVector, anyIf, boost::is_space(), boost::token_compress_on);
            string bbc_assign_id = (boost::format("&BBC%02d") %(i+1)).str();
            string if_id_link = "&IF_" + splitVector.at(2);

            bbc.addBbc(bbc_assign_id, static_cast<unsigned int>(i + 1), if_id_link);
        }

        // add BBC to Mode
        addBbc(bbc, ids);
    }
}

void Mode::readSkdTrackFrameFormat(const SkdCatalogReader &skd) {

    const auto &staNames = skd.getStaNames();

    const map<string, vector<string> > & cat = skd.getEquipCatalog();
    const map<string, vector<string> > & acat = skd.getAntennaCatalog();
    vector<string> recorders;

    for(const auto &staName: staNames) {

        const string &id_EQ = skd.equipKey(staName);

        const vector<string> &eq = cat.at(id_EQ);
        string recorder = eq.at(eq.size() - 1);
        if(recorder == "MARK5B" || recorder == "K5"){
            recorder = "Mark5B";
        }else{
            recorder = "Mark4";
        }
        recorders.push_back(recorder);
    }

    vector<string> uniqueRecorders = recorders;
    sort( uniqueRecorders.begin(), uniqueRecorders.end() );
    uniqueRecorders.erase( unique( uniqueRecorders.begin(), uniqueRecorders.end() ), uniqueRecorders.end() );

    for(const auto & thisRecorder : uniqueRecorders){

        // get corresponding station ids
        vector<unsigned long> ids;
        for(unsigned long i=0; i<recorders.size(); ++i){
            const auto &thisStation = staNames[i];
            if(thisRecorder == recorders[i]){
                ids.push_back(i);
            }
        }

        addTrackFrameFormat(thisRecorder, ids);
    }

}

void Mode::summary(const std::vector<std::string> &stations, std::ofstream &of) const{

    of << "observing mode: " << getName() << "\n";
    auto bands = getAllBands();

    for(const auto &band : bands){

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

std::set<std::string> Mode::getAllBands() const{
    set<string> bands;
    for(const auto &any:freqs_){
        bands.insert(any.first.getBands().begin(), any.first.getBands().end());
    }
    return bands;
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

void Mode::calcMeanWavelength() {
    set<string> bands = getAllBands();

    for(const auto &band : bands){
        vector<double> frequencies;
        for(const auto &freq : freqs_){
            auto tmp = freq.first.getFrequencies(band);
            frequencies.insert(frequencies.end(), tmp.begin(), tmp.end());
        }

        double meanFrequency = std::accumulate(frequencies.begin(),frequencies.end(),0.0)/frequencies.size();
        double meanWavelength = util::freqency2wavelenth(meanFrequency*1e6);
        band2meanWavelength_[band] = meanWavelength;
    }
}

