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

/**
 * @file LogParser.h
 * @brief class LogParser
 *
 * @author Matthias Schartner
 * @date 07.03.2018
 */

#ifndef LOGPARSER_H
#define LOGPARSER_H

#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <iostream>

#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#endif

#include "../Misc/TimeSystem.h"
#include "../Misc/VieVS_Object.h"

namespace VieVS {

/**
 * @class LogParser
 * @brief parser for station log files
 *
 * @author Matthias Schartner
 * @date 07.03.2018
 */
class LogParser : public VieVS_Object {
   public:
    /**
     * @brief station log file scan
     * @author Matthias Schartner
     */
    struct LogScan {
        bool error = false;      ///< flag if error occured
        std::string scanName;    ///< scan name
        std::string sourceName;  ///< source name

        boost::optional<boost::posix_time::ptime> slewStart;  ///< slew time start
        boost::optional<boost::posix_time::ptime> slewEnd;    ///< slew time end

        boost::optional<boost::posix_time::ptime> preobStart;  ///< preob start
        boost::optional<boost::posix_time::ptime> preobEnd;    ///< preob end

        boost::optional<boost::posix_time::ptime> recordOn;   ///< recording start
        boost::optional<boost::posix_time::ptime> recordOff;  ///< recording off

        double realSlewTime = 0;   ///< actual slew time
        double realPreobTime = 0;  ///< actual preob time
        double realScanTime = 0;   ///< actual scan time
        double realIdleTime = 0;   ///< actual idle time

        int scheduledSlewTime = 0;   ///< scheduled slew time
        int scheduledPreobTime = 0;  ///< scheduled preob time
        int scheduledIdleTime = 0;   ///< scheduled idle time
        int scheduledScanTime = 0;   ///< scheduled scan time
    };

    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param filename path to log file
     */
    explicit LogParser( const std::string &filename );

    /**
     * @brief parse log file
     * @author Matthias Schartner
     *
     * @param slewStart regular expression for slew start
     * @param slewEnd regular expression for slew end
     */
    void parseLogFile( const std::string &slewStart, const std::string &slewEnd );

    /**
     * @brief output parser to text file
     * @author Matthias Schartner
     *
     * @param outfile output text file name
     */
    void output( const std::string &outfile );

    /**
     * @brief add scheduled times for comparison
     * @author Matthias Schartner
     *
     * content of vector:
     * Entry for each scan with
     * - scheduled slew time
     * - scheduled idle time
     * - scheduled preob time
     * - scheduled scan time
     *
     * number of elements in times parameter must be equal to number of scans in log file
     *
     * @param times scheduled times
     * @return true if scheduled times can be compared to log file
     */
    bool addScheduledTimes( const std::vector<std::vector<unsigned int>> &times );

   private:
    static unsigned long nextId;  ///< next id for this object type

    bool addedScheduledTimes_ = false;  ///< flag if scheduled times are added

    std::string filename_;  ///< file name to log file

    std::vector<LogScan> logScans_;  ///< list of scans found in log file

    boost::posix_time::ptime getTime( const std::string &line );  ///< get time from log file line
};
}  // namespace VieVS

#endif  // VLBI_SCHEDULER_LOGPARSER_H
