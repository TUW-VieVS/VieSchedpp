/* 
 *  VieSched++ Very Long Baseline Interferometry (VLBI) Scheduling Software
 *  Copyright (C) 2018  Matthias Schartner
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

#include "../XML/ParameterGroup.h"
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
         * @author Matthias Schartner
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
             * @author Matthias Schartner
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

            /**
             * @brief output function to write header to statistics file
             * @author Matthias Schartner
             *
             * @param of outfile stream object
             */
            void statisticsHeaderOutput(std::ofstream &of) const{
                if (start.is_initialized()) {
                    of << "start,";
                }
                if (subnetting.is_initialized()) {
                    of << "subnetting,";
                }
                if (subnetting_minSourceAngle.is_initialized()) {
                    of << "subnetting_minSourceAngle,";
                }
                if (subnetting_minParticipatingStations.is_initialized()) {
                    of << "subnetting_minParticipatingStations,";
                }

                if (fillinmode_duringScanSelection.is_initialized()) {
                    of << "fillinmode_duringScanSelection,";
                }
                if (fillinmode_aPosteriori.is_initialized()) {
                    of << "fillinmode_aPosteriori,";
                }
                if (fillinmode_influenceOnScanSelection.is_initialized()) {
                    of << "fillinmode_influenceOnScanSelection,";
                }

                if (weightSkyCoverage.is_initialized()) {
                    of << "weightSkyCoverage,";
                }
                if (weightNumberOfObservations.is_initialized()) {
                    of << "weightNumberOfObservations,";
                }
                if (weightDuration.is_initialized()) {
                    of << "weightDuration,";
                }
                if (weightAverageSources.is_initialized()) {
                    of << "weightAverageSources,";
                }
                if (weightAverageStations.is_initialized()) {
                    of << "weightAverageStations,";
                }
                if (weightAverageBaselines.is_initialized()) {
                    of << "weightAverageBaselines,";
                }
                if (weightIdleTime.is_initialized()) {
                    of << "weightIdleTime,";
                }
                if (weightIdleTime_interval.is_initialized()) {
                    of << "weightIdleTime_interval,";
                }
                if (weightLowDeclination.is_initialized()) {
                    of << "weightLowDeclination,";
                }
                if (weightLowDeclination_begin.is_initialized()) {
                    of << "weightLowDeclination_begin,";
                }
                if (weightLowDeclination_full.is_initialized()) {
                    of << "weightLowDeclination_full,";
                }
                if (weightLowElevation.is_initialized()) {
                    of << "weightLowElevation,";
                }
                if (weightLowElevation_begin.is_initialized()) {
                    of << "weightLowElevation_begin,";
                }
                if (weightLowElevation_full.is_initialized()) {
                    of << "weightLowElevation_full,";
                }

                if (skyCoverageInfluenceDistance.is_initialized()) {
                    of << "skyCoverageInfluenceDistance,";
                }
                if (skyCoverageInfluenceTime.is_initialized()) {
                    of << "skyCoverageInfluenceTime,";
                }

                for (const auto &any: stationWeight) {
                    of << "stationWeight_" << any.first << ",";
                }
                for (const auto &any: stationMaxSlewtime) {
                    of << "stationMaxSlewtime_" << any.first << ",";
                }
                for (const auto &any: stationMinSlewDistance) {
                    of << "stationMinSlewDistance_" << any.first << ",";
                }
                for (const auto &any: stationMaxSlewDistance) {
                    of << "stationMaxSlewDistance_" << any.first << ",";
                }
                for (const auto &any: stationMaxWait) {
                    of << "stationMaxWait_" << any.first << ",";
                }
                for (const auto &any: stationMinElevation) {
                    of << "stationMinElevation_" << any.first << ",";
                }
                for (const auto &any: stationMaxNumberOfScans) {
                    of << "stationMaxNumberOfScans_" << any.first << ",";
                }
                for (const auto &any: stationMaxScan) {
                    of << "stationMaxScan_" << any.first << ",";
                }
                for (const auto &any: stationMinScan) {
                    of << "stationMinScan_" << any.first << ",";
                }

                for (const auto &any: sourceWeight) {
                    of << "sourceWeight_" << any.first << ",";
                }
                for (const auto &any: sourceMinNumberOfStations) {
                    of << "sourceMinNumberOfStations_" << any.first << ",";
                }
                for (const auto &any: sourceMinFlux) {
                    of << "sourceMinFlux_" << any.first << ",";
                }
                for (const auto &any: sourceMaxNumberOfScans) {
                    of << "sourceMaxNumberOfScans_" << any.first << ",";
                }
                for (const auto &any: sourceMinElevation) {
                    of << "sourceMinElevation_" << any.first << ",";
                }
                for (const auto &any: sourceMinSunDistance) {
                    of << "sourceMinSunDistance_" << any.first << ",";
                }
                for (const auto &any: sourceMaxScan) {
                    of << "sourceMaxScan_" << any.first << ",";
                }
                for (const auto &any: sourceMinScan) {
                    of << "sourceMinScan_" << any.first << ",";
                }
                for (const auto &any: sourceMinRepeat) {
                    of << "sourceMinRepeat_" << any.first << ",";
                }

                for (const auto &any: baselineWeight) {
                    of << "baselineWeight_" << any.first << ",";
                }
                for (const auto &any: baselineMaxScan) {
                    of << "baselineMaxScan_" << any.first << ",";
                }
                for (const auto &any: baselineMinScan) {
                    of << "baselineMinScan_" << any.first << ",";
                }
            }

            /**
             * @brief output function to data to statistics file
             * @author Matthias Schartner
             *
             * @param of outfile stream object
             */
            void statisticsOutput(std::ofstream &of) const{
                std::string str;
                if (start.is_initialized()) {
                    of << *start << ",";
                }
                if (subnetting.is_initialized()) {
                    of << *subnetting << ",";
                }
                if (subnetting_minSourceAngle.is_initialized()) {
                    of << *subnetting_minSourceAngle*rad2deg << ",";
                }
                if (subnetting_minParticipatingStations.is_initialized()) {
                    of << *subnetting_minParticipatingStations << " [%]\n";
                }

                if (fillinmode_duringScanSelection.is_initialized()) {
                    of << *fillinmode_duringScanSelection << ",";
                }
                if (fillinmode_aPosteriori.is_initialized()) {
                    of << *fillinmode_aPosteriori << ",";
                }
                if (fillinmode_influenceOnScanSelection.is_initialized()) {
                    of << *fillinmode_influenceOnScanSelection << ",";
                }

                if (weightSkyCoverage.is_initialized()) {
                    of << *weightSkyCoverage << ",";
                }
                if (weightNumberOfObservations.is_initialized()) {
                    of << *weightNumberOfObservations << ",";
                }
                if (weightDuration.is_initialized()) {
                    of << *weightDuration << ",";
                }
                if (weightAverageSources.is_initialized()) {
                    of << *weightAverageSources << ",";
                }
                if (weightAverageStations.is_initialized()) {
                    of << *weightAverageStations << ",";
                }
                if (weightAverageBaselines.is_initialized()) {
                    of << *weightAverageBaselines << ",";
                }
                if (weightIdleTime.is_initialized()) {
                    of << *weightIdleTime << ",";
                }
                if (weightIdleTime_interval.is_initialized()) {
                    of << *weightIdleTime_interval << ",";
                }
                if (weightLowDeclination.is_initialized()) {
                    of << *weightLowDeclination << ",";
                }
                if (weightLowDeclination_begin.is_initialized()) {
                    of << *weightLowDeclination_begin*rad2deg << ",";
                }
                if (weightLowDeclination_full.is_initialized()) {
                    of << *weightLowDeclination_full*rad2deg << ",";
                }
                if (weightLowElevation.is_initialized()) {
                    of << *weightLowElevation << ",";
                }
                if (weightLowElevation_begin.is_initialized()) {
                    of << *weightLowElevation_begin*rad2deg << ",";
                }
                if (weightLowElevation_full.is_initialized()) {
                    of << *weightLowElevation_full*rad2deg << ",";
                }

                if (skyCoverageInfluenceDistance.is_initialized()) {
                    of << *skyCoverageInfluenceDistance*rad2deg << ",";
                }
                if (skyCoverageInfluenceTime.is_initialized()) {
                    of << *skyCoverageInfluenceTime*rad2deg << ",";
                }

                for (const auto &any: stationWeight) {
                    of << any.second << ",";
                }
                for (const auto &any: stationMaxSlewtime) {
                    of << any.second << ",";
                }
                for (const auto &any: stationMinSlewDistance) {
                    of << any.second*rad2deg << ",";
                }
                for (const auto &any: stationMaxSlewDistance) {
                    of << any.second*rad2deg << ",";
                }
                for (const auto &any: stationMaxWait) {
                    of << any.second << ",";
                }
                for (const auto &any: stationMinElevation) {
                    of << any.second*rad2deg << ",";
                }
                for (const auto &any: stationMaxNumberOfScans) {
                    of << any.second << ",";
                }
                for (const auto &any: stationMaxScan) {
                    of << any.second << ",";
                }
                for (const auto &any: stationMinScan) {
                    of << any.second << ",";
                }

                for (const auto &any: sourceWeight) {
                    of << any.second << ",";
                }
                for (const auto &any: sourceMinNumberOfStations) {
                    of << any.first << ": " << any.second << ",";
                }
                for (const auto &any: sourceMinFlux) {
                    of << any.second << ",";
                }
                for (const auto &any: sourceMaxNumberOfScans) {
                    of << any.second << ",";
                }
                for (const auto &any: sourceMinElevation) {
                    of << any.second*rad2deg << ",";
                }
                for (const auto &any: sourceMinSunDistance) {
                    of << any.second*rad2deg << ",";
                }
                for (const auto &any: sourceMaxScan) {
                    of << any.second << ",";
                }
                for (const auto &any: sourceMinScan) {
                    of << any.second << ",";
                }
                for (const auto &any: sourceMinRepeat) {
                    of << any.second << ",";
                }

                for (const auto &any: baselineWeight) {
                    of << any.second << ",";
                }
                for (const auto &any: baselineMaxScan) {
                    of << any.second << ",";
                }
                for (const auto &any: baselineMinScan) {
                    of << any.second << ",";
                }
            }

        };


        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param sta_group station groups
         * @param src_group source groups
         * @param bls_group baseline groups
         */
        MultiScheduling(std::unordered_map<std::string, std::vector<std::string> > sta_group,
                        std::unordered_map<std::string, std::vector<std::string> > src_group,
                        std::unordered_map<std::string, std::vector<std::string> > bls_group);

        /**
         * @brief set session start times
         * @author Matthias Schartner
         *
         * @param start session start times
         */
        void setStart(const std::vector<boost::posix_time::ptime> &start) {
            MultiScheduling::start_ = start;
        }

        /**
         * @brief add new boolean parameter
         * @author Matthias Schartner
         *
         * @param name parameter name
         */
        void addParameters(const std::string &name);

        /**
         * @brief add new value based parameter
         * @author Matthias Schartner
         *
         * @param name parameter name
         * @param values parameter values
         */
        void addParameters(const std::string &name, const std::vector<double> &values);

        /**
         * @brief add new value and member based parameter
         * @author Matthias Schartner
         *
         * @param name parameter name
         * @param member parameter member
         * @param values parameter values
         */
        void addParameters(const std::string &name, const std::string &member, const std::vector<double> &values);


        /**
         * @brief create vector of all possible multi scheduling parameter combinations
         * @author Matthias Schartner
         *
         * @param maxNr maximum number of parameters
         * @param seed seed for random number generator
         * @return all possible multi scheduling parameter combinations
         */
        std::vector<Parameters> createMultiScheduleParameters(unsigned int maxNr, unsigned int seed);

        /**
         * @brief create property tree used for parameter.xml file
         * @author Matthias Schartner
         *
         * @return property tree for multiSched block
         */
        boost::property_tree::ptree createPropertyTree() const;

    private:
        static unsigned long nextId;

        std::unordered_map<std::string, std::vector<std::string> > stationGroups_; ///< used station groups
        std::unordered_map<std::string, std::vector<std::string> > sourceGroups_; ///< used source groups
        std::unordered_map<std::string, std::vector<std::string> > baselineGroups_; ///< used baseline groups


        std::vector<boost::posix_time::ptime> start_; ///< start times

        std::vector<std::string> singleArgumentLogical; ///< boolean parameters
        std::vector<std::pair<std::string, std::vector<double>>> singleArgumentNumeric; ///< value based parameters
        std::vector<std::pair<std::string, std::pair<std::string, std::vector<double>>>> doubleArgumentNumeric; ///< value and member based parameters

        /**
         * @brief add boolean parameters
         * @author Matthias Schartner
         *
         * @param allPara list of all parameters
         * @param n_before number of blocks
         * @param name parameter name
         */
        void addParameter(std::vector<Parameters> &allPara, unsigned long &n_before, const std::string &name);

        /**
         * @brief add value based parameters
         * @author Matthias Schartner
         *
         * @param allPara list of all parameters
         * @param n_before number of blocks
         * @param name parameter name
         * @param value number of values
         */
        void addParameter(std::vector<Parameters> &allPara, unsigned long &n_before, const std::string &name,
                          const std::vector<double> &value);

        /**
         * @brief add value and member based parameters
         * @author Matthias Schartner
         *
         * @param allPara list of all parameters
         * @param n_before number of blocks
         * @param name parameter name
         * @param member member name
         * @param value number of values
         */
        void addParameter(std::vector<Parameters> &allPara, unsigned long &n_before, const std::string &name,
                          const std::string& member, const std::vector<double> &value);

    };
}


#endif //MULTISCHEDULING_H
