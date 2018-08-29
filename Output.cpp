//
// Created by mschartn on 22.08.17.
//

#include "Output.h"

#include <utility>

using namespace std;
using namespace VieVS;
unsigned long Output::nextId = 0;

Output::Output(Scheduler &sched, std::string path, string fname, int version): VieVS_NamedObject(move(fname), nextId++),
                                                                               xml_{sched.xml_},
                                                                               network_{std::move(sched.network_)},
                                                                               sources_{std::move(sched.sources_)},
                                                                               scans_{std::move(sched.scans_)},
                                                                               path_{std::move(path)},
                                                                               multiSchedulingParameters_{std::move(sched.multiSchedulingParameters_)},
                                                                               version_{version}{

}


void Output::writeSkdsum() {

    string fileName = getName();
    fileName.append("_skdsum.txt");

    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(info) << "writing statistics to: " << fileName;
    #else
    cout << "[info] writing statistics to: " << fileName;
    #endif

    ofstream of(path_+fileName);
    displayGeneralStatistics(of);
    displayBaselineStatistics(of);
    displayStationStatistics(of);
    displaySourceStatistics(of);
    displayNstaStatistics(of);
    displayScanDurationStatistics(of);
    displayTimeStatistics(of);
    displayAstronomicalParameters(of);
    of.close();
}

void  Output::displayGeneralStatistics(ofstream &of) {
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

    of << ".--------------------------------------.\n";
    of << boost::format("| number of total scans:         %5d |\n") %n_scans;
    of << "|--------------------------------------|\n";
    of << boost::format("| number of single source scans: %5d |\n") %n_single;
    of << boost::format("| number of subnetting scans:    %5d |\n") %n_subnetting;
    of << "|--------------------------------------|\n";
    of << boost::format("| number of standard scans:      %5d |\n") %n_standard;
    of << boost::format("| number of high impact scans    %5d |\n") %n_highImpact;
    of << boost::format("| number of fillin mode scans:   %5d |\n") %n_fillin;
    of << boost::format("| number of calibrator scans:    %5d |\n") %n_calibrator;
    of << "'--------------------------------------'\n\n";

}

void Output::displayBaselineStatistics(ofstream &of) {
    unsigned long nsta = network_.getNSta();
    int n_bl = 0;
    for (const auto&any:scans_){
        unsigned long this_n_bl = any.getNObs();
        n_bl += this_n_bl;
    }

    of << "number of scheduled observations: " << n_bl << "\n";
    of << ".-----------";
    for (int i = 0; i < nsta-1; ++i) {
        of << "----------";
    }
    of << "-----------";
    of << "----------.\n";


    of << boost::format("| %8s |") % "STATIONS";
    for (const auto &any:network_.getStations()) {
        of << boost::format(" %8s ") % any.getName();
    }
    of << "|";
    of << boost::format(" %8s ") % "TOTAL";
    of << "|\n";

    of << "|----------|";
    for (int i = 0; i < nsta-1; ++i) {
        of << "----------";
    }
    of << "----------|";
    of << "----------|\n";

    for (unsigned long staid1 = 0; staid1 < nsta; ++staid1) {
        unsigned long counter = 0;
        of << boost::format("| %8s |") % network_.getStation(staid1).getName();
        for (unsigned long staid2 = 0; staid2 < nsta; ++staid2) {
            if (staid2<staid1+1){
                of << "          ";
            }else{
                unsigned long nBl = network_.getBaseline(staid1,staid2).getStatistics().scanStartTimes.size();
                of << boost::format(" %8d ") % nBl;
                counter += nBl;
            }
        }
        of << "|";
        of << boost::format(" %8d ") % counter;
        of << "|\n";
    }

    of << "'-----------";
    for (int i = 0; i < nsta-1; ++i) {
        of << "----------";
    }
    of << "-----------";
    of << "----------'\n\n";

}

void Output::displayStationStatistics(ofstream &of) {

    of << "number of scans per 15 minutes:\n";
    of << ".-------------------------------------------------------------"
           "----------------------------------------------------------------------------.\n";
    of << "|          time since session start (1 char equals 15 minutes)"
           "                                             | #SCANS #OBS |   OBS Time [s] |\n";
    of << "| STATION |0   1   2   3   4   5   6   7   8   9   10  11  12 "
           " 13  14  15  16  17  18  19  20  21  22  23  |             |   sum  average |\n";
    of << "|---------|+---+---+---+---+---+---+---+---+---+---+---+---+--"
           "-+---+---+---+---+---+---+---+---+---+---+---|-------------|----------------|\n";
    for (const auto &thisStation : network_.getStations()) {
        of << boost::format("| %8s|") % thisStation.getName();
        const Station::Statistics &stat = thisStation.getStatistics();
        const auto& time_sta = stat.scanStartTimes;
        unsigned int timeStart = 0;
        unsigned int timeEnd = 900;
        for (int j = 0; j < 96; ++j) {
            long c = count_if(time_sta.begin(), time_sta.end(), [timeEnd,timeStart](unsigned int k){
                return k>=timeStart && k<timeEnd ;
            });
            if(c==0){
                of << " ";
            }else if (c<=9){
                of << c;
            }else{
                of << "X";
            }

            timeEnd += 900;
            timeStart += 900;
        }
        of << boost::format("| %6d %4d ") % thisStation.getNTotalScans() % thisStation.getNObs();
        of << boost::format("| %5d %8.1f |\n") % thisStation.getStatistics().totalObservingTime
               % (static_cast<double>(thisStation.getStatistics().totalObservingTime) / static_cast<double>(thisStation.getNTotalScans()));
    }
    of << "'--------------------------------------------------------------"
           "---------------------------------------------------------------------------'\n\n";
}

void Output::displaySourceStatistics(ofstream &of) {
    of << "number of available sources:   " << sources_.size() << "\n";

    long number = count_if(sources_.begin(), sources_.end(), [](const Source &any){
        return any.getNTotalScans() > 0;
    });

    of << "number of scheduled sources:   " << number << "\n";
    of << "number of scans per 15 minutes:\n";
    of << ".-------------------------------------------------------------"
           "----------------------------------------------------------------------------.\n";
    of << "|          time since session start (1 char equals 15 minutes)"
           "                                             | #SCANS #OBS |   OBS Time [s] |\n";
    of << "|  SOURCE |0   1   2   3   4   5   6   7   8   9   10  11  12 "
           " 13  14  15  16  17  18  19  20  21  22  23  |             |   sum  average |\n";
    of << "|---------|+---+---+---+---+---+---+---+---+---+---+---+---+--"
           "-+---+---+---+---+---+---+---+---+---+---+---|-------------|----------------|\n";
    for (const auto &thisSource : sources_) {
        const Source::Statistics &stat = thisSource.getStatistics();
        const auto& time_sta = stat.scanStartTimes;

        if (thisSource.getNObs() == 0) {
            continue;
        }
        of << boost::format("| %8s|") % thisSource.getName();

        unsigned int timeStart = 0;
        unsigned int timeEnd = 900;
        for (int j = 0; j < 96; ++j) {
            long c = count_if(time_sta.begin(), time_sta.end(), [timeEnd,timeStart](unsigned int k){
                return k>=timeStart && k<timeEnd ;
            });
            if(c==0){
                of << " ";
            }else if (c<=9){
                of << c;
            }else{
                of << "X";
            }

            timeEnd += 900;
            timeStart += 900;
        }
        of << boost::format("| %6d %4d ") % thisSource.getNTotalScans() % thisSource.getNObs();
        of << boost::format("| %5d %8.1f |\n") % thisSource.getStatistics().totalObservingTime
               % (static_cast<double>(thisSource.getStatistics().totalObservingTime) / static_cast<double>(thisSource.getNTotalScans()));
    }
    of << "'--------------------------------------------------------------"
           "---------------------------------------------------------------------------'\n\n";
}

void Output::displayNstaStatistics(std::ofstream &of) {
    unsigned long nsta = network_.getNSta();
    vector<int> nstas(nsta+1,0);
    for(const auto &scan:scans_){
        ++nstas[scan.getNSta()];
    }

    unsigned long sum = scans_.size();

    of << "number of scans per number of participating stations:\n";
    for(int i=2; i<=nsta; ++i){
        of << boost::format("    number of %2d station scans: %4d (%6.2f %%)\n")%i %nstas[i] %(static_cast<double>(nstas[i])/
                                                                                                static_cast<double>(sum)*100);
    }
    of << "\n";
}


void Output::displayScanDurationStatistics(ofstream &of) {
    unsigned long nsta = network_.getNSta();
    of << "scan observing durations:\n";
    vector<vector<vector<unsigned int>>> bl_durations(nsta,vector<vector<unsigned int>>(nsta));
    vector< unsigned int> maxScanDurations;

    for(const auto&any: scans_){
        unsigned long nbl = any.getNObs();
        maxScanDurations.push_back(any.getTimes().getObservingDuration());

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

    of << "scan duration:\n";
    for (int i = 0; i < hist.size(); ++i) {
        of << boost::format("%3d-%3d | ") % bins[i] % (bins[i + 1] - 1);
        double percent = 100 * static_cast<double>(hist[i]) / static_cast<double>(maxScanDurations.size());
        percent = round(percent);
        for (int j = 0; j < percent; ++j) {
            of << "+";
        }
        of << "\n";
    }
    of << "\n";

    of << "required scan length:\n";
    of << ".----------------------------------------------------------------------------------.\n";
    of << "| S1-S2 |  min    10%    50%    90%    95%  97.5%    99%    max   |   sum  average |\n";
    of << "|-------|---------------------------------------------------------|----------------|\n";

    {
        auto n = static_cast<int>(maxScanDurations.size() - 1);
        sort(maxScanDurations.begin(),maxScanDurations.end());
        of << boost::format("|  ALL  | ");
        of << boost::format("%4d   ") % maxScanDurations[0];
        of << boost::format("%4d   ") % maxScanDurations[n * 0.1];
        of << boost::format("%4d   ") % maxScanDurations[n / 2];
        of << boost::format("%4d   ") % maxScanDurations[n * 0.9];
        of << boost::format("%4d   ") % maxScanDurations[n * 0.95];
        of << boost::format("%4d   ") % maxScanDurations[n * 0.975];
        of << boost::format("%4d   ") % maxScanDurations[n * 0.99];
        of << boost::format("%4d   ") % maxScanDurations[n];
        int sum = accumulate(maxScanDurations.begin(), maxScanDurations.end(), 0);
        double average = static_cast<double>(sum) / (n + 1);
        of << boost::format("| %5d %8.1f |") % sum % average;
        of << "\n";
    }

    of << "|-------|---------------------------------------------------------|----------------|\n";

    for (unsigned long i = 1; i < nsta; ++i) {
        for (unsigned long j = 0; j < i; ++j) {
            vector<unsigned int>& this_duration = bl_durations[i][j];
            if(this_duration.empty()){
                continue;
            }
            int n = (int) this_duration.size()-1;
            sort(this_duration.begin(),this_duration.end());
            of << boost::format("| %5s | ") % network_.getBaseline(i,j).getName();
            of << boost::format("%4d   ") % this_duration[0];
            of << boost::format("%4d   ") % this_duration[n * 0.1];
            of << boost::format("%4d   ") % this_duration[n / 2];
            of << boost::format("%4d   ") % this_duration[n * 0.9];
            of << boost::format("%4d   ") % this_duration[n * 0.95];
            of << boost::format("%4d   ") % this_duration[n * 0.975];
            of << boost::format("%4d   ") % this_duration[n * 0.99];
            of << boost::format("%4d   ") % this_duration[n];
            int sum = accumulate(this_duration.begin(), this_duration.end(), 0);
            double average = static_cast<double>(sum) / (n + 1);
            of << boost::format("| %5d %8.1f |") % sum % average;
            of << "\n";
        }

    }
    of << "'----------------------------------------------------------------------------------'\n\n";
}

void Output::displayTimeStatistics(std::ofstream &of) {

    unsigned long nstaTotal = network_.getNSta();

    of << ".-----------------";
    of << "------------";
    for (int i = 0; i < nstaTotal-1; ++i) {
        of << "----------";
    }
    of << "----------.\n";

    of << "|                 ";
    of << boost::format("| %8s |") % "average";
    for (const auto &station: network_.getStations()) {
        of << boost::format(" %8s ") % station.getName();
    }
    of << "|\n";

    of << "|-----------------";
    of << "|----------|";
    for (int i = 0; i < nstaTotal-1; ++i) {
        of << "----------";
    }
    of << "----------|\n";

    of << "| % obs. time:    ";
    vector<double> obsPer;
    for (const auto &station: network_.getStations()) {
        int t = station.getStatistics().totalObservingTime;
        obsPer.push_back(static_cast<double>(t)/static_cast<double>(TimeSystem::duration)*100);
    }
    of << boost::format("| %8.2f |") % (accumulate(obsPer.begin(),obsPer.end(),0.0)/(network_.getNSta()));
    for(auto p:obsPer){
        of << boost::format(" %8.2f ") % p;
    }
    of << "|\n";

    of << "| % preob time:   ";
    vector<double> preobPer;
    for (const auto &station: network_.getStations()) {
        int t = station.getStatistics().totalPreobTime;
        preobPer.push_back(static_cast<double>(t)/static_cast<double>(TimeSystem::duration)*100);
    }
    of << boost::format("| %8.2f |") % (accumulate(preobPer.begin(),preobPer.end(),0.0)/(network_.getNSta()));
    for(auto p:preobPer){
        of << boost::format(" %8.2f ") % p;
    }
    of << "|\n";

    of << "| % slew time:    ";
    vector<double> slewPer;
    for (const auto &station: network_.getStations()) {
        int t = station.getStatistics().totalSlewTime;
        slewPer.push_back(static_cast<double>(t)/static_cast<double>(TimeSystem::duration)*100);
    }
    of << boost::format("| %8.2f |") % (accumulate(slewPer.begin(),slewPer.end(),0.0)/(network_.getNSta()));
    for(auto p:slewPer){
        of << boost::format(" %8.2f ") % p;
    }
    of << "|\n";

    of << "| % idle time:    ";
    vector<double> idlePer;
    for (const auto &station: network_.getStations()) {
        int t = station.getStatistics().totalIdleTime;
        idlePer.push_back(static_cast<double>(t)/static_cast<double>(TimeSystem::duration)*100);
    }
    of << boost::format("| %8.2f |") % (accumulate(idlePer.begin(),idlePer.end(),0.0)/(network_.getNSta()));
    for(auto p:idlePer){
        of << boost::format(" %8.2f ") % p;
    }
    of << "|\n";

    of << "| % field system: ";
    vector<double> fieldPer;
    for (const auto &station: network_.getStations()) {
        int t = station.getStatistics().totalFieldSystemTime;
        fieldPer.push_back(static_cast<double>(t)/static_cast<double>(TimeSystem::duration)*100);
    }
    of << boost::format("| %8.2f |") % (accumulate(fieldPer.begin(),fieldPer.end(),0.0)/(network_.getNSta()));
    for(auto p:fieldPer){
        of << boost::format(" %8.2f ") % p;
    }
    of << "|\n";

    of << "|-----------------";
    of << "|----------|";
    for (int i = 0; i < nstaTotal-1; ++i) {
        of << "----------";
    }
    of << "----------|\n";

    of << "|   #scans:       ";
    vector<int> scans;
    for (const auto &station: network_.getStations()) {
        scans.push_back(station.getNTotalScans());
    }
    of << boost::format("| %8.2f |") % (accumulate(scans.begin(),scans.end(),0.0)/(network_.getNSta()));
    for(auto p:scans){
        of << boost::format(" %8.2f ") % p;
    }
    of << "|\n";

    of << "|   scans per h:  ";
    vector<double> scansPerH;
    for (const auto &station: network_.getStations()) {
        scansPerH.push_back(static_cast<double>(station.getNTotalScans())/(TimeSystem::duration/3600.));
    }
    of << boost::format("| %8.2f |") % (accumulate(scansPerH.begin(),scansPerH.end(),0.0)/(network_.getNSta()));
    for(auto p:scansPerH){
        of << boost::format(" %8.2f ") % p;
    }
    of << "|\n";

    of << "|-----------------";
    of << "|----------|";
    for (int i = 0; i < nstaTotal-1; ++i) {
        of << "----------";
    }
    of << "----------|\n";

    of << "|   Total TB:     ";
    unsigned int nChannels = 0;
    for(const auto &any: ObservationMode::nChannels){
        nChannels += any.second;
    }
    double obsFreq = ObservationMode::sampleRate * ObservationMode::bits * nChannels;

    vector<double> total_tb;
    for (const auto &station: network_.getStations()) {
        int t = station.getStatistics().totalObservingTime;

        total_tb.push_back(static_cast<double>(t) * obsFreq / (1024*1024*8) );
    }
    of << boost::format("| %8.2f |") % (accumulate(total_tb.begin(),total_tb.end(),0.0)/(network_.getNSta()));
    for(auto p:total_tb){
        of << boost::format(" %8.2f ") % p;
    }
    of << "|\n";

    of << "'-----------------";
    of << "------------";
    for (int i = 0; i < nstaTotal-1; ++i) {
        of << "----------";
    }
    of << "----------'\n\n";

}

void Output::displayAstronomicalParameters(std::ofstream &of) {
    of << ".------------------------------------------.\n";
    of << "| sun position:        | earth velocity:   |\n";
    of << "|----------------------|-------------------|\n";
    of << "| RA:   " << util::ra2dms(AstronomicalParameters::sun_radc[0]) << " " << boost::format("| x: %8.0f [m/s] |\n")%AstronomicalParameters::earth_velocity[0];
    of << "| DEC: "  << util::dc2hms(AstronomicalParameters::sun_radc[1]) << " " << boost::format("| y: %8.0f [m/s] |\n")%AstronomicalParameters::earth_velocity[1];
    of << "|                      " << boost::format("| z: %8.0f [m/s] |\n")%AstronomicalParameters::earth_velocity[2];
    of << "'------------------------------------------'\n\n";

    of << ".--------------------------------------------------------------------.\n";
    of << "| earth nutation:                                                    |\n";
    of << boost::format("| %=19s | %=14s %=14s %=14s |\n") %"time" %"X" %"Y" %"S";
    of << "|---------------------|----------------------------------------------|\n";
    for(int i=0; i<AstronomicalParameters::earth_nutTime.size(); ++i){
        of << boost::format("| %19s | %+14.6e %+14.6e %+14.6e |\n")
               % TimeSystem::ptime2string(TimeSystem::internalTime2PosixTime(AstronomicalParameters::earth_nutTime[i]))
               % AstronomicalParameters::earth_nutX[i]
               % AstronomicalParameters::earth_nutY[i]
               % AstronomicalParameters::earth_nutS[i];
    }
    of << "'--------------------------------------------------------------------'\n\n";

}

void Output::writeNGS() {

    string fname;
    if (version_ == 0) {
        fname = TimeSystem::date2string(TimeSystem::startTime).erase(0,2).append("VS_N000");
    } else {
        fname = (boost::format("%sVS_v%03d") % TimeSystem::date2string(TimeSystem::startTime).erase(0,2) % (version_)).str();
    }
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(info) << "writing empty NGS file to " << fname;
    #else
    cout << "[info] writing empty NGS file to " << fname;
    #endif
    ofstream of(path_+fname);


    boost::posix_time::ptime start = TimeSystem::startTime;
    unsigned long counter = 1;

    for (const auto &any: scans_) {
        for (int i = 0; i < any.getNObs(); ++i) {
            const Observation &obs = any.getObservation(i);
            string sta1 = network_.getStation(obs.getStaid1()).getName();
            string sta2 = network_.getStation(obs.getStaid2()).getName();
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

            of << boost::format("%-8s  %-8s  %-8s %4d %02d %02d %02d %02d  %13.10f            ") % sta1 % sta2 % src %
                   year % month % day % hour % minute % second;
            of << boost::format("%6d") % counter << "01\n";

            of << "    0000000.00000000    .00000  -000000.0000000000    .00000 0      I   ";
            of << boost::format("%6d") % counter << "02\n";

            of << "    .00000    .00000    .00000    .00000   0.000000000000000        0.  ";
            of << boost::format("%6d") % counter << "03\n";

            of << "       .00   .0       .00   .0       .00   .0       .00   .0            ";
            of << boost::format("%6d") % counter << "04\n";

            of << "   -.00000   -.00000    .00000    .00000    .00000    .00000            ";
            of << boost::format("%6d") % counter << "05\n";

            of << "     0.000    00.000   000.000   000.000    00.000    00.000 0 0        ";
            of << boost::format("%6d") % counter << "06\n";

            of << "        0.0000000000    .00000        -.0000000000    .00000  0         ";
            of << boost::format("%6d") % counter << "08\n";

            of << "          0.00000000    .00000        0.0000000000    .00000 0      I   ";
            of << boost::format("%6d") % counter << "09\n";

            ++counter;
        }
    }

    of.close();
}


void Output::writeVex(const SkdCatalogReader &skdCatalogReader) {
    string fileName = getName();
    fileName.append(".vex");
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(info) << "writing vex file to: " << fileName;
    #else
    cout << "[info] writing vex file to: " << fileName;
    #endif
    Vex vex(path_+fileName);
    vex.writeVex(network_,sources_,scans_,skdCatalogReader,xml_);
}

void Output::writeSkd(const SkdCatalogReader &skdCatalogReader) {
    string fileName = getName();
    fileName.append(".skd");
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(info) << "writing skd file to: " << fileName;
    #else
    cout << "[info] writing skd file to: " << fileName;
    #endif
    Skd skd(path_+fileName);
    skd.writeSkd(network_,sources_,scans_,skdCatalogReader,xml_);
}


void Output::writeStatisticsPerSourceGroup() {

    const auto & tmp0 = xml_.get_child_optional("master.source");

    if(tmp0.is_initialized()){
        boost::property_tree::ptree PARA_source = *tmp0;
        unordered_map<std::string, std::vector<std::string> > group_source = readGroups(PARA_source, GroupType::source);

        string expName = xml_.get("master.output.experimentName","schedule");
        string fileName = getName();
        fileName.append("_sourceStatistics.txt");
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(info) << "writing source statistics file to: " << fileName;
        #else
        cout << "[info] writing source statistics file to: " << fileName;
        #endif

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
                unsigned int duration = scan.getTimes().getObservingDuration(i);
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
                unsigned long srcid = src.getId();
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
        for (unsigned long ista=0; ista<network_.getNSta(); ++ista){
            const string name = network_.getStation(ista).getName();
            unsigned int seconds = xxxstps[ista];
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

            thisSta.calcAzEl_simple(source, p);

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

void Output::createAllOutputFiles(std::ofstream &of, const SkdCatalogReader &skdCatalogReader) {

    if(scans_.empty()){
        return;
    }

    writeStatistics(of);

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

    string fileName = getName();
    fileName.append("_operationsNotes.txt");
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(info) << "writing operationsNotes file to: " << fileName;
    #else
    cout << "[info] writing operationsNotes file to: " << fileName;
    #endif
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
        of << "    Version: " << version_ << " from multi scheduling setup:\nUsed multi scheduling parameters:\n";
        multiSchedulingParameters_->output(of);
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
    displayTimeStatistics(of);
    displayBaselineStatistics(of);
    displayStationStatistics(of);
    displaySourceStatistics(of);
    displayNstaStatistics(of);
    displayScanDurationStatistics(of);
    displayAstronomicalParameters(of);

    WeightFactors::summary(of);

    if(scans_.size()>=2){
        of << "Scans:\n";
        of << ".----------------------------------------------------------------------------------------------------------------.\n";
        for(unsigned long i=0; i<3; ++i){
            const auto &thisScan = scans_[i];
            thisScan.output(i,network_,sources_[thisScan.getSourceId()],of);
        }
        of << "...\n";
        for(unsigned long i= scans_.size() - 3; i < scans_.size(); ++i){
            const auto &thisScan = scans_[i];
            thisScan.output(i,network_,sources_[thisScan.getSourceId()],of);
        }
        of << "\n";
    }

    of << ".------------------------.\n";
    of << "| full scheduling setup: |\n";
    of << "'------------------------'\n";
    boost::property_tree::xml_parser::write_xml(of, xml_,
                                                boost::property_tree::xml_writer_make_settings<std::string>('\t', 1));
//    boost::property_tree::xml_parser::write_xml(of, xml_, boost::property_tree::xml_writer_make_settings<string>('\t', 1));
    of << "==================\n";
}

void Output::writeStatistics(std::ofstream &of) {
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
    oString.append(std::to_string(n_src)).append(",");

    of << oString << endl;

}



