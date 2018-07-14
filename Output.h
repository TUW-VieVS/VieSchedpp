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
    class Output: public VieVS_NamedObject {
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
         * @brief constructor
         *
         * @param sched scheduler
         */
        Output(Scheduler &sched, std::string path, std::string fname, int version);

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
        void writeSkdsum();

        void writeStatistics(std::ofstream &of);

        /**
         * @brief create a ngs file
         */
        void writeNGS();

        void writeOperationsNotes();

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

        void createAllOutputFiles(std::ofstream& of, const SkdCatalogReader &skdCatalogReader);

    private:
        static unsigned long nextId;

        boost::property_tree::ptree xml_; ///< content of parameters.xml file

        std::string path_;
        int version_; ///< number of this schedule
        Network network_;
        std::vector<Source> sources_; ///< all sources
        std::vector<Scan> scans_; ///< all scans in schedule
        boost::optional<MultiScheduling::Parameters> multiSchedulingParameters_;

        /**
         * @brief displays some general statistics of the schedule
         *
         * @param of outsteam file object
         * @return vector of statistical values
         */
        void displayGeneralStatistics(std::ofstream &of);

        /**
         * @brief displays some baseline dependent statistics of the schedule
         *
         * @param of outsteam file object
         * @param number of baselines
         */
        void displayBaselineStatistics(std::ofstream &of);

        /**
         * @brief displays some station dependent statistics of the schedule
         *
         * @param of outsteam file object
         * @return vector of statistical values
         */
        void displayStationStatistics(std::ofstream &of);

        /**
         * @brief displays some source dependent statistics of the schedule
         *
         * @param of outsteam file object
         * @param number of scheduled sources
         */
        void displaySourceStatistics(std::ofstream &of);

        void displayNstaStatistics(std::ofstream &of);

        void displayAstronomicalParameters(std::ofstream &of);

        /**
         * @brief displays some source dependent statistics of the schedule
         *
         * @param of outsteam file object
         */
        void displayScanDurationStatistics(std::ofstream &of);


        void displayTimeStatistics(std::ofstream &of);

        std::unordered_map<std::string, std::vector<std::string> > readGroups(boost::property_tree::ptree root,
                                                                              GroupType type) noexcept;

        std::vector<unsigned int> minutesVisible(const Source &source);

    };
}


#endif //OUTPUT_H
