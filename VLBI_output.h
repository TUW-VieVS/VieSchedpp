//
// Created by mschartn on 22.08.17.
//

#ifndef VLBI_OUTPUT_H
#define VLBI_OUTPUT_H
#include "VLBI_scheduler.h"

namespace VieVS{
    class VLBI_output {
    public:
        /**
         * @brief empty default constructor
         */
        VLBI_output();

        /**
         * @brief constructor
         *
         * @param sched scheduler
         */
        VLBI_output(const VLBI_scheduler &sched);

       /**
        * @brief displays some basic statistics of the schedule
        */
        void displayStatistics();


    private:

        vector<VLBI_station> stations; ///< all stations
        vector<VLBI_source> sources; ///< all sources
        vector<VLBI_skyCoverage> skyCoverages; ///< all sky coverages
        vector<VLBI_scan> scans; ///< all scans in schedule

        /**
         * @brief displays some general statistics of the schedule
         */
        void displayGeneralStatistics();

        /**
         * @brief displays some baseline dependent statistics of the schedule
         */
        void displayBaselineStatistics();

        /**
         * @brief displays some station dependent statistics of the schedule
         */
        void displayStationStatistics();

        /**
         * @brief displays some source dependent statistics of the schedule
         */
        void displaySourceStatistics();

        /**
         * @brief displays some source dependent statistics of the schedule
         */
        void displayScanDurationStatistics();


    };
}


#endif //VLBI_OUTPUT_H
