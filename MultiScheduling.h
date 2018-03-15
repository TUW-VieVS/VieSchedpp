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
#include "ParameterGroup.h"
#include "WeightFactors.h"
#include "VieVS_Object.h"

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
            boost::optional<bool> fillinmode; ///< fillin mode flag

            boost::optional<double> weightSkyCoverage; ///< weight factors for sky Coverage
            boost::optional<double> weightNumberOfObservations; ///< weight factors for number of observations
            boost::optional<double> weightDuration; ///< weight factors for duration
            boost::optional<double> weightAverageSources; ///< weight factors for average source observations
            boost::optional<double> weightAverageStations; ///< weight factors for average station usage

            std::map<std::string, unsigned int> stationMaxSlewtime; ///< station maximum slew time in seconds
            std::map<std::string, unsigned int> stationMaxWait; ///< station maximum wait time in seconds
            std::map<std::string, unsigned int> stationMaxScan; ///< station maximum scan time in seconds
            std::map<std::string, unsigned int> stationMinScan; ///< station minimum scan time in seconds
            std::map<std::string, double> stationWeight; ///< station weight

            std::map<std::string, unsigned int> sourceMinNumberOfStations; ///< source minimum number of stations
            std::map<std::string, double> sourceMinFlux; ///< source minimum flux required
            std::map<std::string, unsigned int> sourceMinRepeat; ///< source minimum repeat time in seconds
            std::map<std::string, unsigned int> sourceMaxScan; ///< source maximum scan time in seconds
            std::map<std::string, unsigned int> sourceMinScan; ///< source minimum scan time in seconds
            std::map<std::string, double> sourceWeight; ///< source weight

            std::map<std::string, unsigned int> baselineMaxScan; ///< baseline maximum scan time in seconds
            std::map<std::string, unsigned int> baselineMinScan; ///< baseline minimum scan time in seconds
            std::map<std::string, double> baselineWeight; ///< baseline weight

            /**
             * @brief output function to stream object
             *
             * @param of outfile stream object
             */
            void output(std::ofstream &of) const {
                unsigned long nsta = stationWeight.size();

                if (start.is_initialized()) {
                    of << "start " << *start << std::endl;
                }
                if (subnetting.is_initialized()) {
                    of << "multisched_subnetting " << *subnetting << std::endl;
                }
                if (fillinmode.is_initialized()) {
                    of << "multisched_fillinmode " << *fillinmode << std::endl;
                }

                if (weightSkyCoverage.is_initialized()) {
                    of << "weight_skyCoverage          " << *weightSkyCoverage << std::endl;
                }
                if (weightNumberOfObservations.is_initialized()) {
                    of << "weight_numberOfObservations " << *weightNumberOfObservations << std::endl;
                }
                if (weightDuration.is_initialized()) {
                    of << "weight_duration             " << *weightDuration << std::endl;
                }
                if (weightAverageSources.is_initialized()) {
                    of << "weight_averageSources       " << *weightAverageSources << std::endl;
                }
                if (weightAverageStations.is_initialized()) {
                    of << "weight_averageStations      " << *weightAverageStations << std::endl;
                }

                int c;
                for (const auto &any: stationMaxSlewtime) {
                    of << "station_maxSlewtime " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: stationMaxWait) {
                    of << "stationMaxWait_     " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: stationMaxScan) {
                    of << "station_maxScan     " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: stationMinScan) {
                    of << "station_minScan     " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: stationWeight) {
                    of << "station_weight      " << any.first << ": " << any.second << std::endl;
                }

                for (const auto &any: sourceMinNumberOfStations) {
                    of << "source_minNumberOfStations " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: sourceMinFlux) {
                    of << "source_minFlux             " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: sourceMinRepeat) {
                    of << "source_minRepeat           " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: sourceMaxScan) {
                    of << "source_maxScan             " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: sourceMinScan) {
                    of << "source_minScan             " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: sourceWeight) {
                    of << "source_weight              " << any.first << ": " << any.second << std::endl;
                }

                for (const auto &any: baselineMaxScan) {
                    of << "baseline_maxScan " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: baselineMinScan) {
                    of << "baseline_minScan " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: baselineWeight) {
                    of << "baseline_weight  " << any.first << ": " << any.second << std::endl;
                }

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

        /**
         * @brief set subnetting flag
         * @param flag subnetting flag
         */
        void setMultiSched_subnetting(bool flag) {
            MultiScheduling::subnetting_ = flag;
        }

        /**
         * @brief set fillin mode flag
         * @param flag fillin mode flag
         */
        void setMultiSched_fillinmode(bool flag) {
            MultiScheduling::fillinmode_ = flag;
        }

        /**
         * @brief weight factors for sky coverage
         *
         * @param weight_skyCoverage weights
         */
        void setWeight_skyCoverage(const std::vector<double> &weight_skyCoverage) {
            MultiScheduling::weightSkyCoverage_ = weight_skyCoverage;
        }

        /**
         * @brief weight factors for number of observations
         *
         * @param weight_skyCoverage weights
         */
        void setWeight_numberOfObservations(const std::vector<double> &weight_numberOfObservations) {
            MultiScheduling::weightNumberOfObservations_ = weight_numberOfObservations;
        }

        /**
         * @brief weight factors for duration
         *
         * @param weight_skyCoverage weights
         */
        void setWeight_duration(const std::vector<double> &weight_duration) {
            MultiScheduling::weightDuration_ = weight_duration;
        }

        /**
         * @brief weight factors for average source observations
         *
         * @param weight_skyCoverage weights
         */
        void setWeight_averageSources(const std::vector<double> &weight_averageSources) {
            MultiScheduling::weightAverageSources_ = weight_averageSources;
        }

        /**
        * @brief weight factors for average station observations
        *
        * @param weight_skyCoverage weights
        */
        void setWeight_averageStations(const std::vector<double> &weight_averageStations) {
            MultiScheduling::weightAverageStations_ = weight_averageStations;
        }

        /**
         * @brief set station maximum slew time
         *
         * @param new_id station name
         * @param values maximum allowed slew times in seconds
         */
        void setStation_maxSlewtime(const std::string &new_id, const std::vector<unsigned int> &values);

        /**
         * @brief set station maximum slew time
         *
         * @param new_id station group
         * @param values maximum allowed slew times in seconds
         */
        void setStation_maxSlewtime(const ParameterGroup &group, const std::vector<unsigned int> &values) {
            setStation_maxSlewtime(group.name, values);
            stationGroups_[group.name] = group.members;
        }

        /**
         * @brief set station maximum wait time
         *
         * @param new_id station name
         * @param values maximum allowed wait times in seconds
         */
        void setStation_maxWait(const std::string &new_id, const std::vector<unsigned int> &values);

        /**
         * @brief set station maximum wait time
         *
         * @param new_id station group
         * @param values maximum allowed wait times in seconds
         */
        void setStation_maxWait(const ParameterGroup &group, const std::vector<unsigned int> &values) {
            setStation_maxWait(group.name, values);
            stationGroups_[group.name] = group.members;
        }

        /**
         * @brief set station maximum scan time
         *
         * @param new_id station name
         * @param values maximum allowed scan times in seconds
         */
        void setStation_maxScan(const std::string &new_id, const std::vector<unsigned int> &values);

        /**
         * @brief set station maximum scan time
         *
         * @param new_id station group
         * @param values maximum allowed scan times in seconds
         */
        void setStation_maxScan(const ParameterGroup &group, const std::vector<unsigned int> &values) {
            setStation_maxScan(group.name, values);
            stationGroups_[group.name] = group.members;
        }

        /**
         * @brief set station minimum scan time
         *
         * @param new_id station name
         * @param values minimum allowed scan times in seconds
         */
        void setStation_minScan(const std::string &new_id, const std::vector<unsigned int> &values);

        /**
         * @brief set station minimum scan time
         *
         * @param new_id station group
         * @param values minimum allowed scan times in seconds
         */
        void setStation_minScan(const ParameterGroup &group, const std::vector<unsigned int> &values) {
            setStation_minScan(group.name, values);
            stationGroups_[group.name] = group.members;
        }

        /**
         * @brief set station weight
         *
         * @param new_id station name
         * @param values weights
         */
        void setStation_weight(const std::string &new_id, const std::vector<double> &values);

        /**
         * @brief set station weight
         *
         * @param new_id station group
         * @param values weights
         */
        void setStation_weight(const ParameterGroup &group, const std::vector<double> &values) {
            setStation_weight(group.name, values);
            stationGroups_[group.name] = group.members;
        }

        /**
         * @brief set source minimum number of stations
         *
         * @param new_id source name
         * @param values minimum allowed network size
         */
        void setSource_minNumberOfStations(const std::string &new_id, const std::vector<unsigned int> &values);

        /**
         * @brief set source minimum number of stations
         *
         * @param new_id source group
         * @param values minimum allowed network size
         */
        void setSource_minNumberOfStations(const ParameterGroup &group, const std::vector<unsigned int> &values) {
            setSource_minNumberOfStations(group.name, values);
            sourceGroups_[group.name] = group.members;
        }

        /**
         * @brief set source minimum required flux
         *
         * @param new_id source name
         * @param values minimum required flux
         */
        void setSource_minFlux(const std::string &new_id, const std::vector<double> &values);

        /**
         * @brief set source minimum required flux
         *
         * @param new_id source group
         * @param values minimum required flux
         */
        void setSource_minFlux(const ParameterGroup &group, const std::vector<double> &values) {
            setSource_minFlux(group.name, values);
            sourceGroups_[group.name] = group.members;
        }

        /**
         * @brief set source minimum time between two scans in seconds
         *
         * @param new_id source name
         * @param values minimum times between two scans in seconds
         */
        void setSource_minRepeat(const std::string &new_id, const std::vector<unsigned int> &values);

        /**
        * @brief set source minimum time between two scans in seconds
        *
        * @param new_id source group
        * @param values minimum times between two scans in seconds
        */
        void setSource_minRepeat(const ParameterGroup &group, const std::vector<unsigned int> &values) {
            setSource_minRepeat(group.name, values);
            sourceGroups_[group.name] = group.members;
        }

        /**
        * @brief set source maximum allowed scan time in seconds
        *
        * @param new_id source name
        * @param values maximum allowed scan times in seconds in seconds
        */
        void setSource_maxScan(const std::string &new_id, const std::vector<unsigned int> &values);

        /**
          * @brief set source maximum allowed scan time in seconds
          *
          * @param new_id source group
          * @param values maximum allowed scan time in seconds
          */
        void setSource_maxScan(const ParameterGroup &group, const std::vector<unsigned int> &values) {
            setSource_maxScan(group.name, values);
            sourceGroups_[group.name] = group.members;
        }

        /**
         * @brief set source minimum allowed scan time in seconds
         *
         * @param new_id source name
         * @param values minimum allowed scan time in seconds
         */
        void setSource_minScan(const std::string &new_id, const std::vector<unsigned int> &values);

        /**
        * @brief set source minimum allowed scan time in seconds
        *
        * @param new_id source group
        * @param values minimum allowed scan time in seconds
        */
        void setSource_minScan(const ParameterGroup &group, const std::vector<unsigned int> &values) {
            setSource_minScan(group.name, values);
            sourceGroups_[group.name] = group.members;
        }

        /**
        * @brief set source weight
        *
        * @param new_id source name
        * @param values weights
        */
        void setSource_weight(const std::string &new_id, const std::vector<double> &values);

        /**
        * @brief set source weight
        *
        * @param new_id source group
        * @param values weights
        */
        void setSource_weight(const ParameterGroup &group, const std::vector<double> &values) {
            setSource_weight(group.name, values);
            sourceGroups_[group.name] = group.members;
        }

        /**
        * @brief set baseline maximum allowed scan time in seconds
        *
        * @param new_id baseline name
        * @param values maximum allowed scan time in seconds
        */
        void setBaseline_maxScan(const std::string &new_id, const std::vector<unsigned int> &values);

        /**
        * @brief set baseline maximum allowed scan time in seconds
        *
        * @param new_id baseline group
        * @param values maximum allowed scan time in seconds
        */
        void setBaseline_maxScan(const ParameterGroup &group, const std::vector<unsigned int> &values) {
            setBaseline_maxScan(group.name, values);
            baselineGroups_[group.name] = group.members;
        }

        /**
        * @brief set baseline minimum allowed scan time in seconds
        *
        * @param new_id baseline name
        * @param values minimum allowed scan time in seconds
        */
        void setBaseline_minScan(const std::string &new_id, const std::vector<unsigned int> &values);

        /**
        * @brief set baseline minimum allowed scan time in seconds
        *
        * @param new_id baseline group
        * @param values minimum allowed scan time in seconds
        */
        void setBaseline_minScan(const ParameterGroup &group, const std::vector<unsigned int> &values) {
            setBaseline_minScan(group.name, values);
            baselineGroups_[group.name] = group.members;
        }

        /**
        * @brief set baseline weight
        *
        * @param new_id baseline name
        * @param values weight
        */
        void setBaseline_weight(const std::string &new_id, const std::vector<double> &values);

        /**
        * @brief set baseline weight
        *
        * @param new_id baseline group
        * @param values weight
        */
        void setBaseline_weight(const ParameterGroup &group, const std::vector<double> &values) {
            setBaseline_weight(group.name, values);
            baselineGroups_[group.name] = group.members;
        }

        /**
         * @brief create vector of all possible multi scheduling parameter combinations
         * @return all possible multi scheduling parameter combinations
         */
        std::vector<Parameters> createMultiScheduleParameters();

        /**
         * @brief create property tree used for parameter.xml file
         * @return property tree for multiSched block
         */
        boost::property_tree::ptree createPropertyTree() const;

    private:
        static int nextId;

        std::unordered_map<std::string, std::vector<std::string> > stationGroups_; ///< used station groups
        std::unordered_map<std::string, std::vector<std::string> > sourceGroups_; ///< used source groups
        std::unordered_map<std::string, std::vector<std::string> > baselineGroups_; ///< used baseline groups


        std::vector<boost::posix_time::ptime> start_; ///< start times
        bool subnetting_ = false; ///< subnetting flag
        bool fillinmode_ = false; ///< fillin mode flag


        std::vector<double> weightSkyCoverage_; ///< sky coverage weight factors
        std::vector<double> weightNumberOfObservations_; ///< number of observations weight factors
        std::vector<double> weightDuration_; ///< duration weight factors
        std::vector<double> weightAverageSources_; ///< average source observations weight factors
        std::vector<double> weightAverageStations_; ///< average station usage weight factors


        std::vector<std::pair<std::string, std::vector<unsigned int> > > stationMaxSlewtime_; ///< station maximum allowed slew times in seconds
        std::vector<std::pair<std::string, std::vector<unsigned int> > > stationMaxWait_; ///< station maximum allowed weight times in seconds
        std::vector<std::pair<std::string, std::vector<unsigned int> > > stationMaxScan_; ///< station maximum allowed scan times in seconds
        std::vector<std::pair<std::string, std::vector<unsigned int> > > stationMinScan_; ///< station minimum allowed scan times in seconds
        std::vector<std::pair<std::string, std::vector<double> > > stationWeight_; ///< station weight factors


        std::vector<std::pair<std::string, std::vector<unsigned int> > > sourceMinNumberOfStations_; ///< source minimum reqired station network size
        std::vector<std::pair<std::string, std::vector<double> > > sourceMinFlux_; ///< source minimum required flux
        std::vector<std::pair<std::string, std::vector<unsigned int> > > sourceMinRepeatability_; ///< source minimum time between two scans in seconds
        std::vector<std::pair<std::string, std::vector<unsigned int> > > sourceMaxScan_; ///< source maximum allowed scan times in seconds
        std::vector<std::pair<std::string, std::vector<unsigned int> > > sourceMinScan_; ///< source minimum allowed scan times in seconds
        std::vector<std::pair<std::string, std::vector<double> > > sourceWeight_; ///< source weight factors


        std::vector<std::pair<std::string, std::vector<unsigned int> > > baselineMaxScan_; ///< baseline maximum allowed scan times in seconds
        std::vector<std::pair<std::string, std::vector<unsigned int> > > baselineMinScan_; ///< baseline minimum allowed scan times in seconds
        std::vector<std::pair<std::string, std::vector<double> > > baselineWeight_; ///< baseline weight factors

    };
}


#endif //MULTISCHEDULING_H
