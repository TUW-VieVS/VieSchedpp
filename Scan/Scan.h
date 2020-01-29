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
 * @file Scan.h
 * @brief class Scan
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */

#ifndef SCAN_H
#define SCAN_H


#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

#include "../Misc/CalibratorBlock.h"
#include "../Misc/StationEndposition.h"
#include "../Misc/TimeSystem.h"
#include "../Misc/WeightFactors.h"
#include "../Misc/util.h"
#include "../ObservingMode/Mode.h"
#include "../Source/Source.h"
#include "../Station/Network.h"
#include "Observation.h"
#include "PointingVector.h"
#include "ScanTimes.h"


namespace VieVS {

/**
 * @class Scan
 * @brief representation of a VLBI scan
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */
class Scan : public VieVS_Object {
   public:
    static unsigned int nScanSelections;  ///< number of selected main scans

    /**
     * @brief scan constellation type
     * @author Matthias Schartner
     */
    enum class ScanConstellation {
        single,      ///< single source scan
        subnetting,  ///< subnetting scan
    };

    /**
     * @brief scan type
     * @author Matthias Schartner
     */
    enum class ScanType {
        highImpact,       ///< high impact scan
        standard,         ///< standard scan
        fillin,           ///< fillin mode scan
        astroCalibrator,  ///< astrometric calibrator scan
        calibrator        ///< fringe finder
    };


    /**
     * @brief translates ScanType to string
     * @author Matthias Schartner
     *
     * @return string of scan type
     */
    static std::string toString( ScanType type_ ) {
        switch ( type_ ) {
            case ScanType::highImpact:
                return "high impact";
            case ScanType::standard:
                return "target";
            case ScanType::fillin:
                return "fillin mode";
            case ScanType::astroCalibrator:
                return "astrometric calibrator";
            case ScanType::calibrator:
                return "calibrator";
        }
    }

    /**
     * @brief translates ScanConstellation to string
     * @author Matthias Schartner
     *
     * @return string of scan constellation
     */
    static std::string toString( ScanConstellation type_ ) {
        switch ( type_ ) {
            case ScanConstellation::single:
                return "single source scan";
            case ScanConstellation::subnetting:
                return "subnetting scan";
        }
    }

    /**
     * @brief specify custom scan sequence rules
     * @author Matthias Schartner
     */
    struct ScanSequence {
        bool customScanSequence = false;       ///< true if you have a custom scan sequence
        unsigned int cadence = 0;              ///< cadence of source sequence rule
        unsigned int moduloScanSelctions = 0;  ///< modulo of scan selection cadence
        std::map<unsigned int, std::vector<unsigned long>>
            targetSources;  ///< map with modulo number as key and list of target source ids as value

        /**
         * @brief increases the modulo value for this ScanSequence
         * @author Matthias Schartner
         */
        void newScan() {
            if ( moduloScanSelctions == cadence - 1 ) {
                moduloScanSelctions = 0;
            } else {
                ++moduloScanSelctions;
            }
        }
    };


    static ScanSequence scanSequence;  ///< scan sequence rules

    /**
     * @brief internal debugging function that checks if the number of pointing vectors is equal to nsta
     * @author Matthias Schartner
     *
     * Usually unused
     *
     * @return true if everything is ok
     */
    bool check() { return pointingVectorsStart_.size() != nsta_; }


    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * This constructor is used to create all single source scans
     *
     * @param pointingVectors all pointing vectors
     * @param endOfLastScan time information for endtime of last scan for each station in seconds since session start
     * @param type scan type
     */
    Scan( std::vector<PointingVector> &pointingVectors, std::vector<unsigned int> &endOfLastScan, ScanType type );


    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * This constructor is used to create subnetting scans if the information for each single source scan is already
     * available
     *
     * @param pv all pointing vectors
     * @param times all scan times
     * @param obs all observations
     * @param type scan type (default = Scan::ScanType::standard)
     */
    Scan( std::vector<PointingVector> pv, ScanTimes times, std::vector<Observation> obs,
          ScanType type = Scan::ScanType::standard );


    /**
     * @brief sets the scan type
     * @author Matthias Schartner
     *
     * @param type new scan type
     */
    void setType( ScanType type ) noexcept { Scan::type_ = type; }


    /**
     * @brief getter to get all scan times
     * @author Matthias Schartner
     *
     * @return all scan times
     */
    const ScanTimes &getTimes() const noexcept { return times_; }


    /**
     * @brief referenct to all scan times
     * @author Matthias Schartner
     *
     * @return all scan times
     */
    ScanTimes &referenceTime() noexcept { return times_; }


    /**
     * @brief getter for number of stations
     * @author Matthias Schartner
     *
     * @return number of stations
     */
    unsigned long getNSta() const noexcept { return nsta_; }


    /**
     * @brief getter for station id for one index
     * @author Matthias Schartner
     *
     * @param idx index of required station id
     * @return id of station
     */
    unsigned long getStationId( int idx ) const noexcept { return pointingVectorsStart_[idx].getStaid(); }


    /**
     * @brief getter for source id
     * @author Matthias Schartner
     *
     * @return source id
     */
    unsigned long getSourceId() const noexcept { return pointingVectorsStart_[0].getSrcid(); }


    /**
     * @brief getter for pointing vector for an index
     * @author Matthias Schartner
     *
     * @param idx index of required pointing vector
     * @param ts time stamp flag
     * @return pointing vector
     */
    const PointingVector &getPointingVector( int idx, Timestamp ts = Timestamp::start ) const noexcept {
        switch ( ts ) {
            case VieVS::Timestamp::start: {
                return pointingVectorsStart_[idx];
            }
            case VieVS::Timestamp::end: {
                return pointingVectorsEnd_[idx];
            }
        }
    }


    /**
     * @brief reference to pointing vector for an index
     * @author Matthias Schartner
     *
     * @param idx index of required pointing vector
     * @param ts time stamp flag
     * @return pointing vector
     */
    PointingVector &referencePointingVector( int idx, Timestamp ts = Timestamp::start ) noexcept {
        switch ( ts ) {
            case VieVS::Timestamp::start: {
                return pointingVectorsStart_[idx];
            }
            case VieVS::Timestamp::end: {
                return pointingVectorsEnd_[idx];
            }
        }
    }


    /**
     * @brief getter for the total score of this scan
     * @author Matthias Schartner
     *
     * @return score of this scan
     */
    double getScore() const noexcept { return score_; }


    /**
     * @brief getter for scan type
     * @author Matthias Schartner
     *
     * @return scan type
     */
    ScanType getType() const noexcept { return type_; }


    /**
     * @brief getter for scan constellation type
     * @author Matthias Schartner
     *
     * @return scan constellation type
     */
    ScanConstellation getScanConstellation() const noexcept { return constellation_; }


    /**
     * @brief getter for a single observations
     * @author Matthias Schartner
     *
     * @param idx index of observation
     * @return observation
     */
    const Observation &getObservation( int idx ) const noexcept { return observations_[idx]; }


    /**
     * @brief get all observations
     * @author Matthias Schartner
     *
     * @return all observations
     */
    const std::vector<Observation> &getObservations() const noexcept { return observations_; }


    /**
     * @brief delete the pointing vector at position idx and all its corresponding times and observations
     * @author Matthias Schartner
     *
     * If a scan no longer has enough stations or the number of baselines will get zero, it gets invalid.
     *
     * @param idx index of element to delete
     * @param source observed source
     * @return true if scan is still valid, false if scan is no longer valid
     */
    bool removeStation( int idx, const Source &source ) noexcept;


    /**
     * @brief delete the observation at position iobs from scan
     * @author Matthias Schartner
     *
     * If a station has no longer any observations it gets removed
     *
     * @param iobs index of observation
     * @param source observed source
     * @return true if scan is still valid, false if scan is no longer valid
     */
    bool removeObservation( int iobs, const Source &source ) noexcept;


    /**
     * @brief finds the index of an station id
     * @author Matthias Schartner
     *
     * @param id station id
     * @return index
     */
    boost::optional<unsigned long> findIdxOfStationId( unsigned long id ) const noexcept;


    /**
     * @brief adds scan times
     * @author Matthias Schartner
     *
     * @param idx index of element
     * @param fieldSystem field system time in seconds
     * @param slew slew time in seconds
     * @param preob calibration time in secons
     */
    void addTimes( int idx, unsigned int fieldSystem, unsigned int slew, unsigned int preob ) noexcept;


    /**
     * @brief constructs all possible observations
     * @author Matthias Schartner
     *
     * @param source observed source
     */
    bool constructObservations( const Network &network, const Source &source ) noexcept;


    /**
     * @brief checks if the idle time is not too long
     * @author Matthias Schartner
     *
     * this function removes stations whichs idle times is too long. The order of the station pointing vectors must
     * be the same as the order of the max Idle parameter.
     *
     * @param maxIdle maximum allowed idle time
     * @param source observed source
     * @return true if scan is still valid, false if scan is no longer valid
     */
    bool checkIdleTimes( std::vector<unsigned int> &maxIdle, const Source &source ) noexcept;


    /**
     * @brief calculates the scan durations per observation
     * @author Matthias Schartner
     *
     * @param network station network
     * @param sources observed source
     * @param mode observing mode
     * @return true is scan is still valid, otherwise false
     */
    bool calcObservationDuration( const Network &network, const Source &sources,
                                  const std::shared_ptr<const Mode> &mode ) noexcept;

    /**
     * @brief calculates the average SNR of all observations in this scan
     * @author Matthias Schartner
     *
     * @param network station network
     * @param source observed source
     * @param mode observing mode
     * @return average SNR of all observations in this scan
     */
    double getAverageSNR( const Network &network, const Source &source, const std::shared_ptr<const Mode> &mode );

    /**
     * @brief calculates the total scan duration per station
     * @author Matthias Schartner
     *
     * removes stations if the scan duration is longer than the maximum allowed scan duration
     *
     * @param network station network
     * @param source observed source
     * @return true if scan is still valid, false if scan is no longer valid
     */
    bool scanDuration( const Network &network, const Source &source ) noexcept;


    /**
     * @brief getter for all station ids
     * @author Matthias Schartner
     *
     * @return all station ids
     */
    std::vector<unsigned long> getStationIds() const noexcept;


    /**
     * @brief calculates the score of a scan
     * @author Matthias Schartner
     *
     * usually used for single scan sources
     *
     * @param astas precalculated vector of average station score
     * @param asrcs precalculated vector of average source score
     * @param abls precalculated vector of average baseline score
     * @param minTime minimum time required for a scan in seconds
     * @param maxTime maximum time required for a scan in seconds
     * @param network station network
     * @param source observed source
     * @param subnetting subnetting flag
     * @param idleScore precalculated vector of extra scores due to long idle time
     */
    void calcScore( const std::vector<double> &astas, const std::vector<double> &asrcs, const std::vector<double> &abls,
                    unsigned int minTime, unsigned int maxTime, const Network &network, const Source &source,
                    bool subnetting, const std::vector<double> &idleScore ) noexcept;


    /**
     * @brief calculates the score of a scan
     * @author Matthias Schartner
     *
     * usually used for single scan sources
     *
     * @param astas precalculated vector of average station score
     * @param asrcs precalculated vector of average source score
     * @param abls precalculated vector of average baselines score
     * @param minTime minimum time required for a scan in seconds
     * @param maxTime maximum time required for a scan in seconds
     * @param network station network
     * @param source observed source
     * @param staids2skyCoverageScore stores the score of each pointing vector
     * @param idleScore precalculated vector of extra scores due to long idle time
     */
    void calcScore( const std::vector<double> &astas, const std::vector<double> &asrcs, const std::vector<double> &abls,
                    unsigned int minTime, unsigned int maxTime, const Network &network, const Source &source,
                    std::unordered_map<unsigned long, double> &staids2skyCoverageScore,
                    const std::vector<double> &idleScore ) noexcept;


    /**
     * @brief calculates the score of a scan
     * @author Matthias Schartner
     *
     * usually used for subnetting sources. This is for improved runtime because the skyCoverage score for each
     * pointing vector already exists.
     *
     * @param astas precalculated vector of average station score
     * @param asrcs precalculated vector of average source score
     * @param abls precalculated vector of average baseline score
     * @param minTime minimum time required for a scan in seconds
     * @param maxTime maximum time required for a scan in seconds
     * @param network station network
     * @param source observed source
     * @param staids2skyCoverageScore stored score for each pointing vector
     * @param idleScore precalculated vector of extra scores due to long idle time
     */
    void calcScore_subnetting( const std::vector<double> &astas, const std::vector<double> &asrcs,
                               const std::vector<double> &abls, unsigned int minTime, unsigned int maxTime,
                               const Network &network, const Source &source,
                               const std::unordered_map<unsigned long, double> &staids2skyCoverageScore,
                               const std::vector<double> &idleScore ) noexcept;


    /**
     * @brief calc score for high impact scans
     * @author Matthias Schartner
     *
     * @param minTime minimum time required for a scan in seconds
     * @param maxTime maximum time required for a scan in seconds
     * @param network station network
     * @param source observed source
     * @param hiscore high impact score
     * @param subnetting subnetting flag
     */
    void calcScore( unsigned int minTime, unsigned int maxTime, const Network &network, const Source &source,
                    double hiscore, bool subnetting );


    /**
     * @brief calculates the score for a astrometric calibrator block scan
     * @author Matthias Schartner
     *
     * @param prevLowElevationScores score for previouse low elevation scans
     * @param prevHighElevationScores score for previouse high elevation scans
     * @param network station network
     * @param minRequiredTime minimum time required for a scan in seconds
     * @param maxRequiredTime maximum time required for a scan in seconds
     * @param source observed source
     * @param subnetting subnetting flag
     */
    bool calcScore( const std::vector<double> &prevLowElevationScores,
                    const std::vector<double> &prevHighElevationScores, const Network &network,
                    unsigned int minRequiredTime, unsigned int maxRequiredTime, const Source &source, bool subnetting );


    /**
     * @brief calculates the score for a calibrator block scan
     * @author Matthias Schartner
     *
     * @param network station network
     * @param source observed source
     * @param astas precalculated vector of average station score
     * @param meanSNR average SNR
     * @param minRequiredTime minimum time required for a scan in seconds
     * @param maxRequiredTime maximum time required for a scan in seconds
     */
    void calcScoreCalibrator( const Network &network, const Source &source, const std::vector<double> &astas,
                              double meanSNR, unsigned int minRequiredTime, unsigned int maxRequiredTime );

    /**
     * @brief checks a scan with rigorous models
     * @author Matthias Schartner
     *
     * @param network station network
     * @param source observed source
     * @param mode observing mode
     * @param endposition required endposition
     * @return true if scan is still valid, false if scan is no longer valid
     */
    bool rigorousUpdate( Network &network, const Source &source, const std::shared_ptr<const Mode> &mode,
                         const boost::optional<StationEndposition> &endposition = boost::none ) noexcept;


    /**
     * @brief adds observation to scan in tagalong mode
     * @author Matthias Schartner
     *
     * @param pv_start pointing vector at start time
     * @param pv_end pointing vector at end time
     * @param observations all observations
     * @param slewtime slew time in seconds
     * @param station tagalong station
     */
    void addTagalongStation( const PointingVector &pv_start, const PointingVector &pv_end,
                             const std::vector<Observation> &observations, unsigned int slewtime,
                             const Station &station );


    /**
     * @brief makes a hard copy of a scan with all stations from parameter ids
     * @author Matthias Schartner
     *
     * @param ids ids of all stations which should be copied
     * @param source observed source
     * @return copy of scan with the stations from ids parameter or none if no valid scan can be created
     */
    boost::optional<Scan> copyScan( const std::vector<unsigned long> &ids, const Source &source ) const noexcept;


    /**
     * @brief getter for number of observations
     * @author Matthias Schartner
     *
     * @return number of baselines
     */
    unsigned long getNObs() const noexcept { return observations_.size(); }


    /**
     * @brief getter for number of observations per station
     * @author Matthias Schartner
     *
     * @param staid station id
     * @return number of observations with this station
     */
    unsigned long getNObs( unsigned long staid ) const noexcept;


    /**
     * @brief set fixed scan duration
     * @author Matthias Schartner
     *
     * @param scanDuration scan duration in seconds
     */
    void setFixedScanDuration( unsigned int scanDuration ) noexcept;


    /**
     * @brief outputs information of this scan to the current console
     * @author Matthias Schartner
     *
     * @param observed_scan_nr scan number
     * @param network station network
     * @param source observed source
     * @param of outstream file object
     */
    void output( unsigned long observed_scan_nr, const Network &network, const Source &source, std::ofstream &of ) const
        noexcept;


    /**
     * @brief set scan times
     * @author Matthias Schartner
     *
     * @param eols end of last scan per station
     * @param fieldSystemTime  field system time per station
     * @param slewTime slew time per station
     * @param preob calibration time per station
     * @param scanStart scan start time
     * @param observingTimes observing time per station
     * @return
     */
    bool setScanTimes( const std::vector<unsigned int> &eols, const std::vector<unsigned int> &fieldSystemTime,
                       const std::vector<unsigned int> &slewTime, const std::vector<unsigned int> &preob,
                       unsigned int scanStart, const std::vector<unsigned int> &observingTimes );


    /**
     * @brief set pointing vector at scan end time
     * @author Matthias Schartner
     *
     * @param pv_end pointing vector at scan end time
     */
    void setPointingVectorsEndtime( std::vector<PointingVector> pv_end );


    /**
     * @brief create dummy observations
     * @author Matthias Schartner
     *
     * @param network station network
     */
    void createDummyObservations( const Network &network );


    /**
     * @brief get total number of created scans
     * @author Matthias Schartner
     *
     * @return total number of created scans
     */
    static unsigned long numberOfCreatedObjects() { return nextId - 1; }


    /**
     * @brief set pointing vector
     * @author Matthias Schartner
     *
     * @param idx index of pointing vector
     * @param pv new pointing vector
     * @param ts time stamp flag
     */
    void setPointingVector( int idx, PointingVector pv, Timestamp ts );


    /**
     * @brief remove unnecessary observing time
     * @author Matthias Schartner
     *
     * @param network station network
     * @param thisSource observed source
     * @param of outfile stream
     * @param ts time stamp flag
     */
    void removeUnnecessaryObservingTime( Network &network, const Source &thisSource, std::ofstream &of, Timestamp ts );


    /**
     * @brief remove additional observing time
     * @author Matthias Schartner
     *
     * deprecated! use removeUnnecessaryObservingTime() istead
     *
     * @param time time
     * @param thisSta station
     * @param thisSource observed source
     * @param of outfile stream
     * @param ts time stamp flag
     */
    void removeAdditionalObservingTime( unsigned int time, const Station &thisSta, const Source &thisSource,
                                        std::ofstream &of, Timestamp ts );


    /**
     * @brief updates the time of each observation according to the observing times of the station
     * @author Matthias Schartner
     */
    void updateObservingTime();


    /**
     * @brief remove stations from scan whose observations last longer then session
     * @author Matthias Schartner
     *
     * @param network station network
     * @param source observed source
     * @param mode observing mode
     * @param endTime session end time
     * @return flag if scan is still valid
     */
    bool prepareForScanEnd( Network &network, const Source &source, const std::shared_ptr<const Mode> &mode,
                            unsigned int endTime );


    /**
     * @brief get scan name in vex format
     * @author Matthias Schartner
     *
     * Scan name is based on observation start.
     * If two scans start at same time a suffix is added to distinguish the scans.
     *
     * @param indexOfThisScanInList index of this scan in scan list
     * @param otherScans list of all other scans
     * @return name of this scan
     */
    std::string getName( unsigned long indexOfThisScanInList, const std::vector<Scan> &otherScans ) const;


    /**
     * @brief check if a obseration betwen two stations is scheduled
     * @author Matthias Schartner
     *
     * @param staid1 first station id
     * @param staid2 second station id
     * @return true if observation is scheduled
     */
    bool hasObservation( unsigned long staid1, unsigned long staid2 ) const;


    /**
     * @brief output observing duration in sked output format
     * @author Matthias Schartner
     *
     * @param source observedSource
     * @param nMaxSta maximum number of station
     * @return line output in sked format
     */
    std::string toSkedOutputTimes( const Source &source, unsigned long nMaxSta ) const;


    /**
     * @brief checks if station is participating and notes it in flag vector
     * @author Matthias Schartner
     *
     * @param flag flag lists which stations were found
     */
    void includesStations( std::vector<char> &flag ) const;


   private:
    static unsigned long nextId;  ///< next id for this object type

    unsigned long nsta_;   ///< number of stations in this scan
    unsigned long srcid_;  ///< observed source id

    double score_;  ///< total score

    ScanTimes times_;                                   ///< time information
    std::vector<PointingVector> pointingVectorsStart_;  ///< pointing vectors at start of the scan
    std::vector<PointingVector> pointingVectorsEnd_;    ///< pointing vectors at end of the scan
    std::vector<Observation> observations_;             ///< all observed baselines

    ScanType type_;                    ///< type of the scan
    ScanConstellation constellation_;  /// scan constellation type

    /**
     * @brief rigorous slew time calculation
     * @author Matthias Schartner
     *
     * @param network station network
     * @param source observed source
     * @return true if scan is still valid, otherwise false
     */
    bool rigorousSlewtime( Network &network, const Source &source ) noexcept;

    /**
     * @brief rigorous total observing duration calculation
     * @author Matthias Schartner
     *
     * @param network station network
     * @param source observed source
     * @return true if scan is still valid, otherwise false
     */
    bool rigorousTotalObservingDuration( Network &network, const Source &source ) noexcept;


    /**
     * @brief rigorous scan alignment
     * @author Matthias Schartner
     *
     * @param network station network
     * @param source observed source
     * @param mode observing mode
     * @return true if scan is still valid, otherwise false
     */
    bool rigorousScanStartTimeAlignment( Network &network, const Source &source,
                                         const std::shared_ptr<const Mode> &mode ) noexcept;


    /**
     * @brief rigorous scan visibility
     * @author Matthias Schartner
     *
     * @param network station network
     * @param source observed source
     * @param stationRemoved flag if a station got removed
     * @return true if scan is still valid, otherwise false
     */
    bool rigorousScanVisibility( Network &network, const Source &source, bool &stationRemoved ) noexcept;


    /**
     * @brief rigorous check if scan can reach required endposition
     * @author Matthias Schartner
     *
     * @param network station network
     * @param thisSource observed source
     * @param endposition required endposition
     * @param stationRemoved flag if a station got removed
     * @return true if scan is still valid, otherwise false
     */
    bool rigorousScanCanReachEndposition( const Network &network, const Source &thisSource,
                                          const boost::optional<StationEndposition> &endposition,
                                          bool &stationRemoved );


    /**
     * @brief calculates the score for number of observations
     * @author Matthias Schartner
     *
     * @param maxObs maximum possible number of observations
     * @return score
     */
    double calcScore_numberOfObservations( unsigned long maxObs ) const noexcept;


    /**
     * @brief calculates score for average baseline observations
     * @author Matthias Schartner
     *
     * @param abls precalculated vector of average baseline observations
     * @return score
     */
    double calcScore_averageBaselines( const std::vector<double> &abls ) const noexcept;


    /**
     * @brief calculates score for average station observations
     * @author Matthias Schartner
     *
     * @param astas precalculated vector of average station observations
     * @param nMaxBls maximum possible number of baselines
     * @return score
     */
    double calcScore_averageStations( const std::vector<double> &astas, unsigned long nMaxBls ) const noexcept;


    /**
     * @brief calculates score for average source observations
     * @author Matthias Schartner
     *
     * @param asrcs precalculated vector of average source observations
     * @param nMaxBls maximum possible number of baselines
     * @return score
     */
    double calcScore_averageSources( const std::vector<double> &asrcs, unsigned long nMaxBls ) const noexcept;


    /**
     * @brief calculates score for duration
     * @author Matthias Schartner
     *
     * @param nMaxSta number of stations in network
     * @param minTime minimum time required for a scan in seconds
     * @param maxTime maximum time required for a scan in seconds
     * @return score
     */
    double calcScore_duration( unsigned long nMaxSta, unsigned int minTime, unsigned int maxTime ) const noexcept;


    /**
     * @brief calculates score based on idle time
     * @author Matthias Schartner
     *
     * @param idleScore precalculated vector of extra scores due to long idle time
     * @return score
     */
    double calcScore_idleTime( const std::vector<double> &idleScore ) const noexcept;


    /**
     * @brief mean of the weight factors for each participating station
     * @author Matthias Schartner
     *
     * @param stations list of all stations
     * @return mean of weight factors
     */
    double weight_stations( const std::vector<Station> &stations );


    /**
     * @brief mean of the weight factors for each participating baselines
     * @author Matthias Schartner
     *
     * @return mean of the weight factors
     */
    double weight_baselines( const std::vector<Baseline> &baselines );


    /**
     * @brief calculate score for low elevation scans
     * @author Matthias Schartner
     *
     * @param nmaxsta number of stations in network
     * @return score for low elevation scans
     */
    double calcScore_lowElevation( unsigned long nmaxsta );


    /**
     * @brief calculate score for low declination scans
     * @author Matthias Schartner
     *
     * @param nMaxObs maximum number of possible observations in network
     * @return score for low declination scans
     */
    double calcScore_lowDeclination( unsigned long nMaxObs );


    /**
     * @brief first part of score calculation
     * @author Matthias Schartner
     *
     * mainly additive scores
     *
     * @param astas precalculated vector of average station observations
     * @param asrcs precalculated vector of average source observations
     * @param abls precalculated vector of average baseline observations
     * @param minTime minimum time required for a scan in seconds
     * @param maxTime maximum time required for a scan in seconds
     * @param network station network
     * @param source observed source
     * @param subnetting subnetting flag
     * @param idleScore precalculated vector of extra scores due to long idle time
     * @return score
     */
    double calcScore_firstPart( const std::vector<double> &astas, const std::vector<double> &asrcs,
                                const std::vector<double> &abls, unsigned int minTime, unsigned int maxTime,
                                const Network &network, const Source &source, bool subnetting,
                                const std::vector<double> &idleScore );


    /**
     * @brief second part of score calculations
     * @author Matthias Schartner
     *
     * mainly multiplicative scores
     *
     * @param this_score current score
     * @param network station network
     * @param source observed source
     * @return total score
     */
    double calcScore_secondPart( double this_score, const Network &network, const Source &source );
};
}  // namespace VieVS
#endif /* SCAN_H */
