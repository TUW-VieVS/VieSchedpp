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

#include "SkdParser.h"
using namespace VieVS;
using namespace std;

unsigned long SkdParser::nextId = 0;

SkdParser::SkdParser(const std::string &filename):VieVS_Object(nextId++), filename_{filename} {

}

void SkdParser::read() {

    Initializer init;
    vector<string> staNames;
    ifstream fid(filename_);

    int bits = 0;
    double samRate = 0;
    std::unordered_map<std::string, unsigned int> band2channel{{"X",10},{"S",6}};

    if (!fid.is_open()) {
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(error) << "unable to open " << filename_;
        #else
        cout << "[error] unable to open " << filename_;
        #endif
        return;
    } else {
        vector <string> splitVector;
        string line;
        bool found = false;
        bool sourceFound = false;
        bool tapeFound = false;
        bool corSynchFound = false;
        bool calibrationFound = false;
        bool bandsFound = false;



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
                    #ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL(error) << "session start time not found";
                    #else
                    cout << "[error] session start time not found";
                    #endif
                    return;
                }
                if(splitVector[6] == "END"){
                    TimeSystem::endTime = TimeSystem::string_doy2ptime(splitVector[7]);
                }else{
                    #ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL(error) << "session end time not found";
                    #else
                    cout << "[error] session end time not found";
                    #endif
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

            if(!found && trimmed == "$STATIONS") {
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
            if(!bandsFound && trimmed == "$CODES"){
                bandsFound = true;
                bool first = true;
                while (getline(fid, line)){
                    if (line == "* no sked observind mode used! "){
                        break;
                    }
                    if (line.empty() || line.at(0) == '*') {
                        continue;
                    }
                    string trimmed2 = boost::trim_copy(line);

                    boost::split(splitVector, trimmed2, boost::is_space(), boost::token_compress_on);
                    if (splitVector[0] == "C") {
                        if(splitVector.size() >= 4){
                            string band = splitVector[2];
                            try{
                                auto freq = boost::lexical_cast<double>(splitVector.at(3));
                                if(freqs_.find(band) != freqs_.end()){
                                    std::vector<double> &f = freqs_.at(band);
                                    if(find(f.begin(), f.end(), freq) == f.end()){
                                        f.push_back(freq);
                                    }
                                }else{
                                    std::vector<double> f{freq};
                                    freqs_[band] = f;
                                }
                            }
                            catch(const std::exception& e){
                                #ifdef VIESCHEDPP_LOG
                                BOOST_LOG_TRIVIAL(warning) << "band " << band << " cannot read frequency";
                                #else
                                cout << "[error] band " << band << " cannot read frequency\n";
                                #endif
                            }

                            if(first){
                                string txt = splitVector[8];
                                int x = std::count( txt.begin(), txt.end(), ',' );
                                if(x == 3){
                                    bits = 2;
                                }else{
                                    bits = 1;
                                }
                            }
                            first = false;

                        }else{
                            #ifdef VIESCHEDPP_LOG
                            BOOST_LOG_TRIVIAL(warning) << "cannot read frequency setup";
                            #else
                            cout << "cannot read frequency setup\n";
                            #endif
                        }
                    }

                    if(splitVector[0] == "R"){
                        samRate = boost::lexical_cast<double>(splitVector.at(2));
                    }

                    if (splitVector[0] == "L") {
                        break;
                    }

                }
            }
            ++counter;
        }
    }
    fid.close();

    int sec_ = TimeSystem::startTime.time_of_day().total_seconds();
    TimeSystem::mjdStart = TimeSystem::startTime.date().modjulian_day() + sec_ / 86400.0;

    int sec = util::duration(TimeSystem::startTime,TimeSystem::endTime);
    if (sec < 0) {
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(error) << "duration is less than zero seconds";
        #else
        cout << "[error] duration is less than zero seconds";
        #endif
    }
    auto duration = static_cast<unsigned int>(sec);
    TimeSystem::duration = duration;


    skd_.setStationNames(staNames);
    skd_.setCatalogFilePathes(filename_);
    skd_.initializeStationCatalogs();
    skd_.initializeSourceCatalogs();

    string path = filename_.substr(0,filename_.find_last_of('/'));
    path.append("/skdParser.log");
    ofstream of(path);


    std::unordered_map<std::string, double> band2wavelength;
    for(const auto &any : freqs_){
        double mfreq = accumulate(any.second.begin(), any.second.end(), 0.0) / any.second.size();
        band2wavelength[any.first] = util::freqency2wavelenth(mfreq);
    }

    init.initializeObservingMode(staNames.size(), samRate, bits, band2channel, band2wavelength);

    init.createSources(skd_, of);
    init.createStations(skd_, of);
    init.initializeAstronomicalParameteres();
    init.precalcAzElStations();

    network_  = init.network_;
    sources_  = init.sources_;
    obsModes_ = init.obsModes_;


    createScans(of);
    copyScanMembersToObjects(of);

    of.close();
}

void SkdParser::createScans(std::ofstream &of) {

    ifstream fid(filename_);
    if (!fid.is_open()) {
        of << "ERROR: Unable to open " << filename_ << " file!;\n";
        return;
    } else {

        string line;
        // read until you reach "$SKED"
        while (getline(fid, line)) {
            if (line.empty() || line.at(0) == '*') {
                continue;
            }
            string trimmed = boost::trim_copy(line);
            if(trimmed == "$SKED"){
                break;
            }
        }

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
                of << "ERROR: duration is less than zero seconds!;\n";
            }
            auto time = static_cast<unsigned int>(sec);

            // calc pointingVectors
            vector<PointingVector> pv;
            vector<PointingVector> pv_end;
            vector<unsigned int>thisEols(nsta,0);
            vector<unsigned int>slewTimes(nsta,0);
            vector<unsigned int>preobTimes(nsta,0);
            vector<unsigned int>fieldSystemTimes(nsta,0);
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
                thisSta.calcAzEl_rigorous(thisSource,p);
                bool error = thisSta.getCableWrap().unwrapAzInSection(p,cwflag);
                if(error){
                    pair<double,double>limits = thisSta.getCableWrap().getLimits(cwflag);
                    of << boost::format("Station %8s scan %4d source %8s time %s azimuth error! Flag: %c (from %7.2f to %7.2f) calculated: %7.2f (or %7.2f)\n")
                            %thisSta.getName()%counter%thisSource.getName()%TimeSystem::ptime2string_doy(scanStart)%cwflag%(limits.first*rad2deg)%(limits.second*rad2deg)%(p.getAz()*rad2deg)%(p.getAz()*rad2deg-360);
                }
                pv.push_back(p);

                PointingVector p_end(staid,srcid);
                p_end.setTime(time+durations[i]);
                thisSta.calcAzEl_rigorous(thisSource,p_end);
                thisSta.getCableWrap().unwrapAzNearAz(p_end,p.getAz());
                pv_end.push_back(p_end);

                thisEols[i] = eols[staid];
                eols[staid] = p_end.getTime();


                if(thisSta.getPARA().firstScan){

                    thisSta.referencePARA().firstScan = false;
                    fieldSystemTimes[i] = 0;
                    preobTimes[i] = 0;
                    slewTimes[i] = 0;

                }else{

                    unsigned int thisSlewTime = thisSta.getAntenna().slewTime(thisSta.getCurrentPointingVector(), p);
                    fieldSystemTimes[i] = fieldSystemTimes_;
                    preobTimes[i] = preob;
                    slewTimes[i] = thisSlewTime;

                }
                thisSta.setCurrentPointingVector(p_end);
            }

            Scan scan(pv,thisEols,Scan::ScanType::standard);
            bool valid;

            valid = scan.setScanTimes(thisEols,fieldSystemTimes,slewTimes,preobTimes,time,durations);

            if(!valid){
                const auto &tmp = scan.getTimes();
                for (int i = 0; i < nsta; ++i) {
                    if(tmp.getObservingTime(i, Timestamp::start)- tmp.getPreobDuration(i) < tmp.getSlewTime(i, Timestamp::end)-2){
                        unsigned int eost = tmp.getSlewTime(i, Timestamp::end);
                        unsigned int eoit = tmp.getObservingTime(i, Timestamp::start)- tmp.getPreobDuration(i);
                        boost::posix_time::ptime eostp = TimeSystem::internalTime2PosixTime(eost);
                        boost::posix_time::ptime eoitp = TimeSystem::internalTime2PosixTime(eoit);

                        of << boost::format("Station %8s scan %4d source %8s time %s idle time error!\n")
                                %network_.getStation(scan.getPointingVector(i).getStaid()).getName()
                                %counter
                                %sources_[scan.getPointingVector(i).getSrcid()].getName()
                                %TimeSystem::ptime2string_doy(scanStart);
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

void SkdParser::copyScanMembersToObjects(std::ofstream &of) {
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
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(error) << "station name "<< station << "unknown";
        #else
        cout << "[error] station name "<< station << "unknown";
        #endif
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
        fname = fname.substr(0, fname.size()-4);
    }

    xml.add("output.experimentName",fname);
    string description = "created from skd file: " + fname;

    xml.add("output.experimentDescription",description);

    Scheduler sched(fname, network_, sources_, scans_, xml, obsModes_);
    ofstream dummy;
    sched.checkAndStatistics(dummy);
    return sched;
}

void SkdParser::setLogFiles() {

#ifdef VIESCHEDPP_LOG
    
    boost::log::add_common_attributes();

    boost::log::core::get()->set_filter(
            boost::log::trivial::severity >= boost::log::trivial::info
    );

    auto fmtTimeStamp = boost::log::expressions::
    format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f");
    auto fmtSeverity = boost::log::expressions::
    attr<boost::log::trivial::severity_level>("Severity");

    boost::log::formatter logFmt= boost::log::expressions::format("[%1%] [%2%] %3%") % fmtTimeStamp  % fmtSeverity
                                                                                     % boost::log::expressions::smessage;

    auto consoleSink = boost::log::add_console_log(std::cout);
    consoleSink->set_formatter(logFmt);

    consoleSink->set_filter(
            boost::log::trivial::severity >= boost::log::trivial::info
    );

    std::size_t found = filename_.find_last_of("/\\");
    std::string path_;
    if(found == std::string::npos){
        path_ = "";
    }else{
        path_ = filename_.substr(0,found+1);
    }

    auto fsSink = boost::log::add_file_log(
            boost::log::keywords::file_name = path_+"VieSchedpp_sked_parser_%Y-%m-%d_%H-%M-%S.%3N.log",
//            boost::log::keywords::file_name = path_+"VieSchedpp_%3N.log",
            boost::log::keywords::rotation_size = 10 * 1024 * 1024,
            boost::log::keywords::min_free_space = 30 * 1024 * 1024,
            boost::log::keywords::open_mode = std::ios_base::app);
    fsSink->set_formatter(logFmt);
    fsSink->locked_backend()->auto_flush(true);

#endif
}
