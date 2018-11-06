//
// Created by matth on 27.10.2018.
//

#include "ObservingMode.h"

using namespace VieVS;
using namespace std;

unsigned long VieVS::ObservingMode::nextId = 0;

bool VieVS::ObservingMode::simple = false;

std::unordered_map<std::string, double> VieVS::ObservingMode::minSNR;                             ///< backup min SNR

std::unordered_map<std::string, VieVS::ObservingMode::Property> VieVS::ObservingMode::stationProperty; ///< is band required or optional for station
std::unordered_map<std::string, VieVS::ObservingMode::Backup> VieVS::ObservingMode::stationBackup;     ///< backup version for station
std::unordered_map<std::string, double> VieVS::ObservingMode::stationBackupValue;                 ///< backup value for station

std::unordered_map<std::string, VieVS::ObservingMode::Property> VieVS::ObservingMode::sourceProperty;  ///< is band required or optional for source
std::unordered_map<std::string, VieVS::ObservingMode::Backup> VieVS::ObservingMode::sourceBackup;      ///< backup version for source
std::unordered_map<std::string, double> VieVS::ObservingMode::sourceBackupValue;                  ///< backup value for source

ObservingMode::ObservingMode(): VieVS_Object(nextId++) {

}


void ObservingMode::readFromSkedCatalogs(const SkdCatalogReader &skd) {

    auto mode = make_shared<Mode>(skd.getModeName(), skd.getStaNames().size());

    const auto &channelNr2Bbc = readSkdTracks(mode, skd);
    readSkdFreq(mode, skd, channelNr2Bbc);
    readSkdIf(mode, skd);
    readSkdBbc(mode, skd);
    readSkdTrackFrameFormat(mode, skd);
    mode->calcRecordingRates();

    addMode(mode);
    calcMeanFrequencies();
}


void ObservingMode::simpleMode( unsigned long nsta, double samplerate, unsigned int bits,
        const std::unordered_map<std::string, unsigned int> &band2channel,
        const std::unordered_map<std::string, double> &band2wavelength) {

    auto mode = make_shared<Mode>("simple", nsta);

    for(const auto &any : band2channel){
        bands_.insert(any.first);
        double recRate = samplerate * bits * any.second * 1e6;
        mode->setRecordingRates(any.first, recRate);
    }

    mode->setBands(bands_);
    addMode(mode);
    wavelength_ = band2wavelength;
}

void ObservingMode::readSkdFreq(const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd, const std::map<int,int> &channelNr2Bbc) {

    const auto &staNames = skd.getStaNames();

    const auto &freqName = skd.getFreqName();
    vector<unsigned long> freqIds = vector<unsigned long>(staNames.size());
    std::iota(freqIds.begin(),freqIds.end(), 0);
    auto thisFreq = make_shared<Freq>(freqName);

    thisFreq->setSampleRate(skd.getSampleRate());
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
        thisFreq->addChannel(channelNumber2band.at(bbcNr), skyFreq, sideband,  bandwidth, chStr, bbcStr, "&U_cal");
        lastBbcNr = bbcNr;
    }


    // add freq to mode
    addFreq(thisFreq);
    mode->addFreq(thisFreq, freqIds);
}

std::map<int,int> ObservingMode::readSkdTracks(const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd) {
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
        auto track = make_shared<Track>(tracksId);
        track->setBits(tracksId2bits.at(tracksId));

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
                        track->addFanout("A", chStr, Track::Bitstream::sign, 1, nr+3);
                        channelNr2Bbc[chn] = bbcNr;
                        ++ chn;
                    }
                }else if(splitVector.size()>2){
                    // 2 bit
                    for(int i=0; i<splitVector.size()/2; ++i){
                        string chStr = (boost::format("&CH%02d") % chn).str();
                        auto nr = boost::lexical_cast<int>(splitVector.at(i));
                        track->addFanout("A", chStr, Track::Bitstream::sign, 1, nr+3);
                        nr = boost::lexical_cast<int>(splitVector.at(i+2));
                        track->addFanout("A", chStr, Track::Bitstream::mag, 1, nr+3);
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
                        track->addFanout("A", chStr, Track::Bitstream::sign, 1, nr+3, nr+5);
                        channelNr2Bbc[chn] = bbcNr;
                        ++ chn;
                    }
                }else if(splitVector.size()>2){
                    // 2 bits
                    for(int i=0; i<splitVector.size()/2; ++i){
                        string chStr = (boost::format("&CH%02d") % chn).str();
                        auto nr = boost::lexical_cast<int>(splitVector.at(i));
                        track->addFanout("A", chStr, Track::Bitstream::sign, 1, nr+3, nr+5);
                        nr = boost::lexical_cast<int>(splitVector.at(i+2));
                        track->addFanout("A", chStr, Track::Bitstream::mag, 1, nr+3, nr+5);
                        channelNr2Bbc[chn] = bbcNr;
                        ++ chn;
                    }
                }
                bbcNr++;
            }
        }

        // add track to mode
        addTrack(track);
        mode->addTrack(track, ids);
    }

    return channelNr2Bbc;
}

void ObservingMode::readSkdIf(const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd) {

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
        auto thisIf = make_shared<If>(loifId);

        vector<string> alreadyDefined;
        for (const auto &anyIf : ifs) {
            vector<string> splitVector;
            boost::split(splitVector, anyIf, boost::is_space(), boost::token_compress_on);
            if (lastId == splitVector.at(2)) {
                continue;
            }
            string if_id_link = "&IF_" + splitVector.at(2);
            if(find(alreadyDefined.begin(), alreadyDefined.end(), if_id_link) != alreadyDefined.end()){
                continue;
            }else{
                alreadyDefined.push_back(if_id_link);
            }
            const string &physicalName = splitVector.at(2);
            If::Polarization polarization = If::Polarization::R;
            auto total_IO = boost::lexical_cast<double>(splitVector.at(4));
            If::Net_sidband net_sidband = If::Net_sidband::U;
            if(splitVector.at(5) == "D"){
                net_sidband = If::Net_sidband::D;
            }else if(splitVector.at(5) == "L"){
                net_sidband = If::Net_sidband::L;
            }

            thisIf->addIf(if_id_link, physicalName, polarization, total_IO, net_sidband, 1, 0);

            lastId = if_id_link;
        }

        // add IF to Mode
        addIf(thisIf);
        mode->addIf(thisIf, ids);
    }
}

void ObservingMode::readSkdBbc(const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd) {

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
        auto bbc = make_shared<Bbc>(bbcId);

        for(int i=0; i<ifs.size(); ++i) {
            const string & anyIf = ifs.at(i);
            vector<string> splitVector;
            boost::split(splitVector, anyIf, boost::is_space(), boost::token_compress_on);
            string bbc_assign_id = (boost::format("&BBC%02d") %(i+1)).str();
            string if_id_link = "&IF_" + splitVector.at(2);

            bbc->addBbc(bbc_assign_id, static_cast<unsigned int>(i + 1), if_id_link);
        }

        // add BBC to Mode
        addBbc(bbc);
        mode->addBbc(bbc, ids);
    }
}

void ObservingMode::readSkdTrackFrameFormat(const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd) {

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

        addTrackFrameFormat(thisRecorder);
        mode->addTrackFrameFormat(make_shared<std::string>(thisRecorder), ids);
    }

}

void ObservingMode::toVexModeBlock(std::ofstream &of) const {
    for(const auto &any : modes_){
        any->toVexModeDefiniton(of, stationNames_);
    }
}


void ObservingMode::toVexFreqBlock(std::ofstream &of) const {

    for(const auto &any : freqs_){
        string c = "* ";
        for (const auto &mode : modes_) {
            c.append(mode.get()->getName());
            const auto &o_all = mode->getAllStationsWithFreq(any);
            if(o_all.is_initialized()){
                for(auto i : *o_all){
                    c.append(" : ").append(stationNames_.at(i));
                }
            }
            c.append("; ");
        }

        any->toVexFreqDefinition(of, c);
    }
}

void ObservingMode::toVexBbcBlock(std::ofstream &of) const {

    for(const auto &any : bbcs_){
        string c = "* ";
        for (const auto &mode : modes_) {
            c.append(mode.get()->getName());
            const auto &o_all = mode->getAllStationsWithBbc(any);
            if(o_all.is_initialized()){
                for(auto i : *o_all){
                    c.append(" : ").append(stationNames_.at(i));
                }
            }
            c.append("; ");
        }

        any->toVexBbcDefinition(of, c);
    }
}

void ObservingMode::toVexIfBlock(std::ofstream &of) const {

    for(const auto &any : ifs_){
        string c = "* ";
        for (const auto &mode : modes_) {
            c.append(mode.get()->getName());
            const auto &o_all = mode->getAllStationsWithIf(any);
            if(o_all.is_initialized()){
                for(auto i : *o_all){
                    c.append(" : ").append(stationNames_.at(i));
                }
            }
            c.append("; ");
        }

        any->toVecIfDefinition(of, c);
        of << "*\n";
    }
}

void ObservingMode::toVexTracksBlock(std::ofstream &of) const {

    for(const auto &any : tracks_){
        string c = "* ";
        for (const auto &mode : modes_) {
            c.append(mode.get()->getName());
            const auto &o_all = mode->getAllStationsWithTrack(any);
            if(o_all.is_initialized()){
                for(auto i : *o_all){
                    c.append(" : ").append(stationNames_.at(i));
                }
            }
            c.append("; ");
        }
        any->toVexTracksDefinition(of, c);
    }
    toTrackFrameFormatDefinitions( of );
}

void ObservingMode::toTrackFrameFormatDefinitions(std::ofstream &of) const {
    for(const auto &any : trackFrameFormats_){
        string c = "* ";
        for (const auto &mode : modes_) {
            c.append(mode.get()->getName());
            const auto &o_all = mode->getAllStationsWithTrackFrameFormat(any);
            if(o_all.is_initialized()){
                for(auto i : *o_all){
                    c.append(" : ").append(stationNames_.at(i));
                }
            }
            c.append("; ");
        }

        of << "    def " << any << "_format;    " << c << "\n";
        of << "        track_frame_format = " << any << ";\n";
        of << "    enddef;\n";
    }
}

void ObservingMode::summary(std::ofstream &of) const {

    of << "Summary of observing mode(s):\n";
    for(const auto &band : bands_){
        double meanFrequency = 0;
        auto it = wavelength_.find(band);
        if(it == wavelength_.end()){
            continue;
        }else{
            meanFrequency = util::wavelength2frequency(it->second) * 1e-6;
        }
        of << boost::format("    band %2s mean frequency %9.3f [MHz]\n") % band %meanFrequency;
    }
    of << "\n";

    for (const auto &any : modes_) {
        any->summary(of, stationNames_);
    }
}

void ObservingMode::calcMeanFrequencies() {

    for(const auto &band : bands_){
        vector<double> frequencies;
        for(const auto &freq : freqs_){
            auto tmp = freq->getFrequencies(band);
            frequencies.insert(frequencies.end(), tmp.begin(), tmp.end());
        }

        double meanFrequency = std::accumulate(frequencies.begin(),frequencies.end(),0.0)/frequencies.size();
        double meanWavelength = util::freqency2wavelenth(meanFrequency*1e6);
        wavelength_[band] = meanWavelength;
    }

}

void ObservingMode::addDummyBands(const std::map<std::string, std::vector<double>> &band) {
    for(const auto &any : band){
        bands_.insert(any.first);
        double mfreq = accumulate(any.second.begin(), any.second.end(), 0.0) / any.second.size();
        wavelength_[any.first] = mfreq;
    }
}

