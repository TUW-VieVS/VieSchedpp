//
// Created by mschartn on 30.01.18.
//

#include "SkdParser.h"
using namespace VieVS;
using namespace std;

unsigned long SkdParser::nextId = 0;

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
        bool corSynchFound = false;
        bool calibrationFound = false;
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
            if(!corSynchFound && trimmed.find("CORSYNCH") != trimmed.npos){
                boost::split(splitVector, trimmed, boost::is_space(), boost::token_compress_on);
                for(int i=0;i<splitVector.size();++i){
                    if(splitVector[i] == "CORSYNCH"){
                        midob_ = boost::lexical_cast<unsigned int>(splitVector[i+1]);
                    }
                }
                corSynchFound = true;
            }
            if(!calibrationFound && trimmed.find("CALIBRATION") != trimmed.npos){
                boost::split(splitVector, trimmed, boost::is_space(), boost::token_compress_on);
                for(int i=0;i<splitVector.size();++i){
                    if(splitVector[i] == "CALIBRATION"){
                        preob_ = boost::lexical_cast<unsigned int>(splitVector[i+1]);
                    }
                }
                calibrationFound = true;
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

    int sec = util::duration(TimeSystem::startTime,TimeSystem::endTime);
    if (sec < 0) {
        cerr << "ERROR: duration is less than zero seconds!;\n";
    }
    auto duration = static_cast<unsigned int>(sec);
    TimeSystem::duration = duration;


    skd_.setStationNames(staNames);
    skd_.setCatalogFilePathes(filename_);
    skd_.initializeStationCatalogs();
    skd_.initializeSourceCatalogs();

    ofstream of;
    of.close();

    ObservationMode::bands.emplace_back("X");
    ObservationMode::bands.emplace_back("S");

    init.createStations(skd_, of);
    init.createSources(skd_, of);

    network_ = init.network_;
    sources_ = move(init.sources_);

    init.initializeAstronomicalParameteres();
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
        vector<unsigned int>eols(network_.getNSta(),0); // end of last scan
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
            unsigned long srcid = numeric_limits<unsigned long>::max();
            for(unsigned long isrc=0; isrc<sources_.size(); ++isrc){
                if(sources_[isrc].hasName(srcName)){
                    srcid = isrc;
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

            int sec = util::duration(TimeSystem::startTime,scanStart);
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
                unsigned long staid = network_.getStation(staName).getId();

                PointingVector p(staid,srcid);

                p.setTime(time);
                const auto &thisSource = sources_[srcid];
                Station &thisSta = network_.refStation(staid);
                thisSta.calcAzEl(thisSource,p);
                bool error = thisSta.getCableWrap().unwrapAzInSection(p,cwflag);
                if(error){
                    pair<double,double>limits = thisSta.getCableWrap().getLimits(cwflag);
                    cerr << boost::format("Station %8s scan %4d source %8s time %s azimuth error! Flag: %c (from %7.2f to %7.2f) calculated: %7.2f (or %7.2f)\n")
                            %thisSta.getName()%counter%thisSource.getName()%TimeSystem::ptime2string_doy(scanStart)%cwflag%(limits.first*rad2deg)%(limits.second*rad2deg)%(p.getAz()*rad2deg)%(p.getAz()*rad2deg-360);
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
                    unsigned int thisSlewTime = thisSta.getAntenna().slewTime(thisSta.getCurrentPointingVector(), p);

                    slewTimes[i] = thisSlewTime;
//                    idleTimes[i] = (time-preob)-(thisEols[i]+fieldSystemTimes_+thisSlewTime);
                }
                thisSta.setCurrentPointingVector(p_end);
            }

            Scan scan(pv,thisEols,Scan::ScanType::standard);
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
                    if(tmp.getObservingTime(i, Timestamp::start)- tmp.getPreobDuration(i) < tmp.getSlewTime(i, Timestamp::end)-2){
                        unsigned int eost = tmp.getSlewTime(i, Timestamp::end);
                        unsigned int eoit = tmp.getObservingTime(i, Timestamp::start)- tmp.getPreobDuration(i);
                        boost::posix_time::ptime eostp = TimeSystem::internalTime2PosixTime(eost);
                        boost::posix_time::ptime eoitp = TimeSystem::internalTime2PosixTime(eoit);

                        cerr << boost::format("Station %8s scan %4d source %8s time %s idle time error! end of slew time: %s end of idle time: %s (diff -%d [s])\n")
                                %network_.getStation(scan.getPointingVector(i).getStaid()).getName()
                                %counter
                                %sources_[scan.getPointingVector(i).getSrcid()].getName()
                                %TimeSystem::ptime2string_doy(scanStart)
                                %TimeSystem::ptime2string_doy(eostp)
                                %TimeSystem::ptime2string_doy(eoitp)
                                %(eost-eoit);
                    }
                }
            }

            scan.setPointingVectorsEndtime(move(pv_end));

            scan.createDummyObservations(network_);

            scans_.push_back(scan);
            ++counter;
        }
    }
}

void SkdParser::copyScanMembersToObjects() {
    for(const auto &scan:scans_){
        unsigned long srcid = scan.getSourceId();

        for (int i = 0; i < scan.getNSta(); ++i) {
            const PointingVector &pv = scan.getPointingVector(i);
            unsigned long staid = pv.getStaid();
            const PointingVector &pv_end = scan.getPointingVector(i, Timestamp::end);
            unsigned long nObs = scan.getNObs(staid);
            network_.update(nObs, pv_end);
        }
        for (int i=0; i< scan.getNObs(); ++i){
            const Observation &obs = scan.getObservation(i);
            network_.update(obs.getBlid());
        }

        unsigned long nbl = (scan.getNSta()*(scan.getNSta()-1))/2;
        unsigned int latestTime = scan.getTimes().getObservingTime(Timestamp::start);
        Source &thisSource = sources_[srcid];
        thisSource.update(nbl, latestTime, true);
    }
}

std::vector<vector<unsigned int>> SkdParser::getScheduledTimes(const string &station) {
    vector<vector<unsigned int>> times;

    unsigned long staid = network_.getStation(station).getId();
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
                times.emplace_back(vector<unsigned int>{
                        scan.getTimes().getSlewDuration(idx),
                        scan.getTimes().getIdleDuration(idx),
                        scan.getTimes().getPreobDuration(idx),
                        scan.getTimes().getObservingDuration(idx)});
            }
        }
    }
    return times;
}

Scheduler SkdParser::createScheduler() {

    Station::WaitTimes wt;
    wt.fieldSystem = fieldSystemTimes_;
    wt.preob = preob_;
    wt.midob = midob_;
    wt.postob = postob_;
    for(Station &station:network_.refStations()){
        station.setWaitTimes(wt);
    }

    boost::property_tree::ptree xml;
    xml.add("general.startTime",TimeSystem::ptime2string(TimeSystem::startTime));
    xml.add("general.endTime",TimeSystem::ptime2string(TimeSystem::endTime));

    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    xml.add("created.time",now);

    string fname;
    std::size_t found = filename_.find_last_of("/\\");
    std::string path;
    if(found == std::string::npos){
        fname = filename_;
    }else{
        fname = filename_.substr(found+1);
    }

    xml.add("output.experimentName",fname);
    string description = "created from skd file: " + fname;

    xml.add("output.experimentDescription",description);

    return Scheduler(filename_, network_, sources_, scans_, xml);
}
