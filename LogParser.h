//
// Created by mschartn on 06.03.18.
//

#ifndef LOGPARSER_H
#define LOGPARSER_H

#include <iostream>
#include <fstream>
#include <boost/date_time.hpp>
#include <boost/format.hpp>

namespace VieVS {
    class LogParser {
    public:
        struct LogScan{
            bool error = false;
            std::string scanName;
            std::string sourceName;
            boost::posix_time::ptime slewStart;
            boost::posix_time::ptime slewEnd;
            double realSlewTime;
            unsigned int scheduledSlewTime = 0;
        };

        LogParser() = default;

        explicit LogParser(const std::string &filename);

        void parseLogFile();

        void output(const std::string &outfile);

        bool addScheduledSlewTimes(const std::vector<unsigned int> &slewTimes);

    private:
        bool addedScheduledSlewTimes_ = false;

        std::string filename_;

        std::vector<LogScan> logScans_;

        boost::posix_time::ptime getTime(const std::string &line);
    };
}

#endif //VLBI_SCHEDULER_LOGPARSER_H
