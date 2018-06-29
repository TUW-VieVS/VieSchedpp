//
// Created by mschartn on 22.08.17.
//

#include "Output.h"

#include <utility>

using namespace std;
using namespace VieVS;

Output::Output() = default;

Output::Output(Scheduler &sched, std::string path, int version) : xml_{sched.xml_},
                                                                  version_{version},
                                                                  network_{std::move(sched.network_)},
                                                                  sources_{std::move(sched.sources_)},
                                                                  scans_{std::move(sched.scans_)},
                                                                  path_{std::move(path)} {

}


void Output::writeSkdsum() {

    string expName = xml_.get("master.output.experimentName","schedule");
    string fileName = boost::to_lower_copy(expName);
    if (version_ == 0) {
        fileName.append("_skdsum.txt");
        string txt = (boost::format("writing statistics to: %s;\n") % fileName).str();
        cout << txt;

    } else {
        fileName.append((boost::format("V%03d_skdsum.txt") % (version_)).str());
        string txt = (boost::format("version %d: writing statistics to: %s;\n") %version_ % fileName).str();
        cout << txt;
    }

    ofstream out(path_+fileName);
    displayGeneralStatistics(out);
    displayBaselineStatistics(out);
    displayStationStatistics(out);
    displaySourceStatistics(out);
    displayNstaStatistics(out);
    displayScanDurationStatistics(out);
    displayTimeStatistics(out);
    displayAstronomicalParameters(out);
    out.close();
}

void  Output::displayGeneralStatistics(ofstream &out) {
    auto n_scans = static_cast<int>(scans_.size());
    int n_standard = 0;
    int n_highImpact = 0;
    int n_fillin = 0;
    int n_calibrator = 0;
    int n_single = 0;
    int n_subnetting = 0;

    for (const auto&any:scans_){
        switch (any.getType()){
            case Scan::ScanType::fillin:{
                ++n_fillin;
                break;
            }
            case Scan::ScanType::calibrator:{
                ++n_calibrator;
                break;
            }
            case Scan::ScanType::standard:{
                ++n_standard;
                break;
            }
            case Scan::ScanType::highImpact:{
                ++n_highImpact;
                break;}
        }
        switch (any.getScanConstellation()){
            case Scan::ScanConstellation::single:{
                ++n_single;
                break;
            }
            case Scan::ScanConstellation::subnetting:{
                ++n_subnetting;
                break;
            }
        }
    }

    out << ".--------------------------------------.\n";
    out << boost::format("| number of total scans:         %5d |\n") %n_scans;
    out << "|--------------------------------------|\n";
    out << boost::format("| number of single source scans: %5d |\n") %n_single;
    out << boost::format("| number of subnetting scans:    %5d |\n") %n_subnetting;
    out << "|--------------------------------------|\n";
    out << boost::format("| number of standard scans:      %5d |\n") %n_standard;
    out << boost::format("| number of high impact scans    %5d |\n") %n_highImpact;
    out << boost::format("| number of fillin mode scans:   %5d |\n") %n_fillin;
    out << boost::format("| number of calibrator scans:    %5d |\n") %n_calibrator;
    out << "'--------------------------------------'\n\n";

}

void Output::displayBaselineStatistics(ofstream &out) {
    unsigned long nsta = network_.getNSta();
    int n_bl = 0;
    vector< vector <unsigned long> > bl_storage(nsta,vector<unsigned long>(nsta,0));
    for (const auto&any:scans_){
        int this_n_bl = static_cast<int>(any.getNObs());
        n_bl += this_n_bl;

        for (int i = 0; i < this_n_bl; ++i) {
            unsigned long staid1 = any.getObservation(i).getStaid1();
            unsigned long staid2 = any.getObservation(i).getStaid2();
            if(staid1>staid2){
                swap(staid1,staid2);
            }
            ++bl_storage[staid1][staid2];
        }
    }

    out << "number of scheduled observations: " << n_bl << "\n";
    out << ".-----------";
    for (int i = 0; i < nsta-1; ++i) {
        out << "----------";
    }
    out << "-----------";
    out << "----------.\n";


    out << boost::format("| %8s |") % "STATIONS";
    for (const auto &any:network_.getStations()) {
        out << boost::format(" %8s ") % any.getName();
    }
    out << "|";
    out << boost::format(" %8s ") % "TOTAL";
    out << "|\n";

    out << "|----------|";
    for (int i = 0; i < nsta-1; ++i) {
        out << "----------";
    }
    out << "----------|";
    out << "----------|\n";

    for (int i = 0; i < nsta; ++i) {
        unsigned long counter = 0;
        out << boost::format("| %8s |") % network_.getStation(i).getName();
        for (int j = 0; j < nsta; ++j) {
            if (j<i+1){
                out << "          ";
                counter += bl_storage[j][i];
            }else{
                out << boost::format(" %8d ") % bl_storage[i][j];
                counter += bl_storage[i][j];
            }
        }
        out << "|";
        out << boost::format(" %8d ") % counter;
        out << "|\n";
    }

    out << "'-----------";
    for (int i = 0; i < nsta-1; ++i) {
        out << "----------";
    }
    out << "-----------";
    out << "----------'\n\n";

}

void Output::displayStationStatistics(ofstream &out) {

    out << "number of scans per 15 minutes:\n";
    out << ".-------------------------------------------------------------"
           "----------------------------------------------------------------------------.\n";
    out << "|          time since session start (1 char equals 15 minutes)"
           "                                             | #SCANS #OBS |   OBS Time [s] |\n";
    out << "| STATION |0   1   2   3   4   5   6   7   8   9   10  11  12 "
           " 13  14  15  16  17  18  19  20  21  22  23  |             |   sum  average |\n";
    out << "|---------|+---+---+---+---+---+---+---+---+---+---+---+---+--"
           "-+---+---+---+---+---+---+---+---+---+---+---|-------------|----------------|\n";
    for (const auto &thisStation : network_.getStations()) {
        out << boost::format("| %8s|") % thisStation.getName();
        const Station::Statistics &stat = thisStation.getStatistics();
        const auto& time_sta = stat.scanStartTimes;
        unsigned int timeStart = 0;
        unsigned int timeEnd = 900;
        for (int j = 0; j < 96; ++j) {
            long c = count_if(time_sta.begin(), time_sta.end(), [timeEnd,timeStart](unsigned int k){
                return k>=timeStart && k<timeEnd ;
            });
            if(c==0){
                out << " ";
            }else if (c<=9){
                out << c;
            }else{
                out << "X";
            }

            timeEnd += 900;
            timeStart += 900;
        }
        out << boost::format("| %6d %4d ") % thisStation.getNTotalScans() % thisStation.getNbls();
        out << boost::format("| %5d %8.1f |\n") % thisStation.getStatistics().totalObservingTime
               % (static_cast<double>(thisStation.getStatistics().totalObservingTime) / static_cast<double>(thisStation.getNTotalScans()));
    }
    out << "'--------------------------------------------------------------"
           "---------------------------------------------------------------------------'\n\n";
}

void Output::displaySourceStatistics(ofstream &out) {
    out << "number of available sources:   " << sources_.size() << "\n";

    long number = count_if(sources_.begin(), sources_.end(), [](const Source &any){
        return any.getNTotalScans() > 0;
    });

    out << "number of scheduled sources:   " << number << "\n";
    out << "number of scans per 15 minutes:\n";
    out << ".-------------------------------------------------------------"
           "----------------------------------------------------------------------------.\n";
    out << "|          time since session start (1 char equals 15 minutes)"
           "                                             | #SCANS #OBS |   OBS Time [s] |\n";
    out << "|  SOURCE |0   1   2   3   4   5   6   7   8   9   10  11  12 "
           " 13  14  15  16  17  18  19  20  21  22  23  |             |   sum  average |\n";
    out << "|---------|+---+---+---+---+---+---+---+---+---+---+---+---+--"
           "-+---+---+---+---+---+---+---+---+---+---+---|-------------|----------------|\n";
    for (const auto &thisSource : sources_) {
        const Source::Statistics &stat = thisSource.getStatistics();
        const auto& time_sta = stat.scanStartTimes;

        if (thisSource.getNbls() == 0) {
            continue;
        }
        out << boost::format("| %8s|") % thisSource.getName();

        unsigned int timeStart = 0;
        unsigned int timeEnd = 900;
        for (int j = 0; j < 96; ++j) {
            long c = count_if(time_sta.begin(), time_sta.end(), [timeEnd,timeStart](unsigned int k){
                return k>=timeStart && k<timeEnd ;
            });
            if(c==0){
                out << " ";
            }else if (c<=9){
                out << c;
            }else{
                out << "X";
            }

//            if(any_of(time_sta.begin(), time_sta.end(), [timeEnd,timeStart](unsigned int k){return k<timeEnd && k>=timeStart;})){
//                out << "x";
//            }else{
//                out << " ";
//            }
            timeEnd += 900;
            timeStart += 900;
        }
        out << boost::format("| %6d %4d ") % thisSource.getNTotalScans() % thisSource.getNbls();
        out << boost::format("| %5d %8.1f |\n") % thisSource.getStatistics().totalObservingTime
               % (static_cast<double>(thisSource.getStatistics().totalObservingTime) / static_cast<double>(thisSource.getNTotalScans()));
    }
    out << "'--------------------------------------------------------------"
           "---------------------------------------------------------------------------'\n\n";
}

void Output::displayNstaStatistics(std::ofstream &out) {
    unsigned long nsta = network_.getNSta();
    vector<int> nstas(nsta+1,0);
    for(const auto &scan:scans_){
        ++nstas[scan.getNSta()];
    }

    unsigned long sum = scans_.size();

    out << "number of scans per number of participating stations:\n";
    for(int i=2; i<=nsta; ++i){
        out << boost::format("    number of %2d station scans: %4d (%6.2f %%)\n")%i %nstas[i] %(static_cast<double>(nstas[i])/
                                                                                                static_cast<double>(sum)*100);
    }
    out << "\n";
}


void Output::displayScanDurationStatistics(ofstream &out) {
    unsigned long nsta = network_.getNSta();
    out << "required scan durations:\n";
    vector<vector<vector<unsigned int>>> bl_durations(nsta,vector<vector<unsigned int>>(nsta));
    vector< unsigned int> maxScanDurations;

    for(const auto&any: scans_){
        unsigned long nbl = any.getNObs();
        maxScanDurations.push_back(any.getTimes().getObservingTime());

        for (int i = 0; i < nbl; ++i) {
            const Observation &obs = any.getObservation(i);
            unsigned long staid1 = obs.getStaid1();
            unsigned long staid2 = obs.getStaid2();
            unsigned int obs_duration = obs.getObservingTime();

            if(staid1<staid2){
                swap(staid1,staid2);
            }
            bl_durations[staid1][staid2].push_back(obs_duration);
        }
    }
    if(maxScanDurations.empty()){
        return;
    }
    unsigned int maxMax = *max_element(maxScanDurations.begin(),maxScanDurations.end());
    maxMax = maxMax/10*10+10;
    unsigned int cache_size = (1+maxMax/100)*10;
    vector<unsigned int> bins;

    unsigned int upper_bound = 0;
    while (upper_bound<maxMax+cache_size){
        bins.push_back(upper_bound);
        upper_bound+=cache_size;
    }

    vector<unsigned int> hist(bins.size()-1,0);
    for (unsigned int any:maxScanDurations) {
        int i = 1;
        while(any>bins[i]){
            ++i;
        }
        ++hist[i-1];
    }

    out << "scan duration:\n";
    for (int i = 0; i < hist.size(); ++i) {
        out << boost::format("%3d-%3d | ") % bins[i] % (bins[i + 1] - 1);
        double percent = 100 * static_cast<double>(hist[i]) / static_cast<double>(maxScanDurations.size());
        percent = round(percent);
        for (int j = 0; j < percent; ++j) {
            out << "+";
        }
        out << "\n";
    }
    out << "\n";

    out << "scan length:\n";
    out << ".-------------------------------------------------------------------------------------.\n";
    out << "| STATION1-STATION2 |  min    10%    50%    90%    95%  97.5%    99%    max   average |\n";
    out << "|-------------------|-----------------------------------------------------------------|\n";

    {
        int n = static_cast<int>(maxScanDurations.size() - 1);
        sort(maxScanDurations.begin(),maxScanDurations.end());
        out << boost::format("|        ALL        | ");
        out << boost::format("%4d   ") % maxScanDurations[0];
        out << boost::format("%4d   ") % maxScanDurations[n * 0.1];
        out << boost::format("%4d   ") % maxScanDurations[n / 2];
        out << boost::format("%4d   ") % maxScanDurations[n * 0.9];
        out << boost::format("%4d   ") % maxScanDurations[n * 0.95];
        out << boost::format("%4d   ") % maxScanDurations[n * 0.975];
        out << boost::format("%4d   ") % maxScanDurations[n * 0.99];
        out << boost::format("%4d   ") % maxScanDurations[n];
        double average = accumulate(maxScanDurations.begin(), maxScanDurations.end(), 0.0) / (n + 1);
        out << boost::format("%7.2f |") % average;
        out << "\n";
    }

    out << "|-------------------|-----------------------------------------------------------------|\n";

    for (unsigned long i = 1; i < nsta; ++i) {
        for (unsigned long j = 0; j < i; ++j) {
            vector<unsigned int>& this_duration = bl_durations[i][j];
            if(this_duration.empty()){
                continue;
            }
            int n = (int) this_duration.size()-1;
            sort(this_duration.begin(),this_duration.end());
            out << boost::format("| %17s | ") % network_.getBaseline(i,j).getName();
            out << boost::format("%4d   ") % this_duration[0];
            out << boost::format("%4d   ") % this_duration[n * 0.1];
            out << boost::format("%4d   ") % this_duration[n / 2];
            out << boost::format("%4d   ") % this_duration[n * 0.9];
            out << boost::format("%4d   ") % this_duration[n * 0.95];
            out << boost::format("%4d   ") % this_duration[n * 0.975];
            out << boost::format("%4d   ") % this_duration[n * 0.99];
            out << boost::format("%4d   ") % this_duration[n];
            double average = accumulate(this_duration.begin(), this_duration.end(), 0.0) / (n + 1);
            out << boost::format("%7.2f |") % average;
            out << "\n";
        }

    }
    out << "'-------------------------------------------------------------------------------------'\n\n";
}

void Output::displayTimeStatistics(std::ofstream &ofstream) {

    unsigned long nstaTotal = network_.getNSta();

    ofstream << ".------------------";
    for (int i = 0; i < nstaTotal-1; ++i) {
        ofstream << "----------";
    }
    ofstream << "----------.\n";

    ofstream << "|                 |";
    for (const auto &station: network_.getStations()) {
        ofstream << boost::format(" %8s ") % station.getName();
    }
    ofstream << "|\n";

    ofstream << "|-----------------|";
    for (int i = 0; i < nstaTotal-1; ++i) {
        ofstream << "----------";
    }
    ofstream << "----------|\n";

    ofstream << "| % obs. time:    |";
    for (const auto &station: network_.getStations()) {
        int t = station.getStatistics().totalObservingTime;
        ofstream << boost::format(" %8.2f ") % (static_cast<double>(t)/static_cast<double>(TimeSystem::duration)*100);
    }
    ofstream << "|\n";

    ofstream << "| % preob time:   |";
    for (const auto &station: network_.getStations()) {
        int t = station.getStatistics().totalPreobTime;
        ofstream << boost::format(" %8.2f ") % (static_cast<double>(t)/static_cast<double>(TimeSystem::duration)*100);
    }
    ofstream << "|\n";

    ofstream << "| % slew time:    |";
    for (const auto &station: network_.getStations()) {
        int t = station.getStatistics().totalSlewTime;
        ofstream << boost::format(" %8.2f ") % (static_cast<double>(t)/static_cast<double>(TimeSystem::duration)*100);
    }
    ofstream << "|\n";

    ofstream << "| % idle time:    |";
    for (const auto &station: network_.getStations()) {
        int t = station.getStatistics().totalIdleTime;
        ofstream << boost::format(" %8.2f ") % (static_cast<double>(t)/static_cast<double>(TimeSystem::duration)*100);
    }
    ofstream << "|\n";

    ofstream << "| % field system: |";
    for (const auto &station: network_.getStations()) {
        int t = station.getStatistics().totalFieldSystemTime;
        ofstream << boost::format(" %8.2f ") % (static_cast<double>(t)/static_cast<double>(TimeSystem::duration)*100);
    }
    ofstream << "|\n";

    ofstream << "'------------------";
    for (int i = 0; i < nstaTotal-1; ++i) {
        ofstream << "----------";
    }
    ofstream << "----------'\n\n";

    ofstream << "|   #scans     :  |";
    for (const auto &station: network_.getStations()) {
        ofstream << boost::format(" %8d ") % (station.getNTotalScans());
    }
    ofstream << "|\n";

    ofstream << "|   scans per h:  |";
    for (const auto &station: network_.getStations()) {
        ofstream << boost::format(" %8.2f ") % (static_cast<double>(station.getNTotalScans())/static_cast<double>(TimeSystem::duration/3600.)*100);
    }
    ofstream << "|\n";

    ofstream << "'------------------";
    for (int i = 0; i < nstaTotal-1; ++i) {
        ofstream << "----------";
    }
    ofstream << "----------'\n\n";

}

void Output::displayAstronomicalParameters(std::ofstream &out) {
    out << ".------------------------------------------.\n";
    out << "| sun position:        | earth velocity:   |\n";
    out << "|----------------------|-------------------|\n";
    out << "| RA:   " << util::ra2dms(AstronomicalParameters::sun_radc[0]) << " " << boost::format("| x: %8.0f [m/s] |\n")%AstronomicalParameters::earth_velocity[0];
    out << "| DEC: "  << util::dc2hms(AstronomicalParameters::sun_radc[1]) << " " << boost::format("| y: %8.0f [m/s] |\n")%AstronomicalParameters::earth_velocity[1];
    out << "|                      " << boost::format("| z: %8.0f [m/s] |\n")%AstronomicalParameters::earth_velocity[2];
    out << "'------------------------------------------'\n\n";

    out << ".--------------------------------------------------------------------.\n";
    out << "| earth nutation:                                                    |\n";
    out << boost::format("| %=19s | %=14s %=14s %=14s |\n") %"time" %"X" %"Y" %"S";
    out << "|---------------------|----------------------------------------------|\n";
    for(int i=0; i<AstronomicalParameters::earth_nutTime.size(); ++i){
        out << boost::format("| %19s | %+14.6e %+14.6e %+14.6e |\n")
               % TimeSystem::ptime2string(TimeSystem::internalTime2PosixTime(AstronomicalParameters::earth_nutTime[i]))
               % AstronomicalParameters::earth_nutX[i]
               % AstronomicalParameters::earth_nutY[i]
               % AstronomicalParameters::earth_nutS[i];
    }
    out << "'--------------------------------------------------------------------'\n\n";

}

void Output::writeNGS() {

    string fname;
    if (version_ == 0) {
        fname = TimeSystem::date2string(TimeSystem::startTime).erase(0,2).append("MS");
        string txt = (boost::format("writing NGS file: %s;\n") % fname).str();
        cout << txt;
    } else {
        fname = (boost::format("%sMS_v%03d") % TimeSystem::date2string(TimeSystem::startTime).erase(0,2) % (version_)).str();
        string txt = (boost::format("version %d: writing empty ngs file: %s;\n") % version_ % fname).str();
        cout << txt;
    }
    ofstream out(path_+fname);


    boost::posix_time::ptime start = TimeSystem::startTime;
    unsigned long counter = 1;

    for (const auto &any: scans_) {
        for (int i = 0; i < any.getNObs(); ++i) {
            const Observation &obs = any.getObservation(i);
            string sta1 = network_.getStation(obs.getStaid1()).getName();
            string sta2 = network_.getStation(obs.getStaid1()).getName();
            if (sta1 > sta2) {
                swap(sta1, sta2);
            }
            string src = sources_[obs.getSrcid()].getName();
            unsigned int time = obs.getStartTime();

            boost::posix_time::ptime tmp = TimeSystem::internalTime2PosixTime(time);
            int year = tmp.date().year();
            int month = tmp.date().month();
            int day = tmp.date().day();
            int hour = tmp.time_of_day().hours();
            int minute = tmp.time_of_day().minutes();
            double second = tmp.time_of_day().seconds();

            out << boost::format("%-8s  %-8s  %-8s %4d %02d %02d %02d %02d  %13.10f            ") % sta1 % sta2 % src %
                   year % month % day % hour % minute % second;
            out << boost::format("%6d") % counter << "01\n";

            out << "    0000000.00000000    .00000  -000000.0000000000    .00000 0      I   ";
            out << boost::format("%6d") % counter << "02\n";

            out << "    .00000    .00000    .00000    .00000   0.000000000000000        0.  ";
            out << boost::format("%6d") % counter << "03\n";

            out << "       .00   .0       .00   .0       .00   .0       .00   .0            ";
            out << boost::format("%6d") % counter << "04\n";

            out << "   -.00000   -.00000    .00000    .00000    .00000    .00000            ";
            out << boost::format("%6d") % counter << "05\n";

            out << "     0.000    00.000   000.000   000.000    00.000    00.000 0 0        ";
            out << boost::format("%6d") % counter << "06\n";

            out << "        0.0000000000    .00000        -.0000000000    .00000  0         ";
            out << boost::format("%6d") % counter << "08\n";

            out << "          0.00000000    .00000        0.0000000000    .00000 0      I   ";
            out << boost::format("%6d") % counter << "09\n";

            ++counter;
        }
    }

    out.close();
}


void Output::writeVex(const SkdCatalogReader &skdCatalogReader) {
    string expName = xml_.get("master.output.experimentName","schedule");
    string fileName = boost::to_lower_copy(expName);
    if (version_ == 0) {
        fileName.append(".vex");
        string txt = (boost::format("writing vex file: %s;\n") % fileName).str();
        cout << txt;
    } else {
        fileName.append((boost::format("v%03d.vex") % (version_)).str());
        string txt = (boost::format("version %d: writing vex file: %s;\n") %version_ % fileName).str();
        cout << txt;
    }

    Vex vex(path_+fileName);
    vex.writeVex(network_,sources_,scans_,skdCatalogReader,xml_);
}

void Output::writeSkd(const SkdCatalogReader &skdCatalogReader) {
    string expName = xml_.get("master.output.experimentName","schedule");
    string fileName = boost::to_lower_copy(expName);
    if (version_ == 0) {
        fileName.append(".skd");
        string txt = (boost::format("writing skd file: %s;\n") % fileName).str();
        cout << txt;
    } else {
        fileName.append((boost::format("v%03d.skd") % (version_)).str());
        string txt = (boost::format("version %d: writing skd file: %s;\n") %version_ % fileName).str();
        cout << txt;
    }

    Skd skd(path_+fileName);
    skd.writeSkd(network_,sources_,scans_,skdCatalogReader,xml_);
}


void Output::writeStatisticsPerSourceGroup() {

    const auto & tmp0 = xml_.get_child_optional("master.source");

    if(tmp0.is_initialized()){
        boost::property_tree::ptree PARA_source = *tmp0;
        unordered_map<std::string, std::vector<std::string> > group_source = readGroups(PARA_source, GroupType::source);

        string expName = xml_.get("master.output.experimentName","schedule");
        string fileName = boost::to_lower_copy(expName);
        if (version_ == 0) {
            fileName.append("_sourceStatistics.txt");
            string txt = (boost::format("writing source statistics file: %s;\n") % fileName).str();
            cout << txt;
        } else {
            fileName.append((boost::format("v%03d_sourceStatistics.txt") % (version_)).str());
            string txt = (boost::format("version %d: writing source statistics file: %s;\n") %version_ % fileName).str();
            cout << txt;
        }

        vector<string> interestedSrcGroups;
        const auto & tmp = xml_.get_child_optional("master.output.sourceGroupsForStatistic");
        if(tmp.is_initialized()){
            for(const auto &any :*tmp){
                if(any.first == "name"){
                    interestedSrcGroups.push_back(any.second.get_value<string>());
                }
            }
            if(interestedSrcGroups.empty()){
                return;
            }
        }else{
            return;
        }

        ofstream of(path_+fileName);

        auto nsrc = sources_.size();
        vector<double> sWeight;
        vector<unsigned int> nscansTarget;
        vector<unsigned int> targetScans;
        vector<double> minRepeat;
        vector<vector<pair<boost::posix_time::ptime,boost::posix_time::ptime>>> visibleTimes(nsrc);
        bool hardBreak = false;
        for(auto &src:sources_){
            src.setNextEvent(0);
            src.checkForNewEvent(0, hardBreak);
            sWeight.push_back(src.getPARA().weight);
            if(src.getPARA().tryToObserveXTimesEvenlyDistributed.is_initialized()){
                nscansTarget.push_back(*src.getPARA().tryToObserveXTimesEvenlyDistributed);
            }else{
                nscansTarget.push_back(0);
            }
            minRepeat.push_back(static_cast<double>(src.getPARA().minRepeat)/3600.0);
            targetScans.push_back(src.getPARA().maxNumberOfScans);
            auto visTimes = minutesVisible(src);

            unsigned int start = 0;
            unsigned int lastElement=0;
            for(auto const &t: visTimes){
                if(start == 0 ){
                    start = t;
                    lastElement = t;
                }else {
                    if (t - lastElement != 60) {
                        boost::posix_time::ptime ptstart = TimeSystem::internalTime2PosixTime(start);
                        boost::posix_time::ptime ptend = TimeSystem::internalTime2PosixTime(lastElement);
                        visibleTimes[src.getId()].emplace_back(ptstart, ptend);
                        start = 0;
                    }
                    lastElement = t;
                }
            }
            if(start != 0){
                boost::posix_time::ptime ptstart = TimeSystem::internalTime2PosixTime(start);
                boost::posix_time::ptime ptend = TimeSystem::internalTime2PosixTime(lastElement);
                visibleTimes[src.getId()].emplace_back(ptstart, ptend);
            }
        }



        vector<vector<unsigned int> > scanTime(nsrc);
        vector<vector<unsigned long> > scanNsta(nsrc);
        vector<vector<unsigned long> > scanNbl(nsrc);
        vector<vector<char> > flag(nsrc);
        vector<vector<unsigned int> > scanTimePerStation(nsrc,vector<unsigned int>(network_.getNSta(),0));

        for(const auto &scan:scans_){
            unsigned long srcid = scan.getSourceId();
            scanTime[srcid].push_back(scan.getPointingVector(0).getTime());
            scanNsta[srcid].push_back(scan.getNSta());
            scanNbl[srcid].push_back(scan.getNObs());
            switch (scan.getType()){
                case Scan::ScanType::fillin:{
                    flag[srcid].push_back('*');
                    break;
                }
                case Scan::ScanType::calibrator: {
                    flag[srcid].push_back('#');
                    break;
                }
                case Scan::ScanType::standard: {
                    flag[srcid].push_back(' ');
                    break;
                }
                case Scan::ScanType::highImpact:{
                    flag[srcid].push_back(' ');
                    break;
                }
            }
            for(int i=0; i<scan.getNSta(); ++i){
                unsigned long staid = scan.getPointingVector(i).getStaid();
                unsigned int duration = scan.getTimes().getObservingTime(i);
                scanTimePerStation[srcid][staid]+=duration;
            }
        }

        unsigned long nMaxScans = 0;
        for(const auto &any: scanTime){
            unsigned long thisScans = any.size();
            if(thisScans>nMaxScans){
                nMaxScans = thisScans;
            }
        }

        of << "* Columns:\n";
        of << "*     1  : Name\n";
        of << "*     2  : Id\n";
        of << "*     3  : total scans\n";
        of << "*     4  : standard scans\n";
        of << "*     5  : fillin mode scans\n";
        of << "*     6  : calibrator scans\n";
        of << "*     7  : weight at start of session\n";
        of << "*     8  : target scans\n";
        of << "*     9  : minimum source repeat time [h] (at session start)\n";
        of << "*     11+: list of scans containing:\n";
        of << "*             scan start time (UTC)\n";
        of << "*             scan flag:\n";
        of << "*                 ' ': single source or subnetting scan\n";
        of << "*                 '*': fillin mode scan\n";
        of << "*                 '#': calibrator scan\n";
        of << "*             number of stations\n";
        of << "*     end: source visibility time (estimated with parameters at session start)\n";

        map<string,vector<int>> srcgrpstat;
        map<string,vector<int>> srcgrpgeneralstat;
        map<string,vector<unsigned int>> srcGrpStationScanTime;

        of << "*\n";
        of << "* ============================= GROUP BASED STATISTICS =============================\n";

        for(const auto &group: group_source){
            if(find(interestedSrcGroups.begin(),interestedSrcGroups.end(),group.first) == interestedSrcGroups.end()){
                continue;
            }
            vector<int> nscansPerGroup(nMaxScans+1,0);
            vector<unsigned int> groupScanTimePerStation(network_.getNSta(),0);
            int sumTotalScans = 0;
            int sumScans = 0;
            int sumFillinModeScans = 0;
            int sumCalibratorScans = 0;

            int baselines = 0;

            of << "*\n";
            of << "* ----------------------------- GROUP: "<< group.first <<" -----------------------------\n";
            of << "*\n";
            for(const auto &src: sources_) {
                int srcid = src.getId();
                if ( find(group.second.begin(),group.second.end(),src.getName()) != group.second.end() ) {

                    unsigned long nscans = scanTime[srcid].size();
                    unsigned long nscansStd = 0;
                    unsigned long nscansFillin = 0;
                    unsigned long nscansCalibrator = 0;
                    ++nscansPerGroup[nscans];

                    for(int i=0; i<nscans; ++i) {
                        if(flag[srcid][i] == ' ') {
                            ++nscansStd;
                        }else if(flag[srcid][i] == '*') {
                            ++nscansFillin;
                        }else if(flag[srcid][i] == '#') {
                            ++nscansCalibrator;
                        }
                    }
                    sumTotalScans += nscans;
                    sumScans += nscansStd;
                    sumFillinModeScans += nscansFillin;
                    sumCalibratorScans += nscansCalibrator;

                    for(int i=0; i<network_.getNSta(); ++i){
                        groupScanTimePerStation[i] += scanTimePerStation[srcid][i];
                    }

                    of << boost::format("%8s, %4d, %4d, %4d, %4d, %4d, %6.2f, %4d, %5.2f, ||, ") %src.getName() %src.getId() %nscans %nscansStd %nscansFillin %nscansCalibrator %sWeight[srcid] %nscansTarget[srcid] %minRepeat[srcid];
                    for (int i=0; i<scanTime[srcid].size(); ++i){
                        unsigned int ttt = scanTime[srcid][i];

                        boost::posix_time::ptime pt = TimeSystem::internalTime2PosixTime(ttt);
                        of << TimeSystem::ptime2string(pt).substr(11,5).append("[");
                        of << flag[srcid][i] ;
                        of << boost::format("%02d], ") % scanNsta[srcid][i];
                    }
                    for (unsigned long i=scanTime[srcid].size(); i < nMaxScans; ++i){
                        of << "          , ";
                    }
                    of << "||, ";
                    for (auto &i : visibleTimes[srcid]) {
                        of << "[" << TimeSystem::ptime2string(i.first).substr(11,5)
                           << " - " << TimeSystem::ptime2string(i.second).substr(11,5) << "], ";
                    }
                    of << "\n";
                }
            }

            srcgrpstat[group.first] = nscansPerGroup;
            srcgrpgeneralstat[group.first] = vector<int> {sumTotalScans, sumScans, sumFillinModeScans, sumCalibratorScans};
            srcGrpStationScanTime[group.first] = groupScanTimePerStation;
        }

        of << "*\n";
        of << "* ============================= SESSION SUMMARY =============================\n";
        of << "*\n";
        of << " # scans: " << scans_.size() <<"\n";
        unsigned int xxxstdScans = 0;
        unsigned int xxxfiScans = 0;
        unsigned int xxxcalScans = 0;
        for(const auto &persource:flag){
            for(const auto &perscan:persource){
                if(perscan == ' '){
                    ++xxxstdScans;
                }else if(perscan == '*'){
                    ++xxxfiScans;
                }else if(perscan == '#'){
                    ++xxxcalScans;
                }
            }
        }
        of << "   # standard scans:    " << xxxstdScans <<"\n";
        of << "   # fillin mode scans: " << xxxfiScans  <<"\n";
        of << "   # calibrator scans:  " << xxxcalScans <<"\n";

        vector<unsigned int> xxxstps(network_.getNSta(),0);
        for(const auto &stps: scanTimePerStation){
            for(int i=0; i<network_.getNSta(); ++i){
                xxxstps[i]+=stps[i];
            }
        }
        of << "* \n";
        of << "observing time per station:\n";
        for (int i=0; i<network_.getNSta(); ++i){
            const string name = network_.getStation(i).getName();
            unsigned int seconds = xxxstps[i];
            double percent = (static_cast<double>(seconds)/ static_cast<double>(TimeSystem::duration))*100;
            of << boost::format("    %8s: %8d [s]  (%5.1f [%%])\n") %name %seconds %percent;
        }


        of << "*\n";
        of << "* ============================= GROUP BASED SUMMARY =============================\n";

        for(const auto &any:srcgrpstat){
            const string &grpName = any.first;
            const auto &scans = any.second;
            const auto &sum = srcgrpgeneralstat[grpName];

            of << "*\n";
            of << "* ----------------------------- SUMMARY GROUP: "<< grpName <<" -----------------------------\n";
            of << "*\n";
            of << boost::format(" # total scans:         %4d\n") %sum[0];
            of << boost::format("   # standard scans:    %4d\n") %sum[1];
            of << boost::format("   # fillin mode scans: %4d\n") %sum[2];
            of << boost::format("   # calibrator scans:  %4d\n") %sum[3];
            of << "* \n";

            bool first = false;
            for (unsigned long i= scans.size() - 1; i >= 0; --i) {
                if (first || scans[i] != 0) {
                    of << boost::format(" %3d sources are observed in %4d scans\n") % scans[i] % i;
                    first = true;
                }
                if(i==0){
                    break;
                }
            }

            of << "* \n";
            of << "observing time per station:\n";
            for (int i=0; i<network_.getNSta(); ++i){
                const string name = network_.getStation(i).getName();
                unsigned int seconds = srcGrpStationScanTime[grpName][i];
                double percent = (static_cast<double>(seconds)/ static_cast<double>(TimeSystem::duration))*100;
                of << boost::format("    %8s: %8d [s]  (%5.1f [%%])\n") %name %seconds %percent;
            }
        }
    }


}


unordered_map<string, vector<string> > Output::readGroups(boost::property_tree::ptree root, GroupType type) noexcept {
    unordered_map<std::string, std::vector<std::string> > groups;
    auto groupTree = root.get_child_optional("groups");
    if(groupTree.is_initialized()){
        for (auto &it: *groupTree) {
            string name = it.first;
            if (name == "group") {
                string groupName = it.second.get_child("<xmlattr>.name").data();
                std::vector<std::string> members;
                for (auto &it2: it.second) {
                    if (it2.first == "member") {
                        members.push_back(it2.second.data());
                    }
                }
                groups[groupName] = members;
            }
        }
    }

    switch(type){
        case GroupType::source:{
            std::vector<std::string> members;
            for(const auto&any : sources_){
                members.push_back(any.getName());
            }
            groups["__all__"] = members;
            break;
        }
        case GroupType::station:{
            std::vector<std::string> members;
            for(const auto&any : network_.getStations()){
                members.push_back(any.getName());
            }
            groups["__all__"] = members;
            break;
        }
        case GroupType::baseline:{
            std::vector<std::string> members;
            for(const auto&any : network_.getBaselines()){
                members.push_back(any.getName());
            }
            groups["__all__"] = members;
            break;
        }
    }

    return groups;
}


vector<unsigned int> Output::minutesVisible(const Source &source) {
    vector<unsigned int> visibleTimes;
    const auto &parameters = source.getPARA();
    unsigned int minVisible = parameters.minNumberOfStations;

    vector<unsigned long> reqSta = parameters.requiredStations;
    vector<unsigned long> ignSta = parameters.ignoreStations;

    for(unsigned int t = 0; t<TimeSystem::duration; t+=60){
        unsigned int visible = 0;

        bool requiredStationNotVisible = false;
        for(unsigned long staid = 0; staid<network_.getNSta(); ++staid){

            if(find(ignSta.begin(),ignSta.end(),staid) != ignSta.end()){
                continue;
            }

            const Station &thisSta = network_.getStation(staid);
            PointingVector p(staid,source.getId());
            p.setTime(t);

            thisSta.calcAzEl(source, p, Station::AzelModel::simple);

            // check if source is up from station
            bool flag = thisSta.isVisible(p, source.getPARA().minElevation);
            if(flag){
                ++visible;
            }else{
                if(find(reqSta.begin(),reqSta.end(),staid) != reqSta.end()){
                    requiredStationNotVisible = true;
                    break;
                }
            }
        }
        if(requiredStationNotVisible){
            continue;
        }
        if(visible>=minVisible){
            visibleTimes.push_back(t);
        }

    }
    return visibleTimes;
}

void Output::createAllOutputFiles(std::ofstream &statisticsLog, const SkdCatalogReader &skdCatalogReader) {

    writeStatistics(statisticsLog);

    if(xml_.get<bool>("master.output.createSummary",false)){
        writeSkdsum();
    }
    if(xml_.get<bool>("master.output.createNGS",false)) {
        writeNGS();
    }
    if(xml_.get<bool>("master.output.createSKD",false)) {
        writeSkd(skdCatalogReader);
    }
    if(xml_.get<bool>("master.output.createVEX",false)) {
        writeVex(skdCatalogReader);
    }
    if(xml_.get<bool>("master.output.createOperationsNotes",false)) {
        writeOperationsNotes();
    }
    if(xml_.get<bool>("master.output.createSourceGroupStatistics",false)) {
        writeStatisticsPerSourceGroup();
    }

}

void Output::writeOperationsNotes() {
    string expName = xml_.get("master.output.experimentName","schedule");
    string fileName = boost::to_lower_copy(expName);
    if (version_ == 0) {
        fileName.append("_operationsNotes.txt");
        string txt = (boost::format("writing operationsNotes file: %s;\n") % fileName).str();
        cout << txt;
    } else {
        fileName.append((boost::format("v%03d_operationsNotes.txt") % (version_)).str());
        string txt = (boost::format("version %d: operationsNotes skd file: %s;\n") %version_ % fileName).str();
        cout << txt;
    }

    ofstream of(path_+fileName);

    of << "Session Notes for: " << expName << "\n";

    of << "    experiment description: " << xml_.get("master.output.experimentDescription","no description") << "\n";
    of << "    created with          : VieSched++ \n";
    of << "    creation time (local) : " << xml_.get("master.created.time","unknown") << "\n\n";

    of << "    nominal start time    : " << xml_.get("master.general.startTime","unknown") << "\n";
    of << "    nominal end time      : " << xml_.get("master.general.endTime","unknown") << "\n\n";

    std::string piName = xml_.get("master.output.piName","");
    if(!piName.empty()) {
        of << "    PI  name              : " << xml_.get("master.output.piName", "") << "\n";
    }
    std::string piEmail = xml_.get("master.output.piEmail", "");
    if(!piEmail.empty()) {
        of << "    PI email              : " << xml_.get("master.output.piEmail", "") << "\n\n";
    }
    std::string contactName = xml_.get("master.output.contactName","");
    if(!contactName.empty()) {
        of << "    contact name          : " << xml_.get("master.output.contactName", "") << "\n";
    }
    std::string contactEmail = xml_.get("master.output.contactEmail","");
    if(!contactEmail.empty()) {
        of << "    contact email         : " << xml_.get("master.output.contactEmail", "") << "\n\n";
    }

    of << "    scheduler             : " << xml_.get("master.output.scheduler","unknown") << "\n";
    of << "    scheduler name        : " << xml_.get("master.created.name","unknown") << "\n";
    of << "    scheduler email       : " << xml_.get("master.created.email","unknown") << "\n\n";

    of << "    target correlator     : " << xml_.get("master.output.correlator","unknown") << "\n\n";

    std::string notes = xml_.get("master.output.notes","");
    if(!notes.empty()){
        of << "Notes : \n" <<  boost::replace_all_copy(notes,"\\n","\n") << "\n";
    }

    of << "Operation notes: \n";
    of << "---------------------------------------------------------------------------------------------------------\n";
    string newStr = boost::replace_all_copy(xml_.get("master.output.operationNotes","no additional notes"),"\\n","\n");
    of << newStr << "\n";
    if(version_>0){
        of << "    Version: " << version_ << " from multi scheduling setup\n";
    }
    of << "---------------------------------------------------------------------------------------------------------\n\n";

    of << ".---------------------.\n";
    of << "| scheduled stations: |\n";
    of << "|---------------------|\n";
    for (const auto &any:network_.getStations()){
        of << boost::format("| %-8s     |  %2s  |\n") %any.getName() %any.getAlternativeName();
    }
    of << "'---------------------'\n\n";

    displayGeneralStatistics(of);
    displayBaselineStatistics(of);
    displayStationStatistics(of);
    displaySourceStatistics(of);
    displayNstaStatistics(of);
    displayScanDurationStatistics(of);
    displayTimeStatistics(of);
    displayAstronomicalParameters(of);

    WeightFactors::summary(of);

    if(scans_.size()>=2){
        of << "Scans:\n";
        for(unsigned long i=0; i<3; ++i){
            const auto &thisScan = scans_[i];
            thisScan.output(i,network_,sources_[thisScan.getSourceId()],of);
        }
        of << "...\n";
        for(unsigned long i= scans_.size() - 3; i < scans_.size(); ++i){
            const auto &thisScan = scans_[i];
            thisScan.output(i,network_,sources_[thisScan.getSourceId()],of);
        }
    }

    of << ".------------------------.\n";
    of << "| full scheduling setup: |\n";
    of << "'------------------------'\n";
    boost::property_tree::xml_parser::write_xml(of, xml_,
                                                boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
//    boost::property_tree::xml_parser::write_xml(of, xml_, boost::property_tree::xml_writer_make_settings<string>('\t', 1));
    of << "==================\n";
}

void Output::writeStatistics(std::ofstream &statisticsLog) {
    string oString;

    auto n_scans = static_cast<int>(scans_.size());
    int n_standard = 0;
    int n_fillin = 0;
    int n_calibrator = 0;
    int n_single = 0;
    int n_subnetting = 0;
    int n_bl = 0;
    vector<unsigned int> nscan_sta(network_.getNSta(),0);
    vector<unsigned int> nobs_sta(network_.getNSta(),0);
    vector<unsigned int> nscan_src(sources_.size(),0);

    for (const auto&any:scans_){
        switch (any.getType()){
            case Scan::ScanType::fillin:{
                ++n_fillin;
                break;
            }
            case Scan::ScanType::calibrator:{
                ++n_calibrator;
                break;
            }
            case Scan::ScanType::standard:{
                ++n_standard;
                break;
            }
            case Scan::ScanType::highImpact:{
                ++n_standard;
                break;
            }
        }
        switch (any.getScanConstellation()){
            case Scan::ScanConstellation::single:{
                ++n_single;
                break;
            }
            case Scan::ScanConstellation::subnetting:{
                ++n_subnetting;
                break;
            }
        }
        n_bl += any.getNObs();
        for (int ista = 0; ista < any.getNSta(); ++ista) {
            const PointingVector& pv =  any.getPointingVector(ista);
            unsigned long id = pv.getStaid();
            ++nscan_sta[id];
        }
        for (int ibl = 0; ibl < any.getNObs(); ++ibl){
            const Observation &obs = any.getObservation(ibl);
            ++nobs_sta[obs.getStaid1()];
            ++nobs_sta[obs.getStaid2()];
        }
        unsigned long id = any.getSourceId();
        ++nscan_src[id];
    }
    int n_src = static_cast<int>(count_if(nscan_src.begin(), nscan_src.end(), [](int i) {return i > 0;}));


    oString.append(std::to_string(version_)).append(",");
    oString.append(std::to_string(n_scans)).append(",");
    oString.append(std::to_string(n_single)).append(",");
    oString.append(std::to_string(n_subnetting)).append(",");
    oString.append(std::to_string(n_fillin)).append(",");
    oString.append(std::to_string(n_calibrator)).append(",");
    oString.append(std::to_string(n_bl)).append(",");
    oString.append(std::to_string(network_.getNSta())).append(",");
    for (int i = 0; i < network_.getNSta(); ++i) {
        oString.append(std::to_string(nscan_sta[i])).append(",");
    }
    for (int i = 0; i < network_.getNSta(); ++i) {
        oString.append(std::to_string(nobs_sta[i])).append(",");
    }
    oString.append(std::to_string(n_src)).append(",\n");

    statisticsLog << oString;

}



