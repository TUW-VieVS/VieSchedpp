/**
 * @file VieVS_time.h
 * @brief class VieVS_time
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
     * @class VieVS_time
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
         *
         * @param mjd modified julian date
         * @return Greenwich mean sidereal time
         */
        static double mjd2gmst(double mjd);

        static std::string ptime2string(const boost::posix_time::ptime &ptime);
        static std::string ptime2string_units(const boost::posix_time::ptime &ptime);
        static std::string ptime2string_doy(const boost::posix_time::ptime &ptime);
        static boost::posix_time::ptime string_doy2ptime(std::string);
        static std::string ptime2string_doy_units(const boost::posix_time::ptime &ptime);
        static boost::posix_time::ptime string2ptime(std::string);
        static std::string date2string(const boost::posix_time::ptime &ptime);

        static boost::posix_time::ptime internalTime2PosixTime(unsigned int time);
        static unsigned int posixTime2InternalTime(const boost::posix_time::ptime &ptime);
    };
}


#endif //VIEVS_TIMEEVENTS_H
