/**
 * @file VieVS_timeEvents.h
 * @brief class VieVS_timeEvents
 *
 *
 * @author Matthias Schartner
 * @date 22.08.2017
 */

#ifndef VIEVS_TIMEEVENTS_H
#define VIEVS_TIMEEVENTS_H

#include <boost/date_time.hpp>

namespace VieVS {
    class VieVS_time {
    public:
        static thread_local double mjdStart; ///< modified julian date of session start
        static thread_local boost::posix_time::ptime startTime; ///< session start time
        static thread_local boost::posix_time::ptime endTime; ///< session end time
        static unsigned int duration; ///< session duration in seconds
    };
}


#endif //VIEVS_TIMEEVENTS_H
