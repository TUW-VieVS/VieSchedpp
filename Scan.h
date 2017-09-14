/**
 * @file Scan.h
 * @brief class Scan
 *
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */

#ifndef SCAN_H
#define SCAN_H

#include <iostream>
#include <vector>
#include <boost/format.hpp>
#include <boost/date_time.hpp>
#include <boost/optional.hpp>
#include <utility>
#include <limits>

#include "PointingVector.h"
#include "Baseline.h"
#include "ScanTimes.h"
#include "Station.h"
#include "Source.h"
#include "SkyCoverage.h"
#include "WeightFactors.h"
#include "ObservationMode.h"
#include "TimeSystem.h"

namespace VieVS{
    /**
     * @class Scan
     * @brief representation of a VLBI scan
     *
     * @author Matthias Schartner
     * @date 29.06.2017
     */
    class Scan {
    public:
        /**
         * @brief scan type
         */
        enum class ScanType {
            single, ///< single source scan
            subnetting, ///< subnetting scan
            fillin, ///< fillin mode scan
        };

        /**
         * @brief internal debugging function that checks if the number of pointing vectors is equal to nsta
         *
         * Usually unused
         *
         * @return true if everything is ok
         */
        bool check() {
            return pointingVectors_.size() != nsta_;
        }

        /**
         * @brief empty default constructor
         */
        Scan();

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
        Scan(const std::vector<PointingVector> &pointingVectors, const std::vector<unsigned int> &endOfLastScan,
                  int minimumNumberOfStations, ScanType type);

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
        Scan(const std::vector<PointingVector> &pv, const ScanTimes &times, const std::vector<Baseline> &bl, int minNumSta);

        /**
         * @brief default copy constructor
         *
         * @param other other scan
         */
        Scan(const Scan &other) = default;

        /**
         * @brief default move constructor
         *
         * @param other other scan
         */
        Scan(Scan &&other) = default;

        /**
         * @brief default copy assignment operator
         *
         * @param other other scan
         * @return copy of other scan
         */
        Scan &operator=(const Scan &other) = default;

        /**
         * @brief default move assignment operator
         *
         * @param other other scan
         * @return moved other scan
         */
        Scan &operator=(Scan &&other) = default;

        /**
         * @brief destructor
         */
        virtual ~Scan() {}

        /**
         * @brief sets the scan type
         * @param type new scan type
         */
        void setType(ScanType type) noexcept {
            Scan::type_ = type;
        }

        /**
         * @brief getter to get all scan times
         *
         * @return all scan times
         */
        const ScanTimes &getTimes() const noexcept {
            return times_;
        }

        /**
         * @brief getter for number of stations
         *
         * @return number of stations
         */
        unsigned long getNSta() const noexcept {
            return nsta_;
        }

        /**
         * @brief getter for station id for one index
         *
         * @param idx index of required station id
         * @return id of station
         */
        int getStationId(int idx) const noexcept {
            return pointingVectors_[idx].getStaid();
        }

        /**
         * @brief getter for source id
         *
         * @return source id
         */
        int getSourceId() const noexcept {
            return pointingVectors_[0].getSrcid();
        }

        /**
         * @brief getter for pointing vector for an index
         *
         * @param idx index of required pointing vector
         * @return pointing vector
         */
        const PointingVector &getPointingVector(int idx) const noexcept {
            return pointingVectors_[idx];
        }

        /**
         * @brief reference to pointing vector for an index
         *
         * @param idx index of required pointing vector
         * @return pointing vector
         */
        PointingVector &referencePointingVector(int idx) noexcept {
            return pointingVectors_[idx];
        }

        /**
         * @brief getter for pointing vector at the end of a scan for an index
         *
         * @param idx index of required pointing vector
         * @return pointing vector at the end of a scan
         */
        const PointingVector &getPointingVectors_endtime(int idx) const noexcept {
            return pointingVectorsEndtime_[idx];
        }

        /**
         * @brief getter for minimum number of required stations for this scan
         *
         * @return minimum number of required stations for this scan
         */
        int getMinimumNumberOfStations() const noexcept {
            return minNumberOfStations_;
        }

        /**
         * @brief getter for the total score of this scan
         *
         * @return score of this scan
         */
        double getScore() const noexcept {
            return score_;
        }

        /**
         * @brief getter for scan type
         *
         * @return scan type
         */
        ScanType getType() const noexcept {
            return type_;
        }

        /**
         * @brief getter for a single baseline
         *
         * @param idx index of baseline
         * @return baseline
         */
        const Baseline &getBaseline(int idx) const noexcept {
            return baselines_[idx];
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
         *
         * @param source observed source
         */
        bool constructBaselines(const Source &source) noexcept;

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
        bool checkIdleTimes(std::vector<unsigned int> maxIdle) noexcept;

        /**
         * @brief calculates the scan durations per baseline
         *
         * @param stations all stations
         * @param sources observed source
         * @return true is scan is still valid, otherwise false
         */
        bool calcBaselineScanDuration(const std::vector<Station> &stations, const Source &sources) noexcept;

        /**
         * @brief calculates the total scan duration per station
         *
         * removes stations if the scan duration is longer than the maximum allowed scan duration
         *
         * @param stations all stations
         * @param source observed source
         * @return true if scan is still valid, false if scan is no longer valid
         */
        bool scanDuration(const std::vector<Station> &stations, const Source &source) noexcept;

        /**
         * @brief getter for all station ids
         *
         * @return all station ids
         */
        std::vector<int> getStationIds() const noexcept;

        /**
         * @brief removes all stations except the ones in the station_ids parameter
         *
         * Currently unused
         *
         * @param station_ids ids of all stations which should not be removed
         * @return true if scan is still valid, false if scan is no longer valid
         */
        bool removeAllBut(const std::vector<int> &station_ids) noexcept;

        /**
         * @brief calculates the score of a scan
         *
         * usually used for single scan sources
         *
         * @param nmaxsta maximum number of stations
         * @param nmaxbl maximum number of baselines
         * @param astas precalculated vector of average station score @see subcon
         * @param asrcs precalculated vector of average source score @see subcon
         * @param minTime minimum time required for a scan
         * @param maxTime maximum time required for a scan
         * @param skyCoverages sky Coverages
         * @param stations list of all VLBI stations
         */
        void calcScore(unsigned long nmaxsta, unsigned long nmaxbl, const std::vector<double> &astas,
                       const std::vector<double> &asrcs, unsigned int minTime, unsigned int maxTime,
                       const std::vector<SkyCoverage> &skyCoverages, const std::vector<Station> &stations) noexcept;

        /**
         * @brief calculates the score of a scan
         *
         * !!! This function changes firstScorePerPv !!!
         *
         * usually used for single scan sources
         *
         * @param nmaxsta maximum number of stations
         * @param nmaxbl maximum number of baselines
         * @param astas precalculated vector of average station score @see subcon
         * @param asrcs precalculated vector of average source score @see subcon
         * @param minTime minimum time required for a scan
         * @param maxTime maximum time required for a scan
         * @param skyCoverages sky Coverages
         * @param firstScorePerPv stores the score of each pointing vector without twin station influences
         * @param stations list of all VLBI stations
        */
        void calcScore(unsigned long nmaxsta, unsigned long nmaxbl, const std::vector<double> &astas,
                       const std::vector<double> &asrcs, unsigned int minTime, unsigned int maxTime,
                       const std::vector<SkyCoverage> &skyCoverages, std::vector<double> &firstScorePerPv,
                       const std::vector<Station> &stations) noexcept;

        /**
         * @brief calculates the score of a scan
         *
         * usually used for subnetting sources. This is for improved runtime because the skyCoverage score for each
         * pointing vector already exists.
         *
         * @param nmaxsta maximum number of stations
         * @param nmaxbl maximum number of baselines
         * @param astas precalculated vector of average station score @see subcon
         * @param asrcs precalculated vector of average source score @see subcon
         * @param minTime minimum time required for a scan
         * @param maxTime maximum time required for a scan
         * @param skyCoverages sky Coverages
         * @param firstScorePerPv stored score for each pointing vector without twin station influences
         * @param stations list of all VLBI stations
         */
        void calcScore_subcon(unsigned long nmaxsta, unsigned long nmaxbl, const std::vector<double> &astas,
                              const std::vector<double> &asrcs, unsigned int minTime, unsigned int maxTime,
                              const std::vector<SkyCoverage> &skyCoverages, const std::vector<double> & firstScorePerPv,
                              const std::vector<Station> &stations) noexcept;

        /**
         * @brief calculates the score for number of observations
         *
         * @param maxObs maximum possible number of observations
         * @return score
         */
        double calcScore_numberOfObservations(unsigned long maxObs) const noexcept;

        /**
         * @brief calculates score for average station observations
         *
         * @param astas precalculated vector of average station observations @see subcon
         * @param nmaxsta maximum possible number of stations
         * @return score
        */
        double calcScore_averageStations(const std::vector<double> &astas, unsigned long nmaxsta) const noexcept;

        /**
         * @brief calculates score for average source observations
         *
         * @param asrcs precalculated vector of average source observations @see subcon
         * @return score
         */
        double calcScore_averageSources(const std::vector<double> &asrcs) const noexcept;

        /**
         * @brief calculates score for duration
         *
         * @param minTime minimum time required for a scan
         * @param maxTime maximum time required for a scan
         * @return score
         */
        double calcScore_duration(unsigned int minTime, unsigned int maxTime) const noexcept;

        /**
         * @brief calculates score for improvement in sky coverage
         *
         * @param skyCoverages all sky coverages
         * @return score
         */
        double calcScore_skyCoverage(const std::vector<SkyCoverage> &skyCoverages,
                                     const std::vector<Station> &stations) const noexcept;

        /**
         * @brief calculates score for improvement in sky coverage
         *
         * !!! This function changes firstScorePerPv !!!
         *
         * @param skyCoverages all sky coverages
         * @param firstScorePerPv stores the score of each pointing vector without twin station influences
         * @return score
         */
        double calcScore_skyCoverage(const std::vector<SkyCoverage> &skyCoverages,
                                     const std::vector<Station> &stations,
                                     std::vector<double> &firstScorePerPv) const noexcept;

        /**
         * @brief calculates score for improvement in sky coverage
         *
         * @param skyCoverages all sky coverages
         * @param firstScorePerPv stored score for each pointing vector without twin station influences
         * @return score
         */
        double calcScore_skyCoverage_subcon(const std::vector<SkyCoverage> &skyCoverages,
                                            const std::vector<Station> &stations,
                                            const std::vector<double> &firstScorePerPv) const noexcept;

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
         * @param stations all stations
         * @param source observed source
         * @return true if scan is still valid, false if scan is no longer valid
         */
        bool rigorousUpdate(const std::vector<Station> &stations, const Source &source) noexcept;


        /**
         * @brief checks a scan with rigorous models !!!UNUSED!!!
         *
         * This function is replaced by rigorousUpdate
         *
         * @param stations all stations
         * @param source observed source
         * @return true if scan is still valid, false if scan is no longer valid
         */
        bool rigorousUpdate2(const std::vector<Station> &stations, const Source &source) noexcept;



        bool rigorousUpdate2(const vector<VLBI_station> &stations, const VLBI_source &source) noexcept;


        /**
         * @brief makes a hard copy of a scan with all stations from parameter ids
         *
         * @param ids ids of all stations which should be copied
         * @return copy of scan with the stations from ids parameter or none if no valid scan can be created
         */
        boost::optional<Scan> copyScan(const std::vector<int> &ids) const noexcept;

        /**
         * @brief getter for number of baselines
         *
         * @return number of baselines
         */
        unsigned long getNBl() const noexcept {
            return baselines_.size();
        }

        /**
         * @brief checks if parts of this scan can be used as a fillin mode scan UNUSED
         *
         * This implementation is replaced by the other possibleFillinScan()
         *
         * @param stations list of all stations
         * @param source list of all sources
         * @param possible vector of flags for all possible stations (see VLBI_fillin_endpositions)
         * @param unused vector of flags for all unused stations (see VLBI_fillin_endpositions)
         * @param pv_final_position final required end position of all stations (see VLBI_fillin_endpositions)
         * @return possible fillin scan
         */
        boost::optional<Scan>
        possibleFillinScan(const std::vector<Station> &stations, const Source &source,
                           const std::vector<char> &possible, const std::vector<char> &unused,
                           const std::vector<PointingVector> &pv_final_position) const noexcept;

        /**
         * @brief checks if parts of this scan can be used as a fillin mode scan
         *
         * @param stations list of all stations
         * @param source list of all sources
         * @param unused vector of flags for all unused stations (see VLBI_fillin_endpositions)
         * @param pv_final_position final required end position of all stations (see VLBI_fillin_endpositions)
         * @return possible fillin scan
         */
        bool possibleFillinScan(const std::vector<Station> &stations, const Source &source,
                                const std::vector<char> &unused, const std::vector<PointingVector> &pv_final_position);


        bool possibleFillinScan(const vector<VLBI_station> &stations, const VLBI_source &source,
                                const std::vector<char> &unused, const vector<VLBI_pointingVector> &pv_final_position);


        /**
         * @brief outputs information of this scan to the current console
         *
         * @param observed_scan_nr scan number
         * @param stations all stations
         * @param source observed source
         * @param of outstream file object
         */
        void output(unsigned long observed_scan_nr, const std::vector<Station> &stations, const Source &source,
                    std::ofstream &of) const noexcept;

    private:
        unsigned long nsta_; ///< number of stations in this scan
        int srcid_; ///< observed source id

        int minNumberOfStations_; ///< minimum number of stations required for this scan

        double score_; ///< total score

        ScanTimes times_; ///< time informations
        std::vector<PointingVector> pointingVectors_; ///< pointing vectors at start of the scan
        std::vector<PointingVector> pointingVectorsEndtime_; ///< pointing vectors at end of the scan
        std::vector<Baseline> baselines_; ///< all observed baselines

        ScanType type_; ///< type of the scan
    };
}
#endif /* SCAN_H */

