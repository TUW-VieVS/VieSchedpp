//
// Created by mschartn on 22.08.17.
//

#include <set>
#include "Output.h"
using namespace std;
using namespace VieVS;

Output::Output() = default;

Output::Output(Scheduler &sched, std::string path) : xml_{std::move(sched.xml_)},
                                   stations_{std::move(sched.stations_)}, sources_{std::move(sched.sources_)},
                                   skyCoverages_{std::move(sched.skyCoverages_)}, scans_{std::move(sched.scans_)},
                                   path{path} {
}


void Output::writeStatistics(bool general, bool station, bool source, bool baseline, bool duration) {

    string fname;
    if (iSched_ == 0) {
        fname = "statistics.txt";
        string txt = (boost::format("writing statistics to %s;\n") % fname).str();
        cout << txt;

    } else {
        fname = (boost::format("statistics_%04d.txt") % (iSched_)).str();
        string txt = (boost::format("version %4d: writing statistics to %s;\n") %iSched_ % fname).str();
        cout << txt;

    }
    ofstream out(path+fname);


    if (general) {
        displayGeneralStatistics(out);
    }

    if (station) {
        displayStationStatistics(out);
    }

    if (source) {
        displaySourceStatistics(out);
    }

    if (baseline) {
        displayBaselineStatistics(out);
    }

    if (duration) {
        displayScanDurationStatistics(out);
    }
    out.close();

}

void Output::displayGeneralStatistics(ofstream &out) {
    unsigned long n_scans = scans_.size();
    unsigned long n_single = 0;
    unsigned long n_subnetting = 0;
    unsigned long n_fillin = 0;

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
        }
    }

    out << "number of total scans:         " << n_scans << "\n";
    out << "number of single source scans: " << n_single << "\n";
    out << "number of subnetting scans:    " << n_subnetting << "\n";
    out << "number of fillin mode scans:   " << n_fillin << "\n\n";

}

void Output::displayBaselineStatistics(ofstream &out) {
    unsigned long nsta = stations_.size();
    unsigned long n_bl = 0;
    vector< vector <unsigned long> > bl_storage(nsta,vector<unsigned long>(nsta,0));
    for (const auto&any:scans_){
        unsigned long this_n_bl = any.getNBl();
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

}

void Output::displayStationStatistics(ofstream &out) {
    vector<unsigned int> nscan_sta(stations_.size());
    vector< vector<unsigned int> > time_sta(stations_.size());
    vector< unsigned int> nbl_sta(stations_.size(),0);

    for (auto& any:scans_){
        for (int ista = 0; ista < any.getNSta(); ++ista) {
            const PointingVector& pv =  any.getPointingVector(ista);
            int id = pv.getStaid();
            ++nscan_sta[id];
            time_sta[id].push_back(any.maxTime());
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
            << "|---------|------------------------------------------------------------------------------------------------|-------------|\n";
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

}

void Output::displaySourceStatistics(ofstream &out) {
    out << "number of available sources:   " << sources_.size() << "\n";
    vector<unsigned int> nscan_src(sources_.size(),0);
    vector<unsigned int> nbl_src(sources_.size(),0);
    vector< vector<unsigned int> > time_src(sources_.size());

    for (const auto& any:scans_){
        int id = any.getSourceId();
        ++nscan_src[id];
        nbl_src[id] += any.getNBl();
        time_src[id].push_back(any.maxTime());
    }
    long number = count_if(nscan_src.begin(), nscan_src.end(), [](int i) {return i > 0;});
    out << "number of scheduled sources:   " << number << "\n";
    out
            << ".------------------------------------------------------------------------------------------------------------------------.\n";
    out
            << "|          time since session start (1 char equals 15 minutes)                                                           |\n";
    out
            << "|  SOURCE |0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23  | #SCANS #OBS |\n";
    out
            << "|---------|------------------------------------------------------------------------------------------------|-------------|\n";
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

}

void Output::displayScanDurationStatistics(ofstream &out) {
    unsigned long nsta = stations_.size();
    out << "required scan durations:\n";
    vector< vector< vector< unsigned int > > > bl_durations(nsta,vector<vector<unsigned int> >(nsta));
    vector< unsigned int> maxScanDurations;

    for(const auto&any: scans_){
        unsigned long nbl = any.getNBl();
        unsigned int endScan = any.maxTime();
        unsigned int startScan = numeric_limits<unsigned int>::max();
        for (int i = 0; i < any.getNSta(); ++i) {
            unsigned int thisStart = any.getTimes().getEndOfCalibrationTime(i);
            if (thisStart<startScan){
                startScan = thisStart;
            }
        }

        maxScanDurations.push_back(endScan-startScan);

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

    out << "scan duration (without corsynch):\n";
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

void Output::writeNGS() {

    string fname;
    if (iSched_ == 0) {
        fname = "ngs.txt";
        string txt = (boost::format("writing NGS file %s;\n") % fname).str();
        cout << txt;
    } else {
        fname = (boost::format("ngs_%04d.txt") % (iSched_)).str();
        string txt = (boost::format("version %4d: writing NGS file %s;\n") % iSched_ % fname).str();
        cout << txt;
    }
    ofstream out(path+fname);


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

            boost::posix_time::ptime tmp = start + boost::posix_time::seconds(static_cast<long>(time));
            int year = tmp.date().year();
            int month = tmp.date().month();
            int day = tmp.date().day();
            int hour = tmp.time_of_day().hours();
            int minute = tmp.time_of_day().minutes();
            double second = tmp.time_of_day().seconds();

            out << boost::format("%8s  %8s  %8s %4d %02d %02d %02d %02d  %13.10f            ") % sta1 % sta2 % src %
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

void Output::writeSkd(const SkdCatalogReader &skdCatalogReader) {
    int x = 0;
    std::string fileName;
    if (iSched_ == 0) {
        fileName = "schedule.skd";
        string txt = (boost::format("writing SKD file %s;\n") % fileName).str();
        cout << txt;
    } else {
        fileName = (boost::format("schedule_%04d.skd") % (iSched_)).str();
        string txt = (boost::format("version %4d: writing SKD file %s;\n") % iSched_ % fileName).str();
        cout << txt;
    }


    ofstream of(path+fileName);

    of << "$EXPER " << xml_.get<string>("master.output.experimentName") << endl;
    skd_PARAM(skdCatalogReader, of);
    skd_OP(of);
    skd_DOWNTIME(of);
    skd_MAJOR(skdCatalogReader, of);
    skd_MINOR(of);
    skd_ASTROMETRIC(of);
    skd_BROADBAND(of);
    skd_CATALOG_USED(of);
    skd_CODES(skdCatalogReader, of);
    skd_STATIONS(skdCatalogReader, of);
    skd_STATWT(of);
    skd_SOURCES(skdCatalogReader, of);
    skd_SRCWT(of);
    skd_SKED(skdCatalogReader, of);
    skd_FLUX(skdCatalogReader, of);
}


void Output::skd_PARAM(const SkdCatalogReader &skdCatalogReader, ofstream &of) {
    of << "$PARAM\n";
    of << "DESCRIPTION " << xml_.get<string>("master.output.experimentDescription") << endl;
    of << "SCHEDULING_SoFTWARE NEW_VIE_SCHED\n";
    of << "SOFTWARE_VERSION NEW_VIE_SCHED 0.1\n";
    auto ctstr = xml_.get<string>("master.created.time");
    boost::posix_time::ptime ct = TimeSystem::string2ptime(ctstr);
    of << boost::format("SCHEDULE_CREATE_DATE %04d%03d%02d%02d%02d ") % ct.date().year() % ct.date().day_of_year() %
          ct.time_of_day().hours() % ct.time_of_day().minutes() % ct.time_of_day().seconds();
    of << "SCHEDULER " << xml_.get<string>("master.output.scheduler") << " ";
    of << "CORRELATOR " << xml_.get<string>("master.output.correlator") << " ";
    auto st = TimeSystem::startTime;
    of << boost::format("START %04d%03d%02d%02d%02d ") % st.date().year() % st.date().day_of_year() %
          st.time_of_day().hours() % st.time_of_day().minutes() % st.time_of_day().seconds();
    auto et = TimeSystem::endTime;
    of << boost::format("END %04d%03d%02d%02d%02d \n") % et.date().year() % et.date().day_of_year() %
          et.time_of_day().hours() % et.time_of_day().minutes() % et.time_of_day().seconds();

    of << boost::format("%-12s %4d ") % "CALIBRATION" % stations_[0].getWaittimes().calibration;
    of << boost::format("%-12s %4d ") % "CORSYNCH" % stations_[0].getWaittimes().corsynch;
    of << boost::format("%-12s %4d\n") % "DURATION" % 196;

    of << boost::format("%-12s %4d ") % "EARLY" % 0;
    of << boost::format("%-12s %4d ") % "IDLE" % 0;
    of << boost::format("%-12s %4d\n") % "LOOKAHEAD" % 0;

    of << boost::format("%-12s %4d ") % "MAXSCAN" % stations_[0].getPARA().maxScan;
    of << boost::format("%-12s %4d ") % "MINSCAN" % stations_[0].getPARA().minScan;
    of << boost::format("%-12s %4d\n") % "MINIMUM" % 0;

    of << boost::format("%-12s %4d ") % "MIDTP" % 0;
    of << boost::format("%-12s %4d ") % "MODULAR" % 1;
    of << boost::format("%-12s %4d ") % "MODSCAN" % 1;
    of << boost::format("%-12s %4d\n") % "PARITY" % 0;

    of << boost::format("%-12s %4d ") % "SETUP" % stations_[0].getWaittimes().setup;
    of << boost::format("%-12s %4d ") % "SOURCE" % stations_[0].getWaittimes().source;
    of << boost::format("%-12s %4d ") % "TAPETM" % stations_[0].getWaittimes().tape;
    of << boost::format("%-12s %4d\n") % "WIDTH" % 0;

    of << boost::format("%-12s %4s ") % "CONFIRM" % "Y";
    of << boost::format("%-12s %4s\n") % "VSCAN" % "Y";

    of << boost::format("%-12s %4s ") % "DEBUG" % "N";
    of << boost::format("%-12s %4s ") % "KEEP_LOG" % "N";
    of << boost::format("%-12s %4s\n") % "VERBOSE" % "N";

    of << boost::format("%-12s %4s ") % "PRFLAG" % "YYNN";
    of << boost::format("%-12s %4s\n") % "SNR" % "AUTO";

    of << "FREQUENCY   SX PREOB      PREOB  MIDOB     MIDOB  POSTOB     POSTOB\n";

    of << boost::format("%-12s %4.1d\n") % "ELEVATION _" % xml_.get<double>("master.general.minElevation");

    of << "TAPE_MOTION _ START&STOP\n";

    const std::map<std::string, std::string> &twoLetterCode = skdCatalogReader.getTwoLetterCode();

    string antennaCat = xml_.get<string>("master.catalogs.antenna");
    string equipCat = xml_.get<string>("master.catalogs.equip");

    const map<string, vector<string> > &ant = skdCatalogReader.getAntennaCatalog();
    const map<string, vector<string> > &equ = skdCatalogReader.getEquipCatalog();

    int counter = 0;
    for (const auto &any:stations_) {
        const string &staName = any.getName();
        vector<string> tmp = ant.at(staName);
        string id_EQ = boost::algorithm::to_upper_copy(tmp.at(14)) + "|" + staName;
        vector<string> tmp2 = equ.at(id_EQ);

        const string &equip = tmp2[tmp2.size() - 1];
        if (counter == 0) {
            of << "TAPE_TYPE ";
        }
        of << twoLetterCode.at(staName) << " " << equip << " ";
        ++counter;
        if (counter == 4) {
            of << "\n";
            counter = 0;
        }
    }
    if (counter != 0) {
        of << "\n";
    }

    counter = 0;
    for (const auto &any:stations_) {
        const string &staName = any.getName();

        if (counter == 0) {
            of << "TAPE_ALLOCATION ";
        }
        of << twoLetterCode.at(staName) << " AUTO ";
        ++counter;
        if (counter == 4) {
            of << "\n";
            counter = 0;
        }
    }
    if (counter != 0) {
        of << "\n";
    }

    counter = 0;
    for (int i = 0; i < stations_.size(); ++i) {
        const string &sta1 = stations_[i].getName();
        for (int j = i + 1; j < stations_.size(); ++j) {
            const string &sta2 = stations_[j].getName();

            if (counter == 0) {
                of << "SNR ";
            }
            double minSNR_X = Baseline::PARA.minSNR.at("X")[i][j];
            if (stations_[i].getPARA().minSNR.at("X") > minSNR_X) {
                minSNR_X = stations_[i].getPARA().minSNR.at("X");
            }
            if (stations_[j].getPARA().minSNR.at("X") > minSNR_X) {
                minSNR_X = stations_[j].getPARA().minSNR.at("X");
            }
            if (counter == 0) {
                of << "SNR ";
            }
            double minSNR_S = Baseline::PARA.minSNR.at("S")[i][j];
            if (stations_[i].getPARA().minSNR.at("S") > minSNR_S) {
                minSNR_S = stations_[i].getPARA().minSNR.at("S");
            }
            if (stations_[j].getPARA().minSNR.at("S") > minSNR_S) {
                minSNR_S = stations_[j].getPARA().minSNR.at("S");
            }
            of << boost::format(" %2s-%2s X %4d %2s-%2s S %4d ") % twoLetterCode.at(sta1) % twoLetterCode.at(sta2)
                  % minSNR_X % twoLetterCode.at(sta1) % twoLetterCode.at(sta2) % minSNR_S;
            ++counter;
            if (counter == 3) {
                of << "\n";
                counter = 0;
            }
        }
    }
}

void Output::skd_OP(std::ofstream &of) {
    of << "$OP\n";
}

void Output::skd_DOWNTIME(std::ofstream &of) {
    of << "$DOWNTIME\n";
}

void Output::skd_MAJOR(const SkdCatalogReader &skdCatalogReader, ofstream &of) {
    of << "$MAJOR\n";
    of << "Subnet ";
    const std::map<std::string, std::string> &twoLetterCode = skdCatalogReader.getTwoLetterCode();
    for (const auto &any:twoLetterCode) {
        of << any.second;
    }
    of << "\n";
    of << boost::format("%-14s %6s\n") % "SkyCov" % "No";
    of << boost::format("%-14s %6s\n") % "AllBlGood" % "Yes";
    of << boost::format("%-14s %6.2f\n") % "MaxAngle" % 180;
    of << boost::format("%-14s %6.2f\n") % "MinAngle" % 0;
    of << boost::format("%-14s %6d\n") % "MinBetween" % (*sources_[0].getPARA().minRepeat / 60);
    of << boost::format("%-14s %6d\n") % "MinSunDist" % 0;
    of << boost::format("%-14s %6d\n") % "MaxSlewTime" % *stations_[0].getPARA().maxSlewtime;
    of << boost::format("%-14s %6.2f\n") % "TimeWindow" % (SkyCoverage::maxInfluenceTime / 3600);
    of << boost::format("%-14s %6.2f\n") % "MinSubNetSize" % *sources_[0].getPARA().minNumberOfStations;
    if (xml_.get<bool>("master.general.subnetting")) {
        of << boost::format("%-14s %6d\n") % "NumSubNet" % 1;
    } else {
        of << boost::format("%-14s %6d\n") % "NumSubNet" % 2;
    }
    of << boost::format("%-14s %6d\n") % "Best" % 100;
    if (xml_.get<bool>("master.general.fillinmode")) {
        of << boost::format("%-14s %6s\n") % "FillIn" % "Yes";
    } else {
        of << boost::format("%-14s %6s\n") % "FillIn" % "No";
    }
    of << boost::format("%-14s %6d\n") % "FillMinSub" % *sources_[0].getPARA().minNumberOfStations;
    of << boost::format("%-14s %6d\n") % "FillMinTime" % 0;
    of << boost::format("%-14s %6d\n") % "FillBest" % 100;
    of << boost::format("%-14s %6.2f\n") % "Add_ps" % 0.00;
    of << boost::format("%-14s %6s\n") % "SNRWts" % "No";

}

void Output::skd_MINOR(std::ofstream &of) {
    of << "$MINOR\n";
    of << boost::format("%-14s %-3s %-3s %8.2f\n") % "Astro" % "No" % "Abs" % 0.00;
    of << boost::format("%-14s %-3s %-3s %8.2f\n") % "BegScan" % "No" % "Abs" % 0.00;
    of << boost::format("%-14s %-3s %-3s %8.2f\n") % "Covar" % "No" % "Abs" % 0.00;
    of << boost::format("%-14s %-3s %-3s %8.2f\n") % "EndScan" % "Yes" % "Abs" % WeightFactors::weightDuration;
    of << boost::format("%-14s %-3s %-3s %8.2f\n") % "LowDec" % "No" % "Abs" % 0.00;
    of << boost::format("%-14s %-3s %-3s %8.2f 0.00\n") % "NumLoEl" % "No" % "Abs" % 0.00;
    of << boost::format("%-14s %-3s %-3s %8.2f\n") % "NumRiseSet" % "No" % "Abs" % 0.00;
    of << boost::format("%-14s %-3s %-3s %8.2f\n") % "NumObs" % "Yes" % "Abs" %
          WeightFactors::weightNumberOfObservations;
    of << boost::format("%-14s %-3s %-3s %8.2f\n") % "SkyCov" % "Yes" % "Abs" % WeightFactors::weightSkyCoverage;
    of << boost::format("%-14s %-3s %-3s %8.2f EVN\n") % "SrcEvn" % "Yes" % "Abs" % WeightFactors::weightAverageSources;
    of << boost::format("%-14s %-3s %-3s %8.2f\n") % "SrcWt" % "Yes" % "Abs" % 1.00;
    of << boost::format("%-14s %-3s %-3s %8.2f EVN\n") % "StatEvn" % "Yes" % "Abs" %
          WeightFactors::weightAverageStations;
    of << boost::format("%-14s %-3s %-3s %8.2f\n") % "StatIdle" % "No" % "Abs" % 0.00;
    of << boost::format("%-14s %-3s %-3s %8.2f\n") % "StatWt" % "Yes" % "Abs" % 1.00;
    of << boost::format("%-14s %-3s %-3s %8.2f\n") % "TimeVar" % "No" % "Abs" % 0.00;
}

void Output::skd_ASTROMETRIC(std::ofstream &of) {
    of << "$ASTROMETRIC\n";
}

void Output::skd_STATWT(std::ofstream &of) {
    of << "$STATWT\n";
    for (const auto &any:stations_) {
        if(*any.getPARA().weight != 1) {
            of << boost::format("%-10s %6.2f\n") % any.getName() % *any.getPARA().weight;
        }
    }
}

void Output::skd_SRCWT(std::ofstream &of) {
    of << "$SRCWT\n";
    for (const auto &any:sources_) {
        if (any.getNTotalScans() > 0) {
            if(*any.getPARA().weight != 1){
                of << boost::format("%-10s %6.2f\n") % any.getName() % *any.getPARA().weight;
            }
        }
    }
}

void Output::skd_CATALOG_USED(std::ofstream &of) {
    of << "$CATALOG_USED\n";
    string source = xml_.get<string>("master.catalogs.source");
    string flux = xml_.get<string>("master.catalogs.flux");

    string antenna = xml_.get<string>("master.catalogs.antenna");
    string position = xml_.get<string>("master.catalogs.position");
    string equip = xml_.get<string>("master.catalogs.equip");
    string mask = xml_.get<string>("master.catalogs.mask");

    string modes = xml_.get<string>("master.catalogs.modes");;
    string freq = xml_.get<string>("master.catalogs.freq");;
    string rec = xml_.get<string>("master.catalogs.rec");;
    string rx = xml_.get<string>("master.catalogs.rx");;
    string loif = xml_.get<string>("master.catalogs.loif");;
    string tracks = xml_.get<string>("master.catalogs.tracks");;
    string hdpos = xml_.get<string>("master.catalogs.hdpos");;

    of << boost::format("%-10s %13s %s\n") % "SOURCE" % "UNKNOWN" % source;
    of << boost::format("%-10s %13s %s\n") % "FLUX" % "UNKNOWN" % flux;

    of << boost::format("%-10s %13s %s\n") % "ANTENNA" % "UNKNOWN" % antenna;
    of << boost::format("%-10s %13s %s\n") % "POSITION" % "UNKNOWN" % position;
    of << boost::format("%-10s %13s %s\n") % "EQUIP" % "UNKNOWN" % equip;
    of << boost::format("%-10s %13s %s\n") % "MASK" % "UNKNOWN" % mask;

    of << boost::format("%-10s %13s %s\n") % "MODES" % "UNKNOWN" % modes;
    of << boost::format("%-10s %13s %s\n") % "FREQ" % "UNKNOWN" % freq;
    of << boost::format("%-10s %13s %s\n") % "REC" % "UNKNOWN" % rec;
    of << boost::format("%-10s %13s %s\n") % "RX" % "UNKNOWN" % rx;
    of << boost::format("%-10s %13s %s\n") % "LOIF" % "UNKNOWN" % loif;
    of << boost::format("%-10s %13s %s\n") % "TRACKS" % "UNKNOWN" % tracks;
    of << boost::format("%-10s %13s %s\n") % "HDPOS" % "UNKNOWN" % hdpos;

}

void Output::skd_BROADBAND(ofstream &of) {
    of << "$BROADBAND\n";
}

void Output::skd_SOURCES(const SkdCatalogReader &skdCatalogReader, std::ofstream &of) {
    of << "$SOURCES\n";
    const map<string, vector<string> > &src = skdCatalogReader.getSourceCatalog();

    for (const auto &any:sources_) {
        if (any.getNTotalScans() > 0) {
            vector<string> tmp = src.at(any.getName());
            of << boost::format(" %-8s %-8s   %2s %2s %9s    %3s %2s %9s %6s %3s ")
                  % tmp[0] % tmp[1] % tmp[2] % tmp[3] % tmp[4] % tmp[5] % tmp[6] % tmp[7] % tmp[8] % tmp[9];
            for (int i = 10; i < tmp.size(); ++i) {
                of << tmp[i] << " ";
            }
            of << "\n";
        }
    }
}

void Output::skd_STATIONS(const SkdCatalogReader &skdCatalogReader, std::ofstream &of) {
    of << "$STATIONS\n";
    const map<string, vector<string> > &ant = skdCatalogReader.getAntennaCatalog();
    const map<string, vector<string> > &pos = skdCatalogReader.getPositionCatalog();
    const map<string, vector<string> > &equ = skdCatalogReader.getEquipCatalog();
    const map<string, vector<string> > &mas = skdCatalogReader.getMaskCatalog();

    map<string, string> posMap;
    map<string, string> equMap;
    map<string, string> masMap;

    for (const auto &any:stations_) {
        const string &staName = any.getName();

        vector<string> tmp = ant.at(staName);

        string id_PO = boost::algorithm::to_upper_copy(tmp.at(13));
        posMap[staName] = id_PO;
        string id_EQ = boost::algorithm::to_upper_copy(tmp.at(14)) + "|" + staName;
        equMap[staName] = id_EQ;
        string id_MS = boost::algorithm::to_upper_copy(tmp.at(15));
        masMap[staName] = id_MS;

        const auto &olc = skdCatalogReader.getOneLetterCode();

        of << boost::format("A %2s %-8s %4s %8s  %6s %3s %6s %6s %6s %3s %6s %6s %5s  %3s %3s %3s \n")
              % olc.at(tmp[1]) % tmp[1] % tmp[2] % tmp[3] % tmp[4] % tmp[5] % tmp[6] % tmp[7] % tmp[8] % tmp[9] %
              tmp[10] % tmp[11]
              % tmp[12] % tmp[13] % tmp[14] % tmp[15];
    }

    for (const auto &any:stations_) {
        vector<string> tmp = pos.at(posMap[any.getName()]);
        of << boost::format("P %2s %-8s %14s  %14s  %14s  %8s  %7s %6s ")
              % tmp[0] % tmp[1] % tmp[2] % tmp[3] % tmp[4] % tmp[5] % tmp[6] % tmp[7];
        for (int i = 8; i < tmp.size(); ++i) {
            of << tmp[i] << " ";
        }
        of << "\n";
    }

    for (const auto &any:stations_) {
        string staname = any.getName();
        vector<string> tmp = equ.at(equMap[staname]);
        of << boost::format("T %3s %8s  %7s %8s   %1s %5s  %1s %5s ")
              % tmp[1] % tmp[2] % tmp[3] % tmp[4] % tmp[5] % tmp[6] % tmp[7] % tmp[8];
        for (int i = 9; i < tmp.size(); ++i) {
            of << tmp[i] << " ";
        }
        of << "\n";
    }

    for (const auto &any:stations_) {
        string id = masMap[any.getName()];
        if (id != "--") {
            vector<string> tmp = mas.at(id);
            tmp.erase(next(tmp.begin(),1));
            if (!tmp.empty()) {

                for (const auto &any2:tmp) {
                    if (any2 != "-") {
                        of << any2 << " ";
                    }
                }
                of << "\n";
            }
        }
    }
}

void Output::skd_FLUX(const SkdCatalogReader &skdCatalogReader, std::ofstream &of) {
    of << "$FLUX\n";
    string fluxCat = xml_.get<string>("master.catalogs.flux");

    const map<string, vector<string> > &flu = skdCatalogReader.getFluxCatalog();
    for (const auto &any:sources_) {
        if (any.getNTotalScans() > 0) {
            const string &name = any.getName();
            vector<string> tmp = flu.at(name);

            for (const auto &any2:tmp) {
                of << any2 << "\n";
            }
        }
    }
}

void Output::skd_SKED(const SkdCatalogReader &skdCatalogReader, std::ofstream &of) {
    of << "$SKED\n";
    int preob = stations_[0].getWaittimes().calibration;

    const map<string, char> &olc = skdCatalogReader.getOneLetterCode();

    for (const auto &scan:scans_) {
        const string &srcName = sources_[scan.getSourceId()].getName();
        boost::posix_time::ptime start = TimeSystem::startTime + boost::posix_time::seconds(
                static_cast<long>(scan.getTimes().getEndOfCalibrationTime(0)));

        unsigned int scanTime = scan.getTimes().maxTime() - scan.getTimes().getEndOfCalibrationTime(0);

        of << boost::format("%-8s %3d 8F PREOB  %02d%03d%02d%02d%02d  %8d MIDOB         0 POSTOB ")
              % srcName % preob % (start.date().year() % 100) % start.date().day_of_year() % start.time_of_day().hours()
              % start.time_of_day().minutes() % start.time_of_day().seconds() % scanTime;

        for (int i = 0; i < scan.getNSta(); ++i) {
            const PointingVector &pv = scan.getPointingVector(i);
            const Station &thisSta = stations_[pv.getStaid()];
            const string &staName = thisSta.getName();
            of << olc.at(staName) << thisSta.getCableWrap().cableWrapFlag(pv);
        }
        of << " ";
        for (int i = 0; i < scan.getNSta(); ++i) {
            of << "1F000000 ";
        }
        of << "YYNN ";
        for (int i = 0; i < scan.getNSta(); ++i) {
            unsigned int thisScanTime =
                    scan.getTimes().getEndOfScanTime(i) - scan.getTimes().getEndOfCalibrationTime(i);
            of << boost::format("%5d ") % thisScanTime;
        }
        of << "\n";
    }
}

void Output::skd_CODES(const SkdCatalogReader &skd, std::ofstream &of) {
    of << "$CODES\n";
    unsigned long nchannels = skd.getChannelNumber2band().size();
    const std::map<std::string, char> &olc = skd.getOneLetterCode();

    for (const auto &trackId:skd.getTracksIds()) {

        //output first line!
        of << "F " << skd.getFreqName() << " " << skd.getFreqTwoLetterCode();
        for (const auto &any:skd.getStaName2tracksMap()) {
            if (any.second == trackId) {
                of << " " << any.first;
            }
        }
        of << "\n";

        //output C block
        for (int i = 1; i < nchannels + 1; ++i) {
            of << "C " << skd.getFreqTwoLetterCode() << " " << skd.getChannelNumber2band().at(i) << " "
               << skd.getChannelNumber2skyFreq().at(i) << " "
               << skd.getChannelNumber2phaseCalFrequency().at(i) << " " << boost::format("%2d") % skd.getChannelNumber2BBC().at(i) << " MK341:"
               << skd.getTracksId2fanoutMap().at(trackId) << boost::format("%6.2f") % skd.getBandWidth() << " "
               << skd.getChannelNumber2tracksMap().at(i) << "\n";
        }

    }
    for (const auto &sta:stations_){
        if(skd.getStaName2tracksMap().find(sta.getName()) == skd.getStaName2tracksMap().end()){
            cerr << "WARNING: skd output: F" << skd.getFreqName() << " " << skd.getFreqTwoLetterCode() << " " << sta.getName() << " MISSING in this mode!;\n";
            of << "*** F" << skd.getFreqName() << " " << skd.getFreqTwoLetterCode() << " " << sta.getName() << " MISSING in this mode! ***\n";
        }
    }

    of << "R " << skd.getFreqTwoLetterCode() << " " << skd.getSampleRate() << "\n";
    of << "B " << skd.getFreqTwoLetterCode() << "\n";

    for (const auto &staName:skd.getStaNames()) {
        const auto &loifId = skd.getStaName2loifId().at(staName);
        const vector<string> loif = skd.getLoifId2loifInfo().at(loifId);
        for (auto any:loif) {
            any = boost::algorithm::trim_copy(any);
            vector<string> splitVector;
            boost::split(splitVector, any, boost::is_space(), boost::token_compress_on);
            string nr = splitVector[1];
            string IF = splitVector[2];
            string band = splitVector[3];
            string freq = splitVector[4];
            string sideBand = splitVector[5];

            of << boost::format("L %c %2s %2s %8s %3s %s\n") % olc.at(staName) % band % IF % freq % nr % sideBand;
        }

    }

}

