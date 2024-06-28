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
 * @file Scheduler.h
 * @brief class Scheduler
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H


#include <boost/date_time.hpp>
#include <boost/optional.hpp>
#include <tuple>
#include <utility>
#include <vector>

#include "Algorithm/FocusCorners.h"
#include "Initializer.h"
#include "Misc/Constants.h"
#include "Misc/StationEndposition.h"
#include "Misc/Subnetting.h"
#include "Scan/Subcon.h"
#include "Station/Network.h"


namespace VieVS {
/**
 * @class Scheduler
 * @brief this is the VLBI scheduling class which is responsible for the scan selection and the creation of the
 * schedule
 *
 * @author Matthias Schartner
 * @date 28.06.2017
 */
class Scheduler : public VieVS_NamedObject {
    friend class Output;


   public:
    /**
     * @brief general parameters used for scheduling
     * @author Matthias Schartner
     */
    struct Parameters {
        std::shared_ptr<Subnetting> subnetting = nullptr;  ///< subnetting parameters
        double subnettingMinNSta = 0.60;                   /// minimum number of subnetting station percent (deprecated)
        bool fillinmodeDuringScanSelection = true;         ///< flag if fillin modes are allowed
        bool fillinmodeInfluenceOnSchedule = true;         ///< fillin modes scans influence schedule if set to true
        bool fillinmodeAPosteriori = false;                ///< fillin mode a posteriori
        boost::optional<int> fillinmodeAPosteriori_minSta =
            boost::none;  ///< fillin mode a posteriori min number of stations
        boost::optional<int> fillinmodeAPosteriori_minRepeat =
            boost::none;  ///< fillin mode a posteriori min source repeat

        bool idleToObservingTime = true;                        ///< idle to observing time
        std::vector<unsigned long> idleToObservingTime_staids;  ///< stations for idle to observing time

        bool andAsConditionCombination = true;            ///< condition combination model
        unsigned int currentIteration = 0;                ///< current iteration number
        unsigned int maxNumberOfIterations = 999;         ///< max number of iterations
        unsigned int numberOfGentleSourceReductions = 0;  ///< number of gentle source reductions
        unsigned int minNumberOfSourcesToReduce = 0;      ///< min number of sources to reduce
        double reduceFactor = .5;                         ///< number of sources which should be reduced during
        ///< gentle source reduction

        bool ignoreSuccessiveScansSameSrc = true;         ///< ignore successive scans to same source
        bool doNotObserveSourcesWithinMinRepeat =
            true;  ///< consider scans (with reduced weight) if they are within min repeat time

        bool writeSkyCoverageData = false;  ///< flag if sky coverage data should be printed to file
    };

    /**
     * @brief pre calculated values (deprecated)
     * @author Matthias Schartner
     */
    struct PreCalculated {
        std::vector<std::vector<int>> subnettingSrcIds;  ///< list of all available second sources in subnetting
    };


    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param init initializer
     * @param path path to VieSchedpp.xml file
     * @param fname file name
     */
    Scheduler( Initializer &init, std::string path, std::string fname );


    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param name session name
     * @param path session path
     * @param network_ station network
     * @param sourceList source list
     * @param scans list of scans
     * @param xml VieSchedpp.xml file
     */
    Scheduler(std::string name, std::string path, Network network_, SourceList sourceList,
              std::vector<Scan> scans,
              boost::property_tree::ptree xml, std::shared_ptr<ObservingMode> obsModes_ = nullptr );


    /**
     * @brief main function that starts the scheduling
     * @author Matthias Schartner
     */
    void start() noexcept;


    /**
     * @brief this function creates a subcon with all scans, times and scores
     * @author Matthias Schartner
     *
     * @param subnetting true if subnetting is allowed, false otherwise
     * @param type scan type
     * @param endposition required endposition
     * @return subcon with all information
     */
    Subcon createSubcon( const std::shared_ptr<Subnetting> &subnetting, Scan::ScanType type,
                         const boost::optional<StationEndposition> &endposition = boost::none ) noexcept;


    /**
     * @brief constructs all visible scans
     * @author Matthias Schartner
     *
     * @param type scan type
     * @param endposition required endposition
     * @param doNotObserveSourcesWithinMinRepeat consider scans (with reduced weight) if they are within min repeat time
     * @return subcon with all visible single source scans
     */
    Subcon allVisibleScans( Scan::ScanType type, const boost::optional<StationEndposition> &endposition = boost::none,
                            bool doNotObserveSourcesWithinMinRepeat = true ) noexcept;


    /**
     * @brief updates the selected next scans to the schedule
     * @author Matthias Schartner
     *
     * @param scan best possible next scans
     * @param of outstream file object
     */
    void update( Scan &scan, std::ofstream &of ) noexcept;


    /**
     * @brief updates and prints the number of all considered scans
     * @author Matthias Schartner
     *
     * @param n1scans number of single source scans
     * @param n2scans number of subnetting scans
     * @param depth recursion depth
     * @param of outstream file object
     */
    void consideredUpdate( unsigned long n1scans, unsigned long n2scans, int depth, std::ofstream &of ) noexcept;


    /**
     * @brief statistics output
     * @author Matthias Schartner
     *
     * @param of output stream
     */
    void statistics( std::ofstream &of );


    /**
     * @brief schedule high impact scans
     * @author Matthias Schartner
     *
     * @param himp high impact scan descriptor
     * @param of outstream object
     */
    void highImpactScans( HighImpactScanDescriptor &himp, std::ofstream &of );

    /**
     * @brief schedule fringeFinder blocks
     * @author Matthias Schartner
     *
     * @param of outstream object
     */
    void calibratorBlocks( std::ofstream &of );

    /**
     * @brief schedule fringeFinder blocks
     * @author Matthias Schartner
     *
     * @param of outstream object
     */
    void parallacticAngleBlocks( std::ofstream &of );

    /**
     * @brief schedule fringeFinder blocks
     * @author Matthias Schartner
     *
     * @param of outstream object
     */
    void differentialParallacticAngleBlocks( std::ofstream &of );


    /**
     * @brief checks the schedule with an independend methode
     * @author Matthias Schartner
     *
     * @param of outstream file object
     */
    bool checkAndStatistics( std::ofstream &of ) noexcept;


    /**
     * @brief get all sources
     * @author Matthias Schartner
     *
     * @return all sources
     */
    const SourceList &getSourceList() const noexcept { return sourceList_; }


    /**
     * @brief get station network
     * @author Matthias Schartner
     *
     * @return station network
     */
    const Network &getNetwork() const noexcept { return network_; }

    /**
     * @brief get path
     * @author Matthias Schartner
     *
     * @return path
     */
    const std::string &getPath() const noexcept { return path_; }

    /**
     * @brief get observing mode network
     * @author Matthias Schartner
     *
     * @return observing mode
     */
    const std::shared_ptr<const ObservingMode> &getObservingMode() const noexcept { return obsModes_; }


    /**
     * @brief get number of observations scheduled in this observation
     * @author Matthias Schartner
     *
     * @return number of observations scheduled in this schedule
     */
    int getNumberOfObservations() const noexcept;


    /**
     * @brief get all scans
     * @author Matthias Schartner
     *
     * @return all scans
     */
    const std::vector<Scan> &getScans() const noexcept { return scans_; }


    /**
     * @brief check if there is a satellite too close to a scan
     * @author Matthias Schartner
     */
    void checkSatelliteAvoidance();

   private:
    static unsigned long nextId;  ///< next id for this object type
    int version_;                 ///< version
    std::string path_;            ///< path to VieSchedpp.xml directory

    boost::property_tree::ptree xml_;  ///< content of VieSchedpp.xml file

    SourceList sourceList_;                                       ///< session source list
    Network network_;                                             ///< station network
    std::shared_ptr<const ObservingMode> obsModes_ = nullptr;     ///< observing modes
    std::shared_ptr<const Mode> currentObservingMode_ = nullptr;  ///< current observing mode
    std::vector<Scan> scans_;                                     ///< all scans in schedule

    Parameters parameters_;        ///< general scheduling parameters
    PreCalculated preCalculated_;  ///< pre calculated values

    unsigned long nSingleScansConsidered = 0;      ///< considered single source scans
    unsigned long nSubnettingScansConsidered = 0;  ///< considered subnetting scans
    unsigned long nObservationsConsidered = 0;     ///< considered baselines

    boost::optional<HighImpactScanDescriptor> himp_;                          ///< high impact scan descriptor
    std::vector<CalibratorBlock> calib_;                                      ///< fringeFinder impact scan descriptor
    boost::optional<MultiScheduling::Parameters> multiSchedulingParameters_;  ///< multi scheduling paramters


    /**
     * @brief start recursive scan selection
     * @author Matthias Schartner
     *
     * @param endTime end time of this recursion
     * @param of outstream object
     * @param type scan type
     * @param opt_endposition required endposition
     * @param subcon precalculated subcon
     * @param depth recursion depth
     */
    void startScanSelection( unsigned int endTime, std::ofstream &of, Scan::ScanType type,
                             boost::optional<StationEndposition> &opt_endposition, boost::optional<Subcon> &subcon,
                             int depth );


    /**
     * @brief checks if some parameters need to be changed
     * @author Matthias Schartner
     *
     * @param time current time in seconds since start
     * @param output flag if output to log file is required
     * @param of outstream file object
     * @param tagalong schedule tagalong scans
     * @return true if a hard break was found
     */
    bool checkForNewEvents( unsigned int time, bool output, std::ofstream &of, bool tagalong ) noexcept;


    /**
     * @brief output of source overview
     * @author Matthias Schartner
     *
     * @param of object
     */
    void listSourceOverview( std::ofstream &of ) noexcept;


    /**
     * @brief start tagalong mode
     * @author Matthias Schartner
     *
     * @param station tagalong station
     * @param skyCoverage sky coverage of tagalong station
     * @param of outstream object
     */
    void startTagelongMode( Station &station, SkyCoverage &skyCoverage, std::ofstream &of,
                            bool ignoreFillinMode = true );

    /**
     * @brief start thinning mode
     * @author Matthias Schartner
     *
     * @param station station to be thinned
     * @param skyCoverage sky coverage of station
     * @param start start time
     * @param end end time
     * @param nscans number of scans
     * @param of outstream object
     */
    void startThinMode( Station &station, SkyCoverage &skyCoverage, unsigned int start, unsigned int end,
                        unsigned long nscans, std::ofstream &of );
    /**
     * @brief check optimization conditions
     * @author Matthias Schartner
     *
     * @param of outstream object
     */
    bool checkOptimizationConditions( std::ofstream &of );


    /**
     * @brief change station availability
     * @author Matthias Schartner
     *
     * @param endposition required endpositions
     * @param change change type
     */
    void changeStationAvailability( const boost::optional<StationEndposition> &endposition,
                                    StationEndposition::change change );


    /**
     * @brief start scan selection between scans
     * @author Matthias Schartner
     *
     * for example fillin mode a posteriori
     *
     * @param duration duration (deprecated?)
     * @param of outstream object
     * @param type scan type
     * @param output output flag
     * @param ignoreTagalong ignore tagalong modes flag
     */
    void startScanSelectionBetweenScans( unsigned int duration, std::ofstream &of, Scan::ScanType type,
                                         bool output = false, bool ignoreTagalong = false );


    /**
     * @brief reset all events to time zero
     * @author Matthias Schartner
     *
     * @param of outstream object
     * @param resetCurrentPointingVector change the current pointing vector
     */
    void resetAllEvents( std::ofstream &of, bool resetCurrentPointingVector = true );


    /**
     * @brief ignore tagalong parameter
     * @author Matthias Schartner
     */
    void ignoreTagalongParameter();


    /**
     * @brief transform idle time to observing time
     * @author Matthias Schartner
     *
     * @param ts time stamp
     * @param of outstrem object
     */
    void idleToScanTime( Timestamp ts, std::ofstream &of );


    /**
     * @brief sort schedule
     * @author Matthias Schartner
     *
     * @param ts time stamp
     */
    void sortSchedule( Timestamp ts = Timestamp::start );


    /**
     * @brief sort schedule based on station
     * @author Matthias Schartner
     *
     * @param staid station id
     * @param ts time stamp
     */
    void sortSchedule( unsigned long staid, Timestamp ts = Timestamp::start );


    /**
     * @brief astrometric fringeFinder update
     * @author Matthias Schartner
     *
     * @param bestScans scheduled scans
     * @param prevHighElevationScores previouse high elevation scores
     * @param prevLowElevationScores previouse low elevation scores
     * @param highestElevations highest elevation scores
     * @param lowestElevations lowest elevation scores
     * @return true if no more astrometric fringeFinder scans are needed, otherwise false
     */
    static bool calibratorUpdate( const std::vector<Scan> &bestScans, std::vector<double> &prevHighElevationScores,
                                  std::vector<double> &prevLowElevationScores, std::vector<double> &highestElevations,
                                  std::vector<double> &lowestElevations );


    /**
     * @brief write astrometric fringeFinder statistics
     * @author Matthias Schartner
     *
     * @param of output stream object
     * @param highestElevations highest elevations scheduled so far in astrometric fringeFinder block
     * @param lowestElevations lowest elevations scheduled so far in astrometric fringeFinder block
     */
    void writeCalibratorStatistics( std::ofstream &of, std::vector<double> &highestElevations,
                                    std::vector<double> &lowestElevations );


    /**
     * @brief write astrometric fringeFinder block header
     * @author Matthias Schartner
     *
     * @param of outstream object
     */
    void writeCalibratorHeader( std::ofstream &of );


    /**
     * @brief updates the time of each observation according to the observing times of the station
     * @author Matthias Schartner
     */
    void updateObservingTimes();

    /**
     * @brief add a priori scan
     * @author Matthias Schartner
     *
     * @param of outfile stream
     * @param ptree property tree including a priori scans
     */
    void scheduleAPrioriScans( const boost::property_tree::ptree &ptree, std::ofstream &of );
};
}  // namespace VieVS
#endif /* SCHEDULER_H */
