/**
 * @file VLBI_baseline.h
 * @brief class VLBI_baseline
 *
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */

#ifndef BASELINE_H
#define BASELINE_H

#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <unordered_map>

using namespace std;
namespace VieVS{
    class VLBI_baseline {
    public:
        /**
         * @brief baseline parameters.
         *
         * Unlike the parameters for station and source this holds the information for all possible baselines.
         * If you want to get the parameter for a specific baseline use the station ids as indices for the vectors.
         */
        struct PARAMETERS{
            unordered_map<string, vector< vector <double> > > minSNR = {}; ///< minimum SNR per band for each baseline
            vector< vector<char> > ignore = {}; ///< ignore specific baselines
            vector< vector<double> > weight = {}; ///< multiplicative factor of score for scans with this baseline
            vector< vector<unsigned int> > minScan = {}; ///< minimum required scan duration of this baseline
            vector< vector<unsigned int> > maxScan = {}; ///< maximum allowed scan duration of this baseline
        };

        static PARAMETERS PARA; ///< parameters for all baselines

        /**
         * @brief empty default constructor
         */
        VLBI_baseline();

        /**
         * @brief constructor
         *
         * @param staid1 id of first station
         * @param staid2 id of second station
         * @param srcid id of observed source
         * @param startTime observation start time in seconds after session start
         */
        VLBI_baseline(int staid1, int staid2, int srcid, unsigned int startTime);

        /**
         * @brief default copy constructor
         *
         * @param other other baseline
         */
        VLBI_baseline(const VLBI_baseline &other) = default;

        /**
         * @brief default move constructor
         *
         * @param other other baseline
         */
        VLBI_baseline(VLBI_baseline &&other) = default;

        /**
         * @brief default copy assignment operator
         *
         * @param other other baseline
         * @return copy of other baseline
         */
        VLBI_baseline &operator=(const VLBI_baseline &other) = default;

        /**
         * @brief default move assignment operator
         *
         * @param other other baseline
         * @return moved other baseline
         */
        VLBI_baseline &operator=(VLBI_baseline &&other) = default;

        /**
         * @brief destructor
         */
        virtual ~VLBI_baseline() {}

        /**
         * @brief getter method for first station id
         *
         * @return first station id
         */
        int getStaid1() const {
            return staid1;
        }

        /**
         * @brief getter method for second station id
         *
         * @return second station id
         */
        int getStaid2() const {
            return staid2;
        }

        /**
         * @brief getter method for source id
         *
         * @return source id
         */
        int getSrcid() const {
            return srcid;
        }

        /**
         * @brief getter function for scan start time
         * @return start time in seconds from session start
         */
        unsigned int getStartTime() const {
            return startTime;
        }


        /**
         * @brief getter function for scan duration
         *
         * @return scan duration in seconds
         */
        unsigned int getScanDuration() const {
            return scanDuration;
        }

        /**
         * sets scan duration of this baseline
         *
         * @param scanDuration scan duration in seconds
         */
        void setScanDuration(unsigned int scanDuration) {
            VLBI_baseline::scanDuration = scanDuration;
        }


    private:
        int staid1; ///< id of first antenna
        int staid2; ///< id of second antenna
        int srcid; ///< id of observed source
        unsigned int startTime; ///< start time of observion in seconds since session start

        unsigned int scanDuration; ///< required scan duration in seconds

    };
}
#endif /* BASELINE_H */

