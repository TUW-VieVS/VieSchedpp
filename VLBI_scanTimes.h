/**
 * @file VLBI_scanTimes.h
 * @brief class VLBI_scanTimes
 *
 *
 * @author Matthias Schartner
 * @date 14.07.2017
 */

#ifndef VLBI_SCANTIMES_H
#define VLBI_SCANTIMES_H

#include <vector>

using namespace std;
namespace VieVS {

    class VLBI_scanTimes {
    public:
        /**
         * @brief empty default constructor
         */
        VLBI_scanTimes();

        /**
         * @brief constructor
         * @param nsta number of stations
         */
        VLBI_scanTimes(int nsta);

        /**
         * @brief default copy constructor
         *
         * @param other other scan times object
         */
        VLBI_scanTimes(const VLBI_scanTimes &other) = default;

        /**
         * @brief default move constructor
         *
         * @param other other scan times object
         */
        VLBI_scanTimes(VLBI_scanTimes &&other) = default;

        /**
         * @brief default copy assignment operator
         *
         * @param other other scan times object
         * @return copy of other scan times object
         */
        VLBI_scanTimes &operator=(const VLBI_scanTimes &other) = default;

        /**
         * @brief default move assignment operator
         *
         * @param other other scan times object
         * @return copy of other scan times object
         */
        VLBI_scanTimes &operator=(VLBI_scanTimes &&other) = default;

        /**
         * @brief destructor
         */
        virtual ~VLBI_scanTimes() {}

        /**
         * @brief sets the endtime of the last scan
         *
         * this is simulaneouly the start time of this new scan
         *
         * @param endOfLastScan end time of last scan in seconds since session start
         */
        void setEndOfLastScan(const vector<unsigned int> &endOfLastScan) noexcept {
            VLBI_scanTimes::endOfLastScan = endOfLastScan;
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
        const vector<unsigned int> &getEndOfSlewTime() const noexcept {
            return endOfSlewTime;
        }

        /**
         * @brief getter for end of idle time of one element
         *
         * @param idx index of element
         * @return end of idle time in seconds since start for element at position of idx parameter
         */
        const unsigned int getEndOfIdleTime(int idx) const noexcept {
            return endOfIdleTime[idx];
        }

        /**
         * @brief getter for end of slew time of one element
         *
         * @param idx index of element
         * @return end of slew time in seconds since start for element at position of idx parameter
         */
        const unsigned int getEndOfSlewTime(int idx) const noexcept {
            return endOfSlewTime[idx];
        }

        /**
         * @brief getter for end of source time of one element
         *
         * @param idx index of element
         * @return end of source time in seconds since start for element at position of idx parameter
         */
        const unsigned int getEndOfSourceTime(int idx) const noexcept {
            return endOfSourceTime[idx];
        }

        /**
         * @brief getter for end of scan time of one element
         *
         * @param idx index of element
         * @return end of scan time in seconds since start for element at position of idx parameter
         */
        const unsigned int getEndOfScanTime(int idx) const noexcept {
            return endOfScanTime[idx];
        }

        /**
         * @brief getter for end of calibration time of one element
         *
         * @param idx index of element
         * @return end of calibration time in seconds since start for element at position of idx parameter
         */
        const unsigned int getEndOfCalibrationTime(int idx) const noexcept {
            return endOfCalibrationTime[idx];
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
        vector<unsigned int> stationTimes(int idx) const noexcept {
            return vector<unsigned int>{endOfSetupTime[idx], endOfSourceTime[idx], endOfSlewTime[idx],
                                        endOfIdleTime[idx], endOfTapeTime[idx], endOfCalibrationTime[idx],
                                        endOfScanTime[idx]};
        }

        /**
         * @brief calculates the earliest possible start time for this scan
         *
         * Idle times are introduced
         */
        void alignStartTimes() noexcept;

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
        void addScanTimes(const vector<unsigned int> &scanTimes) noexcept;

        /**
         * @brief latest time until scan is finished
         *
         * @return time until all stations are finished with the observation in seconds since session start
         */
        unsigned int maxTime() const noexcept;

        unsigned int slowestStation();

    private:
        vector<unsigned int> endOfLastScan; ///< end of last scan
        vector<unsigned int> endOfSetupTime; ///< end of setup time
        vector<unsigned int> endOfSourceTime; ///< end of source time
        vector<unsigned int> endOfSlewTime; ///< end of slew time
        vector<unsigned int> endOfIdleTime; ///< end of idle time
        vector<unsigned int> endOfTapeTime; ///< end of tape time
        vector<unsigned int> endOfCalibrationTime; ///< end of calibraiton time
        vector<unsigned int> endOfScanTime; ///< end of scan time
    };
}

#endif //VLBI_SCANTIMES_H
