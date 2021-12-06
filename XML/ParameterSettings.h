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
 * @file ParameterSettings.h
 * @brief class ParameterSettings
 *
 * @author Matthias Schartner
 * @date 23.08.17
 */

#ifndef PARAMETERSETTINGS_H
#define PARAMETERSETTINGS_H


#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <ostream>
#include <unordered_map>
#include <utility>

#include "../Misc/CalibratorBlock.h"
#include "../Misc/util.h"
#include "ParameterGroup.h"
#include "ParameterSetup.h"
#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#endif


namespace VieVS {
/**
 * @class ParameterSettings
 * @brief This class stores all setup information and writes a VieSchedpp.xml file
 *
 * @author Matthias Schartner
 * @date 22.08.2017
 */
class ParameterSettings {
   public:
    /**
     * @brief possible setup and optimization flags
     * @author Matthias Schartner
     */
    enum class Type {
        station,     ///< stations wise
        source,      ///< source wise
        satellite,   ///< satellite wise
        spacecraft,  /// spacecraft wise
        baseline,    ///< baseline wise
    };

    /**
     * @brief contact information
     * @author Matthias Schartner
     */
    struct Contact {
        std::string function;     ///< contact function e.g.: PI, scheduler, operator...
        std::string name;         ///< contact name
        std::string email;        ///< contact email
        std::string phone;        ///< contact phone number
        std::string affiliation;  ///< contact affiliation
    };


    /**
     * @brief station parameters
     * @author Matthias Schartner
     */
    struct ParametersStations {
        boost::optional<bool>
            firstScan;  ///< if set to true: no time is spend for setup, source, tape, calibration, and slewing
        boost::optional<bool> available;               ///< if set to true: this station is available for a scan
        boost::optional<bool> tagalong;                ///< if set to true: station is in tagalong mode
        boost::optional<bool> availableForFillinmode;  ///< if set to true: station is available for fillin mode scans

        std::unordered_map<std::string, double> minSNR;  ///< minimum required signal to noise ration for each band

        boost::optional<unsigned int> minSlewtime;       ///< maximum allowed slewtime in seconds
        boost::optional<unsigned int> maxSlewtime;       ///< maximum allowed slewtime in seconds
        boost::optional<double> maxSlewDistance;         ///< maximum slew distance in degrees
        boost::optional<double> minSlewDistance;         ///< minimum slew distance in degrees
        boost::optional<unsigned int> maxWait;           ///< maximum allowed wait time for slow antennas in seconds
        boost::optional<unsigned int> maxScan;           ///< maximum allowed scan time in seconds
        boost::optional<unsigned int> minScan;           ///< minimum required scan time in seconds
        boost::optional<double> minElevation;            ///< minimum elevation of parameter in degrees
        boost::optional<unsigned int> maxNumberOfScans;  ///< maximum number of scans
        boost::optional<unsigned int> maxTotalObsTime;   ///< maximum total observing time
        boost::optional<double> dataWriteRate;           ///< maximum data write speed to disk in Mbps

        boost::optional<double> weight;  ///< multiplicative factor of score for scans with this station

        std::vector<std::string> ignoreSourcesString;  ///< list of all source names which should be ignored
        std::vector<unsigned long> ignoreSources;      ///< list of all source ids which should be ignored

        boost::optional<unsigned int> systemDelay;  ///< time required for field system commands in seconds
        boost::optional<unsigned int> preob;        ///< time required for calibration in seconds
        boost::optional<unsigned int> midob;        ///< extra observing time for correlator synchronization in seconds
    };

    /**
     * @brief try to focus weight increase occurrency
     * @author Matthias Schartner
     */
    enum class TryToFocusOccurrency {
        once,    ///< increase weight only once
        perScan  ///< increadse weight after each scan
    };

    /**
     * @brief try to focus weight increase type
     * @author Matthias Schartner
     */
    enum class TryToFocusType {
        additive,       ///< add weight increase value
        multiplicative  ///< multiply with weight increase value
    };

    /**
     * @brief source parameters
     * @author Matthias Schartner
     */
    struct ParametersSources {
        boost::optional<bool> available;               ///< flag if source is available
        boost::optional<bool> availableForFillinmode;  ///< flas if source is available for fillin mode scans

        boost::optional<double> weight;  ///< multiplicative factor of score for scans to this source

        std::unordered_map<std::string, double> minSNR;  ///< minimum required signal to noise ration for each band

        boost::optional<unsigned int> minNumberOfStations;  ///< minimum number of stations for a scan
        boost::optional<double> minFlux;                    ///< minimum flux density required for this source in jansky
        boost::optional<unsigned int> minRepeat;  ///< minimum time between two observations of this source in seconds
        boost::optional<unsigned int> maxScan;    ///< maximum allowed scan time in seconds
        boost::optional<unsigned int> minScan;    ///< minimum required scan time in seconds
        boost::optional<unsigned int> maxNumberOfScans;  ///< maximum number of scans
        boost::optional<bool> tryToFocusIfObservedOnce;  ///< flag if this source should be focused after observed once
        boost::optional<double> tryToFocusFactor;        ///< weight increase value
        boost::optional<TryToFocusOccurrency> tryToFocusOccurrency;  ///< try to focus weight increase occurrency
        boost::optional<TryToFocusType> tryToFocusType;              ///< try to focus weight increase type
        boost::optional<double> minElevation;                        ///< minimum elevation in degrees
        boost::optional<double> minSunDistance;                      ///< minimum sun distance in degrees
        boost::optional<double> jetAngleBuffer;                   ///< avoid obs along jet angles +- buffer
        boost::optional<double> jetAngleFactor;                   ///< avoid obs along jet angles +- factor*std
        boost::optional<bool> forceSameObservingDuration;         ///< same scan duration for all stations within a scan

        boost::optional<unsigned int>
            tryToObserveXTimesEvenlyDistributed;  ///< tries to observe a source X times over the time span in which the
        ///< source is scanable. Overwrites maxScan and
        ///< tryToFocusIfObservedOnce.
        boost::optional<unsigned int>
            tryToObserveXTimesMinRepeat;  ///< tries to observe a source X times: backup minimum time span between two
        ///< observations to this source in seconds
        boost::optional<unsigned int> fixedScanDuration;  ///< optional fixed scan duration in seconds

        std::vector<std::string> ignoreStationsString;   ///< list of all station names which should be ignored
        std::vector<unsigned long> ignoreStations;       ///< list of all station names which should be ignored
        std::vector<std::string> ignoreBaselinesString;  ///< list of all baseline names which should be ignore
        std::vector<unsigned long> ignoreBaselines;      ///< list of all baseline names which should be ignore
        std::vector<std::string>
            requiredStationsString;  ///< list of station names which are required for a scan to this source
        std::vector<unsigned long>
            requiredStations;  ///< list of station names which are required for a scan to this source
    };

    /**
     * @brief baseline parameters
     * @author Matthias Schartner
     */
    struct ParametersBaselines {
        std::unordered_map<std::string, double> minSNR;  ///< minimum SNR per band for each baseline
        boost::optional<bool> ignore;                    ///< ignore specific baselines

        boost::optional<double> weight;         ///< multiplicative factor of score for scans with this baseline
        boost::optional<unsigned int> minScan;  ///< minimum required scan duration of this baseline
        boost::optional<unsigned int> maxScan;  ///< maximum allowed scan duration of this baseline
    };

    /**
     * @brief all possible observation mode flux information type
     * @author Matthias Schartner
     */
    enum class ObservationModeProperty {
        required,  ///< this band information is required. If this information is missing this object is not used.
        optional,  ///< this band information is only optional. If information is available it is used, otherwise it is
        ///< interpolated
    };

    /**
     * @brief all possible observation mode backup models
     * @author Matthias Schartner
     */
    enum class ObservationModeBackup {
        minValueTimes,  ///< use minimum value found in other bands times a factor
        maxValueTimes,  ///< use maximum value found in other bands times a factor
        internalModel,  ///< use internal model to derive flux density of sources (sources only)
        value,          ///< use specific value
        none,           ///< no backup model
    };


    /**
     * @brief empty default constructor
     * @author Matthias Schartner
     */
    ParameterSettings();


    /**
     * @brief software block in parameter.xml
     * @author Matthias Schartner
     *
     * @param name software name
     * @param version software version
     */
    void software( const std::string &name, const std::string &version );


    /**
     * @brief general block in parameter.xml
     * @author Matthias Schartner
     *
     * @param experimentName experiment name
     * @param startTime session start time
     * @param endTime session end time
     * @param subnetting flag if subnetting is allowed
     * @param subnettingAngle minimum angular distance between subnetting sources in degrees
     * @param useSubnettingPercent_otherwiseAllBut flag to choose minimum number of stations per subnetting scan model
     * @param subnettingNumber value for minimum number of stations per subnetting scan model
     * @param fillinmodeInfluenceOnSchedule flag if fillin mode scans should have an influence on the schedule
     * @param fillinmodeDuringScan flag if fillin mode scans are allowed
     * @param fillinmodeAPosteriori flag if fillin mode a posterior scans are allowed
     * @param fillinmodeAPosteriori_minStations override source parameter "minStations" otherwise -1
     * @param fillinmodeAPosteriori_minRepeat override source parameter "minRepeat" otherwise -1
     * @param idleToObservingTime flag if idle time should be converted to observing time
     * @param idleToObservingTimeGroup station group for idle time to observing time
     * @param stations list of all stations
     * @param useSourcesFromParameter_otherwiseIgnore flag which model is used for srcNames
     * @param srcNames source names for model from useSourcesFromParameter_otherwiseIgnore
     * @param satelliteNames names of satellites that should be scheduled
     * @param scanAlignment scan alignment flag
     * @param logConsole log level for console
     * @param logFile log level for file
     * @param doNotObserveSourcesWithinMinRepeat consider scans (with reduced weight) if they are within min repeat time
     * @param versionOffset version offset
     * @param bool ignore_successive_scans_same_source should be true
     */
    void general( const std::string &experimentName, const boost::posix_time::ptime &startTime,
                  const boost::posix_time::ptime &endTime, bool subnetting, double subnettingAngle,
                  bool useSubnettingPercent_otherwiseAllBut, double subnettingNumber,
                  bool fillinmodeInfluenceOnSchedule, bool fillinmodeDuringScan, bool fillinmodeAPosteriori,
                  int fillinmodeAPosteriori_minStations, int fillinmodeAPosteriori_minRepeat, bool idleToObservingTime,
                  std::string idleToObservingTimeGroup, const std::vector<std::string> &stations,
                  bool useSourcesFromParameter_otherwiseIgnore, const std::vector<std::string> &srcNames,
                  const std::vector<std::string> &satelliteNames, const std::string &scanAlignment,
                  const std::string &logConsole, const std::string &logFile, bool doNotObserveSourcesWithinMinRepeat,
                  int versionOffset, bool ignore_successive_scans_same_source );


    /**
     * @brief created block in VieSchedpp.xml
     * @author Matthias Schartner
     *
     * @param time created time stamp
     * @param name created name
     * @param email created email
     */
    void created( const boost::posix_time::ptime &time, std::string name, std::string email );


    /**
     * @brief catalogs block in VieSchedpp.xml
     * @author Matthias Schartner
     *
     * @param antenna antenna catalog name
     * @param equip equip catalog name
     * @param flux flux catalog name
     * @param freq freq catalog name
     * @param hdpos hdpos catalog name
     * @param loif loif catalog name
     * @param mask mask catalog name
     * @param modes modes catalog name
     * @param position position catalog name
     * @param rec rec catalog name
     * @param rx rx catlog name
     * @param source source catalog name
     * @param tracks tracks catalog name
     * @param satellites satellite TLE file
     * @param stp path
     */
    void catalogs( const std::string &antenna, const std::string &equip, const std::string &flux,
                   const std::string &freq, const std::string &hdpos, const std::string &loif, const std::string &mask,
                   const std::string &modes, const std::string &position, const std::string &rec, const std::string &rx,
                   const std::string &source, const std::string &tracks, const std::string &satellites,
                   const std::string &stp );


    /**
     * @brief group defined in VieSchedpp.xml
     * @author Matthias Schartner
     *
     * @param type group type
     * @param group group object
     */
    void group( Type type, ParameterGroup group );


    /**
     * @brief returns all members of a defined group
     * @author Matthias Schartner
     *
     * @param type group type
     * @param groupName group name
     * @return list of all group members
     */
    const std::vector<std::string> &getGroupMembers( Type type, std::string groupName );


    /**
     * @brief write defined station parameters to VieSchedpp.xml
     * @author Matthias Schartner
     *
     * @param name parameter name
     * @param PARA parameters
     */
    void parameters( const std::string &name, const ParametersStations &PARA );


    /**
     * @brief defined station parameters
     * @author Matthias Schartner
     *
     * @param name parameter name
     * @param PARA parameters
     * @return property tree with station parameters
     */
    static boost::property_tree::ptree parameterStation2ptree( const std::string &name,
                                                               const ParametersStations &PARA );


    /**
     * @brief transforms parameterStations to property_tree
     * @author Matthias Schartner
     *
     * @param ptree property tree
     * @return first entry is parameter name, second are station parameters
     */
    static std::pair<std::string, ParametersStations> ptree2parameterStation( boost::property_tree::ptree ptree );


    /**
     * @brief write defined source parameters to VieSchedpp.xml
     * @author Matthias Schartner
     *
     * @param name parameter name
     * @param PARA parameters
     */
    void parameters( const std::string &name, const ParametersSources &PARA, Type type = Type::source );


    /**
     * @brief defined source parameters
     * @author Matthias Schartner
     *
     * @param name parameter name
     * @param PARA parameters
     * @return property tree with source parameters
     */
    static boost::property_tree::ptree parameterSource2ptree( const std::string &name, const ParametersSources &PARA );


    /**
     * @brief transforms parameterSource to property_tree
     * @author Matthias Schartner
     *
     * @param ptree property tree
     * @return first entry is parameter name, second are source parameters
     */
    static std::pair<std::string, ParametersSources> ptree2parameterSource( boost::property_tree::ptree ptree );


    /**
     * @brief write defined baseline parameters to VieSchedpp.xml
     * @author Matthias Schartner
     *
     * @param name parameter name
     * @param PARA parameters
     */
    void parameters( const std::string &name, const ParametersBaselines &PARA );


    /**
     * @brief defined baseline parameters
     * @author Matthias Schartner
     *
     * @param name parameter name
     * @param PARA parameters
     * @return property tree with baseline parameters
     */
    static boost::property_tree::ptree parameterBaseline2ptree( const std::string &name,
                                                                const ParametersBaselines &PARA );


    /**
     * @brief transforms ParametersBaselines to property_tree
     * @author Matthias Schartner
     *
     * @param ptree property tree
     * @return first entry is parameter name, second are baseline parameters
     */
    static std::pair<std::string, ParametersBaselines> ptree2parameterBaseline( boost::property_tree::ptree ptree );


    /**
     * @brief setup block in parameter.xml
     * @author Matthias Schartner
     *
     * @param type flag where this setup belongs
     * @param setup setup object
     */
    void setup( Type type, const ParameterSetup &setup );


    /**
     * @brief station wait times block
     * @author Matthias Schartner
     *
     * @param name group, station name or "__all__"
     * @param fieldSystem time for field system commands in seconds
     * @param preob time for preob in seconds
     * @param midob extra midob time for correlator synchronization
     * @param postob time for postob
     */
    void stationWaitTimes( const std::string &name, unsigned int fieldSystem, unsigned int preob, unsigned int midob,
                           unsigned int postob );


    /**
     * @brief station cable wrap buffer block
     * @author Matthias Schartner
     *
     * @param name group, station name or "__all__"
     * @param axis1LowOffset axis 1 lower offset in degrees
     * @param axis1UpOffset axis 1 upper offset in degrees
     * @param axis2LowOffset axis 2 lower offset in degrees
     * @param axis2UpOffset axis 2 upper offset in degrees
     */
    void stationCableWrapBuffer( const std::string &name, double axis1LowOffset, double axis1UpOffset,
                                 double axis2LowOffset, double axis2UpOffset );


    /**
     * @brief skyCoverage block in parameter.xml
     * @author Matthias Schartner
     *
     * @param influenceDistance maximum angular influence distance in degrees
     * @param influenceInterval maximum time influence distance in seconds
     * @param maxTwinTelecopeDistance maximum distance between twin telescopes
     * @param interpolationDistance distance model function
     * @param interpolationTime time model function
     */
    void skyCoverage( double influenceDistance, unsigned int influenceInterval, double maxTwinTelecopeDistance,
                      std::string interpolationDistance, std::string interpolationTime );


    /**
     * @brief weightFactor block in parameter.xml
     * @author Matthias Schartner
     *
     * @param weight_skyCoverage weight of sky Coverage
     * @param weight_numberOfObservations weight of number of observations
     * @param weight_duration weight of duration of the scan
     * @param weight_averageSources weight of an average source observation distribution
     * @param weight_closure weight for number of closures
     * @param closures_max max number of closures assumed to be significant
     * @param weight_averageStations weight of an average station usage distribution
     * @param weight_averageBaselines weight of an average baselines usage distribution
     * @param weight_idleTime extra weight for station after long idle time
     * @param intervalIdleTime extra weight for stations after long idle time model in seconds
     * @param weightDeclination weight factor for declination
     * @param declinationSlopeStart start declination of additional weight (everything above has factor 0)
     * @param declinationSlopeEnd end declination of additional declination weight slope (everything below has factor 1)
     * @param weightLowElevation weight factor for low elevation scans
     * @param lowElevationSlopeStart start elevation of additional weight (everything above has factor 0)
     * @param lowElevationSlopeEnd end elevation of additional declination weight slope (everything below has factor 1)
     */
    void weightFactor( double weight_skyCoverage, double weight_numberOfObservations, double weight_duration,
                       double weight_averageSources, double weight_closure, unsigned int closures_max,
                       double weight_averageStations, double weight_averageBaselines,
                       double weight_idleTime, unsigned int intervalIdleTime, double weightDeclination,
                       double declinationSlopeStart, double declinationSlopeEnd, double weightLowElevation,
                       double lowElevationSlopeStart, double lowElevationSlopeEnd );


    /**
     * @brief conditions block in paramter.xml
     * @author Matthias Schartner
     *
     * @param members group, source name or "__all__"
     * @param minScans minimum number of scans
     * @param minBaselines minimum number of observations
     * @param andForCombination combination model
     * @param maxNumberOfIterations maximum number of iterations
     * @param numberOfGentleSourceReductions number of gentle source reductions
     * @param minNumberOfSourcesToReduce minimum number of sources to reduce
     * @param percentage percentage of sources to reduce during gentle source reduction
     */
    void conditions( std::vector<std::string> members, std::vector<int> minScans, std::vector<int> minBaselines,
                     bool andForCombination, int maxNumberOfIterations, int numberOfGentleSourceReductions,
                     int minNumberOfSourcesToReduce, double percentage );


    /**
     * @brief custom mode block in parameter.xml
     * @author Matthias Schartner
     *
     * @param sampleRate sample rate
     * @param bits bits
     * @param efficiencyFactor recording efficiency factor (use -1 for internal calculation)
     */
    void mode( double sampleRate, unsigned int bits, double efficiencyFactor = -1 );


    /**
     * @brief mode block in parameter.xml
     * @author Matthias Schartner
     *
     * @param skdMode name of observing mode in skd catalogs
     */
    void mode( const std::string &skdMode );


    /**
     * @brief custom observing mode to parameter.xml file
     * @author Matthias Schartner
     *
     * @param obsMode custom observing mode property tree
     */
    void mode( const boost::property_tree::ptree &obsMode );


    /**
     * @brief band sub-block in mode block in parameter.xml
     * @author Matthias Schartner
     *
     * @param name band name
     * @param wavelength band wavelength
     * @param channels number of channels
     */
    void mode_band( const std::string &name, double wavelength, unsigned int channels );


    /**
     * @brief bandPolicy sub-block in mode block in parameter.xml
     * @author Matthias Schartner
     *
     * @param name band name
     * @param minSNR minimum signal to noise ratio
     * @param station station policy for this band
     * @param stationBackup station backup model
     * @param stationBackupValue station backup model value
     * @param source source policy for this band
     * @param sourceBackup source backup model
     * @param sourceBackupValue source backup model value
     */
    void mode_bandPolicy( const std::string &name, double minSNR, ObservationModeProperty station,
                          ObservationModeBackup stationBackup, double stationBackupValue,
                          ObservationModeProperty source, ObservationModeBackup sourceBackup,
                          double sourceBackupValue );


    /**
     * @brief multisched block in parameter.xml
     * @author Matthias Schartner
     *
     * @param multiSched multisched xml tree
     * @param number maximum number of schedules model ("all" or "select")
     * @param maxn maximum number of schedules number
     * @param useSeed use seed model ("random" or "select")
     * @param seed number
     * @param pick_random default is false
     */
    void multisched( const boost::property_tree::ptree &multiSched, const std::string &number, int maxn,
                     const std::string &useSeed, int seed, bool pick_random = false );


    /**
     * @brief multisched block in parameter.xml
     * @author Matthias Schartner
     *
     * @param maxIterations maximum number of evolutions
     * @param populationSize population size after crossover
     * @param selectBest percentage of best parents to select
     * @param selectRandom percentage of random parents to select
     * @param mutation mutation acceleration
     * @param minMutation minimum mutation
     * @param parents number of parents for crossover
     */
    void mulitsched_genetic( int maxIterations, int populationSize, double selectBest, double selectRandom,
                             double mutation, double minMutation, int parents );

    /**
     * @brief multiCore multi core support for scheduling
     * @author Matthias Schartner
     *
     * @param threads thread creation schema
     * @param nThreadsManual number of manually selected threads
     * @param jobScheduler job scheduling algorithmus
     * @param chunkSize job scheduling chunk size
     */
    void multiCore( const std::string &threads, int nThreadsManual, const std::string &jobScheduler, int chunkSize );


    /**
     * @brief output routine that produces .xml file
     * @author Matthias Schartner
     *
     * @param name output file name (usually "parameter")
     */
    void write( const std::string &name );


    /**
     * @brief output block in parameter.xml
     * @author Matthias Schartner
     *
     * @param experimentDescription experiment description
     * @param scheduler scheduler
     * @param correlator correlator
     * @param notes schedule notes
     * @param initializer create initializer
     * @param iteration_log iteration log file
     * @param createSummary create summary file
     * @param createNGS create NGS file
     * @param NGS_directory path to NGS output (empty string for default path)
     * @param createSKD create SKD file
     * @param createVex create VEX file
     * @param createSnrTable create SNR table
     * @param operNotes create operation notes file
     * @param srcGrp create source group statistics file
     * @param srcGroupsForStatistic groups for source group statistics file
     * @param createSlewFile create slew file
     * @param contacts list of contacts
     */
    void output( const std::string &experimentDescription, const std::string &scheduler, const std::string &correlator,
                 const std::string &notes, bool initializer, bool iteration_log, bool createSummary, bool createNGS,
                 const std::string &NGS_directory, bool createSKD, bool createVex, bool createSnrTable, bool operNotes,
                 bool srcGrp, const std::vector<std::string> &srcGroupsForStatistic, bool createSlewFile,
                 const std::vector<Contact> &contacts );


    /**
     * @brief specify the sequence in which sources should be observed
     * @author Matthias Schartner
     *
     * If you say cadence = 5, modulo = 0, source = A than every 5th selected scan(or subnetting sequence) will be
     * (if possible) to a source of A. This does not effect the first scan of a schedule!
     *
     * You can use the modulo number to to combine different rules:
     * cadence = 10, modulo = 0, source = "1234+567" and cadence = 10, modulo = 1, source = "1234+568" means that
     * every 10th scan will be (if possible) to source 1234+567 followed by a scan to "1234+568".
     *
     * @param cadence cadence for this rule
     * @param modulo modulo operator for number of scan and cadence
     * @param member source(group) which should be observed
     */
    void ruleScanSequence( unsigned int cadence, const std::vector<unsigned int> &modulo,
                           const std::vector<std::string> &member );


    /**
     * @brief focus observations at the corners of the commonly visible sky (for intensives)
     * @author Matthias Schartner
     *
     * @param cadence cadence for focusing on one corner
     */
    void ruleFocusCorners( int cadence );

    /**
     * @brief define fringe finder block
     * @author Matthias Schartner
     *
     * @param blocks list of all calibration blocks
     * @param intent intent string used in VEX file ("NONE" if empty)
     */
    void calibratorBlock( const std::vector<CalibratorBlock> &blocks, std::string intent );

    /**
     * @brief calibrator block in paramters.xml
     * @author Matthias Schartner
     *
     * @param cadence cadence in seconds
     * @param member calibrator source name
     * @param between_elevation target elevations
     * @param nMaxScans maximum number of scans
     * @param scanTime fixed scan time
     */
    void ruleCalibratorBlockTime( unsigned int cadence, const std::string &member,
                                  const std::vector<std::pair<double, double>> &between_elevation,
                                  unsigned int nMaxScans, unsigned int scanTime );


    /**
     * @brief calibrator block in paramters.xml
     * @author Matthias Schartner
     *
     * @param cadence cadence in number of scans
     * @param member calibrator source name
     * @param between_elevation target elevations
     * @param nMaxScans maximum number of scans
     * @param scanTime fixed scan time
     */
    void ruleCalibratorBlockNScanSelections( unsigned int cadence, const std::string &member,
                                             const std::vector<std::pair<double, double>> &between_elevation,
                                             unsigned int nMaxScans, unsigned int scanTime );


    /**
     * @brief high impact block in VieSchedpp.xml file
     * @author Matthias Schartner
     *
     * @param members high impact stations
     * @param azs target azimuths in degrees
     * @param els target elevations in degrees
     * @param margins target azimuth, elevation margins in degrees
     * @param interval check interval in seconds
     * @param repeat minimum amount of time between tow scans in seconds
     */
    void highImpactAzEl( const std::vector<std::string> &members, const std::vector<double> &azs,
                         const std::vector<double> &els, const std::vector<double> &margins, int interval, int repeat );


    /**
     * @brief adds simulator settings to xml file
     * @author Matthias Schartner
     *
     * @param tree property tree with simulator settings
     */
    void simulator( const boost::property_tree::ptree &tree );

    /**
     * @brief adds solver settings to xml file
     * @author Matthias Schartner
     *
     * @param tree property tree with solver settings
     */
    void solver( const boost::property_tree::ptree &tree );

    /**
     * @brief adds priorities settings to xml file
     * @author Matthias Schartner
     *
     * @param tree property tree with priorities settings
     */
    void priorities( const boost::property_tree::ptree &tree );

    /**
     * @brief get station groups
     * @author Matthias Schartner
     *
     * @return all station groups
     */
    const std::map<std::string, std::vector<std::string>> &getGroupStations() const { return groupStations_; }


    /**
     * @brief get source groups
     * @author Matthias Schartner
     *
     * @return all source groups
     */
    const std::map<std::string, std::vector<std::string>> &getGroupSources() const { return groupSources_; }


    /**
     * @brief get satellite groups
     * @author Matthias Schartner
     *
     * @return all satellite groups
     */
    const std::map<std::string, std::vector<std::string>> &getGroupSatellites() const { return groupSatellites_; }


    /**
     * @brief get spacecraft groups
     * @author Matthias Schartner
     *
     * @return all spacecraft groups
     */
    const std::map<std::string, std::vector<std::string>> &getGroupSpacecrafts() const { return groupSpacecrafts_; }


    /**
     * @brief get baseline groups
     * @author Matthias Schartner
     *
     * @return all baseline groups
     */
    const std::map<std::string, std::vector<std::string>> &getGroupBaselines() const { return groupBaselines_; }


    /**
     * @brief get all station parameters
     * @author Matthias Schartner
     *
     * @return all station parameters and name
     */
    const std::map<std::string, ParametersStations> &getParaStations() const { return paraStations_; }


    /**
     * @brief get all source parameters
     * @author Matthias Schartner
     *
     * @return all source parameters and name
     */
    const std::map<std::string, ParametersSources> &getParaSources() const { return paraSources_; }


    /**
     * @brief get baseline parameters
     * @author Matthias Schartner
     *
     * @return all baseline parameters and name
     */
    const std::map<std::string, ParametersBaselines> &getParaBaselines() const { return paraBaselines_; }


    /**
     * @brief adds a priori scans
     * @author Matthias Schartner
     *
     * @param tree xml tree containing scans
     * @param prefix prefix where property tree should be inserted
     */
    void addAPrioriScans( const boost::property_tree::ptree &tree, const std::string &prefix );


   private:
    boost::property_tree::ptree master_;  ///< master property tree

    std::map<std::string, std::vector<std::string>> groupStations_;     ///< defined station group
    std::map<std::string, std::vector<std::string>> groupSources_;      ///< defined source group
    std::map<std::string, std::vector<std::string>> groupSatellites_;   ///< defined satellite group
    std::map<std::string, std::vector<std::string>> groupSpacecrafts_;  ///< defined spacecraft group
    std::map<std::string, std::vector<std::string>> groupBaselines_;    ///< defined baseline group

    std::map<std::string, ParametersStations> paraStations_;    ///< defined station parameters
    std::map<std::string, ParametersSources> paraSources_;      ///< defined source parameters
    std::map<std::string, ParametersSources> paraSatellites_;   ///< defined satellite parameters
    std::map<std::string, ParametersSources> paraSpacecrafts_;  ///< defined spacecraft parameters
    std::map<std::string, ParametersBaselines> paraBaselines_;  ///< defined baseline parameters

    /**
     * @brief returns a child tree of root setup object
     * @author Matthias Schartner
     *
     * @param setup root setup object
     * @return child property tree
     */
    boost::property_tree::ptree getChildTree( const ParameterSetup &setup );

    /**
     * @brief add contact information to xml tree
     * @author Matthias Schartner
     *
     * @param contact contact information
     * @param tree xml tree
     * @param node node where contact information should be added
     */
    void addContact( const Contact &contact, boost::property_tree::ptree &tree, const std::string &node );


    /**
     * @brief read contact information from xml tree
     * @author Matthias Schartner
     *
     * @param tree xml tree with contact information
     */
    Contact readContact( const boost::property_tree::ptree &tree );
};
}  // namespace VieVS

#endif  // PARAMETERSETTINGS_H
