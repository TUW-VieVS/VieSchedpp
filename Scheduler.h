/**
 * @file Scheduler.h
 * @brief class Scheduler
 *
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <vector>
#include <boost/date_time.hpp>
#include <utility>
#include <tuple>
#include <boost/optional.hpp>

#include "Initializer.h"
#include "Subcon.h"
#include "Constants.h"
#include "StationEndposition.h"
#include "Subnetting.h"

namespace VieVS{
    /**
     * @class Scheduler
     * @brief this is the VLBI scheduling class which is responsible for  the scan selection and the creation of the
     * schedule
     *
     * @author Matthias Schartner
     * @date 28.06.2017
     */
    class Scheduler: public VieVS_NamedObject {
        friend class Output;

    public:

        /**
        * @brief general parameters used for scheduling
        */
        struct Parameters {
            boost::optional<Subnetting> subnetting = boost::none;
            double subnettingMinNSta = 0.60;
            bool fillinmodeDuringScanSelection = true; ///< flag if fillin modes are allowed
            bool fillinmodeInfluenceOnSchedule = true; ///< fillin modes scans influence schedule if set to true
            bool fillinmodeAPosteriori = false;

            bool andAsConditionCombination = true;
            unsigned int currentIteration = 0;
            unsigned int maxNumberOfIterations = 999;
            unsigned int numberOfGentleSourceReductions = 0;
            unsigned int minNumberOfSourcesToReduce = 0;

            bool writeSkyCoverageData = false; ///< flag if sky coverage data should be printed to file
        };

        /**
         * @brief pre calculated values
         */
        struct PreCalculated{
            std::vector<std::vector<int>> subnettingSrcIds; ///< list of all available second sources in subnetting
        };

        /**
         * @brief constructor
         *
         * @param init initializer
         */
        Scheduler(Initializer &init, std::string path, std::string name, int version);

        Scheduler(std::string name, std::vector<Station> stations, std::vector<Source> sources,
                  std::vector<SkyCoverage> skyCoverages, std::vector<Scan> scans, boost::property_tree::ptree xml);

        /**
         * @brief main function that starts the scheduling
         */
        void start(std::ofstream &bodyLog) noexcept;

        /**
         * @brief this function creates a subcon with all scans, times and scores
         *
         * @param subnetting true if subnetting is allowed, false otherwise
         * @return subcon with all information
         */
        Subcon createSubcon(const boost::optional<Subnetting> &subnetting, Scan::ScanType type,
                            const boost::optional<StationEndposition> &endposition = boost::none) noexcept;

        /**
         * @brief constructs all visible scans
         *
         * @return subcon with all visible single source scans
         */
        Subcon allVisibleScans(Scan::ScanType type, const boost::optional<StationEndposition> &endposition= boost::none) noexcept;


        /**
         * @brief updates the selected next scans to the schedule
         *
         * @param scan best possible next scans
         * @param bodyLog outstream file object
         */
        void update(const Scan &scan, std::ofstream &bodyLog) noexcept;

        /**
         * @brief updates and prints the number of all considered scans
         *
         * @param n1scans number of single source scans
         * @param n2scans number of subnetting scans
         * @param bodyLog outstream file object
         */
        void consideredUpdate(unsigned long n1scans, unsigned long n2scans, int depth, std::ofstream &bodyLog) noexcept;

        /**
         * @brief prints the header lines of the output table to the console
         * @param stations
         * @param bodyLog outstream file object
         */
        void outputHeader(const std::vector<Station> &stations, std::ofstream &bodyLog) noexcept;

        /**
         * @brief total number of created scans
         *
         * @return total number of created scans
         */
        unsigned long numberOfCreatedScans() {
            return nSingleScansConsidered + 2 * nSubnettingScansConsidered;
        }

        void statistics(std::ofstream &ofstream);

        void highImpactScans(HighImpactScanDescriptor &himp, std::ofstream &bodyLog);

        /**
         * @brief checks the schedule with an independend methode
         *
         * @param bodyLog outstream file object
         */
        bool checkAndStatistics(std::ofstream &bodyLog) noexcept;

    private:
        static int nextId;
        std::string path_;

        boost::property_tree::ptree xml_; ///< content of parameters.xml file
        int version_;

        std::vector<Station> stations_; ///< all stations
        std::vector<Source> sources_; ///< all sources
        std::vector<SkyCoverage> skyCoverages_; ///< all sky coverages
        std::vector<Scan> scans_; ///< all scans in schedule

        Parameters parameters_; ///< general scheduling parameters
        PreCalculated preCalculated_; ///< pre calculated values

        unsigned long nSingleScansConsidered = 0; ///< considered single source scans
        unsigned long nSubnettingScansConsidered = 0; ///< considered subnetting scans
        unsigned long nObservationsConsidered = 0; ///< considered baselines

        void startScanSelection(unsigned int endTime, std::ofstream &bodyLog, Scan::ScanType type,
                                boost::optional<StationEndposition> &opt_endposition, boost::optional<Subcon> &subcon,
                                int depth);


        /**
         * @brief checks if some parameters need to be changed
         *
         * @param time current time in seconds since start
         * @param output flag if output to log file is required
         * @param bodyLog outstream file object
         * @return true if a hard break was found
         */
        bool checkForNewEvents(unsigned int time, bool output, std::ofstream &bodyLog) noexcept;

        /**
         * @brief calculates number of available sources
         *
         * @return number of available sources
         */
        void listSourceOverview(std::ofstream &log) noexcept;

        /**
         * @brief saves sky coverage data
         *
         * @param time time in seconds since start
         */
        void saveSkyCoverageData(unsigned int time) noexcept;

        /**
         * @brief saves sky coverage azimuth and elevation data
         */
        void saveSkyCoverageMain() noexcept;

        /**
         * @brief internal debugging function to test static member
         *
         * Usually unused
         *
         * @param log outstream log file
         */
        void displaySummaryOfStaticMembersForDebugging(std::ofstream &log);

        /**
         * @brief internal debugging function to test horizon mask
         *
         * Usually unuse
         *
         */
        void printHorizonMasksForDebugging();

        void startCalibrationBlock(std::ofstream &bodyLog);

        void startTagelongMode(Station &station, std::ofstream &bodyLog);

        bool checkOptimizationConditions(std::ofstream &of);

        void changeStationAvailability(const boost::optional<StationEndposition> &endposition,
                                   StationEndposition::change change);

        void startScanSelectionBetweenScans(unsigned int duration, std::ofstream &bodyLog, Scan::ScanType type, bool output=false, bool ignoreTagalong=false);

        void resetAllEvents(std::ofstream &bodyLog);

        void ignoreTagalongParameter();
    };
}
#endif /* SCHEDULER_H */

