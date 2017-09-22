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
#include "FillinmodeEndposition.h"

namespace VieVS{
    /**
     * @class Scheduler
     * @brief this is the VLBI scheduling class which is responsible for  the scan selection and the creation of the
     * schedule
     *
     * @author Matthias Schartner
     * @date 28.06.2017
     */
    class Scheduler {
        friend class Output;

    public:
        /**
        * @brief general parameters used for scheduling
        */
        struct Parameters {
            bool subnetting = true; ///< flag if subnetting is allowed
            bool fillinmode = true; ///< flag if fillin modes are allowed
            bool writeSkyCoverageData = false; ///< flag if sky coverage data should be printed to file
        };

        /**
         * @brief pre calculated values
         */
        struct PreCalculated{
            std::vector<std::vector<int>> subnettingSrcIds; ///< list of all available second sources in subnetting
        };

        /**
         * @brief empty default constructor
         */
        Scheduler();

        /**
         * @brief constructor
         *
         * @param init initializer
         */
        explicit Scheduler(Initializer &init);

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
        Subcon createSubcon(bool subnetting) noexcept;

        /**
         * @brief constructs all visible scans
         *
         * @return subcon with all visible single source scans
         */
        Subcon allVisibleScans() noexcept;


        /**
         * @brief destructor
         */
        virtual ~Scheduler() noexcept;

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
        void consideredUpdate(unsigned long n1scans, unsigned long n2scans, std::ofstream &bodyLog) noexcept;

        /**
         * @brief updates number of considered fillin scans
         *
         * @param n1scans number of fillin scans
         * @param created flag if these scans were created
         * @param bodyLog outstream file object
         */
        void consideredUpdate(unsigned long n1scans, bool created, std::ofstream &bodyLog) noexcept;

        /**
         * @brief prints the header lines of the output table to the console
         * @param stations
         * @param bodyLog outstream file object
         */
        void outputHeader(const std::vector<Station> &stations, std::ofstream &bodyLog) noexcept;

        /**
         * @brief this function starts the fillin mode
         *
         * Besides calculating possible fillin scans this function also updates all selected next scans.
         *
         * @param bestScans list of all scans which will be scheduled next
         * @param bodyLog outstream file object
         */
        void start_fillinMode(std::vector<Scan> &bestScans, std::ofstream &bodyLog) noexcept;

        /**
         * @brief select a fillin scan if one exists
         *
         * @param subcon all fillin scans which could be possible fillin scans
         * @param fi_endp fillin end position object
         * @param sourceWillBeScanned list of all sources which will be scanned next
         * @param bodyLog outstream file object
         * @return selected fillin scan
         */
        boost::optional<Scan> fillin_scan(Subcon &subcon, const FillinmodeEndposition &fi_endp,
                                               const std::vector<int> &sourceWillBeScanned, std::ofstream &bodyLog) noexcept;

        /**
         * @brief checks if the end of the session is reached
         * @param bestScans best next scans
         * @return true if end is reached, otherwise false
         */
        bool endOfSessionReached(const std::vector<Scan> &bestScans) const noexcept;

        /**
         * @brief total number of created scans
         *
         * @return total number of created scans
         */
        unsigned long numberOfCreatedScans() {
            return nSingleScansConsidered + 2 * nSubnettingScansConsidered + nFillinScansConsidered;
        }

    private:
        boost::property_tree::ptree xml_; ///< content of parameters.xml file

        std::vector<Station> stations_; ///< all stations
        std::vector<Source> sources_; ///< all sources
        std::vector<SkyCoverage> skyCoverages_; ///< all sky coverages
        std::vector<Scan> scans_; ///< all scans in schedule

        Parameters parameters_; ///< general scheduling parameters
        PreCalculated preCalculated_; ///< pre calculated values

        unsigned long nSingleScansConsidered; ///< considered single source scans
        unsigned long nSubnettingScansConsidered; ///< considered subnetting scans
        unsigned long nFillinScansConsidered; ///< considered fillin scans


        /**
         * @brief checks the schedule with an independend methode
         *
         * @param bodyLog outstream file object
         */
        bool check(std::ofstream &bodyLog) noexcept;

        /**
         * @brief checks if some parameters need to be changed
         *
         * @param time current time in seconds since start
         * @param output flag if output to log file is required
         * @param bodyLog outstream file object
         * @return true if a hard break was found
         */
        bool checkForNewEvent(unsigned int time, bool output, std::ofstream &bodyLog) noexcept;

        /**
         * @brief calculates number of available sources
         *
         * @return number of available sources
         */
        unsigned int countAvailableSources() noexcept;

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
    };
}
#endif /* SCHEDULER_H */

