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
    class ScanTimes {
    public:
        /**
         * @brief empty default constructor
         */
        ScanTimes();

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
        void addTimes(int idx, unsigned int setup, unsigned int source, unsigned int slew, unsigned int tape,
                      unsigned int calib) noexcept;

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

        /**
         * @brief getter for end of idle time of one element
         *
         * @param idx index of element
         * @return end of idle time in seconds since start for element at position of idx parameter
         */
        const unsigned int getEndOfLastScan(int idx) const noexcept {
            return endOfLastScan_[idx];
        }

        /**
         * @brief getter for end of idle time of one element
         *
         * @param idx index of element
         * @return end of idle time in seconds since start for element at position of idx parameter
         */
        const unsigned int getEndOfIdleTime(int idx) const noexcept {
            return endOfIdleTime_[idx];
        }

        /**
         * @brief getter for end of slew time of one element
         *
         * @param idx index of element
         * @return end of slew time in seconds since start for element at position of idx parameter
         */
        const unsigned int getEndOfSlewTime(int idx) const noexcept {
            return endOfSlewTime_[idx];
        }

        /**
         * @brief getter for end of source time of one element
         *
         * @param idx index of element
         * @return end of source time in seconds since start for element at position of idx parameter
         */
        const unsigned int getEndOfSourceTime(int idx) const noexcept {
            return endOfSourceTime_[idx];
        }

        /**
         * @brief getter for end of tape time of one element
         *
         * @param idx index of element
         * @return end of tape time in seconds since start for element at position of idx parameter
         */
        const unsigned int getEndOfTapeTime(int idx) const noexcept {
            return endOfTapeTime_[idx];
        }

        /**
         * @brief getter for end of scan time of one element
         *
         * @param idx index of element
         * @return end of scan time in seconds since start for element at position of idx parameter
         */
        const unsigned int getEndOfScanTime(int idx) const noexcept {
            return endOfScanTime_[idx];
        }

        /**
         * @brief getter for end of calibration time of one element
         *
         * @param idx index of element
         * @return end of calibration time in seconds since start for element at position of idx parameter
         */
        const unsigned int getEndOfCalibrationTime(int idx) const noexcept {
            return endOfCalibrationTime_[idx];
        }

        /**
         * @brief all times for one station
         *
         * times are returned in the following order:
         * - end of setup time
         * - end of source time
         * - end of slew time
         * - end of idle time
         * - end of tape time
         * - end of calibration time
         * - end of scan time
         *
         * @param idx index of element
         * @return vector of all times for one station
         */
        std::vector<unsigned int> stationTimes(int idx) const noexcept {
            return std::vector<unsigned int>{endOfSetupTime_[idx], endOfSourceTime_[idx], endOfSlewTime_[idx],
                                        endOfIdleTime_[idx], endOfTapeTime_[idx], endOfCalibrationTime_[idx],
                                        endOfScanTime_[idx]};
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
        void addScanTimes(const std::vector<unsigned int> &scanTimes) noexcept;


        void addScanTimes(unsigned int scanTimes) noexcept;

        /**
         * @brief latest time until scan is finished
         *
         * @return time until all stations are finished with the observation in seconds since session start
         */
        unsigned int maxTime() const noexcept;

        /**
         * @brief earliest start time of a scan
         *
         * @return scan start time
         */
        unsigned int scanStart() const noexcept;

        void addTagalongStation(const VieVS::PointingVector &pv_start, const VieVS::PointingVector &pv_end,
                                unsigned int slewtime, unsigned int currentTime, unsigned int setup,
                                unsigned int source, unsigned int tape, unsigned int calibration);

    private:
        std::vector<unsigned int> endOfLastScan_; ///< end of last scan
        std::vector<unsigned int> endOfSetupTime_; ///< end of setup time
        std::vector<unsigned int> endOfSourceTime_; ///< end of source time
        std::vector<unsigned int> endOfSlewTime_; ///< end of slew time
        std::vector<unsigned int> endOfIdleTime_; ///< end of idle time
        std::vector<unsigned int> endOfTapeTime_; ///< end of tape time
        std::vector<unsigned int> endOfCalibrationTime_; ///< end of calibraiton time
        std::vector<unsigned int> endOfScanTime_; ///< end of scan time
    };
}

#endif //SCANTIMES_H
