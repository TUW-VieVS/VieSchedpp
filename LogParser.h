//
// Created by mschartn on 06.03.18.
//

#ifndef LOGPARSER_H
#define LOGPARSER_H

#include <iostream>
#include <fstream>
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include "TimeSystem.h"
#include "VieVS_Object.h"

namespace VieVS {
    class LogParser: public VieVS_Object {
    public:

        struct LogScan{
            bool error = false;
            std::string scanName;
            std::string sourceName;

            boost::optional<boost::posix_time::ptime> slewStart;
            boost::optional<boost::posix_time::ptime> slewEnd;

            boost::optional<boost::posix_time::ptime> preobStart;
            boost::optional<boost::posix_time::ptime> preobEnd;

            boost::optional<boost::posix_time::ptime> recordOn;
            boost::optional<boost::posix_time::ptime> recordOff;

            double realSlewTime = 0;
            double realPreobTime = 0;
            double realScanTime = 0;
            double realIdleTime = 0;

            int scheduledSlewTime = 0;
            int scheduledPreobTime = 0;
            int scheduledIdleTime = 0;
            int scheduledScanTime = 0;

        };

        explicit LogParser(const std::string &filename);

        void parseLogFile();

        void output(const std::string &outfile);

        bool addScheduledTimes(const std::vector<std::vector<unsigned int>> &times);

    private:
        static int nextId;

        bool addedScheduledTimes_ = false;

        std::string filename_;

        std::vector<LogScan> logScans_;

        boost::posix_time::ptime getTime(const std::string &line);
    };
}

#endif //VLBI_SCHEDULER_LOGPARSER_H
