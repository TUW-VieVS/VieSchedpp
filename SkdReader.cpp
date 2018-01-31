//
// Created by mschartn on 30.01.18.
//

#include "SkdReader.h"
using namespace VieVS;
using namespace std;

SkdReader::SkdReader(const std::string &filename):filename_{filename} {

}

void SkdReader::createObjects() {

    Initializer init;
    vector<string> staNames;
    ifstream fid(filename_);
    if (!fid.is_open()) {
        cerr << "ERROR: Unable to open " << filename_ << " file!;\n";
        return;
    } else {
        vector <string> splitVector;
        string line;
        bool found = false;
        // loop through file
        int counter = 0;
        while (getline(fid, line)) {
            if (line.empty() || line.at(0) == '*') {
                continue;
            }
            string trimmed = boost::trim_copy(line);

            if(counter == 6){
                boost::split(splitVector, trimmed, boost::is_space(), boost::token_compress_on);
                if(splitVector[4] == "START"){
                    TimeSystem::startTime = TimeSystem::string_doy2ptime(splitVector[5]);
                }else{
                    cerr << "ERROR: Start time not found! Instead: " << splitVector[4] << "\n";
                    return;
                }
                if(splitVector[6] == "END"){
                    TimeSystem::endTime = TimeSystem::string_doy2ptime(splitVector[7]);
                }else{
                    cerr << "ERROR: End time not found! Instead: " << splitVector[6] << "\n";
                    return;
                }
            }


            if(trimmed == "$STATIONS") {
                while (getline(fid, line)){
                    if (line.empty() || line.at(0) == '*') {
                        continue;
                    }
                    string trimmed2 = boost::trim_copy(line);

                    boost::split(splitVector, trimmed2, boost::is_space(), boost::token_compress_on);
                    if (splitVector[0] == "A") {
                        staNames.push_back(splitVector[2]);
                    } else {
                        found = true;
                        break;
                    }
                    splitVector.clear();
                }
            }
            ++counter;
            if(found){
                break;
            }
        }
    }
    fid.close();

    TimeSystem::mjdStart = TimeSystem::startTime.date().modjulian_day();

    boost::posix_time::time_duration a = TimeSystem::endTime - TimeSystem::startTime;
    int sec = a.total_seconds();
    if (sec < 0) {
        cerr << "ERROR: duration is less than zero seconds!;\n";
    }
    auto duration = static_cast<unsigned int>(sec);
    TimeSystem::duration = duration;


    skd_.setStationNames(staNames);
    skd_.setCatalogFilePathes(filename_,filename_,filename_,"","","",filename_,"",filename_,"","",filename_,"");

    ofstream of;
    of.close();

    ObservationMode::bands.emplace_back("X");
    ObservationMode::bands.emplace_back("S");

    init.createStations(skd_, of);
    init.createSources(skd_, of);

    stations_ = init.stations_;
    sources_ = init.sources_;
    for (int i = 0; i < stations_.size(); ++i) {
        skyCoverages_.emplace_back(vector<int>{i},i);
    }
    unsigned long nsta = stations_.size();
    for(auto&any:stations_){
        any.preCalc(vector<double>(nsta,0),vector<double>(nsta,0),vector<double>(nsta,0),vector<double>(nsta,0));
    }

    init.initializeNutation();
    init.initializeEarth();

}

void SkdReader::createScans() {

    ifstream fid(filename_);
    if (!fid.is_open()) {
        cerr << "ERROR: Unable to open " << filename_ << " file!;\n";
        return;
    } else {
        vector<string> splitVector;
        string line;
        // read until you reach "$SKED"
        while (getline(fid, line)) {
            if (line.empty() || line.at(0) == '*') {
                continue;
            }
            string trimmed = boost::trim_copy(line);
            if(line == "$SKED"){
                break;
            }
        }

        // Read SKED block
        while (getline(fid, line)) {
            if (line.empty() || line.at(0) == '*') {
                continue;
            }
            string trimmed = boost::trim_copy(line);
            if(trimmed[0] == '$'){
                break;
            }

            boost::split(splitVector, trimmed, boost::is_space(), boost::token_compress_on);
            const string &srcName = splitVector[0];
            int srcid = -1;
            for(int i=0; i<sources_.size(); ++i){
                if(sources_[i].getName() == srcName || sources_[i].getAlternativeName() == srcName){
                    srcid = i;
                    break;
                }
            }
            auto preob = boost::lexical_cast<unsigned int>(splitVector[1]);
            auto scanStart = TimeSystem::string_doy2ptime(splitVector[4]);
            auto duration = boost::lexical_cast<unsigned int>(splitVector[5]);
            const string &flags = splitVector[9];
            unsigned long nsta = flags.length() / 2;
            vector<char> oneLetterCode;
            for(int i=0; i<flags.length(); i+=2){
                oneLetterCode.push_back(flags[i]);
            }
            vector<char> cableWrapFlags;
            for(int i=1; i<flags.length(); i+=2){
                cableWrapFlags.push_back(flags[i]);
            }
            vector<unsigned int> durations;
            for(size_t i=11+nsta; i<splitVector.size(); ++i){
                durations.push_back(boost::lexical_cast<unsigned int>(splitVector[i]));
            }

            boost::posix_time::time_duration diff = scanStart-TimeSystem::startTime;
            int sec = diff.total_seconds();
            if (sec < 0) {
                cerr << "ERROR: duration is less than zero seconds!;\n";
            }
            auto time = static_cast<unsigned int>(sec);

            // calc pointingVectors
            vector<PointingVector> pv;
            for(int i=0; i<nsta; ++i){
                char olc = oneLetterCode[i];
                char cwflag = cableWrapFlags[i];

                string staName;
                for(const auto&any: skd_.getOneLetterCode()){
                    char thisOlc = any.second;
                    if(olc == thisOlc){
                        staName = any.first;
                        break;
                    }
                }
                int staid = -1;
                for (int j = 0; j < stations_.size(); ++j) {
                    if(stations_[j].getName() == staName){
                        staid = j;
                        break;
                    }
                }

                PointingVector p(staid,srcid);

                p.setTime(time);
                const auto &thisSource = sources_[srcid];
                const auto &thisSta = stations_[staid];
                thisSta.calcAzEl(thisSource,p);
                thisSta.getCableWrap().unwrapAzInSection(p,cwflag);
                pv.push_back(p);
            }

            vector<unsigned int>eols(nsta,0);

            Scan scan(pv,eols,Scan::ScanType::single);
            scan.setScanTimes(time,durations);

            scans_.push_back(move(scan));
        }
    }
}
