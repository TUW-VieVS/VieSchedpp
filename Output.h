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
#include "boost/format.hpp"
#include "Vex.h"
#include "Skd.h"

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
         * @brief possible group types
         */
        enum class GroupType {
            station, ///< stations wise group
            source, ///< source wise group
            baseline, ///< baseline wise group
        };


        /**
         * @brief empty default constructor
         */
        Output();

        /**
         * @brief constructor
         *
         * @param sched scheduler
         */
        explicit Output(Scheduler &sched, std::string path);

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
         * @param time write time block
         * @param statisticsLog output log file
         */
        void writeStatistics(bool general, bool station, bool source, bool baseline, bool duration, bool time, std::ofstream& statisticsLog);

        /**
         * @brief create a ngs file
         */
        void writeNGS();

        /**
         * @brief creates a skd file
         */
        void writeSkd(const SkdCatalogReader &skdCatalogReader);

        /**
         * @brief creates a vex file
         * @param skdCatalogReader skd catalogs
         */
        void writeVex(const SkdCatalogReader &skdCatalogReader);

        void writeStatisticsPerSourceGroup();

    private:
        boost::property_tree::ptree xml_; ///< content of parameters.xml file

        std::string path_;
        int iSched_; ///< number of this schedule
        std::vector<Station> stations_; ///< all stations
        std::vector<Source> sources_; ///< all sources
        std::vector<SkyCoverage> skyCoverages_; ///< all sky coverages
        std::vector<Scan> scans_; ///< all scans in schedule

        /**
         * @brief displays some general statistics of the schedule
         *
         * @param out outsteam file object
         * @return vector of statistical values
         */
        std::vector<int> displayGeneralStatistics(std::ofstream &out);

        /**
         * @brief displays some baseline dependent statistics of the schedule
         *
         * @param out outsteam file object
         * @param number of baselines
         */
        int displayBaselineStatistics(std::ofstream &out);

        /**
         * @brief displays some station dependent statistics of the schedule
         *
         * @param out outsteam file object
         * @return vector of statistical values
         */
        std::vector<int> displayStationStatistics(std::ofstream &out);

        /**
         * @brief displays some source dependent statistics of the schedule
         *
         * @param out outsteam file object
         * @param number of scheduled sources
         */
        int displaySourceStatistics(std::ofstream &out);

        /**
         * @brief displays some source dependent statistics of the schedule
         *
         * @param out outsteam file object
         */
        void displayScanDurationStatistics(std::ofstream &out);


        void displayTimeStatistics(std::ofstream &ofstream);

        std::unordered_map<std::string, std::vector<std::string> > readGroups(boost::property_tree::ptree root,
                                                                              GroupType type) noexcept;

        std::vector<unsigned int> minutesVisible(const Source &source);

    };
}


#endif //OUTPUT_H
