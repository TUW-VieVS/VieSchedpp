//
// Created by matth on 17.09.2017.
//

#include "skdcatalogreader.h"

using namespace VieVS;
using namespace std;

//TODO only save catalogs of required stations!
std::map<std::string, std::vector<std::string>>
SkdCatalogReader::readCatalog(SkdCatalogReader::CATALOG type) noexcept {

    map<string, vector<string>> all;
    int indexOfKey;
    string filepath;

    // switch between four available catalogs
    switch (type) {
        case CATALOG::antenna: {
            filepath = antennaPath_;
            indexOfKey = 1;
            break;
        }
        case CATALOG::position: {
            filepath = positionPath_;
            indexOfKey = 0;
            break;
        }
        case CATALOG::equip: {
            filepath = equipPath_;
            indexOfKey = 1;
            break;
        }
        case CATALOG::mask: {
            filepath = maskPath_;
            indexOfKey = 2;
            break;
        }
        case CATALOG::source: {
            filepath = sourcePath_;
            indexOfKey = 0;
            break;
        }
        case CATALOG::flux: {
            filepath = fluxPath_;
            indexOfKey = 0;
            break;
        }
    }

    // read in CATALOG. antenna, position and equip use the same routine
    switch (type) {
        case CATALOG::antenna:
        case CATALOG::position:
        case CATALOG::equip:
        case CATALOG::source: {

            // open file
            ifstream fid(filepath);
            if (!fid.is_open()) {
                cerr << "    Unable to open " << filepath << " file!\n";
            } else {
                string line;

                // loop through file
                while (getline(fid, line)) {
                    if (line.length() > 0 && line.at(0) != '*') {

                        // trim leading and trailing blanks
                        boost::trim(line);

                        // split vector
                        vector<string> splitVector;
                        boost::split(splitVector, line, boost::is_space(), boost::token_compress_on);

                        // get key and convert it to upper case for case insensitivity
                        string key = boost::algorithm::to_upper_copy(splitVector[indexOfKey]);

                        // add station name to key if you look at equip.cat because id alone is not unique in catalogs
                        if (type == CATALOG::equip) {
                            key = boost::algorithm::to_upper_copy(key + "|" + splitVector[indexOfKey - 1]);
                        }

                        // look if a key already exists, if not add it.
                        if (all.find(key) != all.end()) {
                            cerr << "    Duplicated element of '" << key << "' in " << filepath << "\n";
                        }
                        all.insert(pair<string, vector<string>>(key, splitVector));
                    }
                }
            }
            // close file
            fid.close();
            break;
        }

        case CATALOG::mask: {

            // open file
            ifstream fid(filepath);
            if (!fid.is_open()) {
                cerr << "    Unable to open " << filepath << " file!\n";
            } else {
                string line;
                vector<string> splitVector_total;

                // loop through CATALOG
                while (getline(fid, line)) {
                    if (line.length() > 0 && line.at(0) != '*') {

                        // trim leading and trailing blanks
                        boost::trim(line);

                        // if first element is not an '-' this line belongs to new station mask
                        if (line.at(0) != '-' && !splitVector_total.empty()) {

                            // get key and convert it to upper case for case insensitivity
                            string key = boost::algorithm::to_upper_copy(splitVector_total[indexOfKey]);

                            // previous mask is finished, add it to map
                            all.insert(pair<string, vector<string>>(key, splitVector_total));
                            splitVector_total.clear();
                        }

                        // split vector
                        vector<string> splitVector;
                        boost::split(splitVector, line, boost::is_space(), boost::token_compress_on);

                        // if it is a new mask add all elements to vector, if not start at the 2nd element (ignore '-')
                        if (splitVector_total.empty()) {
                            splitVector_total.insert(splitVector_total.end(), splitVector.begin(), splitVector.end());
                        } else {
                            splitVector_total.insert(splitVector_total.end(), splitVector.begin() + 1,
                                                     splitVector.end());
                        }
                    }
                }
            }
            break;
        }

        case CATALOG::flux: {
            // open file
            ifstream fid(filepath);
            if (!fid.is_open()) {
                cerr << "    Unable to open " << filepath << " file!\n";
            } else {
                string line;
                vector<string> lines;
                vector<string> splitVector_total;
                string station;

                // get first entry
                while (true) {
                    getline(fid, line);
                    boost::trim(line);
                    if (line.length() > 0 && line.at(0) != '*') {
                        boost::split(splitVector_total, line, boost::is_space(), boost::token_compress_on);
                        station = splitVector_total[indexOfKey];
                        lines.push_back(line);
                        break;
                    }
                }

                // loop through CATALOG
                while (getline(fid, line)) {
                    // trim leading and trailing blanks
                    boost::trim(line);

                    if (line.length() > 0 && line.at(0) != '*') {
                        vector<string> splitVector;
                        boost::split(splitVector, line, boost::is_space(), boost::token_compress_on);
                        string newStation = splitVector[indexOfKey];

                        if (newStation == station) {
                            lines.push_back(line);
                            splitVector_total.insert(splitVector_total.end(), splitVector.begin(), splitVector.end());
                        } else {
                            all.insert(pair<string, vector<string>>(station, lines));
                            lines.clear();
                            lines.push_back(line);
                            station = newStation;
                            splitVector_total = splitVector;
                        }

                    }
                }
                all.insert(pair<string, vector<string>>(station, splitVector_total));
            }
            break;
        }
    }

    return all;
}

void SkdCatalogReader::setCatalogFilePathes(const boost::property_tree::ptree &ptreeWithPathes) {
    sourcePath_ = ptreeWithPathes.get<string>("source");
    fluxPath_ = ptreeWithPathes.get<string>("flux");

    antennaPath_ = ptreeWithPathes.get<string>("antenna");
    positionPath_ = ptreeWithPathes.get<string>("position");
    equipPath_ = ptreeWithPathes.get<string>("equip");
    maskPath_ = ptreeWithPathes.get<string>("mask");

    modesPath_ = ptreeWithPathes.get<string>("modes");
    recPath_ = ptreeWithPathes.get<string>("rec");
    tracksPath_ = ptreeWithPathes.get<string>("tracks");
    freqPath_ = ptreeWithPathes.get<string>("freq");
    rxPath_ = ptreeWithPathes.get<string>("rx");
    loifPath_ = ptreeWithPathes.get<string>("loif");
}


void SkdCatalogReader::setCatalogFilePathes(const std::string &antenna, const std::string &equip, const std::string &flux,
                          const std::string &freq, const std::string &hdpos, const std::string &loif,
                          const std::string &mask, const std::string &modes, const std::string &position,
                          const std::string &rec, const std::string &rx, const std::string &source,
                          const std::string &tracks){

    sourcePath_ = source;
    fluxPath_ = flux;

    antennaPath_ = antenna;
    positionPath_ = position;
    equipPath_ = equip;
    maskPath_ = mask;

    modesPath_ = modes;
    recPath_ = rec;
    tracksPath_ = tracks;
    freqPath_ = freq;
    rxPath_ = rx;
    loifPath_ = loif;

}


void SkdCatalogReader::initializeSourceCatalogs() {
    sourceCatalog_ = readCatalog(CATALOG::source);
    fluxCatalog_ = readCatalog(CATALOG::flux);
}

void SkdCatalogReader::initializeStationCatalogs() {
    antennaCatalog_ = readCatalog(CATALOG::antenna);
    positionCatalog_ = readCatalog(CATALOG::position);
    equipCatalog_ = readCatalog(CATALOG::equip);
    maskCatalog_ = readCatalog(CATALOG::mask);

    saveOneLetterCode();
    saveTwoLetterCode();
}

void SkdCatalogReader::initializeModesCatalogs(const string &obsModeName) {
    readModesCatalog(obsModeName);
    readRecCatalog();
    readTracksCatalog();
    readFreqCatalog();
    readRxCatalog();
    readLoifCatalog();
}

void SkdCatalogReader::readModesCatalog(const string &obsModeName) {
    ifstream fmodes(modesPath_);
    string line;
    while (getline(fmodes, line)) {
        if (line.length() > 0 && (line.at(0) != '*' && line.at(0) != '&' && line.at(0) != '!')) {
            boost::trim(line);
            vector<string> splitVector;
            boost::split(splitVector, line, boost::is_space(), boost::token_compress_on);

            if (splitVector[0] == obsModeName) {
                freqName_ = splitVector[1];
                bandWidth_ = boost::lexical_cast<double>(splitVector[2]);
                sampleRate_ = boost::lexical_cast<double>(splitVector[3]);
                recName_ = splitVector[4];
                break;
            }

        }
    }
    fmodes.close();
}

void SkdCatalogReader::readRecCatalog() {
    ifstream frec(recPath_);
    string line;
    while (getline(frec, line)) {
        try{
            if (line.length() > 0 && (line.at(0) != '*' && line.at(0) != '&' && line.at(0) != '!')) {
                boost::trim(line);
                vector<string> splitVector;
                boost::split(splitVector, line, boost::is_space(), boost::token_compress_on);

                if (splitVector[0] == recName_) {
                    while (getline(frec, line)) {
                        if (line.length() > 0 && (line.at(0) != '*' && line.at(0) != '&' && line.at(0) != '!')) {
                            boost::trim(line);
                            if (line[0] != '-') {
                                break;
                            }

                            vector<string> splitVector2;
                            boost::split(splitVector2, line, boost::is_space(), boost::token_compress_on);

                            string thisStaName = splitVector2[1];
                            if (find(staNames_.begin(), staNames_.end(), thisStaName) != staNames_.end()) {
                                staName2hdposMap_[thisStaName] = splitVector2[2];
                                staName2tracksMap_[thisStaName] = splitVector2[3];
                                if (find(tracksIds_.begin(), tracksIds_.end(), splitVector2[3]) == tracksIds_.end()) {
                                    tracksIds_.push_back(splitVector2[3]);
                                }

                                staName2recFormatMap_[thisStaName] = splitVector2[4];
                                if (splitVector2.size() > 5) {
                                    cout << "WARNING: barrel_roll and max_bw information ignored for station "
                                         << thisStaName << " in rec.cat";
                                }
                            }
                        }
                    }
                }
            }
        }catch(...){

        }
    }
    frec.close();
}

void SkdCatalogReader::readTracksCatalog() {
    for (const auto &tracksId:tracksIds_) {
        ifstream ftracks(tracksPath_);
        string line;
        while (getline(ftracks, line)) {
            try{
                if (line.length() > 0 && (line.at(0) != '*' && line.at(0) != '&' && line.at(0) != '!')) {
                    boost::trim(line);
                    vector<string> splitVector;
                    boost::split(splitVector, line, boost::is_space(), boost::token_compress_on);

                    if (splitVector[0] == tracksId) {
                        tracksId2fanoutMap_[tracksId] = boost::lexical_cast<int>(splitVector[1]);
                        unsigned int bits = boost::lexical_cast<unsigned int>(splitVector[2]);
                        if(bits_ == 0){
                            bits_ = bits;
                        } else if(bits_ != bits){
                            cerr << "Number of recorded bits is different for different track ids\n";
                        }
                        tracksId2bitsMap_[tracksId] = bits;

                        while (getline(ftracks, line)) {
                            if (line.length() > 0 && (line.at(0) != '*' && line.at(0) != '&' && line.at(0) != '!')) {
                                boost::trim(line);
                                if (line[0] != '-') {
                                    break;
                                }

                                vector<string> splitVector2;
                                boost::split(splitVector2, line, boost::is_space(), boost::token_compress_on);
                                int channelNumber = boost::lexical_cast<int>(splitVector2[1]);
                                channelNumber2tracksMap_[channelNumber] = splitVector2[2];
                            }
                        }
                    }
                }
            }catch(...){

            }
        }
        ftracks.close();
    }
}

void SkdCatalogReader::readFreqCatalog() {
    ifstream ffreq(freqPath_);
    string line;
    while (getline(ffreq, line)) {
        try{
            if (line.length() > 0 && (line.at(0) != '*' && line.at(0) != '&' && line.at(0) != '!')) {
                boost::trim(line);
                vector<string> splitVector;
                boost::split(splitVector, line, boost::is_space(), boost::token_compress_on);

                if (splitVector[0] == freqName_) {
                    freqTwoLetterCode_ = splitVector[1];
                    rxName_ = splitVector[3];

                    while (getline(ffreq, line)) {
                        if (line.length() > 0 && (line.at(0) != '*' && line.at(0) != '&' && line.at(0) != '!')) {
                            boost::trim(line);
                            if (line[0] != '-') {
                                break;
                            }

                            vector<string> splitVector2;
                            boost::split(splitVector2, line, boost::is_space(), boost::token_compress_on);

                            string channelNumberStr = splitVector2[5];
                            int channelNumber = boost::lexical_cast<int>(
                                    channelNumberStr.substr(2, channelNumberStr.size() - 2));

                            channelNumber2band_[channelNumber] = splitVector2[1];
                            channelNumber2polarization_[channelNumber] = splitVector2[2];
                            channelNumber2skyFreq_[channelNumber] = splitVector2[3];
                            channelNumber2sideBand_[channelNumber] = splitVector2[4];
                            channelNumber2BBC_[channelNumber] = splitVector2[6];
                            channelNumber2phaseCalFrequency_[channelNumber] = splitVector2[7];
                        }
                    }
                }
            }
        }catch(...){

        }
    }
    ffreq.close();
}

void SkdCatalogReader::readRxCatalog() {
    ifstream frx(rxPath_);
    string line;
    while (getline(frx, line)) {
        try{
            if (line.length() > 0 && (line.at(0) != '*' && line.at(0) != '&' && line.at(0) != '!')) {
                boost::trim(line);
                vector<string> splitVector;
                boost::split(splitVector, line, boost::is_space(), boost::token_compress_on);

                if (splitVector[0] == rxName_) {
                    while (getline(frx, line)) {
                        if (line.length() > 0 && (line.at(0) != '*' && line.at(0) != '&' && line.at(0) != '!')) {
                            boost::trim(line);
                            if (line[0] != '-') {
                                break;
                            }

                            vector<string> splitVector2;
                            boost::split(splitVector2, line, boost::is_space(), boost::token_compress_on);


                            string thisStaName = splitVector2[1];
                            if (find(staNames_.begin(), staNames_.end(), thisStaName) != staNames_.end()) {

                                if (find(loifIds_.begin(), loifIds_.end(), splitVector2[2]) == loifIds_.end()) {
                                    loifIds_.push_back(splitVector2[2]);
                                }

                                staName2loifId_[thisStaName] = splitVector2[2];
                            }
                        }
                    }
                }
            }
        }catch(...){

        }
    }
    frx.close();
}

void SkdCatalogReader::readLoifCatalog() {
    string line;
    for (const auto &loifId:loifIds_) {
        try{
            ifstream floif(loifPath_);
            while (getline(floif, line)) {
                if (line.length() > 0 && (line.at(0) != '*' && line.at(0) != '&' && line.at(0) != '!')) {
                    boost::trim(line);
                    vector<string> splitVector;
                    boost::split(splitVector, line, boost::is_space(), boost::token_compress_on);

                    if (splitVector[0] == loifId) {
                        vector<string> loifInfo;

                        while (getline(floif, line)) {
                            if (line.length() > 0 && (line.at(0) != '*' && line.at(0) != '&' && line.at(0) != '!')) {
                                boost::trim(line);
                                if (line[0] != '-') {
                                    loifId2loifInfo_[loifId] = loifInfo;
                                    break;
                                }
                                loifInfo.push_back(line);
                            }
                        }
                    }
                }
            }
            floif.close();
        }catch(...){

        }
    }

}

void SkdCatalogReader::saveOneLetterCode() {

    const map<string, vector<string> > ant = antennaCatalog_;

    std::set<char> charsUsed;

    for (const auto &staName:staNames_) {
        auto tmp = ant.at(staName);
        char oneLetterCode = tmp[0][0];
        if (charsUsed.find(oneLetterCode) != charsUsed.end()) {
            cout << "One letter code " << oneLetterCode << " of station " << staName << " already used!";
            for (char l = 'A'; l <= 'Z'; ++l) {
                if (charsUsed.find(l) == charsUsed.end()) {
                    cout << "--> using \"" << l << "\" insted!\n";
                    oneLetterCode = l;
                    break;
                }
            }
        }
        charsUsed.insert(oneLetterCode);
        oneLetterCode_[staName] = oneLetterCode;
    }
}

void SkdCatalogReader::saveTwoLetterCode() {

    const map<string, vector<string> > ant = antennaCatalog_;

    map<string, string> twoLetterCodeMap;
    for (const auto &staName:staNames_) {
        auto tmp = ant.at(staName);
        string twoLetterCode = tmp[13];
        twoLetterCode_[staName] = twoLetterCode;
    }

}




