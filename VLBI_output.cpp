//
// Created by mschartn on 22.08.17.
//

#include "VLBI_output.h"

namespace VieVS{
    VLBI_output::VLBI_output() {

    }

    VLBI_output::VLBI_output(const VLBI_scheduler &sched):
            stations{sched.getStations()}, sources{sched.getSources()}, skyCoverages{sched.getSkyCoverages()},
            scans{sched.getScans()}{

    }

    void VLBI_output::displayStatistics(bool general, bool station, bool source, bool baseline, bool duration) {
        cout << "\n=======================   starting statistics   =======================\n";

        if (general) {
            displayGeneralStatistics();
        }

        if (station) {
            displayStationStatistics();
        }

        if (source) {
            displaySourceStatistics();
        }

        if (baseline) {
            displayBaselineStatistics();
        }

        if (duration) {
            displayScanDurationStatistics();
        }

        cout << "=========================   end statistics    =========================\n";
    }

    void VLBI_output::displayGeneralStatistics() {
        unsigned long n_scans = scans.size();
        unsigned long n_single = 0;
        unsigned long n_subnetting = 0;
        unsigned long n_fillin = 0;

        unsigned long nsta = stations.size();

        for (const auto&any:scans){
            VLBI_scan::scanType thisType = any.getType();
            switch (thisType){
                case VLBI_scan::scanType::single:{
                    ++n_single;
                    break;
                }
                case VLBI_scan::scanType::subnetting:{
                    ++n_subnetting;
                    break;
                }
                case VLBI_scan::scanType::fillin:{
                    ++n_fillin;
                    break;
                }
            }
        }

        cout << "number of total scans:         " << n_scans << "\n";
        cout << "number of single source scans: " << n_single << "\n";
        cout << "number of subnetting scans:    " << n_subnetting << "\n";
        cout << "number of fillin mode scans:   " << n_fillin << "\n\n";

    }

    void VLBI_output::displayBaselineStatistics() {
        unsigned long nsta = stations.size();
        unsigned long n_bl = 0;
        vector< vector <unsigned long> > bl_storage(nsta,vector<unsigned long>(nsta,0));
        for (const auto&any:scans){
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

        cout << "number of scheduled baselines: " << n_bl << "\n";
        cout << ".–––––––––––";
        for (int i = 0; i < nsta-1; ++i) {
            cout << "––––––––––";
        }
        cout << "–––––––––––";
        cout << "––––––––––.\n";


        cout << boost::format("| %8s |") % "STATIONS";
        for (int i = 0; i < nsta; ++i) {
            cout << boost::format(" %8s ") % stations[i].getName();
        }
        cout << "|";
        cout << boost::format(" %8s ") % "TOTAL";
        cout << "|\n";

        cout << "|––––––––––|";
        for (int i = 0; i < nsta-1; ++i) {
            cout << "––––––––––";
        }
        cout << "––––––––––|";
        cout << "––––––––––|\n";

        for (int i = 0; i < nsta; ++i) {
            unsigned long counter = 0;
            cout << boost::format("| %8s |") % stations[i].getName();
            for (int j = 0; j < nsta; ++j) {
                if (j<i+1){
                    cout << "          ";
                    counter += bl_storage[j][i];
                }else{
                    cout << boost::format(" %8d ") % bl_storage[i][j];
                    counter += bl_storage[i][j];
                }
            }
            cout << "|";
            cout << boost::format(" %8d ") % counter;
            cout << "|\n";
        }

        cout << "'–––––––––––";
        for (int i = 0; i < nsta-1; ++i) {
            cout << "––––––––––";
        }
        cout << "–––––––––––";
        cout << "––––––––––'\n\n";

    }

    void VLBI_output::displayStationStatistics() {
        vector<unsigned int> nscan_sta(stations.size());
        vector< vector<unsigned int> > time_sta(stations.size());
        vector< unsigned int> nbl_sta(stations.size(),0);

        for (auto& any:scans){
            for (int ista = 0; ista < any.getNSta(); ++ista) {
                const VLBI_pointingVector& pv =  any.getPointingVector(ista);
                int id = pv.getStaid();
                ++nscan_sta[id];
                time_sta[id].push_back(any.maxTime());
            }
            for (int ibl = 0; ibl < any.getNBl(); ++ibl){
                const VLBI_baseline &bl = any.getBaseline(ibl);
                ++nbl_sta[bl.getStaid1()];
                ++nbl_sta[bl.getStaid2()];
            }

        }

        cout << ".––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––.\n";
        cout << "|          time since session start (1 char equals 15 minutes)                                                           |\n";
        cout << "| STATION |0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23  | #SCANS #OBS |\n";
        cout << "|–––––––––|––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––|–––––––––––––|\n";
        for (int i = 0; i<stations.size(); ++i){
            cout << boost::format("| %8s|") % stations[i].getName();
            unsigned int timeStart = 0;
            unsigned int timeEnd = 900;
            for (int j = 0; j < 96; ++j) {
                if(any_of(time_sta[i].begin(), time_sta[i].end(), [timeEnd,timeStart](unsigned int k){return k<timeEnd && k>=timeStart;})){
                    cout << "x";
                }else{
                    cout << " ";
                }
                timeEnd += 900;
                timeStart += 900;
            }
            cout << boost::format("| %6d %4d |\n") %nscan_sta[i] %nbl_sta[i];
        }

        cout << "'––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––'\n\n";

    }

    void VLBI_output::displaySourceStatistics() {
        cout << "number of available sources:   " << sources.size() << "\n";
        vector<unsigned int> nscan_src(sources.size(),0);
        vector<unsigned int> nbl_src(sources.size(),0);
        vector< vector<unsigned int> > time_src(sources.size());

        for (const auto& any:scans){
            int id = any.getSourceId();
            ++nscan_src[id];
            nbl_src[id] += any.getNBl();
            time_src[id].push_back(any.maxTime());
        }
        long number = count_if(nscan_src.begin(), nscan_src.end(), [](int i) {return i > 0;});
        cout << "number of scheduled sources:   " << number << "\n";
        cout << ".––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––.\n";
        cout << "|          time since session start (1 char equals 15 minutes)                                                           |\n";
        cout << "|  SOURCE |0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23  | #SCANS #OBS |\n";
        cout << "|–––––––––|––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––|–––––––––––––|\n";
        for (int i = 0; i<sources.size(); ++i){
            if (sources[i].getNbls() == 0) {
                continue;
            }
            cout << boost::format("| %8s|") % sources[i].getName();
            unsigned int timeStart = 0;
            unsigned int timeEnd = 900;
            for (int j = 0; j < 96; ++j) {
                if(any_of(time_src[i].begin(), time_src[i].end(), [timeEnd,timeStart](unsigned int k){return k<timeEnd && k>=timeStart;})){
                    cout << "x";
                }else{
                    cout << " ";
                }
                timeEnd += 900;
                timeStart += 900;
            }
            cout << boost::format("| %6d %4d |\n") %nscan_src[i] %nbl_src[i];
        }
        cout << "'––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––'\n\n";

    }

    void VLBI_output::displayScanDurationStatistics() {
        unsigned long nsta = stations.size();
        cout << "required scan durations:\n";
        vector< vector< vector< unsigned int > > > bl_durations(nsta,vector<vector<unsigned int> >(nsta));
        vector< unsigned int> maxScanDurations;

        for(const auto&any: scans){
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
                const VLBI_baseline &bl = any.getBaseline(i);
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

        cout << "scan duration (without corsynch):\n";
        for (int i = 0; i < hist.size(); ++i) {
            cout << boost::format("%3d-%3d | ") %bins[i] % (bins[i+1]-1);
            double percent = 100*(double)hist[i]/double(maxScanDurations.size());
            percent = round(percent);
            for (int j = 0; j < percent; ++j) {
                cout << "+";
            }
            cout << "\n";
        }
        cout << "\n";

        cout << "scan length:\n";
        cout << ".––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––.\n";
        cout << "| STATION1-STATION2 |  min    10%    50%    90%    max   average |\n";
        cout << "|––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––|\n";

        {
            int n = (int) maxScanDurations.size()-1;
            sort(maxScanDurations.begin(),maxScanDurations.end());
            cout << boost::format("|       SCANS       | ") ;
            cout << boost::format("%4d   ") % maxScanDurations[0];
            cout << boost::format("%4d   ") % maxScanDurations[n*0.1];
            cout << boost::format("%4d   ") % maxScanDurations[n/2];
            cout << boost::format("%4d   ") % maxScanDurations[n*0.9];
            cout << boost::format("%4d   ") % maxScanDurations[n];
            double average = (double)accumulate(maxScanDurations.begin(),maxScanDurations.end(),0)/(double)(n+1);
            cout << boost::format("%7.2f |") %average;
            cout << "\n";
        }

        cout << "|––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––|\n";

        for (int i = 1; i < nsta; ++i) {
            for (int j = 0; j < i; ++j) {
                vector<unsigned int>& this_duration = bl_durations[i][j];
                if(this_duration.empty()){
                    continue;
                }
                int n = (int) this_duration.size()-1;
                sort(this_duration.begin(),this_duration.end());
                cout << boost::format("| %8s-%8s | ") % stations[i].getName() % stations[j].getName();
                cout << boost::format("%4d   ") % this_duration[0];
                cout << boost::format("%4d   ") % this_duration[n*0.1];
                cout << boost::format("%4d   ") % this_duration[n/2];
                cout << boost::format("%4d   ") % this_duration[n*0.9];
                cout << boost::format("%4d   ") % this_duration[n];
                double average = (double)accumulate(this_duration.begin(),this_duration.end(),0)/(double)(n+1);
                cout << boost::format("%7.2f |") %average;
                cout << "\n";
            }

        }
        cout << "'––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––'\n\n";
    }

    void VLBI_output::writeNGS() {
        ofstream fid;
        fid.open("ngs", ios::out | ios::trunc);
        cout << "writing NGS file... ";

        boost::posix_time::ptime start = VieVS_time::startTime;
        unsigned long counter = 1;

        for (const auto &any: scans) {
            for (int i = 0; i < any.getNBl(); ++i) {
                const VLBI_baseline &bl = any.getBaseline(i);
                string sta1 = stations[bl.getStaid1()].getName();
                string sta2 = stations[bl.getStaid2()].getName();
                if (sta1 > sta2) {
                    swap(sta1, sta2);
                }
                string src = sources[bl.getSrcid()].getName();
                unsigned int time = bl.getStartTime();

                boost::posix_time::ptime tmp = start + boost::posix_time::seconds(time);
                int year = tmp.date().year();
                int month = tmp.date().month();
                int day = tmp.date().day();
                int hour = tmp.time_of_day().hours();
                int minute = tmp.time_of_day().minutes();
                double second = tmp.time_of_day().seconds();

                fid << boost::format("%8s  %8s  %8s %4d %02d %02d %02d %02d  %13.10f            ") % sta1 % sta2 % src %
                       year % month % day % hour % minute % second;
                fid << boost::format("%6d") % counter << "01\n";

                fid << "    0000000.00000000    .00000  -000000.0000000000    .00000 0      I   ";
                fid << boost::format("%6d") % counter << "02\n";

                fid << "    .00000    .00000    .00000    .00000   0.000000000000000        0.  ";
                fid << boost::format("%6d") % counter << "03\n";

                fid << "       .00   .0       .00   .0       .00   .0       .00   .0            ";
                fid << boost::format("%6d") % counter << "04\n";

                fid << "   -.00000   -.00000    .00000    .00000    .00000    .00000            ";
                fid << boost::format("%6d") % counter << "05\n";

                fid << "     0.000    00.000   000.000   000.000    00.000    00.000 0 0        ";
                fid << boost::format("%6d") % counter << "06\n";

                fid << "        0.0000000000    .00000        -.0000000000    .00000  0         ";
                fid << boost::format("%6d") % counter << "08\n";

                fid << "          0.00000000    .00000        0.0000000000    .00000 0      I   ";
                fid << boost::format("%6d") % counter << "09\n";

                ++counter;
            }
        }

        fid.close();
        cout << "done!\n";
    }

}
