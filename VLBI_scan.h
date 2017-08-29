/**
 * @file VLBI_scan.h
 * @brief class VLBI_scan
 *
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */

#ifndef VLBI_SCAN_H
#define VLBI_SCAN_H


#include <iostream>
#include <vector>
#include <boost/format.hpp>
#include <boost/date_time.hpp>
#include <boost/optional.hpp>
#include <utility>
#include <limits>

#include "VLBI_pointingVector.h"
#include "VLBI_baseline.h"
#include "VLBI_scanTimes.h"
#include "VLBI_station.h"
#include "VLBI_source.h"
#include "VLBI_skyCoverage.h"
#include "VLBI_weightFactors.h"
#include "VLBI_obsMode.h"
#include "VieVS_time.h"

using namespace std;
namespace VieVS{

    class VLBI_scan {
    public:
        /**
         * @brief scan type
         */
        enum class scanType {
            single, ///< single source scan
            subnetting, ///< subnetting scan
            fillin, ///< fillin mode scan
        };

//        /**
//         * @brief storage structure of all individual scores
//         */
//        struct SCORES {
//            double numberOfObservations = 0; ///< score for number of observations
//            double averageStations = 0; ///< score for average station observations
//            double averageSources = 0; ///< score for average source observations
//            double duration = 0; ///< score for scan duration
//            double skyCoverage = 0; ///< score for sky coverage improvement
//        };

        bool check() {
            return pointingVectors.size() != nsta;
        }

        /**
         * @brief empty default constructor
         */
        VLBI_scan();

        /**
         * @brief constructor
         *
         * This constructor is used to create all single source scans
         *
         * @param pointingVectors all pointing vectors
         * @param endOfLastScan time information for endtime of last scan for each station in seconds since session start
         * @param minimumNumberOfStations minimum number of stations for this scan
         * @param type scan type
         */
        VLBI_scan(const vector<VLBI_pointingVector> &pointingVectors,const vector<unsigned int> &endOfLastScan,
                  int minimumNumberOfStations, scanType type);

        /**
         * @brief constructor
         *
         * This constructor is uesd to create subnetting scans if the information for each single source scan is already
         * available
         *
         * @param pv all pointing vectors
         * @param times all scan times
         * @param bl all baselines
         * @param minNumSta minimum number of stations for this scan
         */
        VLBI_scan(const vector<VLBI_pointingVector> &pv, const VLBI_scanTimes &times, const vector<VLBI_baseline> &bl, int minNumSta);

        /**
         * @brief default copy constructor
         *
         * @param other other scan
         */
        VLBI_scan(const VLBI_scan &other) = default;

        /**
         * @brief default move constructor
         *
         * @param other other scan
         */
        VLBI_scan(VLBI_scan &&other) = default;

        /**
         * @brief default copy assignment operator
         *
         * @param other other scan
         * @return copy of other scan
         */
        VLBI_scan &operator=(const VLBI_scan &other) = default;

        /**
         * @brief default move assignment operator
         *
         * @param other other scan
         * @return moved other scan
         */
        VLBI_scan &operator=(VLBI_scan &&other) = default;

        /**
         * @brief destructor
         */
        virtual ~VLBI_scan() {}

        /**
         * @brief sets the scan type
         * @param type new scan type
         */
        void setType(scanType type) noexcept {
            VLBI_scan::type = type;
        }

        /**
         * @brief getter to get all scan times
         *
         * @return all scan times
         */
        const VLBI_scanTimes &getTimes() const noexcept {
            return times;
        }

        /**
         * @brief getter for number of stations
         *
         * @return number of stations
         */
        unsigned long getNSta() const noexcept {
            return nsta;
        }

        /**
         * @brief getter for station id for one index
         *
         * @param idx index of required station id
         * @return id of station
         */
        int getStationId(int idx) const noexcept {
            return pointingVectors[idx].getStaid();
        }

        /**
         * @brief getter for source id
         *
         * @return source id
         */
        int getSourceId() const noexcept {
            return pointingVectors[0].getSrcid();
        }

        /**
         * @brief getter for pointing vector for an index
         *
         * @param idx index of required pointing vector
         * @return pointing vector
         */
        const VLBI_pointingVector &getPointingVector(int idx) const noexcept {
            return pointingVectors[idx];
        }

        /**
         * @brief reference to pointing vector for an index
         *
         * @param idx index of required pointing vector
         * @return pointing vector
         */
        VLBI_pointingVector &referencePointingVector(int idx) noexcept {
            return pointingVectors[idx];
        }

        /**
         * @brief getter for pointing vector at the end of a scan for an index
         *
         * @param idx index of required pointing vector
         * @return pointing vector at the end of a scan
         */
        const VLBI_pointingVector &getPointingVectors_endtime(int idx) const noexcept {
            return pointingVectors_endtime[idx];
        }

        /**
         * @brief getter for minimum number of required stations for this scan
         *
         * @return minimum number of required stations for this scan
         */
        int getMinimumNumberOfStations() const noexcept {
            return minimumNumberOfStations;
        }

        /**
         * @brief getter for the total score of this scan
         *
         * @return score of this scan
         */
        double getScore() const noexcept {
            return score;
        }

        /**
         * @brief getter for scan type
         *
         * @return scan type
         */
        scanType getType() const noexcept {
            return type;
        }

        /**
         * @brief getter for a single baseline
         *
         * @param idx index of baseline
         * @return baseline
         */
        const VLBI_baseline &getBaseline(int idx) const noexcept {
            return baselines[idx];
        }

        /**
         * @brief delete the pointing vector at position idx and all its corresponding times and baselines
         *
         * If a scan no longer has enough stations or the number of baselines will get zero, it gets invalid.
         *
         * @param idx index of element to delete
         * @return true if scan is still valid, false if scan is no longer valid
         */
        bool removeStation(int idx) noexcept;

        /**
         * @brief delete the baseline at position idx from scan
         *
         * If a station has no longer any baselines it also gets removed
         *
         * @param idx_bl
         * @return
         */
        bool removeBaseline(int idx_bl) noexcept;

        //TODO boost::optional
        /**
         * @brief finds the index of an station id
         *
         * @param id station id
         * @return index
         */
        int findIdxOfStationId(int id) const noexcept;

        /**
         * @brief adds scan times
         *
         * @param idx index of element
         * @param setup setup time in seconds
         * @param source source time in seconds
         * @param slew slew time in seconds
         * @param tape tape time in seconds
         * @param calib calibration time in secons
         */
        void addTimes(int idx, unsigned int setup, unsigned int source, unsigned int slew, unsigned int tape,
                      unsigned int calib) noexcept;

        /**
         * @brief constructs all possible baselines with the pointing vectors
         */
        void constructBaselines() noexcept;

        /**
         * @brief updates the slewtime of an element
         *
         * @param idx index of element
         * @param new_slewtime new slewtime in seconds
         */
        void updateSlewtime(int idx, unsigned int new_slewtime) noexcept;

        /**
         * @brief checks if the idle time is not too long
         *
         * this function removes stations whichs idle times is too long. The order of the station pointing vectors must
         * be the same as the order of the max Idle parameter.
         *
         * @param maxIdle maximum allowed idle time
         * @return true if scan is still valid, false if scan is no longer valid
         */
        bool checkIdleTimes(vector<unsigned int> maxIdle) noexcept;

        /**
         * @brief calculates the scan durations per baseline
         *
         * @param stations all stations
         * @param sources observed source
         * @param mjdStart modified julian date of session start
         * @return true is scan is still valid, otherwise false
         */
        bool calcBaselineScanDuration(const vector<VLBI_station> &stations, const VLBI_source &sources) noexcept;

        /**
         * @brief calculates the total scan duration per station
         *
         * removes stations if the scan duration is longer than the maximum allowed scan duration
         * //TODO descripe how and which stations are removed
         *
         * @param stations all stations
         * @param source observed source
         * @return true if scan is still valid, false if scan is no longer valid
         */
        bool scanDuration(const vector<VLBI_station> &stations, const VLBI_source &source) noexcept;

        /**
         * @brief getter for all station ids
         *
         * @return all station ids
         */
        vector<int> getStationIds() const noexcept;

        /**
         * @brief removes all stations except the ones in the station_ids parameter
         *
         * @param station_ids ids of all stations which should not be removed
         * @return true if scan is still valid, false if scan is no longer valid
         */
        bool removeAllBut(const vector<int> &station_ids) noexcept;

        /**
         * @brief calculates the score of a scan
         *
         * usually used for single scan sources
         *
         * // TODO: links to all parameter fuctions
         * @param nmaxsta maximum number of stations
         * @param nmaxbl maximum number of baselines
         * @param astas precalculated vector of average station score @see subcon
         * @param asrcs precalculated vector of average source score @see subcon
         * @param minTime minimum time required for a scan
         * @param maxTime maximum time required for a scan
         * @param skyCoverages sky Coverages
         * @return score
         */
        void calcScore(unsigned long nmaxsta, unsigned long nmaxbl, const vector<double> &astas,
                       const vector<double> &asrcs, unsigned int minTime, unsigned int maxTime,
                       const vector<VLBI_skyCoverage> &skyCoverages, const vector<VLBI_station> &stations) noexcept;

        /**
         * @brief calculates the score of a scan
         *
         * !!! This function changes firstScorePerPv !!!
         *
         * usually used for single scan sources
         *
         * // TODO: links to all parameter fuctions
         * @param nmaxsta maximum number of stations
         * @param nmaxbl maximum number of baselines
         * @param astas precalculated vector of average station score @see subcon
         * @param asrcs precalculated vector of average source score @see subcon
         * @param minTime minimum time required for a scan
         * @param maxTime maximum time required for a scan
         * @param skyCoverages sky Coverages
         * @param firstScorePerPv stores the score of each pointing vector without twin station influences
         * @return score
         */
        void calcScore(unsigned long nmaxsta, unsigned long nmaxbl, const vector<double> &astas,
                       const vector<double> &asrcs, unsigned int minTime, unsigned int maxTime,
                       const vector<VLBI_skyCoverage> &skyCoverages, vector<double> &firstScorePerPv,
                       const vector<VLBI_station> &stations) noexcept;

        /**
         * @brief calculates the score of a scan
         *
         * usually used for subnetting sources. This is for improved runtime because the skyCoverage score for each
         * pointing vector already exists.
         *
         * // TODO: links to all parameter fuctions
         * @param nmaxsta maximum number of stations
         * @param nmaxbl maximum number of baselines
         * @param astas precalculated vector of average station score @see subcon
         * @param asrcs precalculated vector of average source score @see subcon
         * @param minTime minimum time required for a scan
         * @param maxTime maximum time required for a scan
         * @param skyCoverages sky Coverages
         * @param firstScorePerPv stored score for each pointing vector without twin station influences
         */
        void calcScore_subcon(unsigned long nmaxsta, unsigned long nmaxbl, const vector<double> &astas,
                              const vector<double> &asrcs, unsigned int minTime, unsigned int maxTime,
                              const vector<VLBI_skyCoverage> &skyCoverages, const vector<double> & firstScorePerPv,
                              const vector<VLBI_station> &stations) noexcept;

        /**
         * @brief calculates the score for number of observations
         *
         * @param maxObs maximum possible number of observations
         */
        double calcScore_numberOfObservations(unsigned long maxObs) const noexcept;

        /**
         * @brief calculates score for average station observations
         *
         * //TODO link
         * @param astas precalculated vector of average station observations @see subcon
         * @param nmaxsta maximum possible number of stations
         */
        double calcScore_averageStations(const vector<double> &astas, unsigned long nmaxsta) const noexcept;

        /**
         * @brief calculates score for average source observations
         *
         * //TODO link
         * @param asrcs precalculated vector of average source observations @see subcon
         */
        double calcScore_averageSources(const vector<double> &asrcs) const noexcept;

        /**
         * @brief calculates score for duration
         *
         * @param minTime minimum time required for a scan
         * @param maxTime maximum time required for a scan
         */
        double calcScore_duration(unsigned int minTime, unsigned int maxTime) const noexcept;

        /**
         * @brief calculates score for improvement in sky coverage
         *
         * @param skyCoverages all sky coverages
         */
        double calcScore_skyCoverage(const vector<VLBI_skyCoverage> &skyCoverages,
                                     const vector<VLBI_station> &stations) const noexcept;

        /**
         * @brief calculates score for improvement in sky coverage
         *
         * !!! This function changes firstScorePerPv !!!
         *
         * @param skyCoverages all sky coverages
         * @param firstScorePerPv stores the score of each pointing vector without twin station influences
         */
        double calcScore_skyCoverage(const vector<VLBI_skyCoverage> &skyCoverages,
                                     const vector<VLBI_station> &stations,
                                     vector<double> &firstScorePerPv) const noexcept;

        /**
         * @brief calculates score for improvement in sky coverage
         *
         * @param skyCoverages all sky coverages
         * @param firstScorePerPv stored score for each pointing vector without twin station influences
         */
        double calcScore_skyCoverage_subcon(const vector<VLBI_skyCoverage> &skyCoverages,
                                            const vector<VLBI_station> &stations,
                                            const vector<double> &firstScorePerPv) const noexcept;

//        /**
//         * @brief sum up all individual scores for this scan
//         */
//        void sumScores();

        /**
         * @brief time required for this scan
         *
         * @return time in seconds since session start until all stations finish scan
         */
        unsigned int maxTime() const noexcept;

        /**
         * @brief checks a scan with rigorous models
         *
         * // TODO: describe what is checked here.
         *
         * @param stations all stations
         * @param source observed source
         * @param mjdStart modified juliand date of session start
         * @return true if scan is still valid, false if scan is no longer valid
         */
        bool rigorousUpdate(const vector<VLBI_station> &stations, const VLBI_source &source) noexcept;

        /**
         * @brief makes a hard copy of a scan with all stations from parameter ids
         *
         * @param ids ids of all stations which should be copied
         * @return copy of scan with the stations from ids parameter or none if no valid scan can be created
         */
        boost::optional<VLBI_scan> copyScan(const vector<int> &ids) const noexcept;

        /**
         * @brief getter for number of baselines
         *
         * @return numbe rof baselines
         */
        unsigned long getNBl() const noexcept {
            return baselines.size();
        }

        /**
         * @brief checks if parts of this scan can be used as a fillin mode scan
         *
         * @param stations list of all stations
         * @param source list of all sources
         * @param possible vector of flags for all possible stations (see VLBI_fillin_endpositions)
         * @param unused vector of flags for all unused stations (see VLBI_fillin_endpositions)
         * @param pv_final_position final required end position of all stations (see VLBI_fillin_endpositions)
         * @return possible fillin scan
         */
        boost::optional<VLBI_scan>
        possibleFillinScan(const vector<VLBI_station> &stations, const VLBI_source &source,
                           const std::vector<char> &possible, const std::vector<char> &unused,
                           const vector<VLBI_pointingVector> &pv_final_position) const noexcept;

        /**
         * @brief outputs information of this scan to the current console
         *
         * @param observed_scan_nr scan number
         * @param stations all stations
         * @param source observed source
         * @param sessionStart session start
         */
        void output(unsigned long observed_scan_nr, const vector<VLBI_station> &stations,
                    const VLBI_source &source) const noexcept;

    private:
        unsigned long nsta; ///< number of stations in this scan
        int srcid; ///< observed source id

        int minimumNumberOfStations; ///< minimum number of stations required for this scan

//        SCORES single_scores; ///< storage for all individual scores

        double score; ///< total score

        VLBI_scanTimes times; ///< time informations
        vector<VLBI_pointingVector> pointingVectors; ///< pointing vectors at start of the scan
        vector<VLBI_pointingVector> pointingVectors_endtime; ///< pointing vectors at end of the scan
        vector<VLBI_baseline> baselines; ///< all observed baselines

        scanType type;
    };
}
#endif /* VLBI_SCAN_H */

