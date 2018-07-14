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

#include "Network.h"
#include "Observation.h"
#include "PointingVector.h"
#include "ScanTimes.h"
#include "Source.h"
#include "WeightFactors.h"
#include "ObservationMode.h"
#include "TimeSystem.h"
#include "CalibratorBlock.h"
#include "StationEndposition.h"
#include "util.h"

namespace VieVS{

    /**
     * @class Scan
     * @brief representation of a VLBI scan
     *
     * @author Matthias Schartner
     * @date 29.06.2017
     */
    class Scan: public VieVS_Object {
    public:
        static unsigned int nScanSelections; ///< number of selected main scans

        /**
         * @brief scan type
         */
        enum class ScanConstellation {
            single, ///< single source scan
            subnetting, ///< subnetting scan
        };

        enum class ScanType {
            highImpact,
            standard,
            fillin,
            calibrator
        };

        /**
         * @brief specify custom scan sequence rules
         */
        struct ScanSequence{
            bool customScanSequence = false; ///< true if you have a custom scan sequence
            unsigned int cadence = 0; ///< cadence of source sequence rule
            unsigned int moduloScanSelctions = 0; ///< modulo of scan selection cadence
            std::map<unsigned int, std::vector<unsigned long> > targetSources; ///< map with modulo number as key and list of target source ids as value

            /**
             * @brief increases the modulo value for this ScanSequence
             */
            void newScan(){
                if(moduloScanSelctions == cadence-1){
                    moduloScanSelctions = 0;
                }else{
                    ++moduloScanSelctions;
                }
            }
        };

        static ScanSequence scanSequence; ///< scan sequence rules

        /**
         * @brief internal debugging function that checks if the number of pointing vectors is equal to nsta
         *
         * Usually unused
         *
         * @return true if everything is ok
         */
        bool check() {
            return pointingVectorsStart_.size() != nsta_;
        }

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
        Scan(std::vector<PointingVector> &pointingVectors, std::vector<unsigned int> &endOfLastScan, ScanType type);

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
        Scan(std::vector<PointingVector> pv, ScanTimes times, std::vector<Observation> obs);

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

        ScanTimes &referenceTime() noexcept {
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
        unsigned long getStationId(int idx) const noexcept {
            return pointingVectorsStart_[idx].getStaid();
        }

        /**
         * @brief getter for source id
         *
         * @return source id
         */
        unsigned long getSourceId() const noexcept {
            return pointingVectorsStart_[0].getSrcid();
        }

        /**
         * @brief getter for pointing vector for an index
         *
         * @param idx index of required pointing vector
         * @return pointing vector
         */
        const PointingVector &getPointingVector(int idx, Timestamp ts = Timestamp::start) const noexcept {
            switch (ts){
                case VieVS::Timestamp::start:{
                    return pointingVectorsStart_[idx];
                }
                case VieVS::Timestamp::end:{
                    return pointingVectorsEnd_[idx];
                }
            }
        }

        /**
         * @brief reference to pointing vector for an index
         *
         * @param idx index of required pointing vector
         * @return pointing vector
         */
        PointingVector &referencePointingVector(int idx, Timestamp ts = Timestamp::start) noexcept {
            switch (ts){
                case VieVS::Timestamp::start:{
                    return pointingVectorsStart_[idx];
                }
                case VieVS::Timestamp::end:{
                    return pointingVectorsEnd_[idx];
                }
            }
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
         * @brief getter for scan type
         *
         * @return scan type
         */
        ScanConstellation getScanConstellation() const noexcept {
            return constellation_;
        }

        /**
         * @brief getter for a single baseline
         *
         * @param idx index of baseline
         * @return baseline
         */
        const Observation &getObservation(int idx) const noexcept {
            return observations_[idx];
        }

        const std::vector<Observation> &getObservations() const noexcept {
            return observations_;
        }

        /**
         * @brief delete the pointing vector at position idx and all its corresponding times and baselines
         *
         * If a scan no longer has enough stations or the number of baselines will get zero, it gets invalid.
         *
         * @param idx index of element to delete
         * @return true if scan is still valid, false if scan is no longer valid
         */
        bool removeStation(int idx, const Source &source) noexcept;

        /**
         * @brief delete the baseline at position idx from scan
         *
         * If a station has no longer any baselines it also gets removed
         *
         * @param idxObs
         * @return
         */
        bool removeObservation(int idxObs, const Source &source) noexcept;

        /**
         * @brief finds the index of an station id
         *
         * @param id station id
         * @return index
         */
        boost::optional<unsigned long> findIdxOfStationId(unsigned long id) const noexcept;

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
        void addTimes(int idx, unsigned int fieldSystem, unsigned int slew, unsigned int preob) noexcept;

        /**
         * @brief constructs all possible baselines with the pointing vectors
         *
         * @param source observed source
         */
        bool constructObservations(const Network &network, const Source &source) noexcept;

        /**
         * @brief checks if the idle time is not too long
         *
         * this function removes stations whichs idle times is too long. The order of the station pointing vectors must
         * be the same as the order of the max Idle parameter.
         *
         * @param maxIdle maximum allowed idle time
         * @return true if scan is still valid, false if scan is no longer valid
         */
        bool checkIdleTimes(std::vector<unsigned int> &maxIdle, const Source &source) noexcept;

        /**
         * @brief calculates the scan durations per baseline
         *
         * @param stations all stations
         * @param sources observed source
         * @return true is scan is still valid, otherwise false
         */
        bool calcObservationDuration(const Network &network, const Source &sources) noexcept;

        /**
         * @brief calculates the total scan duration per station
         *
         * removes stations if the scan duration is longer than the maximum allowed scan duration
         *
         * @param stations all stations
         * @param source observed source
         * @return true if scan is still valid, false if scan is no longer valid
         */
        bool scanDuration(const Network &network, const Source &source) noexcept;

        /**
         * @brief getter for all station ids
         *
         * @return all station ids
         */
        std::vector<unsigned long> getStationIds() const noexcept;

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
        void calcScore(const std::vector<double> &astas, const std::vector<double> &asrcs,
                       const std::vector<double> &abls, unsigned int minTime, unsigned int maxTime,
                       const Network &network, const Source &source) noexcept;

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
         * @param stations list of all VLBI stations
         * @param source observed source
         * @param firstScorePerPv stores the score of each pointing vector without twin station influences
        */
        void calcScore(const std::vector<double> &astas, const std::vector<double> &asrcs,
                       const std::vector<double> &abls, unsigned int minTime, unsigned int maxTime,
                       const Network &network, const Source &source,
                       std::unordered_map<unsigned long, double> &staids2skyCoverageScore) noexcept;

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
         * @param stations list of all VLBI stations
         * @param source observed source
         * @param firstScorePerPv stored score for each pointing vector without twin station influences
         */
        void calcScore_subnetting(const std::vector<double> &astas, const std::vector<double> &asrcs,
                                  const std::vector<double> &abls, unsigned int minTime, unsigned int maxTime,
                                  const Network &network, const Source &source,
                                  const std::unordered_map<unsigned long, double> &staids2skyCoverageScore) noexcept;

        void calcScore(unsigned int minTime, unsigned int maxTime, const Network &network, const Source &source,
                       double hiscore);

        /**
         * @brief calculates the score for a calibrator block scan
         *
         * @param prevLowElevationScores score for previouse low elevation scans
         * @param prevHighElevationScores score for previouse high elevation scans
         * @param minRequiredTime minimum time required for a scan
         * @param maxRequiredTime maximum time required for a scan
         */
        bool calcScore(const std::vector<double> &prevLowElevationScores,
                       const std::vector<double> &prevHighElevationScores,
                       const Network &network, unsigned int minRequiredTime, unsigned int maxRequiredTime,
                       const Source &source);


        /**
         * @brief checks a scan with rigorous models
         *
         * @param stations all stations
         * @param source observed source
         * @return true if scan is still valid, false if scan is no longer valid
         */
        bool rigorousUpdate(const Network &network, const Source &source,
                            const boost::optional<StationEndposition> &endposition = boost::none) noexcept;

        /**
         * @brief adds observation to scan in tagalong mode
         *
         * @param pv_start pointing vector at start time
         * @param pv_end pointing vector at end time
         * @param baselines all baselines
         */
        void addTagalongStation(const PointingVector &pv_start, const PointingVector &pv_end,
                                const std::vector<Observation> &baselines, unsigned int slewtime,
                                const Station &station);

        /**
         * @brief makes a hard copy of a scan with all stations from parameter ids
         *
         * @param ids ids of all stations which should be copied
         * @return copy of scan with the stations from ids parameter or none if no valid scan can be created
         */
        boost::optional<Scan> copyScan(const std::vector<unsigned long> &ids, const Source &source) const noexcept;

        /**
         * @brief getter for number of baselines
         *
         * @return number of baselines
         */
        unsigned long getNObs() const noexcept {
            return observations_.size();
        }

        unsigned long getNObs(unsigned long staid) const noexcept;

        void setFixedScanDuration(unsigned int scanDuration) noexcept;

        /**
         * @brief outputs information of this scan to the current console
         *
         * @param observed_scan_nr scan number
         * @param stations all stations
         * @param source observed source
         * @param of outstream file object
         */
        void output(unsigned long observed_scan_nr, const Network &network, const Source &source,
                    std::ofstream &of) const noexcept;

        bool setScanTimes(const std::vector<unsigned int> &eols, unsigned int fieldSystemTime, const std::vector<unsigned int> &slewTime,
                          unsigned int preob, unsigned int scanStart, const std::vector<unsigned int> &observingTimes);

        void setPointingVectorsEndtime(std::vector<PointingVector> pv_end);

        void createDummyObservations(const Network &network);

        static unsigned long numberOfCreatedObjects(){
            return nextId-1;
        }

        void setPointingVector(int idx, PointingVector pv, Timestamp ts);

        void removeUnnecessaryObservingTime(const Network &network, const Source &thisSource, std::ofstream &of, Timestamp ts);

        void removeAdditionalObservingTime(unsigned int time, const Station &station, const Source &thisSource,
                                           std::ofstream &of, Timestamp ts);

    private:
        static unsigned long nextId;

        unsigned long nsta_; ///< number of stations in this scan
        unsigned long srcid_; ///< observed source id

        double score_; ///< total score

        ScanTimes times_; ///< time informations
        std::vector<PointingVector> pointingVectorsStart_; ///< pointing vectors at start of the scan
        std::vector<PointingVector> pointingVectorsEnd_; ///< pointing vectors at end of the scan
        std::vector<Observation> observations_; ///< all observed baselines

        ScanType type_; ///< type of the scan
        ScanConstellation constellation_; ///

        bool rigorousSlewtime(const Network &network, const Source &source) noexcept;

        bool rigorousScanStartTimeAlignment(const Network &network, const Source &source) noexcept;

        bool rigorousScanVisibility(const Network &network, const Source &source, bool &stationRemoved) noexcept;

        bool rigorousScanCanReachEndposition(const Network &network, const Source &thisSource,
                                                     const boost::optional<StationEndposition> &endposition,
                                                     bool &stationRemoved);

        /**
         * @brief calculates the score for number of observations
         *
         * @param maxObs maximum possible number of observations
         * @return score
         */
        double calcScore_numberOfObservations(unsigned long maxObs) const noexcept;

        double calcScore_averageBaselines(const std::vector<double> &abls) const noexcept;
        /**
         * @brief calculates score for average station observations
         *
         * @param astas precalculated vector of average station observations @see subcon
         * @param nmaxsta maximum possible number of stations
         * @return score
        */
        double calcScore_averageStations(const std::vector<double> &astas, unsigned long nMaxBls) const noexcept;

        /**
         * @brief calculates score for average source observations
         *
         * @param asrcs precalculated vector of average source observations @see subcon
         * @return score
         */
        double calcScore_averageSources(const std::vector<double> &asrcs, unsigned long nMaxBls) const noexcept;

        /**
         * @brief calculates score for duration
         *
         * @param minTime minimum time required for a scan
         * @param maxTime maximum time required for a scan
         * @return score
         */
        double calcScore_duration(unsigned int minTime, unsigned int maxTime) const noexcept;


        double calcScore_idleTime() const noexcept;

        /**
         * @brief mean of the weight factors for each participating station
         *
         * @param stations list of all stations
         * @return mean of weight factors
         */
        double weight_stations(const std::vector<Station> &stations);

        /**
         * @brief mean of the weight factors for each participating baselines
         *
         * @return mean of the weight factors
         */
        double weight_baselines(const std::vector<Baseline> &baselines);

        /**
         * @brief calculate score for low elevation scans
         * @return score for low elevation scans
         */
        double calcScore_lowElevation();


        double calcScore_firstPart(const std::vector<double> &astas, const std::vector<double> &asrcs,
                                   const std::vector<double> &abls, unsigned int minTime, unsigned int maxTime,
                                   const Network &network, const Source &source);

        double calcScore_secondPart(double this_score, const Network &network, const Source &source);

    };
}
#endif /* SCAN_H */

