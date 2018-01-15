/**
 * @file Baseline.h
 * @brief class Baseline
 *
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */

#ifndef BASELINE_H
#define BASELINE_H

#include <iostream>
#include <fstream>

#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/format.hpp>

#include "TimeSystem.h"

namespace VieVS{
    /**
     * @class Baseline
     * @brief representation of an VLBI baseline
     *
     * @author Matthias Schartner
     * @date 29.06.2017
     */
    class Baseline {
    public:

        /**
         * @brief baseline parameters
         */
        struct PARAMETERS {
            std::unordered_map<std::string, double> minSNR; ///< minimum SNR per band for each baseline
            bool ignore; ///< ignore specific baselines

            double weight; ///< multiplicative factor of score for scans with this baseline
            unsigned int minScan; ///< minimum required scan duration of this baseline
            unsigned int maxScan; ///< maximum allowed scan duration of this baseline
        };

        /**
         * @brief changes in parameters
         */
        struct EVENT {
            unsigned int time; ///< time wher parameters should be changed in seconds from start
            bool softTransition; ///< flag if a soft or hard transition is required
            PARAMETERS PARA; ///< new parameters
        };


        /**
         * @brief baseline parameters.
         *
         * Unlike the parameters for station and source this holds the information for all possible baselines.
         * If you want to get the parameter for a specific baseline use the station ids as indices for the vectors.
         */
        struct PARAMETER_STORAGE {
            std::unordered_map<std::string, std::vector<std::vector<double> > > minSNR; ///< minimum SNR per band for each baseline
            std::vector<std::vector<char> > ignore; ///< ignore specific baselines
            std::vector<std::vector<double> > weight; ///< multiplicative factor of score for scans with this baseline
            std::vector<std::vector<unsigned int> > minScan; ///< minimum required scan duration of this baseline
            std::vector<std::vector<unsigned int> > maxScan; ///< maximum allowed scan duration of this baseline
        };

        static thread_local PARAMETER_STORAGE PARA; ///< parameters for all baselines

        static std::vector<std::vector<std::vector<Baseline::EVENT> > > EVENTS; ///< all events per baseline
        static std::vector<std::vector<unsigned int> > nextEvent; ///< next event number per baseline

        /**
         * @brief empty default constructor
         */
        Baseline() = default;

        /**
         * @brief constructor
         *
         * @param staid1 id of first station
         * @param staid2 id of second station
         * @param srcid id of observed source
         * @param startTime observation start time in seconds after session start
         */
        Baseline(int staid1, int staid2, int srcid, unsigned int startTime);

        /**
         * @brief getter method for first station id
         *
         * @return first station id
         */
        int getStaid1() const noexcept {
            return staid1_;
        }

        /**
         * @brief getter method for second station id
         *
         * @return second station id
         */
        int getStaid2() const noexcept {
            return staid2_;
        }

        /**
         * @brief getter method for source id
         *
         * @return source id
         */
        int getSrcid() const noexcept {
            return srcid_;
        }

        /**
         * @brief getter function for scan start time
         * @return start time in seconds from session start
         */
        unsigned int getStartTime() const noexcept {
            return startTime_;
        }


        /**
         * @brief getter function for scan duration
         *
         * @return scan duration in seconds
         */
        unsigned int getScanDuration() const noexcept {
            return scanDuration_;
        }

        /**
         * sets scan duration of this baseline
         *
         * @param scanDuration scan duration in seconds
         */
        void setScanDuration(unsigned int scanDuration) noexcept {
            Baseline::scanDuration_ = scanDuration;
        }

        /**
         * @brief check if it is time to change the parameters (a new event occures)
         *
         * @param time current scheduling time
         * @param hardBreak flag if a hard transition is required
         * @param output flag if output should be print to bodyLog stream
         * @param bodyLog output stream
         */
        static void checkForNewEvent(unsigned int time, bool &hardBreak, bool output, std::ofstream &bodyLog) noexcept;

        /**
         * @brief internal function for debugging
         * Usually unused
         *
         * @param log log file stream
         */
        static void displaySummaryOfStaticMembersForDebugging(std::ofstream &log);

    private:
        int staid1_{}; ///< id of first antenna
        int staid2_{}; ///< id of second antenna
        int srcid_{}; ///< id of observed source
        unsigned int startTime_{}; ///< start time of observion in seconds since session start

        unsigned int scanDuration_{}; ///< required scan duration in seconds

    };
}
#endif /* BASELINE_H */

