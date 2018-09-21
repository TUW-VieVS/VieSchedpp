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
 * @file TimeSystem.h
 * @brief class TimeSystem
 *
 * @author Matthias Schartner
 * @date 22.08.2017
 */

#ifndef VIEVS_TIMEEVENTS_H
#define VIEVS_TIMEEVENTS_H

#include "Constants.h"

#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include "util.h"

namespace VieVS {
    /**
     * @class TimeSystem
     * @brief This class holds importang time information
     *
     * @author Matthias Schartner
     * @date 22.08.2017
     */
    class TimeSystem {
    public:
        static double mjdStart; ///< modified julian date of session start
        static boost::posix_time::ptime startTime; ///< session start time
        static boost::posix_time::ptime endTime; ///< session end time
        static unsigned int duration; ///< session duration in seconds

        /**
         * @brief transforms modified julian date to Greenwich mean sidereal time
         * @author Matthias Schartner
         *
         * @param mjd modified julian date
         * @return Greenwich mean sidereal time
         */
        static double mjd2gmst(double mjd);

        /**
         * @brief converts datetime to string
         * @author Matthias Schartner
         *
         * example output: 2018.01.01 00:00:00
         *
         * @param ptime target datetime
         * @return datetime string
         */
        static std::string ptime2string(const boost::posix_time::ptime &ptime);

        /**
         * @brief converts datetime to string
         * @author Matthias Schartner
         *
         * example output: 2018y01m01d00h00m00s
         *
         * @param ptime target datetime
         * @return datetime string
         */
        static std::string ptime2string_units(const boost::posix_time::ptime &ptime);

        /**
         * @brief converts datetime to string
         * @author Matthias Schartner
         *
         * instead of month and day the day of year is used
         * example output: 18001000000
         *
         * @param ptime target datetime
         * @return datetime string
         */
        static std::string ptime2string_doy(const boost::posix_time::ptime &ptime);

        /**
         * @brief convert string to datetime
         * @author Matthias Schartner
         *
         * input format: yydoyhhmmss or yyyydoyhhmmss
         * yy, yyyy: year
         * doy: day of year
         * hh: hour
         * mm: minute
         * ss: second
         *
         * @param input input string
         * @return datetime
         */
        static boost::posix_time::ptime string_doy2ptime(std::string input);

        /**
         * @brief converts datetime to string
         * @author Matthias Schartner
         *
         * instead of month and day the day of year is used
         * example output: 2018y001d00h00m00s
         *
         * @param ptime target datetime
         * @return datetime string
         */
        static std::string ptime2string_doy_units(const boost::posix_time::ptime &ptime);

        /**
         * @brief convert string to datetime
         * @author Matthias Schartner
         *
         * input format: yyyyMMddhhmmss
         * yyyy: year
         * MM: month
         * dd: day
         * hh: hour
         * mm: minute
         * ss: second
         *
         * @param input input string
         * @return datetime
         */
        static boost::posix_time::ptime string2ptime(std::string input);

        /**
         * @brief date to string
         * @author Matthias Schartner
         *
         * three character month string is used
         * example output: 18JAN01
         *
         * @param ptime target datetime
         * @return date string
         */
        static std::string date2string(const boost::posix_time::ptime &ptime);

        /**
         * @brief internal time format to datetime
         * @author Matthias Schartner
         *
         * @param time target internal time
         * @return datetime
         */
        static boost::posix_time::ptime internalTime2PosixTime(unsigned int time);

        /**
         * @brief internal time format to time
         * @author Matthias Schartner
         *
         * example output: 00:00:00
         *
         * @param time target internal time
         * @return time string
         */
        static std::string internalTime2timeString(unsigned int time);

        /**
         * @brief converts time to internal time format
         * @author Matthias Schartner
         *
         * @param ptime target datetime
         * @return internal time
         */
        static unsigned int posixTime2InternalTime(const boost::posix_time::ptime &ptime);
    };
}


#endif //VIEVS_TIMEEVENTS_H
