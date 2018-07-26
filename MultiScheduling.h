/**
 * @file MultiScheduling.h
 * @brief class MultiScheduling
 *
 * @author Matthias Schartner
 * @date 03.09.2017
 */

#ifndef MULTISCHEDULING_H
#define MULTISCHEDULING_H

#include <boost/date_time.hpp>
#include <vector>
#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/format.hpp>
#include <numeric>
#include <random>

#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#endif

#include "ParameterGroup.h"
#include "WeightFactors.h"
#include "VieVS_Object.h"
#include "Constants.h"

namespace VieVS {
    /**
     * @class MultiScheduling
     * @brief representation of VLBI multi scheduling tool
     *
     * @author Matthias Schartner
     * @date 03.09.2017
     */
    class MultiScheduling: public VieVS_Object {
    public:

        /**
         * @brief possible multi scheduling parameters
         */
        struct Parameters {
            boost::optional<boost::posix_time::ptime> start; ///< start times

            boost::optional<bool> subnetting; ///< subnetting flag
            boost::optional<double> subnetting_minSourceAngle; ///< subnetting flag
            boost::optional<double> subnetting_minParticipatingStations; ///< subnetting flag

            boost::optional<bool> fillinmode_duringScanSelection; ///< fillin mode flag
            boost::optional<bool> fillinmode_influenceOnScanSelection; ///< fillin mode flag
            boost::optional<bool> fillinmode_aPosteriori; ///< fillin mode flag

            boost::optional<double> weightSkyCoverage; ///< weight factors for sky Coverage
            boost::optional<double> weightNumberOfObservations; ///< weight factors for number of observations
            boost::optional<double> weightDuration; ///< weight factors for duration
            boost::optional<double> weightAverageSources; ///< weight factors for average source observations
            boost::optional<double> weightAverageStations; ///< weight factors for average station usage
            boost::optional<double> weightAverageBaselines; ///< weight factors for average baseline usage
            boost::optional<double> weightIdleTime; ///< weight factor for idle time
            boost::optional<double> weightIdleTime_interval; ///< idle time interval
            boost::optional<double> weightLowDeclination; ///< weight factor for low declination
            boost::optional<double> weightLowDeclination_begin; ///< start declination for extra weight
            boost::optional<double> weightLowDeclination_full; ///< start declination with full weight
            boost::optional<double> weightLowElevation; ///< weight factor for low elevation
            boost::optional<double> weightLowElevation_begin; ///< start elevation for extra weight
            boost::optional<double> weightLowElevation_full; ///< start elevation for full weight

            boost::optional<double> skyCoverageInfluenceDistance; ///< sky coverage influence distance
            boost::optional<double> skyCoverageInfluenceTime; ///< sky coverage influence time

            std::map<std::string, double> stationWeight; ///< station weight
            std::map<std::string, unsigned int> stationMaxSlewtime; ///< station maximum slew time in seconds
            std::map<std::string, double> stationMinSlewDistance; ///< station minimum slew distance
            std::map<std::string, double> stationMaxSlewDistance; ///< station maximum slew distance
            std::map<std::string, unsigned int> stationMaxWait; ///< station maximum wait time in seconds
            std::map<std::string, double> stationMinElevation; ///< station minimum elevation
            std::map<std::string, unsigned int> stationMaxNumberOfScans; ///< station maximum number of scans
            std::map<std::string, unsigned int> stationMaxScan; ///< station maximum scan time in seconds
            std::map<std::string, unsigned int> stationMinScan; ///< station minimum scan time in seconds

            std::map<std::string, double> sourceWeight; ///< source weight
            std::map<std::string, unsigned int> sourceMinNumberOfStations; ///< source minimum number of stations
            std::map<std::string, double> sourceMinFlux; ///< source minimum flux required
            std::map<std::string, unsigned int> sourceMaxNumberOfScans; ///< source maximum number of scans
            std::map<std::string, double> sourceMinElevation; ///< source minimum elevation
            std::map<std::string, double> sourceMinSunDistance; ///< source minimum sun distance
            std::map<std::string, unsigned int> sourceMaxScan; ///< source maximum scan time in seconds
            std::map<std::string, unsigned int> sourceMinScan; ///< source minimum scan time in seconds
            std::map<std::string, unsigned int> sourceMinRepeat; ///< source minimum repeat time in seconds

            std::map<std::string, double> baselineWeight; ///< baseline weight
            std::map<std::string, unsigned int> baselineMaxScan; ///< baseline maximum scan time in seconds
            std::map<std::string, unsigned int> baselineMinScan; ///< baseline minimum scan time in seconds

            /**
             * @brief output function to stream object
             *
             * @param of outfile stream object
             */
            void output(std::ofstream &of) const {
                unsigned long nsta = stationWeight.size();

                if (start.is_initialized()) {
                    of << "    start " << *start << "\n";
                }
                if (subnetting.is_initialized()) {
                    of << "    subnetting " << *subnetting << "\n";
                }
                if (subnetting_minSourceAngle.is_initialized()) {
                    of << "    subnetting minimum source distance " << *subnetting_minSourceAngle*rad2deg << " [deg]\n";
                }
                if (subnetting_minParticipatingStations.is_initialized()) {
                    of << "    subnetting minimum participating stations " << *subnetting_minParticipatingStations << " [%]\n";
                }

                if (fillinmode_duringScanSelection.is_initialized()) {
                    of << "    fillinmode during scan selection " << *fillinmode_duringScanSelection << "\n";
                }
                if (fillinmode_aPosteriori.is_initialized()) {
                    of << "    fillinmode a posteriori " << *fillinmode_aPosteriori << "\n";
                }
                if (fillinmode_influenceOnScanSelection.is_initialized()) {
                    of << "    fillinmode influence scan selection " << *fillinmode_influenceOnScanSelection << "\n";
                }

                if (weightSkyCoverage.is_initialized()) {
                    of << "    weight sky coverage " << *weightSkyCoverage << "\n";
                }
                if (weightNumberOfObservations.is_initialized()) {
                    of << "    weight number of observations " << *weightNumberOfObservations << "\n";
                }
                if (weightDuration.is_initialized()) {
                    of << "    weight duration " << *weightDuration << "\n";
                }
                if (weightAverageSources.is_initialized()) {
                    of << "    weight average sources " << *weightAverageSources << "\n";
                }
                if (weightAverageStations.is_initialized()) {
                    of << "    weight average stations " << *weightAverageStations << "\n";
                }
                if (weightAverageBaselines.is_initialized()) {
                    of << "    weight average baselines " << *weightAverageBaselines << "\n";
                }
                if (weightIdleTime.is_initialized()) {
                    of << "    weight idle time " << *weightIdleTime << "\n";
                }
                if (weightIdleTime_interval.is_initialized()) {
                    of << "    weight idle time interval " << *weightIdleTime_interval << " [s]\n";
                }
                if (weightLowDeclination.is_initialized()) {
                    of << "    weight low declination " << *weightLowDeclination << "\n";
                }
                if (weightLowDeclination_begin.is_initialized()) {
                    of << "    weight low declination start " << *weightLowDeclination_begin*rad2deg << " [deg]\n";
                }
                if (weightLowDeclination_full.is_initialized()) {
                    of << "    weight low declination full " << *weightLowDeclination_full*rad2deg << " [deg]\n";
                }
                if (weightLowElevation.is_initialized()) {
                    of << "    weight low elevation " << *weightLowElevation << "\n";
                }
                if (weightLowElevation_begin.is_initialized()) {
                    of << "    weight low elevation start " << *weightLowElevation_begin*rad2deg << " [deg]\n";
                }
                if (weightLowElevation_full.is_initialized()) {
                    of << "    weight low elevation full " << *weightLowElevation_full*rad2deg << " [deg]\n";
                }

                if (skyCoverageInfluenceDistance.is_initialized()) {
                    of << "    sky coverage influence distance " << *skyCoverageInfluenceDistance*rad2deg << " [deg]\n";
                }
                if (skyCoverageInfluenceTime.is_initialized()) {
                    of << "    sky coverage influence time " << *skyCoverageInfluenceTime*rad2deg << " [deg]\n";
                }

                for (const auto &any: stationWeight) {
                    of << "    station weight " << any.first << ": " << any.second << "\n";
                }
                for (const auto &any: stationMaxSlewtime) {
                    of << "    station max slewtime " << any.first << ": " << any.second << " [s]\n";
                }
                for (const auto &any: stationMinSlewDistance) {
                    of << "    station max slew distance " << any.first << ": " << any.second*rad2deg << " [deg]\n";
                }
                for (const auto &any: stationMaxSlewDistance) {
                    of << "    station min slew distance " << any.first << ": " << any.second*rad2deg << " [deg]\n";
                }
                for (const auto &any: stationMaxWait) {
                    of << "    station max wait " << any.first << ": " << any.second << " [s]\n";
                }
                for (const auto &any: stationMinElevation) {
                    of << "    station min elevation " << any.first << ": " << any.second*rad2deg << " [deg]\n";
                }
                for (const auto &any: stationMaxNumberOfScans) {
                    of << "    station max number of scans " << any.first << ": " << any.second << "\n";
                }
                for (const auto &any: stationMaxScan) {
                    of << "    station max scan time " << any.first << ": " << any.second << " [s]\n";
                }
                for (const auto &any: stationMinScan) {
                    of << "    station min scan time " << any.first << ": " << any.second << " [s]\n";
                }

                for (const auto &any: sourceWeight) {
                    of << "    source weight  " << any.first << ": " << any.second << "\n";
                }
                for (const auto &any: sourceMinNumberOfStations) {
                    of << "    source min number of stations " << any.first << ": " << any.second << "\n";
                }
                for (const auto &any: sourceMinFlux) {
                    of << "    source min flux " << any.first << ": " << any.second << " [Jy]\n";
                }
                for (const auto &any: sourceMaxNumberOfScans) {
                    of << "    source max number of scans " << any.first << ": " << any.second << "\n";
                }
                for (const auto &any: sourceMinElevation) {
                    of << "    source min elevation " << any.first << ": " << any.second*rad2deg << " [deg]\n";
                }
                for (const auto &any: sourceMinSunDistance) {
                    of << "    source min sun distance " << any.first << ": " << any.second*rad2deg << " [deg]\n";
                }
                for (const auto &any: sourceMaxScan) {
                    of << "    source max scan time " << any.first << ": " << any.second << " [s]\n";
                }
                for (const auto &any: sourceMinScan) {
                    of << "    source min scan time " << any.first << ": " << any.second << " [s]\n";
                }
                for (const auto &any: sourceMinRepeat) {
                    of << "    source min repeat " << any.first << ": " << any.second << " [s]\n";
                }

                for (const auto &any: baselineWeight) {
                    of << "    baseline weight " << any.first << ": " << any.second << "\n";
                }
                for (const auto &any: baselineMaxScan) {
                    of << "    baseline max scan time " << any.first << ": " << any.second << " [s]\n";
                }
                for (const auto &any: baselineMinScan) {
                    of << "    baseline min scan time " << any.first << ": " << any.second << " [s]\n";
                }
                of << "\n";
            }
        };


        /**
         * @brief empty default constructor
         */
        MultiScheduling();

        /**
         * @brief set session start times
         * @param start session start times
         */
        void setStart(const std::vector<boost::posix_time::ptime> &start) {
            MultiScheduling::start_ = start;
        }

        void addParameters(const std::string &name);

        void addParameters(const std::string &name, const std::vector<double> &values);

        void addParameters(const std::string &name, const std::string &member, const std::vector<double> &values);



        /**
         * @brief create vector of all possible multi scheduling parameter combinations
         * @return all possible multi scheduling parameter combinations
         */
        std::vector<Parameters> createMultiScheduleParameters(unsigned int maxNr, unsigned int seed);

        /**
         * @brief create property tree used for parameter.xml file
         * @return property tree for multiSched block
         */
        boost::property_tree::ptree createPropertyTree() const;

    private:
        static unsigned long nextId;

        std::unordered_map<std::string, std::vector<std::string> > stationGroups_; ///< used station groups
        std::unordered_map<std::string, std::vector<std::string> > sourceGroups_; ///< used source groups
        std::unordered_map<std::string, std::vector<std::string> > baselineGroups_; ///< used baseline groups


        std::vector<boost::posix_time::ptime> start_; ///< start times

        std::vector<std::string> singleArgumentLogical;
        std::vector<std::pair<std::string, std::vector<double>>> singleArgumentNumeric;
        std::vector<std::pair<std::string, std::pair<std::string, std::vector<double>>>> doubleArgumentNumeric;

        void addParameter(std::vector<Parameters> &allPara, unsigned long &n_before, const std::string &name);

        void addParameter(std::vector<Parameters> &allPara, unsigned long &n_before, const std::string &name,
                          const std::vector<double> &value);

        void addParameter(std::vector<Parameters> &allPara, unsigned long &n_before, const std::string &name,
                          const std::string& member, const std::vector<double> &value);

    };
}


#endif //MULTISCHEDULING_H
