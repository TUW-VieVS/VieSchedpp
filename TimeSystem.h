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
        static thread_local double mjdStart; ///< modified julian date of session start
        static thread_local boost::posix_time::ptime startTime; ///< session start time
        static thread_local boost::posix_time::ptime endTime; ///< session end time
        static unsigned int duration; ///< session duration in seconds

        /**
         * @brief transforms modified julian date to Greenwich mean sidereal time
         *
         * @param mjd modified julian date
         * @return Greenwich mean sidereal time
         */
        static double mjd2gmst(double mjd);

    };
}


#endif //VIEVS_TIMEEVENTS_H
