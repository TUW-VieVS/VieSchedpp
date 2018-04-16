//
// Created by mschartn on 22.08.17.
//

#include "Output.h"

#include <utility>

using namespace std;
using namespace VieVS;

Output::Output() = default;

Output::Output(Scheduler &sched, std::string path) : xml_{sched.xml_},
                                   stations_{std::move(sched.stations_)}, sources_{std::move(sched.sources_)},
                                   skyCoverages_{std::move(sched.skyCoverages_)}, scans_{std::move(sched.scans_)},
                                   path_{std::move(path)} {
}


void Output::writeStatistics(bool general, bool station, bool source, bool baseline, bool duration, bool time, ofstream& statisticsLog) {

    string fname = xml_.get("master.output.experimentName","schedule");
    if (iSched_ == 0) {
        fname.append("_skdsum.txt");
        string txt = (boost::format("writing statistics to: %s;\n") % fname).str();
        cout << txt;

    } else {
        fname.append((boost::format("V%03d_skdsum.txt") % (iSched_)).str());
        string txt = (boost::format("version %d: writing statistics to: %s;\n") %iSched_ % fname).str();
        cout << txt;
    }
    ofstream out(path_+fname);

    vector<int> statisticsVector{iSched_};

    if (general) {
        vector<int> v = displayGeneralStatistics(out);
        statisticsVector.insert(statisticsVector.end(),v.begin(),v.end());
    }

    if (baseline) {
        int v = displayBaselineStatistics(out);
        statisticsVector.push_back(v);
    }

    if (station) {
        vector<int> v = displayStationStatistics(out);
        statisticsVector.insert(statisticsVector.end(),v.begin(),v.end());
    }

    if (source) {
        int v = displaySourceStatistics(out);
        statisticsVector.push_back(v);
    }

    if (duration) {
        displayScanDurationStatistics(out);
    }

    if(time){
        displayTimeStatistics(out);
    }
    out.close();

    string oString = "";
    for(int any:statisticsVector){
        oString.append(std::to_string(any)).append(",");
    }
    oString.append("\n");

    statisticsLog << oString;
}

std::vector<int>  Output::displayGeneralStatistics(ofstream &out) {
    int n_scans = static_cast<int>(scans_.size());
    int n_single = 0;
    int n_subnetting = 0;
    int n_fillin = 0;
    int n_calibrator = 0;

    for (const auto&any:scans_){
        Scan::ScanType thisType = any.getType();
        switch (thisType){
            case Scan::ScanType::single:{
                ++n_single;
                break;
            }
            case Scan::ScanType::subnetting:{
                ++n_subnetting;
                break;
            }
            case Scan::ScanType::fillin:{
                ++n_fillin;
                break;
            }
            case Scan::ScanType::calibrator:{
                ++n_calibrator;
                break;
            }
        }
    }

    out << "number of total scans:         " << n_scans << "\n";
    out << "number of single source scans: " << n_single << "\n";
    out << "number of subnetting scans:    " << n_subnetting << "\n";
    out << "number of fillin mode scans:   " << n_fillin << "\n";
    out << "number of calibrator scans:    " << n_calibrator << "\n\n";

    return std::vector<int>{n_scans,n_single,n_subnetting,n_fillin,n_calibrator};
}

int Output::displayBaselineStatistics(ofstream &out) {
    unsigned long nsta = stations_.size();
    int n_bl = 0;
    vector< vector <unsigned long> > bl_storage(nsta,vector<unsigned long>(nsta,0));
    for (const auto&any:scans_){
        int this_n_bl = static_cast<int>(any.getNBl());
        n_bl += this_n_bl;

        for (int i = 0; i < this_n_bl; ++i) {
            int staid1 = any.getBaseline(i).getStaid1();
            int staid2 = any.getBaseline(i).getStaid2();
            if(staid1>staid2){
                swap(staid1,staid2);
            }
            ++bl_storage[staid1][staid2];
        }

    }

    out << "number of scheduled baselines: " << n_bl << "\n";
    out << ".-----------";
    for (int i = 0; i < nsta-1; ++i) {
        out << "----------";
    }
    out << "-----------";
    out << "----------.\n";


    out << boost::format("| %8s |") % "STATIONS";
    for (int i = 0; i < nsta; ++i) {
        out << boost::format(" %8s ") % stations_[i].getName();
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
        out << boost::format("| %8s |") % stations_[i].getName();
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

    return n_bl;
}

std::vector<int> Output::displayStationStatistics(ofstream &out) {
    vector<int> nscan_sta(stations_.size());
    vector< vector<int> > time_sta(stations_.size());
    vector< int> nbl_sta(stations_.size(),0);

    for (auto& any:scans_){
        for (int ista = 0; ista < any.getNSta(); ++ista) {
            const PointingVector& pv =  any.getPointingVector(ista);
            int id = pv.getStaid();
            ++nscan_sta[id];
            time_sta[id].push_back(any.getTimes().getScanStart());
        }
        for (int ibl = 0; ibl < any.getNBl(); ++ibl){
            const Baseline &bl = any.getBaseline(ibl);
            ++nbl_sta[bl.getStaid1()];
            ++nbl_sta[bl.getStaid2()];
        }

    }

    out
            << ".------------------------------------------------------------------------------------------------------------------------.\n";
    out
            << "|          time since session start (1 char equals 15 minutes)                                                           |\n";
    out
            << "| STATION |0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23  | #SCANS #OBS |\n";
    out
            << "|---------|+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---|-------------|\n";
    for (int i = 0; i<stations_.size(); ++i){
        out << boost::format("| %8s|") % stations_[i].getName();
        unsigned int timeStart = 0;
        unsigned int timeEnd = 900;
        for (int j = 0; j < 96; ++j) {
            if(any_of(time_sta[i].begin(), time_sta[i].end(), [timeEnd,timeStart](unsigned int k){return k<timeEnd && k>=timeStart;})){
                out << "x";
            }else{
                out << " ";
            }
            timeEnd += 900;
            timeStart += 900;
        }
        out << boost::format("| %6d %4d |\n") % nscan_sta[i] % nbl_sta[i];
    }

    out
            << "'------------------------------------------------------------------------------------------------------------------------'\n\n";

    vector<int> returnVec {static_cast<int>(stations_.size())};
    returnVec.insert(returnVec.end(),nscan_sta.begin(),nscan_sta.end());
    returnVec.insert(returnVec.end(),nbl_sta.begin(),nbl_sta.end());
    return returnVec;
}

int Output::displaySourceStatistics(ofstream &out) {
    out << "number of available sources:   " << sources_.size() << "\n";
    vector<unsigned int> nscan_src(sources_.size(),0);
    vector<unsigned int> nbl_src(sources_.size(),0);
    vector< vector<unsigned int> > time_src(sources_.size());

    for (const auto& any:scans_){
        int id = any.getSourceId();
        ++nscan_src[id];
        nbl_src[id] += any.getNBl();
        time_src[id].push_back(any.getTimes().getScanStart());
    }
    int number = static_cast<int>(count_if(nscan_src.begin(), nscan_src.end(), [](int i) {return i > 0;}));
    out << "number of scheduled sources:   " << number << "\n";
    out
            << ".------------------------------------------------------------------------------------------------------------------------.\n";
    out
            << "|          time since session start (1 char equals 15 minutes)                                                           |\n";
    out
            << "|  SOURCE |0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23  | #SCANS #OBS |\n";
    out
            << "|---------|+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---|-------------|\n";
    for (int i = 0; i<sources_.size(); ++i){
        if (sources_[i].getNbls() == 0) {
            continue;
        }
        out << boost::format("| %8s|") % sources_[i].getName();
        unsigned int timeStart = 0;
        unsigned int timeEnd = 900;
        for (int j = 0; j < 96; ++j) {
            if(any_of(time_src[i].begin(), time_src[i].end(), [timeEnd,timeStart](unsigned int k){return k<timeEnd && k>=timeStart;})){
                out << "x";
            }else{
                out << " ";
            }
            timeEnd += 900;
            timeStart += 900;
        }
        out << boost::format("| %6d %4d |\n") % nscan_src[i] % nbl_src[i];
    }
    out
            << "'------------------------------------------------------------------------------------------------------------------------'\n\n";
    return number;
}

void Output::displayScanDurationStatistics(ofstream &out) {
    unsigned long nsta = stations_.size();
    out << "required scan durations:\n";
    vector<vector<vector<unsigned int>>> bl_durations(nsta,vector<vector<unsigned int>>(nsta));
    vector< unsigned int> maxScanDurations;

    for(const auto&any: scans_){
        unsigned long nbl = any.getNBl();
        maxScanDurations.push_back(any.getTimes().getScanTime());

        for (int i = 0; i < nbl; ++i) {
            const Baseline &bl = any.getBaseline(i);
            int staid1 = bl.getStaid1();
            int staid2 = bl.getStaid2();
            unsigned int bl_duration = bl.getScanDuration();

            if(staid1<staid2){
                swap(staid1,staid2);
            }
            bl_durations[staid1][staid2].push_back(bl_duration);
        }

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
    out << ".----------------------------------------------------------------.\n";
    out << "| STATION1-STATION2 |  min    10%    50%    90%    max   average |\n";
    out << "|----------------------------------------------------------------|\n";

    {
        int n = maxScanDurations.size() - 1;
        sort(maxScanDurations.begin(),maxScanDurations.end());
        out << boost::format("|       SCANS       | ");
        out << boost::format("%4d   ") % maxScanDurations[0];
        out << boost::format("%4d   ") % maxScanDurations[n * 0.1];
        out << boost::format("%4d   ") % maxScanDurations[n / 2];
        out << boost::format("%4d   ") % maxScanDurations[n * 0.9];
        out << boost::format("%4d   ") % maxScanDurations[n];
        double average = accumulate(maxScanDurations.begin(), maxScanDurations.end(), 0.0) / (n + 1);
        out << boost::format("%7.2f |") % average;
        out << "\n";
    }

    out << "|----------------------------------------------------------------|\n";

    for (int i = 1; i < nsta; ++i) {
        for (int j = 0; j < i; ++j) {
            vector<unsigned int>& this_duration = bl_durations[i][j];
            if(this_duration.empty()){
                continue;
            }
            int n = (int) this_duration.size()-1;
            sort(this_duration.begin(),this_duration.end());
            out << boost::format("| %8s-%8s | ") % stations_[i].getName() % stations_[j].getName();
            out << boost::format("%4d   ") % this_duration[0];
            out << boost::format("%4d   ") % this_duration[n * 0.1];
            out << boost::format("%4d   ") % this_duration[n / 2];
            out << boost::format("%4d   ") % this_duration[n * 0.9];
            out << boost::format("%4d   ") % this_duration[n];
            double average = accumulate(this_duration.begin(), this_duration.end(), 0.0) / (n + 1);
            out << boost::format("%7.2f |") % average;
            out << "\n";
        }

    }
    out << "'----------------------------------------------------------------'\n\n";
}

void Output::displayTimeStatistics(std::ofstream &ofstream) {

    int nstaTotal = static_cast<int>(stations_.size());
    vector<int> scanTime(nstaTotal,0);
    vector<int> calibrationTime(nstaTotal,0);
//    vector<int> slewTime(nstaTotal,0);
//    vector<int> idleTime(nstaTotal,0);
    vector<int> fieldSystemTime(nstaTotal,0);
    vector<int> totalTime(nstaTotal,0);


    for(const auto&any: scans_){
        unsigned long nsta = any.getNSta();
        for (int i = 0; i < nsta; ++i) {

            int staid = any.getPointingVector(i).getStaid();

            int fieldSystem = any.getTimes().getFieldSystemTime(i);
            int preob = any.getTimes().getPreobTime(i);
            int scan = any.getTimes().getScanTime(i);

            fieldSystemTime[staid] += any.getTimes().getFieldSystemTime(i);
            calibrationTime[staid] += any.getTimes().getPreobTime(i);
            scanTime[staid] += any.getTimes().getScanTime(i);
            totalTime[staid] = any.getTimes().getScanEnd(i);
        }
    }

    ofstream << ".--------------------";
    for (int i = 0; i < nstaTotal-1; ++i) {
        ofstream << "----------";
    }
    ofstream << "----------.\n";

    ofstream << "|                   |";
    for (int i = 0; i < nstaTotal; ++i) {
        ofstream << boost::format(" %8s ") % stations_[i].getName();
    }
    ofstream << "|\n";

    ofstream << "|--------------------";
    for (int i = 0; i < nstaTotal-1; ++i) {
        ofstream << "----------";
    }
    ofstream << "----------|\n";

    ofstream << "| % obs. time:      |";
    for (int i = 0; i < nstaTotal; ++i) {
        ofstream << boost::format(" %8.2f ") % (static_cast<double>(scanTime[i])/static_cast<double>(totalTime[i])*100);
    }
    ofstream << "|\n";

    ofstream << "| % cal. time:      |";
    for (int i = 0; i < nstaTotal; ++i) {
        ofstream << boost::format(" %8.2f ") % (static_cast<double>(calibrationTime[i])/static_cast<double>(totalTime[i])*100);
    }
    ofstream << "|\n";

    ofstream << "| % slew+idle time: |";
    for (int i = 0; i < nstaTotal; ++i) {
        ofstream << boost::format(" %8.2f ") % (static_cast<double>(totalTime[i]-calibrationTime[i]-scanTime[i]-fieldSystemTime[i])/static_cast<double>(totalTime[i])*100);
    }
    ofstream << "|\n";

//    ofstream << "| % idle time:  |";
//    for (int i = 0; i < nstaTotal; ++i) {
//        ofstream << boost::format(" %8.1f ") % (static_cast<double>(idleTime[i])/static_cast<double>(totalTime[i])*100);
//    }
//    ofstream << "|\n";

    ofstream << "| % other time:     |";
    for (int i = 0; i < nstaTotal; ++i) {
        ofstream << boost::format(" %8.2f ") % (static_cast<double>(fieldSystemTime[i])/static_cast<double>(totalTime[i])*100);
    }
    ofstream << "|\n";

    ofstream << "'--------------------";
    for (int i = 0; i < nstaTotal-1; ++i) {
        ofstream << "----------";
    }
    ofstream << "----------'\n";

}


void Output::writeNGS() {

    string fname;
    if (iSched_ == 0) {
        fname = TimeSystem::date2string(TimeSystem::startTime).erase(0,2).append("MS");
        string txt = (boost::format("writing NGS file: %s;\n") % fname).str();
        cout << txt;
    } else {
        fname = (boost::format("%sMS_V%03d") % TimeSystem::date2string(TimeSystem::startTime).erase(0,2) % (iSched_)).str();
        string txt = (boost::format("version %d: writing empty ngs file: %s;\n") % iSched_ % fname).str();
        cout << txt;
    }
    ofstream out(path_+fname);


    boost::posix_time::ptime start = TimeSystem::startTime;
    unsigned long counter = 1;

    for (const auto &any: scans_) {
        for (int i = 0; i < any.getNBl(); ++i) {
            const Baseline &bl = any.getBaseline(i);
            string sta1 = stations_[bl.getStaid1()].getName();
            string sta2 = stations_[bl.getStaid2()].getName();
            if (sta1 > sta2) {
                swap(sta1, sta2);
            }
            string src = sources_[bl.getSrcid()].getName();
            unsigned int time = bl.getStartTime();

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
    string fileName = expName;
    if (iSched_ == 0) {
        fileName.append(".vex");
        string txt = (boost::format("writing vex file: %s;\n") % fileName).str();
        cout << txt;
    } else {
        fileName.append((boost::format("V%03d.vex") % (iSched_)).str());
        string txt = (boost::format("version %d: writing vex file: %s;\n") %iSched_ % fileName).str();
        cout << txt;
    }

    Vex vex(path_+fileName);
    vex.writeVex(stations_,sources_,scans_,skdCatalogReader,xml_);

}

void Output::writeSkd(const SkdCatalogReader &skdCatalogReader) {
    string expName = xml_.get("master.output.experimentName","schedule");
    string fileName = expName;
    if (iSched_ == 0) {
        fileName.append(".skd");
        string txt = (boost::format("writing skd file: %s;\n") % fileName).str();
        cout << txt;
    } else {
        fileName.append((boost::format("V%03d.skd") % (iSched_)).str());
        string txt = (boost::format("version %d: writing skd file: %s;\n") %iSched_ % fileName).str();
        cout << txt;
    }

    Skd skd(path_+fileName);
    skd.writeSkd(stations_,sources_,scans_,skdCatalogReader,xml_);
}


void Output::writeStatisticsPerSourceGroup() {
    boost::property_tree::ptree PARA_source = xml_.get_child("master.source");
    unordered_map<std::string, std::vector<std::string> > group_source = readGroups(PARA_source, GroupType::source);

    string expName = xml_.get("master.output.experimentName","schedule");
    string fileName = expName;
    if (iSched_ == 0) {
        fileName.append("_sourceStatistics.txt");
        string txt = (boost::format("writing source statistics file: %s;\n") % fileName).str();
        cout << txt;
    } else {
        fileName.append((boost::format("V%03d_sourceStatistics.txt") % (iSched_)).str());
        string txt = (boost::format("version %d: writing source statistics file: %s;\n") %iSched_ % fileName).str();
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
    ofstream dummy;
    for(auto &src:sources_){
        src.setNextEvent(0);
        src.checkForNewEvent(0, hardBreak, false, dummy);
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
    vector<vector<unsigned int> > scanTimePerStation(nsrc,vector<unsigned int>(stations_.size(),0));

    for(const auto &scan:scans_){
        int srcid = scan.getSourceId();
        scanTime[srcid].push_back(scan.getPointingVector(0).getTime());
        scanNsta[srcid].push_back(scan.getNSta());
        scanNbl[srcid].push_back(scan.getNBl());
        switch (scan.getType()){
            case Scan::ScanType::single: {
                flag[srcid].push_back(' ');
                break;
            }
            case Scan::ScanType::subnetting:{
                flag[srcid].push_back(' ');
                break;
            }
            case Scan::ScanType::fillin:{
                flag[srcid].push_back('*');
                break;
            }
            case Scan::ScanType::calibrator: {
                flag[srcid].push_back('#');
                break;
            }
        }
        for(int i=0; i<scan.getNSta(); ++i){
            int staid = scan.getPointingVector(i).getStaid();
            unsigned int duration = scan.getTimes().getScanTime(i);
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
        vector<unsigned int> groupScanTimePerStation(stations_.size(),0);
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

                for(int i=0; i<stations_.size(); ++i){
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

    vector<unsigned int> xxxstps(stations_.size(),0);
    for(const auto &stps: scanTimePerStation){
        for(int i=0; i<stations_.size(); ++i){
            xxxstps[i]+=stps[i];
        }
    }
    of << "* \n";
    of << "observing time per station:\n";
    for (int i=0; i<stations_.size(); ++i){
        const string name = stations_[i].getName();
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
        for (int i=0; i<stations_.size(); ++i){
            const string name = stations_[i].getName();
            unsigned int seconds = srcGrpStationScanTime[grpName][i];
            double percent = (static_cast<double>(seconds)/ static_cast<double>(TimeSystem::duration))*100;
            of << boost::format("    %8s: %8d [s]  (%5.1f [%%])\n") %name %seconds %percent;
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
            for(const auto&any:sources_){
                members.push_back(any.getName());
            }
            groups["__all__"] = members;
            break;
        }
        case GroupType::station:{
            std::vector<std::string> members;
            for(const auto&any:stations_){
                members.push_back(any.getName());
            }
            groups["__all__"] = members;
            break;
        }
        case GroupType::baseline:{
            std::vector<std::string> members;
            for(int i = 0; i<stations_.size(); ++i){
                for (int j = i+1; j<stations_.size(); ++j){
                    members.push_back(stations_[i].getName() + "-" + stations_[j].getName());
                }
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

    vector<int> reqSta = parameters.requiredStations;
    vector<int> ignSta = parameters.ignoreStations;

    for(unsigned int t = 0; t<TimeSystem::duration; t+=60){
        unsigned int visible = 0;

        bool requiredStationNotVisible = false;
        for(int staid = 0; staid<stations_.size(); ++staid){

            if(find(ignSta.begin(),ignSta.end(),staid) != ignSta.end()){
                continue;
            }

            PointingVector p(staid,source.getId());
            p.setTime(t);

            stations_[staid].calcAzEl(source, p, Station::AzelModel::simple);

            // check if source is up from station
            bool flag = stations_[staid].isVisible(p, source.getPARA().minElevation);
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

    if(xml_.get<bool>("master.output.createSummary",false)){
        writeStatistics(true, true, true, true, true, true, statisticsLog);
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
    string fileName = expName;
    if (iSched_ == 0) {
        fileName.append("_operationsNotes.txt");
        string txt = (boost::format("writing operationsNotes file: %s;\n") % fileName).str();
        cout << txt;
    } else {
        fileName.append((boost::format("V%03d_operationsNotes.txt") % (iSched_)).str());
        string txt = (boost::format("version %d: operationsNotes skd file: %s;\n") %iSched_ % fileName).str();
        cout << txt;
    }

    ofstream of(path_+fileName);

    of << "Session Notes for: " << expName << "\n";

    of << "    experiment description: " << xml_.get("master.output.experimentDescription","no description") << "\n";
    of << "    scheduler name        : " << xml_.get("master.created.name","unknown") << "\n";
    of << "    scheduler email       : " << xml_.get("master.created.email","unknown") << "\n";
    of << "    creation time (local) : " << xml_.get("master.created.time","unknown") << "\n";
    of << "    created with: new VieVS Scheduler\n\n";

    of << ".-------------------.\n";
    of << "| scheduling setup: |\n";
    of << "'-------------------'\n";
    boost::property_tree::xml_parser::write_xml(of, xml_, boost::property_tree::xml_writer_make_settings<string>('\t', 1));
    of << "==================\n";
}

