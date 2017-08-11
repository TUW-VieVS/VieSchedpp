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

#include "VLBI_initializer.h"
#include "VLBI_subcon.h"
#include "VieVS_constants.h"

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
         * @brief updates the selected scan to be part of the schedule
         *
         * @param scan best possible scan
         * @return true if end of session is reached, otherwise false
         */
        bool update(VLBI_scan &scan);

        /**
         * @brief updates and prints the number of all considered scans
         *
         * @param n1scans number of single source scans
         * @param n2scans number of subnetting scans
         */
        void consideredUpdate(unsigned long n1scans, unsigned long n2scans);

        /**
         * @brief prints the header lines of the output table to the console
         * @param stations
         */
        void outputHeader(vector<VLBI_station> &stations);

        /**
         * @brief calculate fillin scans
         *
         * @param subcon subcon with all scan informations
         * @param scans which will be observed next
         * @return list of all fillin scans
         */
        vector<VLBI_scan> fillin(VLBI_subcon &subcon, vector<VLBI_scan> &bestScans);

    private:
        vector<VLBI_station> stations; ///< all stations
        vector<VLBI_source> sources; ///< all sources
        vector<VLBI_skyCoverage> skyCoverages; ///< all sky coverages
        vector<VLBI_scan> scans; ///< all scans in schedule

        PARAMETERS PARA; ///< general scheduling parameters
        PRECALC PRE; ///< pre calculated values

        unsigned long considered_n1scans; ///< considered single source scans
        unsigned long considered_n2scans; ///< considered subnetting scans

        bool subnetting; ///< use subnetting
        bool fillinmode; ///< use fillin modes

    };
}
#endif /* VLBI_SCHEDULER_H */

