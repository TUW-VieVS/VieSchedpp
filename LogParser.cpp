//
// Created by mschartn on 06.03.18.
//

#include "LogParser.h"

using namespace VieVS;
using namespace std;
unsigned long LogParser::nextId = 0;

LogParser::LogParser(const std::string &filename): VieVS_Object(nextId++), filename_{filename}{
}

void LogParser::parseLogFile(const string &slewStart, const string &slewEnd) {
    ifstream fid(filename_);
    if (!fid.is_open()) {
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(error) << "unable to open " << filename_ << " file";
        #else
        cout << "unable to open " << filename_ << " file";
        #endif
        return;
    } else {
        string line;
        string scanName = "scan_name=";
        string sourceName = "source=";
        string preob = "preob";
        string diskon = "disk_record=on";
        string diskoff = "disk_record=off";
        string postob = "postob";

        bool lookForNextWaitStamp = false;

        bool firstScan = true;

        LogScan thisScan = LogParser::LogScan();

        while (getline(fid, line)) {
            if(line.find(scanName) != line.npos){

                auto posOfcomma = line.find(',');
                thisScan.scanName = line.substr(31,posOfcomma-31);

                thisScan.preobStart = boost::none;
                thisScan.preobEnd = boost::none;
                thisScan.slewStart = boost::none;
                thisScan.slewEnd = boost::none;
                thisScan.recordOn = boost::none;
                thisScan.recordOff = boost::none;
                thisScan.sourceName = "";
                thisScan.realSlewTime = -1;
                thisScan.realPreobTime = -1;
                thisScan.realScanTime = -1;
                thisScan.realIdleTime = -1;

            }else if(line.find(sourceName) != line.npos){
                auto posOfcomma = line.find(',');
                thisScan.sourceName = line.substr(28,posOfcomma-28);

            }else if(line.find(slewStart) != line.npos){
                thisScan.slewStart = getTime(line);

            }else if(line.find(slewEnd) != line.npos){
                thisScan.slewEnd = getTime(line);

            }else if(line.find(preob) != line.npos) {
                thisScan.preobStart = getTime(line);
                lookForNextWaitStamp = true;
            }else if(lookForNextWaitStamp){
                if(line.at(21) == '!'){
                    thisScan.preobEnd = getTime(line);
                    lookForNextWaitStamp = false;
                }
            }else if(line.find(diskon) != line.npos){
                thisScan.recordOn = getTime(line);

            }else if(line.find(diskoff) != line.npos){
                thisScan.recordOff = getTime(line);

            }else if(line.find(postob) != line.npos){
                if(thisScan.slewStart.is_initialized() &&
                        thisScan.slewEnd.is_initialized() &&
                        thisScan.recordOn.is_initialized() &&
                        thisScan.recordOff.is_initialized() &&
                        thisScan.preobStart.is_initialized() &&
                        thisScan.preobEnd.is_initialized()){
                    boost::posix_time::time_duration diff_slew = *thisScan.slewEnd-*thisScan.slewStart;
                    if(firstScan){
                        thisScan.realPreobTime = 0;
                        thisScan.realSlewTime = 0;
                        thisScan.realIdleTime = 0;
                        firstScan = false;
                    }else{
                        thisScan.realSlewTime = diff_slew.total_seconds()+ static_cast<double>(diff_slew.fractional_seconds())/1.0e6;

                        boost::posix_time::time_duration diff_preob = *thisScan.preobEnd-*thisScan.preobStart;
                        thisScan.realPreobTime = diff_preob.total_seconds()+ static_cast<double>(diff_preob.fractional_seconds())/1.0e6;

                        boost::posix_time::time_duration diff_idle = *thisScan.preobStart-*thisScan.slewEnd;
                        thisScan.realIdleTime = diff_idle.total_seconds()+ static_cast<double>(diff_idle.fractional_seconds())/1.0e6;
                    }

                    boost::posix_time::time_duration diff_record = *thisScan.recordOff-*thisScan.recordOn;
                    thisScan.realScanTime = diff_record.total_seconds()+ static_cast<double>(diff_record.fractional_seconds())/1.0e6;

                    thisScan.error = false;
                }else{
                    thisScan.error = true;
                }
                logScans_.push_back(thisScan);
            }
        }
    }
}

boost::posix_time::ptime LogParser::getTime(const std::string &line) {

    auto year = boost::lexical_cast<unsigned short>(line.substr(0,4));
    auto doy = boost::lexical_cast<unsigned short>(line.substr(5,3));
    auto h = boost::lexical_cast<unsigned short>(line.substr(9,2));
    auto m = boost::lexical_cast<unsigned short>(line.substr(12,2));
    auto s = boost::lexical_cast<unsigned short>(line.substr(15,2));
    auto mus = boost::lexical_cast<unsigned short>(line.substr(18,2))*1.0e4;
    return {boost::gregorian::date(year,1,1)+boost::gregorian::days(doy-1),
            boost::posix_time::time_duration(h,m,s,mus)};
}

void LogParser::output(const std::string &outfile) {

    ofstream of(outfile);
    if(addedScheduledTimes_){
        of << "*                            slew     slew     slew    preob    preob    preob     scan     scan     scan     idle     idle     idle\n" <<
              "*     scan     source  E      log    sched      diff     log    sched     diff      log    sched     diff      log    sched     diff\n";
    }else{
        of << "scan, source, error, slewTime, preobTime, scanTime, idleTime,\n";
    }

    for(const auto &any:logScans_){
        of << boost::format("%10s, %9s, ")
              %any.scanName%any.sourceName;
        if(any.error){
            of << boost::format("1, ");
        }else{
            of << boost::format("0, ");

            of << boost::format("%7.2f, ")%any.realSlewTime;
            if(addedScheduledTimes_){
                of << boost::format("%7d, %7.2f, ")%any.scheduledSlewTime%(any.realSlewTime-any.scheduledSlewTime);
            }

            of << boost::format("%7.2f, ")%any.realPreobTime;
            if(addedScheduledTimes_){
                of << boost::format("%7d, %7.2f, ")%any.scheduledPreobTime%(any.realPreobTime-any.scheduledPreobTime);
            }

            of << boost::format("%7.2f, ")%any.realScanTime;
            if(addedScheduledTimes_){
                of << boost::format("%7d, %7.2f, ")%any.scheduledScanTime%(any.realScanTime-any.scheduledScanTime);
            }

            of << boost::format("%7.2f, ")%any.realIdleTime;
            if(addedScheduledTimes_){
                of << boost::format("%7d, %7.2f, ")%any.scheduledIdleTime%(any.realIdleTime-any.scheduledIdleTime);
            }
        }
        of << "\n";
    }
}

bool LogParser::addScheduledTimes(const std::vector<std::vector<unsigned int>> &times) {
    if(times.size() != logScans_.size()){
        return false;
    }else{
        for(int i=0; i< logScans_.size(); ++i){
            LogScan &thisLogScan = logScans_[i];
            thisLogScan.scheduledSlewTime = times[i][0];
            if(times[i][1] > numeric_limits<unsigned int>::max()){
                thisLogScan.scheduledIdleTime = numeric_limits<unsigned int>::max()-times[i][1];
            }else{
                thisLogScan.scheduledIdleTime = times[i][1];
            }

            thisLogScan.scheduledPreobTime = times[i][2];
            thisLogScan.scheduledScanTime = times[i][3];
        }
        addedScheduledTimes_ = true;
        return true;
    }
}
