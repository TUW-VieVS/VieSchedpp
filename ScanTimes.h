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

        static void setAlignmentAnchor(AlignmentAnchor newAnchor){
            anchor = newAnchor;
        }

        static AlignmentAnchor getAlignmentAnchor(){
            return anchor;
        }


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

        bool updateAfterFillinmode(int idx, unsigned int endOfLastScan, unsigned int fieldSystem,
                                   unsigned int slewTime, unsigned int preobTime) noexcept {

            endOfLastScan_[idx] = endOfLastScan;
            endOfFieldSystemTime_[idx] = endOfLastScan_[idx]+fieldSystem;
            unsigned int slewEnd = endOfFieldSystemTime_[idx]+slewTime;
            bool valid = setPreobTime(idx, preobTime);

            if(slewEnd <= endOfIdleTime_[idx]){
                endOfSlewTime_[idx] = slewEnd;
                return valid;

            }else{
                // 1 sec tolerance
                valid = valid && slewEnd - endOfIdleTime_[idx] == 1;

                endOfSlewTime_[idx] = endOfIdleTime_[idx];

                return valid;
            }

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

        const std::vector<unsigned int> getEndOfSlewTimes() const noexcept{
            return endOfSlewTime_;
        }

        /**
         * @brief removes an element
         *
         * @param idx index of the element that should be removed
         */
        void removeElement(int idx) noexcept;

        const unsigned int getFieldSystemDuration(int idx) const noexcept{
            return endOfFieldSystemTime_[idx]-endOfLastScan_[idx];
        }
        const unsigned int getSlewDuration(int idx) const noexcept{
            return endOfSlewTime_[idx]-endOfFieldSystemTime_[idx];
        }
        const unsigned int getIdleDuration(int idx) const noexcept{
            return endOfIdleTime_[idx]-endOfSlewTime_[idx];
        }
        const unsigned int getPreobDuration(int idx) const noexcept{
            return endOfPreobTime_[idx]-endOfIdleTime_[idx];
        }
        const unsigned int getObservingDuration(int idx) const noexcept{
            return endOfObservingTime_[idx]-endOfPreobTime_[idx];
        }

        const unsigned int getObservingDuration() const noexcept{
            return getObservingTime(Timestamp::end)- getObservingTime(Timestamp::start);
        }
        const unsigned int getScanDuration() const noexcept{
            return getScanTime(Timestamp::end) - getScanTime(Timestamp::start);
        }

        const unsigned int getFieldSystemTime(int idx, Timestamp ts) const noexcept {
            switch (ts){
                case Timestamp::start:{
                    return endOfLastScan_[idx];
                }
                case Timestamp::end:{
                    return endOfFieldSystemTime_[idx];
                }
            }
        }

        const unsigned int getSlewTime(int idx, Timestamp ts) const noexcept {
            switch (ts){
                case Timestamp::start:{
                    return endOfFieldSystemTime_[idx];
                }
                case Timestamp::end:{
                    return endOfSlewTime_[idx];
                }
            }
        }

        const unsigned int getIdleTime(int idx, Timestamp ts) const noexcept {
            switch (ts){
                case Timestamp::start:{
                    return endOfSlewTime_[idx];
                }
                case Timestamp::end:{
                    return endOfIdleTime_[idx];
                }
            }
        }

        const unsigned int getPreobTime(int idx, Timestamp ts) const noexcept {
            switch (ts){
                case Timestamp::start:{
                    return endOfIdleTime_[idx];
                }
                case Timestamp::end:{
                    return endOfPreobTime_[idx];
                }
            }
        }

        const unsigned int getObservingTime(int idx, Timestamp ts) const noexcept {
            switch (ts){
                case Timestamp::start:{
                    return endOfPreobTime_[idx];
                }
                case Timestamp::end:{
                    return endOfObservingTime_[idx];
                }
            }
        }

        const unsigned int getObservingTime(Timestamp ts) const noexcept{
            switch (ts){
                case Timestamp::start:{
                    return *min_element(endOfPreobTime_.begin(), endOfPreobTime_.end());
                }
                case Timestamp::end:{
                    return *max_element(endOfObservingTime_.begin(), endOfObservingTime_.end());
                }
            }
        }

        const unsigned int getScanTime(Timestamp ts) const noexcept{
            switch (ts){
                case Timestamp::start:{
                    return *min_element(endOfLastScan_.begin(), endOfLastScan_.end());
                }
                case Timestamp::end:{
                    return *max_element(endOfObservingTime_.begin(), endOfObservingTime_.end());
                }
            }
        }

        
        void alignStartTimes() noexcept;

        void setObservingStarts(unsigned int scanStart) noexcept ;

        /**
         * @brief updates the slewtime of one element
         *
         * @param idx index of the element whichs slewtime is chanded
         * @param new_slewtime new slew time in seconds
         */
        void setSlewTime(int idx, unsigned int new_slewtime) noexcept;

        /**
         * @brief adds scan times to each element
         *
         * Order of the scan times must be the same as the order of the element
         *
         * @param scanTimes all scan times in seconds
         */
        void setObservingTimes(const std::vector<unsigned int> &scanTimes) noexcept;

        void setObservingTimes(unsigned int scanTimes) noexcept;

        void setObservingTime(int idx, unsigned int time, Timestamp ts);

        bool reduceObservingTime(int idx, unsigned int maxObsTime, Timestamp ts);

        void addTagalongStationTime(const VieVS::PointingVector &pv_start, const VieVS::PointingVector &pv_end,
                                    unsigned int slewtime, unsigned int currentTime, unsigned int fieldSystem,
                                    unsigned int preob);

        bool setPreobTime(const std::vector<unsigned int> &preob);

        bool setPreobTime(int idx, unsigned int preob);

        int removeUnnecessaryObservingTime(Timestamp ts);

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
