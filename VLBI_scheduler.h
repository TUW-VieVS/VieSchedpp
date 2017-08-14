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
            double minAngleBetweenSubnettingSources =
                    120 * deg2rad; ///< minimum angle between subnetting sources in radians

            boost::posix_time::ptime startTime; ///< start time of the session
            boost::posix_time::ptime endTime; ///< end time of the session
            double mjdStart; ///< modified julian date of the session start
            unsigned int duration; ///< session duration in seconds
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
        VLBI_scheduler(VLBI_initializer &init);

        /**
         * @brief main function that starts the scheduling
         */
        void start();

        /**
         * @brief this function creates a subcon with all scans, times and scores
         *
         * @param subnetting true if subnetting is allowed, false otherwise
         * @return subcon with all information
         */
        VLBI_subcon createSubcon(bool subnetting);

        /**
         * @brief constructs all visible scans
         *
         * @return subcon with all visible single source scans
         */
        VLBI_subcon allVisibleScans();

        /**
         *  @brief pre calculates all possible second scans used for subnetting
         */
        void precalcSubnettingSrcIds();

        /**
         * @brief destructor
         */
        virtual ~VLBI_scheduler();

        /**
         * @brief updates the selected next scans to the schedule
         *
         * @param scan best possible next scans
         */
        void update(VLBI_scan &scan);

        /**
         * @brief updates and prints the number of all considered scans
         *
         * @param n1scans number of single source scans
         * @param n2scans number of subnetting scans
         */
        void consideredUpdate(unsigned long n1scans, unsigned long n2scans);

        /**
         * @brief updates number of considered fillin scans
         *
         * @param n1scans number of fillin scans
         */
        void consideredUpdate(unsigned long n1scans, bool created = false);

        /**
         * @brief prints the header lines of the output table to the console
         * @param stations
         */
        void outputHeader(vector<VLBI_station> &stations);

        /**
         * @brief this function starts the fillin mode
         *
         * Besides calculating possible fillin scans this function also updates all selected next scans.
         *
         * @param subcon current subcon of available scans
         * @param fi_endp current required fillin endpositions
         */
        void start_fillinMode(VLBI_subcon &subcon, vector<VLBI_scan> &bestScans);

        /**
         * @brief calculate fillin scans
         *
         * @param subcon subcon with all scan informations
         * @param scans which will be observed next
         * @return list of all fillin scans
         */
        boost::optional<VLBI_scan> fillin_scan(VLBI_subcon &subcon, VLBI_fillin_endpositions &fi_endp,
                                               vector<int> &sourceWillBeScanned);

        /**
         * @brief checks if the end of the session is reached
         * @param bestScans best next scans
         * @return true if end is reached, otherwise false
         */
        bool endOfSessionReached(vector<VLBI_scan> bestScans);

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

        bool subnetting; ///< use subnetting
        bool fillinmode; ///< use fillin modes

    };
}
#endif /* VLBI_SCHEDULER_H */

