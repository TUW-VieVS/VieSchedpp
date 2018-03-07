//
// Created by mschartn on 06.03.18.
//

#include "LogParser.h"

using namespace VieVS;
using namespace std;

LogParser::LogParser(const std::string &filename):
        filename_{filename}{
}

void LogParser::parseLogFile() {
    ifstream fid(filename_);
    if (!fid.is_open()) {
        cerr << "ERROR: Unable to open " << filename_ << " file!;\n";
        return;
    } else {
        string line;
        string scanName = "scan_name=";
        string sourceName = "source=";
        string slewStart = "#flagr#flagr/antenna,new-source";
        string slewEnd = "#flagr#flagr/antenna,acquired";
        bool slewStartFound = false;
        bool slewEndFound = false;

        bool addScan = false;
        bool firstScan = true;

        LogScan thisScan = LogParser::LogScan();

        while (getline(fid, line)) {
            if(line.find(scanName) != line.npos){
                if(!slewStartFound || !slewEndFound){
                    thisScan.error = true;
                }else{
                    boost::posix_time::time_duration diff = thisScan.slewEnd-thisScan.slewStart;
                    if(firstScan){
                        thisScan.realSlewTime = 0;
                        firstScan = false;
                    }else{
                        thisScan.realSlewTime = diff.seconds()+ static_cast<double>(diff.fractional_seconds())/100000.0;
                    }
                    thisScan.error = false;
                }
                if(addScan){
                    logScans_.push_back(thisScan);
                }else{
                    addScan = true;
                }

                auto posOfcomma = line.find(',');
                thisScan.scanName = line.substr(31,posOfcomma-31);

                slewStartFound = false;
                slewEndFound = false;

            }else if(line.find(sourceName) != line.npos){
                auto posOfcomma = line.find(',');
                thisScan.sourceName = line.substr(28,posOfcomma-28);

            }else if(line.find(slewStart) != line.npos){
                slewStartFound = true;
                thisScan.slewStart = getTime(line);

            }else if(line.find(slewEnd) != line.npos){
                slewEndFound = true;
                thisScan.slewEnd = getTime(line);
            }
        }

        if(!slewStartFound || !slewEndFound){
            thisScan.error = true;
        }else{
            boost::posix_time::time_duration diff = thisScan.slewEnd-thisScan.slewStart;
            thisScan.realSlewTime = diff.seconds()+ static_cast<double>(diff.fractional_seconds())/100000.0;
            thisScan.error = false;
        }
        if(addScan){
            logScans_.push_back(thisScan);
        }else{
            addScan = true;
        }

    }
}

boost::posix_time::ptime LogParser::getTime(const std::string &line) {

    auto year = boost::lexical_cast<unsigned short>(line.substr(0,4));
    auto doy = boost::lexical_cast<unsigned short>(line.substr(5,3));
    auto h = boost::lexical_cast<unsigned short>(line.substr(9,2));
    auto m = boost::lexical_cast<unsigned short>(line.substr(12,2));
    auto s = boost::lexical_cast<unsigned short>(line.substr(15,2));
    auto ms = boost::lexical_cast<unsigned short>(line.substr(18,2))*10000;
    return {boost::gregorian::date(year,1,1)+boost::gregorian::days(doy-1),
            boost::posix_time::time_duration(h,m,s,ms)};
}

void LogParser::output(const std::string &outfile) {

    ofstream of(outfile);

    for(const auto &any:logScans_){
        if(any.error){
            of << boost::format("scan=%s,source=%s,ERROR\n")%any.scanName%any.sourceName;
        }else{
            of << boost::format("scan=%s,source=%s,slewTime=%.2f,")
                  %any.scanName%any.sourceName%any.realSlewTime;
            if(addedScheduledSlewTimes_){
                of << boost::format("scheduledSlewTime=%d,")%any.scheduledSlewTime;
            }
        }
        of << "\n";
    }
}

bool LogParser::addScheduledSlewTimes(const std::vector<unsigned int> &slewTimes) {
    if(slewTimes.size() != logScans_.size()){
        return false;
    }else{
        for(int i=0; i< logScans_.size(); ++i){
            LogScan &thisLogScan = logScans_[i];
            thisLogScan.scheduledSlewTime = slewTimes[i];
        }
        addedScheduledSlewTimes_ = true;
        return true;
    }
}
