/**
 * @file Output.h
 * @brief class Output
 *
 *
 * @author Matthias Schartner
 * @date 22.08.2017
 */

#ifndef OUTPUT_H
#define OUTPUT_H
#include "Scheduler.h"

namespace VieVS{

    /**
     * @class Output
     * @brief this is the VLBI output class that creates all output files
     *
     * @author Matthias Schartner
     * @date 22.08.2017
     */
    class Output {
    public:
        /**
         * @brief empty default constructor
         */
        Output();

        /**
         * @brief constructor
         *
         * @param sched scheduler
         */
        explicit Output(const Scheduler &sched);

        /**
         * @brief sets the number of this schedule
         *
         * If you do not use multi scheduling the number is 0, otherwise the multi scheduling number is used.
         *
         * @param isched number of this schedule
         */
        void setIsched(int isched) {
            Output::iSched_ = isched;
        }

        /**
         * @brief writes a summary text file containing some basic statistics and overviews
         *
         * @param general write general block
         * @param station write station based block
         * @param source write source based block
         * @param baseline write baseline based block
         * @param duration write duration based block
         */
        void displayStatistics(bool general, bool station, bool source, bool baseline, bool duration);

        /**
         * @brief write a ngs file
         */
        void writeNGS();

    private:
        int iSched_; ///< number of this schedule
        std::vector<Station> stations_; ///< all stations
        std::vector<Source> sources_; ///< all sources
        std::vector<SkyCoverage> skyCoverages_; ///< all sky coverages
        std::vector<Scan> scans_; ///< all scans in schedule

        /**
         * @brief displays some general statistics of the schedule
         *
         * @param out outsteam file object
         */
        void displayGeneralStatistics(std::ofstream &out);

        /**
         * @brief displays some baseline dependent statistics of the schedule
         *
         * @param out outsteam file object
         */
        void displayBaselineStatistics(std::ofstream &out);

        /**
         * @brief displays some station dependent statistics of the schedule
         *
         * @param out outsteam file object
         */
        void displayStationStatistics(std::ofstream &out);

        /**
         * @brief displays some source dependent statistics of the schedule
         *
         * @param out outsteam file object
         */
        void displaySourceStatistics(std::ofstream &out);

        /**
         * @brief displays some source dependent statistics of the schedule
         *
         * @param out outsteam file object
         */
        void displayScanDurationStatistics(std::ofstream &out);
    };
}


#endif //OUTPUT_H
