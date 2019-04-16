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
 * @file ScanTimes.h
 * @brief class ScanTimes
 *
 * @author Matthias Schartner
 * @date 14.07.2017
 */

#ifndef SCANTIMES_H
#define SCANTIMES_H


#include <limits>
#include <vector>
#include "../Misc/VieVS_Object.h"
#include "../Misc/util.h"
#include "PointingVector.h"


namespace VieVS {

/**
 * @class ScanTimes
 * @brief representation of the VLBI scan time
 *
 * all times are stored in seconds since session start
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */
class ScanTimes : public VieVS_Object {
   public:
    /**
     * @brief observation time alignment anchor
     * @author Matthias Schartner
     */
    enum class AlignmentAnchor {
        start,      ///< align start time
        end,        ///< align end time
        individual  ///< do not align observations
    };


    /**
     * @brief set observation time alignment anchor
     * @author Matthias Schartner
     *
     * @param newAnchor alignment anchor
     */
    static void setAlignmentAnchor( AlignmentAnchor newAnchor ) { anchor = newAnchor; }


    /**
     * @brief get alignment anchor
     * @author Matthias Schartner
     *
     * @return alignemnt anchor
     */
    static AlignmentAnchor getAlignmentAnchor() { return anchor; }


    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param nsta number of stations
     */
    explicit ScanTimes( unsigned int nsta );


    /**
     * @brief sets the endtime of the last scan
     * @author Matthias Schartner
     *
     * this is simultaneously the start time of this new scan
     *
     * @param endOfLastScan end time of last scan in seconds since session start
     */
    void setEndOfLastScan( const std::vector<unsigned int> &endOfLastScan ) noexcept {
        ScanTimes::endOfLastScan_ = endOfLastScan;
    }


    /**
     * @brief update times after fillin mode scan
     * @author Matthias Schartner
     *
     * @param idx index
     * @param endOfLastScan end of last scan
     * @param fieldSystem field system time in seconds
     * @param slewTime slew time in seconds
     * @param preobTime preob time in seconds
     * @return true if times are valid, otherwise false
     */
    bool updateAfterFillinmode( int idx, unsigned int endOfLastScan, unsigned int fieldSystem, unsigned int slewTime,
                                unsigned int preobTime ) noexcept {
        endOfLastScan_[idx] = endOfLastScan;
        endOfFieldSystemTime_[idx] = endOfLastScan_[idx] + fieldSystem;
        unsigned int slewEnd = endOfFieldSystemTime_[idx] + slewTime;
        bool valid = setPreobTime( idx, preobTime );

        if ( slewEnd <= endOfIdleTime_[idx] ) {
            endOfSlewTime_[idx] = slewEnd;
            return valid;

        } else {
            // 1 sec tolerance
            valid = valid && slewEnd - endOfIdleTime_[idx] == 1;

            endOfSlewTime_[idx] = endOfIdleTime_[idx];

            return valid;
        }
    }


    /**
     * @brief adds the times for an element
     * @author Matthias Schartner
     *
     * @param idx index of which element should be updated
     * @param fieldSystem field system time in seconds
     * @param slew slew time in seconds
     * @param preob calibration time in seconds
     */
    void addTimes( int idx, unsigned int fieldSystem, unsigned int slew, unsigned int preob ) noexcept;


    /**
     * @brief set new id
     * @author Matthias Schartner
     */
    void giveNewId() { setId( nextId++ ); }


    /**
     * @brief get end of slew times
     * @author Matthias Schartner
     *
     * @return end of slew times for each station
     */
    const std::vector<unsigned int> getEndOfSlewTimes() const noexcept { return endOfSlewTime_; }


    /**
     * @brief removes an element
     *
     * @param idx index of the element that should be removed
     */
    void removeElement( int idx ) noexcept;


    /**
     * @brief get field system duration
     * @author Matthias Schartner
     *
     * @param idx index
     * @return field system duration
     */
    const unsigned int getFieldSystemDuration( int idx ) const noexcept {
        return endOfFieldSystemTime_[idx] - endOfLastScan_[idx];
    }


    /**
     * @brief get slew duration
     * @author Matthias Schartner
     *
     * @param idx index
     * @return slew duration
     */
    const unsigned int getSlewDuration( int idx ) const noexcept {
        return endOfSlewTime_[idx] - endOfFieldSystemTime_[idx];
    }


    /**
     * @brief get idle duration
     * @author Matthias Schartner
     *
     * @param idx index
     * @return idle duration
     */
    const unsigned int getIdleDuration( int idx ) const noexcept { return endOfIdleTime_[idx] - endOfSlewTime_[idx]; }


    /**
     * @brief get preob duration
     * @author Matthias Schartner
     *
     * @param idx index
     * @return preob duration
     */
    const unsigned int getPreobDuration( int idx ) const noexcept { return endOfPreobTime_[idx] - endOfIdleTime_[idx]; }


    /**
     * @brief get observing duration
     * @author Matthias Schartner
     *
     * @param idx index
     * @return observing duration
     */
    const unsigned int getObservingDuration( int idx ) const noexcept {
        return endOfObservingTime_[idx] - endOfPreobTime_[idx];
    }


    /**
     * @brief get observing duration between two stations
     * @author Matthias Schartner
     *
     * @param idx1 index of first station
     * @param idx2 index of second station
     * @return observing duration between two stations
     */
    const unsigned int getObservingDuration( unsigned long idx1, unsigned long idx2 ) const noexcept {
        unsigned int start = std::max( {endOfPreobTime_[idx1], endOfPreobTime_[idx2]} );
        unsigned int end = std::min( {endOfObservingTime_[idx1], endOfObservingTime_[idx2]} );
        if ( start > end ) {
            return 0;
        } else {
            return end - start;
        }
    }


    /**
     * @brief get total observing duration
     * @author Matthias Schartner
     *
     * @return total observing duration
     */
    const unsigned int getObservingDuration() const noexcept {
        return getObservingTime( Timestamp::end ) - getObservingTime( Timestamp::start );
    }


    /**
     * @brief get total scan duration
     * @author Matthias Schartner
     *
     * @return total scan duration
     */
    const unsigned int getScanDuration() const noexcept {
        return getScanTime( Timestamp::end ) - getScanTime( Timestamp::start );
    }


    /**
     * @brief get field system time
     * @author Matthias Schartner
     *
     * @param idx index
     * @param ts time stamp
     * @return field system time
     */
    const unsigned int getFieldSystemTime( int idx, Timestamp ts ) const noexcept {
        switch ( ts ) {
            case Timestamp::start: {
                return endOfLastScan_[idx];
            }
            case Timestamp::end: {
                return endOfFieldSystemTime_[idx];
            }
        }
    }


    /**
     * @brief get slew time
     * @author Matthias Schartner
     *
     * @param idx index
     * @param ts time stamp
     * @return slew time
     */
    const unsigned int getSlewTime( int idx, Timestamp ts ) const noexcept {
        switch ( ts ) {
            case Timestamp::start: {
                return endOfFieldSystemTime_[idx];
            }
            case Timestamp::end: {
                return endOfSlewTime_[idx];
            }
        }
    }


    /**
     * @brief get idle time
     * @author Matthias Schartner
     *
     * @param idx index
     * @param ts time stamp
     * @return idle time
     */
    const unsigned int getIdleTime( int idx, Timestamp ts ) const noexcept {
        switch ( ts ) {
            case Timestamp::start: {
                return endOfSlewTime_[idx];
            }
            case Timestamp::end: {
                return endOfIdleTime_[idx];
            }
        }
    }


    /**
     * @brief get preob time
     * @author Matthias Schartner
     *
     * @param idx index
     * @param ts time stamp
     * @return preob time
     */
    const unsigned int getPreobTime( int idx, Timestamp ts ) const noexcept {
        switch ( ts ) {
            case Timestamp::start: {
                return endOfIdleTime_[idx];
            }
            case Timestamp::end: {
                return endOfPreobTime_[idx];
            }
        }
    }


    /**
     * @brief get observing time
     * @author Matthias Schartner
     *
     * @param idx index
     * @param ts time stamp
     * @return observing time
     */
    const unsigned int getObservingTime( int idx, Timestamp ts ) const noexcept {
        switch ( ts ) {
            case Timestamp::start: {
                return endOfPreobTime_[idx];
            }
            case Timestamp::end: {
                return endOfObservingTime_[idx];
            }
        }
    }


    /**
     * @brief get observing time between two stations
     * @author Matthias Schartner
     *
     * @param idx1 index of first station
     * @param idx2 index of second station
     * @param ts time stamp
     * @return observing time
     */
    const unsigned int getObservingTime( int idx1, int idx2, Timestamp ts ) const noexcept {
        switch ( ts ) {
            case Timestamp::start: {
                return std::max( {endOfPreobTime_[idx1], endOfPreobTime_[idx2]} );
            }
            case Timestamp::end: {
                return std::min( {endOfObservingTime_[idx1], endOfObservingTime_[idx2]} );
            }
        }
    }


    /**
     * @brief get total observing time
     * @author Matthias Schartner
     *
     * @param ts time stamp
     * @return total observing time
     */
    const unsigned int getObservingTime( Timestamp ts ) const noexcept {
        switch ( ts ) {
            case Timestamp::start: {
                return *min_element( endOfPreobTime_.begin(), endOfPreobTime_.end() );
            }
            case Timestamp::end: {
                return *max_element( endOfObservingTime_.begin(), endOfObservingTime_.end() );
            }
        }
    }


    /**
     * @brief get total scan time
     * @author Matthias Schartner
     *
     * @param ts time stamp
     * @return total scan time
     */
    const unsigned int getScanTime( Timestamp ts ) const noexcept {
        switch ( ts ) {
            case Timestamp::start: {
                return *min_element( endOfLastScan_.begin(), endOfLastScan_.end() );
            }
            case Timestamp::end: {
                return *max_element( endOfObservingTime_.begin(), endOfObservingTime_.end() );
            }
        }
    }


    /**
     * @brief align start times
     * @author Matthias Schartner
     */
    void alignStartTimes() noexcept;


    /**
     * @brief set observing start time
     * @author Matthias Schartner
     *
     * @param scanStart start time
     */
    void setObservingStarts( unsigned int scanStart ) noexcept;


    /**
     * @brief updates the slewtime of one element
     * @author Matthias Schartner
     *
     * @param idx index of the element whichs slewtime is chanded
     * @param new_slewtime new slew time in seconds
     */
    void setSlewTime( int idx, unsigned int new_slewtime ) noexcept;


    /**
     * @brief adds scan times to each element
     * @author Matthias Schartner
     *
     * Order of the scan times must be the same as the order of the element
     *
     * @param scanTimes all scan times in seconds
     */
    void setObservingTimes( const std::vector<unsigned int> &scanTimes ) noexcept;


    /**
     * @brief set observing times
     * @author Matthias Schartner
     *
     * @param scanTimes new observing times
     */
    void setObservingTimes( unsigned int scanTimes ) noexcept;


    /**
     * @brief set observing time
     * @author Matthias Schartner
     *
     * @param idx index
     * @param time new observing time
     * @param ts time stamp
     */
    void setObservingTime( int idx, unsigned int time, Timestamp ts );


    /**
     * @brief reduce observing time
     * @author Matthias Schartner
     *
     * @param idx index
     * @param maxObsTime maximum observing time
     * @param ts time stamp
     * @return true if time was reduced, otherwise false
     */
    bool reduceObservingTime( int idx, unsigned int maxObsTime, Timestamp ts );


    /**
     * @brief add tagalong station times
     * @author Matthias Schartner
     *
     * @param pv_start start pointing vector
     * @param pv_end end pointing vector
     * @param slewtime slew time in seconds
     * @param currentTime current time of antenna
     * @param fieldSystem field system time in seconds
     * @param preob calibration time in seconds
     */
    void addTagalongStationTime( const VieVS::PointingVector &pv_start, const VieVS::PointingVector &pv_end,
                                 unsigned int slewtime, unsigned int currentTime, unsigned int fieldSystem,
                                 unsigned int preob );


    /**
     * @brief set preob time for all stations
     * @author Matthias Schartner
     *
     * @param preob preob time
     * @return true if value was valid, otherwise false
     */
    bool setPreobTime( const std::vector<unsigned int> &preob );


    /**
     * @brief set preob time for one station
     * @author Matthias Schartner
     *
     * @param idx index
     * @param preob preob time
     * @return true if value was valid, otherwise false
     */
    bool setPreobTime( int idx, unsigned int preob );


    /**
     * @brief remove unnecessary observing time
     * @author Matthias Schartner
     *
     * @param ts time stamp
     * @return index where observing time was removed
     */
    int removeUnnecessaryObservingTime( Timestamp ts );


   private:
    static unsigned long nextId;    ///< next id for this object type
    static AlignmentAnchor anchor;  ///< scan alignment anchor

    std::vector<unsigned int> endOfLastScan_;         ///< end of last scan
    std::vector<unsigned int> endOfFieldSystemTime_;  ///< end of setup time
    std::vector<unsigned int> endOfSlewTime_;         ///< end of slew time
    std::vector<unsigned int> endOfIdleTime_;         ///< end of idle time
    std::vector<unsigned int> endOfPreobTime_;        ///< end of preob time
    std::vector<unsigned int> endOfObservingTime_;    ///< end of scan time

    // TODO: implement endOfPostobTime_
    std::vector<unsigned int> endOfPostobTime_;  ///< end of postob time

    /**
     * @brief remove idle times
     * @author Matthias Schartner
     */
    void removeIdleTime();
};
}  // namespace VieVS

#endif  // SCANTIMES_H
