/**
 * @file VLBI_scheduler.h
 * @brief class VLBI_scheduler
 *
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */

#ifndef VLBI_SCHEDULER_H
#define VLBI_SCHEDULER_H
#include <vector>
#include <boost/date_time.hpp>
#include <utility>
#include <tuple>
#include <boost/optional.hpp>

#include "VLBI_initializer.h"
#include "VLBI_subcon.h"
#include "VieVS_constants.h"
#include "VLBI_fillin_endpositions.h"

using namespace std;
namespace VieVS{
    class VLBI_scheduler {
    public:
        /**
        * @brief general parameters used for scheduling
        */
        struct PARAMETERS {
            bool subnetting = 1;
            bool fillinmode = 1;
            bool writeSkyCoverageData = 0;
        };

        /**
         * @brief pre calculated values
         */
        struct PRECALC{
            vector<vector<int>> subnettingSrcIds; ///< list of all available second sources in subnetting
        };

        /**
         * @brief empty default constructor
         */
        VLBI_scheduler();

        /**
         * @brief constructor
         *
         * @param init initializer
         */
        explicit VLBI_scheduler(VLBI_initializer &init);

        /**
         * @brief main function that starts the scheduling
         */
        void start(ofstream &bodyLog) noexcept;

        /**
         * @brief this function creates a subcon with all scans, times and scores
         *
         * @param subnetting true if subnetting is allowed, false otherwise
         * @return subcon with all information
         */
        VLBI_subcon createSubcon(bool subnetting) noexcept;

        /**
         * @brief constructs all visible scans
         *
         * @return subcon with all visible single source scans
         */
        VLBI_subcon allVisibleScans() noexcept;


        /**
         * @brief destructor
         */
        virtual ~VLBI_scheduler() noexcept;

        /**
         * @brief updates the selected next scans to the schedule
         *
         * @param scan best possible next scans
         */
        void update(const VLBI_scan &scan, ofstream &bodyLog) noexcept;

        /**
         * @brief updates and prints the number of all considered scans
         *
         * @param n1scans number of single source scans
         * @param n2scans number of subnetting scans
         */
        void consideredUpdate(unsigned long n1scans, unsigned long n2scans, ofstream &bodyLog) noexcept;

        /**
         * @brief updates number of considered fillin scans
         *
         * @param n1scans number of fillin scans
         */
        void consideredUpdate(unsigned long n1scans, bool created, ofstream &bodyLog) noexcept;

        /**
         * @brief prints the header lines of the output table to the console
         * @param stations
         */
        void outputHeader(const vector<VLBI_station> &stations, ofstream &bodyLog) noexcept;

        /**
         * @brief this function starts the fillin mode
         *
         * !!! This function changes bestScans (is empty at end) !!!
         * !!! This function changes subcon (will be new subcon for last fillin scan) !!!
         *
         * Besides calculating possible fillin scans this function also updates all selected next scans.
         *
         * @param subcon current subcon of available scans
         * @param fi_endp current required fillin endpositions
         */
        void start_fillinMode(vector<VLBI_scan> &bestScans, ofstream &bodyLog) noexcept;

        /**
         * @brief calculate fillin scans
         *
         * @param subcon subcon with all scan informations
         * @param scans which will be observed next
         * @return list of all fillin scans
         */
        boost::optional<VLBI_scan> fillin_scan(VLBI_subcon &subcon, const VLBI_fillin_endpositions &fi_endp,
                                               const vector<int> &sourceWillBeScanned, ofstream &bodyLog) noexcept;

        /**
         * @brief checks if the end of the session is reached
         * @param bestScans best next scans
         * @return true if end is reached, otherwise false
         */
        bool endOfSessionReached(const vector<VLBI_scan> &bestScans) const noexcept;

        /**
         * @brief getter for all stations
         * @return stations
         */
        const vector<VLBI_station> &getStations() const noexcept {
            return stations;
        }

        /**
         * @brief getter for all sources
         * @return sources
         */
        const vector<VLBI_source> &getSources() const noexcept {
            return sources;
        }

        /**
         * @brief getter for all sky coverages
         * @return sky coverages
         */
        const vector<VLBI_skyCoverage> &getSkyCoverages() const noexcept {
            return skyCoverages;
        }

        /**
         * @brief getter for all scans
         * @return scans
         */
        const vector<VLBI_scan> &getScans() const noexcept {
            return scans;
        }

        unsigned long numberOfCreatedScans() {
            return considered_n1scans + 2 * considered_n2scans + considered_fillin;
        }

    private:
        vector<VLBI_station> stations; ///< all stations
        vector<VLBI_source> sources; ///< all sources
        vector<VLBI_skyCoverage> skyCoverages; ///< all sky coverages
        vector<VLBI_scan> scans; ///< all scans in schedule

        PARAMETERS PARA; ///< general scheduling parameters
        PRECALC PRE; ///< pre calculated values

        unsigned long considered_n1scans; ///< considered single source scans
        unsigned long considered_n2scans; ///< considered subnetting scans
        unsigned long considered_fillin; ///< considered fillin scans


        /**
         * @brief checks the schedule with an independend methode
         */
        bool check(ofstream &bodyLog) noexcept;

        bool checkForNewEvent(unsigned int time, bool output, ofstream &bodyLog) noexcept;

        unsigned int countAvailableSources() noexcept;

        void saveSkyCoverageData(unsigned int time) noexcept;

        void saveSkyCoverageMain() noexcept;

        void displaySummaryOfStaticMembersForDebugging(ofstream &log);

        void printHorizonMasksForDebugging();
    };
}
#endif /* VLBI_SCHEDULER_H */

