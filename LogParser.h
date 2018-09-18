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


#ifndef LOGPARSER_H
#define LOGPARSER_H

#include <iostream>
#include <fstream>
#include <boost/date_time.hpp>
#include <boost/format.hpp>

#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#endif

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

        void parseLogFile(const std::string &slewStart, const std::string &slewEnd);

        void output(const std::string &outfile);

        bool addScheduledTimes(const std::vector<std::vector<unsigned int>> &times);

    private:
        static unsigned long nextId;

        bool addedScheduledTimes_ = false;

        std::string filename_;

        std::vector<LogScan> logScans_;

        boost::posix_time::ptime getTime(const std::string &line);
    };
}

#endif //VLBI_SCHEDULER_LOGPARSER_H
