//
// Created by mschartn on 30.01.18.
//

#include "SkdParser.h"
using namespace VieVS;
using namespace std;

int SkdParser::nextId = 0;

SkdParser::SkdParser(const std::string &filename):VieVS_Object(nextId++), filename_{filename} {
}

void SkdParser::createObjects() {

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
        bool sourceFound = false;
        bool tapeFound = false;
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
                    cerr << "ERROR: Start time not found! Instead: " << splitVector[4] << ";\n";
                    return;
                }
                if(splitVector[6] == "END"){
                    TimeSystem::endTime = TimeSystem::string_doy2ptime(splitVector[7]);
                }else{
                    cerr << "ERROR: End time not found! Instead: " << splitVector[6] << ";\n";
                    return;
                }
            }
            if(!sourceFound && trimmed.find("SOURCE") != trimmed.npos){
                boost::split(splitVector, trimmed, boost::is_space(), boost::token_compress_on);
                for(int i=0;i<splitVector.size();++i){
                    if(splitVector[i] == "SOURCE"){
                        fieldSystemTimes_ += boost::lexical_cast<unsigned int>(splitVector[i+1]);
                    }
                }
                sourceFound = true;
            }
            if(!tapeFound && trimmed.find("TAPETM") != trimmed.npos){
                boost::split(splitVector, trimmed, boost::is_space(), boost::token_compress_on);
                for(int i=0;i<splitVector.size();++i){
                    if(splitVector[i] == "TAPETM"){
                        fieldSystemTimes_ += boost::lexical_cast<unsigned int>(splitVector[i+1]);
                    }
                }
                tapeFound = true;
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

    int sec_ = TimeSystem::startTime.time_of_day().total_seconds();
    TimeSystem::mjdStart = TimeSystem::startTime.date().modjulian_day() + sec_ / 86400.0;

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
        skyCoverages_.emplace_back(vector<int>{i});
        stations_[i].setSkyCoverageId(i);
    }
    unsigned long nsta = stations_.size();
    for(auto&any:stations_){
        any.preCalc(vector<double>(nsta,0),vector<double>(nsta,0),vector<double>(nsta,0),vector<double>(nsta,0));
    }

    init.initializeNutation();
    init.initializeEarth();

}

void SkdParser::createScans() {

    ifstream fid(filename_);
    if (!fid.is_open()) {
        cerr << "ERROR: Unable to open " << filename_ << " file!;\n";
        return;
    } else {

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

        bool firstScan = true;
        vector<unsigned int>eols(stations_.size(),0); // end of last scan
        // Read SKED block
        int counter = 1;
        while (getline(fid, line)) {
            if (line.empty() || line.at(0) == '*') {
                continue;
            }
            string trimmed = boost::trim_copy(line);
            if(trimmed[0] == '$'){
                break;
            }

            vector<string> splitVector;
            boost::split(splitVector, trimmed, boost::is_space(), boost::token_compress_on);
            const string &srcName = splitVector[0];
            int srcid = -1;
            for(int i=0; i<sources_.size(); ++i){
                if(sources_[i].hasName(srcName)){
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
            vector<PointingVector> pv_end;
            vector<unsigned int>thisEols(nsta,0);
            vector<unsigned int>slewTimes(nsta,0);
//            vector<unsigned int>idleTimes(nsta,0);
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
                    if(stations_[j].hasName(staName)){
                        staid = j;
                        break;
                    }
                }

                PointingVector p(staid,srcid);

                p.setTime(time);
                const auto &thisSource = sources_[srcid];
                auto &thisSta = stations_[staid];
                thisSta.calcAzEl(thisSource,p);
                bool error = thisSta.getCableWrap().unwrapAzInSection(p,cwflag);
                if(error){
                    if(thisSta.getCableWrap().getCableWrapType() == CableWrap::CableWrapType::AZEL){
                        pair<double,double>limits = thisSta.getCableWrap().getLimits(cwflag);
                        cerr << boost::format("Station %8s scan %4d source %8s time %s azimuth error! Flag: %c (from %7.2f to %7.2f) calculated: %7.2f (or %7.2f)\n")
                                %thisSta.getName()%counter%thisSource.getName()%TimeSystem::ptime2string_doy(scanStart)%cwflag%(limits.first*rad2deg)%(limits.second*rad2deg)%(p.getAz()*rad2deg)%(p.getAz()*rad2deg-360);
                    }else{
                        cerr << boost::format("Station %8s scan %4d source %8s time %s azimuth error! Non AZEL station!\n")
                                %thisSta.getName()%counter%thisSource.getName()%TimeSystem::ptime2string_doy(scanStart);
                    }
                }
                pv.push_back(p);

                PointingVector p_end(staid,srcid);
                p_end.setTime(time+durations[i]);
                thisSta.calcAzEl(thisSource,p_end);
                thisSta.getCableWrap().unwrapAzNearAz(p_end,p.getAz());
                pv_end.push_back(p_end);

                thisEols[i] = eols[staid];
                eols[staid] = p_end.getTime();

                if(!firstScan){
                    unsigned int thisSlewTime = thisSta.slewTime(p);
                    slewTimes[i] = thisSlewTime;
//                    idleTimes[i] = (time-preob)-(thisEols[i]+fieldSystemTimes_+thisSlewTime);
                }
                thisSta.setCurrentPointingVector(p_end);
            }

            Scan scan(pv,thisEols,Scan::ScanType::single);
            bool valid;
            if(firstScan){
                firstScan = false;
                valid = scan.setScanTimes(thisEols,0,slewTimes,0,time,durations);
            }else{
                valid = scan.setScanTimes(thisEols,fieldSystemTimes_,slewTimes,preob,time,durations);
            }

            if(!valid){
                const auto &tmp = scan.getTimes();
                for (int i = 0; i < nsta; ++i) {
                    if(tmp.getEndOfIdleTime(i)<tmp.getEndOfSlewTime(i)){
                        unsigned int eost = tmp.getEndOfSlewTime(i);
                        unsigned int eoit = tmp.getEndOfIdleTime(i);
                        boost::posix_time::ptime eostp = TimeSystem::toPosixTime(eost);
                        boost::posix_time::ptime eoitp = TimeSystem::toPosixTime(eoit);

                        cerr << boost::format("Station %8s scan %4d source %8s time %s idle time error! end of slew time: %s end of idle time: %s (diff -%d [s])\n")
                                %stations_[scan.getPointingVector(i).getStaid()].getName()
                                %counter
                                %sources_[scan.getPointingVector(i).getSrcid()].getName()
                                %TimeSystem::ptime2string_doy(scanStart)
                                %TimeSystem::ptime2string_doy(eostp)
                                %TimeSystem::ptime2string_doy(eoitp)
                                %(eost-eoit);
                    }
                }
            }

            scan.setPointingVectorsEndtime(pv_end);

            scans_.push_back(scan);
            ++counter;
        }
    }
}

void SkdParser::copyScanMembersToObjects() {
    for(const auto &scan:scans_){
        int srcid = scan.getSourceId();
        unsigned long nbl = (scan.getNSta()*(scan.getNSta()-1))/2;

        for (int i = 0; i < scan.getNSta(); ++i) {
            const PointingVector &pv = scan.getPointingVector(i);
            int staid = pv.getStaid();
            const PointingVector &pv_end = scan.getPointingVectors_endtime(i);
            stations_[staid].update(nbl, pv, pv_end, true);

            int skyCoverageId = stations_[staid].getSkyCoverageID();
            skyCoverages_[skyCoverageId].update(pv, pv_end);
        }

        unsigned int latestTime = scan.maxTime();
        Source &thisSource = sources_[srcid];
        thisSource.update(nbl, latestTime, true);

    }

}

std::vector<vector<unsigned int>> SkdParser::getScheduledTimes(const string &station) {
    vector<vector<unsigned int>> times;

    int staid = -1;
    for(int i=0; i<stations_.size(); ++i){
        if(stations_[i].hasName(station)){
            staid = i;
            break;
        }
    }
    if(staid == -1){
        cerr << "ERROR: get scheduled slew times: station name "<< station << "unknown!;\n";
    }else {
        for (const auto &scan:scans_) {
            int idx = -1;
            for (int i = 0; i < scan.getNSta(); ++i) {
                const auto &pv = scan.getPointingVector(i);
                if (pv.getStaid() == staid) {
                    idx = i;
                    break;
                }
            }
            if (idx != -1) {
                times.emplace_back(vector<unsigned int>{scan.getTimes().getSlewTime(idx), scan.getTimes().getIdleTime(idx), scan.getTimes().getPreobTime(idx), scan.getTimes().getScanTime(idx)});
            }
        }
    }
    return times;
}
