/**
 * @file ScanTimes.h
 * @brief class ScanTimes
 *
 *
 * @author Matthias Schartner
 * @date 14.07.2017
 */

#ifndef SCANTIMES_H
#define SCANTIMES_H

#include <vector>
#include <limits>

#include "util.h"
#include "PointingVector.h"
#include "VieVS_Object.h"

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
    class ScanTimes: public VieVS_Object {
    public:
        enum class AlignmentAnchor{
            start,
            end,
            individual
        };

        /**
         * @brief constructor
         * @param nsta number of stations
         */
        explicit ScanTimes(unsigned int nsta);

        /**
         * @brief sets the endtime of the last scan
         *
         * this is simulaneouly the start time of this new scan
         *
         * @param endOfLastScan end time of last scan in seconds since session start
         */
        void setEndOfLastScan(const std::vector<unsigned int> &endOfLastScan) noexcept {
            ScanTimes::endOfLastScan_ = endOfLastScan;
        }

        /**
         * @brief adds the times for an element
         *
         * @param idx index of which element should be updated
         * @param setup setup time in seconds
         * @param source source time in seconds
         * @param slew slew time in seconds
         * @param tape tape time in seconds
         * @param calib calibration time in seconds
         */
        void addTimes(int idx, unsigned int fieldSystem, unsigned int slew, unsigned int preob) noexcept;

        void giveNewId(){
            setId(nextId++);
        }

        /**
         * @brief removes an element
         *
         * @param idx index of the element that should be removed
         */
        void removeElement(int idx) noexcept;

        /**
         * @brief getter for all end of slew times
         *
         * @return all end of slew times in seconds since session start
         */
        const std::vector<unsigned int> &getEndOfSlewTime() const noexcept {
            return endOfSlewTime_;
        }

        const unsigned int getSlewStart(int idx) const noexcept{
            return endOfFieldSystemTime_[idx];
        }

        const unsigned int getSlewEnd(int idx) const noexcept{
            return endOfSlewTime_[idx];
        }

        const unsigned int getFieldSystemTime(int idx) const noexcept{
            return endOfFieldSystemTime_[idx]-endOfLastScan_[idx];
        }
        const unsigned int getSlewTime(int idx) const noexcept{
            return endOfSlewTime_[idx]-endOfFieldSystemTime_[idx];
        }
        const unsigned int getIdleTime(int idx) const noexcept{
            return endOfIdleTime_[idx]-endOfSlewTime_[idx];
        }
        const unsigned int getPreobTime(int idx) const noexcept{
            return endOfPreobTime_[idx]-endOfIdleTime_[idx];
        }

        const unsigned int getScanStart() const noexcept;

        const unsigned int getScanEnd() const noexcept;

        const unsigned int getScanDuration() const noexcept{
            return getScanEnd() - getScanStart();
        }

        const unsigned int getObservingTime() const noexcept;

        const unsigned int getObservingTime(int idx) const noexcept{
            return endOfObservingTime_[idx]-endOfPreobTime_[idx];
        }

        const unsigned int getObservingStart() const noexcept;

        const unsigned int getObservingStart(int idx) const noexcept{
            if(idx>endOfPreobTime_.size()){
                double x = 0;
            }
            return endOfPreobTime_[idx];
        }

        const unsigned int getObservingEnd() const noexcept;

        const unsigned int getObservingEnd(int idx) const noexcept{
            return endOfObservingTime_[idx];
        }



        /**
         * @brief calculates the earliest possible start time for this scan
         *
         * Idle times are introduced
         */
        void alignStartTimes() noexcept;

        void setStartTime(unsigned int scanStart) noexcept ;

        /**
         * @brief updates the slewtime of one element
         *
         * @param idx index of the element whichs slewtime is chanded
         * @param new_slewtime new slew time in seconds
         */
        void updateSlewtime(int idx, unsigned int new_slewtime) noexcept;

        /**
         * @brief adds scan times to each element
         *
         * Order of the scan times must be the same as the order of the element
         *
         * @param scanTimes all scan times in seconds
         */
        void addObservingTimes(const std::vector<unsigned int> &scanTimes) noexcept;


        void addObservingTime(unsigned int scanTimes) noexcept;


        void addTagalongStation(const VieVS::PointingVector &pv_start, const VieVS::PointingVector &pv_end,
                                unsigned int slewtime, unsigned int currentTime, unsigned int fieldSystem,
                                unsigned int preob);

        bool substractPreobTimeFromStartTime(unsigned int preob);

        static void setAlignmentAnchor(AlignmentAnchor newAnchor){
            anchor = newAnchor;
        }

        static AlignmentAnchor getAlignmentAnchor(){
            return anchor;
        }


        void setObservationEnd(int idx, unsigned int time);

        void setObservationStart(int idx, unsigned int time);

        int removeUnnecessaryObservingTimeEnd();

        int removeUnnecessaryObservingTimeStart();

        bool reduceObservingTimeEnd(int idx, unsigned int maxObsTime);

        bool reduceObservingTimeStart(int idx, unsigned int minObsTime);

    private:
        static unsigned long nextId;
        static AlignmentAnchor anchor;

        std::vector<unsigned int> endOfLastScan_; ///< end of last scan
        std::vector<unsigned int> endOfFieldSystemTime_; ///< end of setup time
        std::vector<unsigned int> endOfSlewTime_; ///< end of slew time
        std::vector<unsigned int> endOfIdleTime_; ///< end of idle time
        std::vector<unsigned int> endOfPreobTime_; ///< end of preob time
        std::vector<unsigned int> endOfObservingTime_; ///< end of scan time

        //TODO: implement endOfPostobTime_
        std::vector<unsigned int> endOfPostobTime_; ///< end of postob time

        void removeIdleTime();
    };
}

#endif //SCANTIMES_H
