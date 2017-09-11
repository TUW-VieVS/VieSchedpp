//
// Created by matth on 03.09.2017.
//

#ifndef VLBI_MULTISCHED_H
#define VLBI_MULTISCHED_H

#include <boost/date_time.hpp>
#include <vector>
#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "VieVS_parameterGroup.h"

namespace VieVS {
    class VLBI_multiSched {
    public:

        struct PARAMETERS {
            boost::optional<boost::posix_time::ptime> start;
            boost::optional<bool> multiSched_subnetting;
            boost::optional<bool> multisched_fillinmode;

            boost::optional<double> weight_skyCoverage;
            boost::optional<double> weight_numberOfObservations;
            boost::optional<double> weight_duration;
            boost::optional<double> weight_averageSources;
            boost::optional<double> weight_averageStations;

            std::map<std::string, unsigned int> station_maxSlewtime;
            std::map<std::string, unsigned int> station_maxWait;
            std::map<std::string, unsigned int> station_maxScan;
            std::map<std::string, unsigned int> station_minScan;
            std::map<std::string, double> station_weight;

            std::map<std::string, unsigned int> source_minNumberOfStations;
            std::map<std::string, double> source_minFlux;
            std::map<std::string, unsigned int> source_minRepeat;
            std::map<std::string, unsigned int> source_maxScan;
            std::map<std::string, unsigned int> source_minScan;
            std::map<std::string, double> source_weight;

            std::map<std::string, unsigned int> baseline_maxScan;
            std::map<std::string, unsigned int> baseline_minScan;
            std::map<std::string, double> baseline_weight;

            void output(std::ofstream &of) const {
                unsigned long nsta = station_weight.size();

                if (start.is_initialized()) {
                    of << "start " << *start << std::endl;
                }
                if (multiSched_subnetting.is_initialized()) {
                    of << "multisched_subnetting " << *multiSched_subnetting << std::endl;
                }
                if (multisched_fillinmode.is_initialized()) {
                    of << "multisched_fillinmode " << *multisched_fillinmode << std::endl;
                }

                if (weight_skyCoverage.is_initialized()) {
                    of << "weight_skyCoverage          " << *weight_skyCoverage << std::endl;
                }
                if (weight_numberOfObservations.is_initialized()) {
                    of << "weight_numberOfObservations " << *weight_numberOfObservations << std::endl;
                }
                if (weight_duration.is_initialized()) {
                    of << "weight_duration             " << *weight_duration << std::endl;
                }
                if (weight_averageSources.is_initialized()) {
                    of << "weight_averageSources       " << *weight_averageSources << std::endl;
                }
                if (weight_averageStations.is_initialized()) {
                    of << "weight_averageStations      " << *weight_averageStations << std::endl;
                }

                int c;
                for (const auto &any: station_maxSlewtime) {
                    of << "station_maxSlewtime " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: station_maxWait) {
                    of << "station_maxWait     " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: station_maxScan) {
                    of << "station_maxScan     " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: station_minScan) {
                    of << "station_minScan     " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: station_weight) {
                    of << "station_weight      " << any.first << ": " << any.second << std::endl;
                }

                for (const auto &any: source_minNumberOfStations) {
                    of << "source_minNumberOfStations " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: source_minFlux) {
                    of << "source_minFlux             " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: source_minRepeat) {
                    of << "source_minRepeat           " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: source_maxScan) {
                    of << "source_maxScan             " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: source_minScan) {
                    of << "source_minScan             " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: source_weight) {
                    of << "source_weight              " << any.first << ": " << any.second << std::endl;
                }

                for (const auto &any: baseline_maxScan) {
                    of << "baseline_maxScan " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: baseline_minScan) {
                    of << "baseline_minScan " << any.first << ": " << any.second << std::endl;
                }
                for (const auto &any: baseline_weight) {
                    of << "baseline_weight  " << any.first << ": " << any.second << std::endl;
                }

            }
        };


        VLBI_multiSched();

        void setStart(const std::vector<boost::posix_time::ptime> &start) {
            VLBI_multiSched::start = start;
        }

        void setMultiSched_subnetting(bool flag) {
            VLBI_multiSched::multisched_subnetting = flag;
        }

        void setMultiSched_fillinmode(bool flag) {
            VLBI_multiSched::multisched_fillinmode = flag;
        }

        void setWeight_skyCoverage(const std::vector<double> &weight_skyCoverage) {
            VLBI_multiSched::weight_skyCoverage = weight_skyCoverage;
        }

        void setWeight_numberOfObservations(const std::vector<double> &weight_numberOfObservations) {
            VLBI_multiSched::weight_numberOfObservations = weight_numberOfObservations;
        }

        void setWeight_duration(const std::vector<double> &weight_duration) {
            VLBI_multiSched::weight_duration = weight_duration;
        }

        void setWeight_averageSources(const std::vector<double> &weight_averageSources) {
            VLBI_multiSched::weight_averageSources = weight_averageSources;
        }

        void setWeight_averageStations(const std::vector<double> &weight_averageStations) {
            VLBI_multiSched::weight_averageStations = weight_averageStations;
        }

        void setStation_maxSlewtime(const std::string &new_id, const std::vector<unsigned int> &values);

        void setStation_maxSlewtime(const VieVS_parameterGroup &group, const std::vector<unsigned int> &values) {
            setStation_maxSlewtime(group.name, values);
            group_station[group.name] = group.members;
        }

        void setStation_maxWait(const std::string &new_id, const std::vector<unsigned int> &values);

        void setStation_maxWait(const VieVS_parameterGroup &group, const std::vector<unsigned int> &values) {
            setStation_maxWait(group.name, values);
            group_station[group.name] = group.members;
        }

        void setStation_maxScan(const std::string &new_id, const std::vector<unsigned int> &values);

        void setStation_maxScan(const VieVS_parameterGroup &group, const std::vector<unsigned int> &values) {
            setStation_maxScan(group.name, values);
            group_station[group.name] = group.members;
        }

        void setStation_minScan(const std::string &new_id, const std::vector<unsigned int> &values);

        void setStation_minScan(const VieVS_parameterGroup &group, const std::vector<unsigned int> &values) {
            setStation_minScan(group.name, values);
            group_station[group.name] = group.members;
        }

        void setStation_weight(const std::string &new_id, const std::vector<double> &values);

        void setStation_weight(const VieVS_parameterGroup &group, const std::vector<double> &values) {
            setStation_weight(group.name, values);
            group_station[group.name] = group.members;
        }

        void setSource_minNumberOfStations(const std::string &new_id, const std::vector<unsigned int> &values);

        void setSource_minNumberOfStations(const VieVS_parameterGroup &group, const std::vector<unsigned int> &values) {
            setSource_minNumberOfStations(group.name, values);
            group_source[group.name] = group.members;
        }

        void setSource_minFlux(const std::string &new_id, const std::vector<double> &values);

        void setSource_minFlux(const VieVS_parameterGroup &group, const std::vector<double> &values) {
            setSource_minFlux(group.name, values);
            group_source[group.name] = group.members;
        }

        void setSource_minRepeat(const std::string &new_id, const std::vector<unsigned int> &values);

        void setSource_minRepeat(const VieVS_parameterGroup &group, const std::vector<unsigned int> &values) {
            setSource_minRepeat(group.name, values);
            group_source[group.name] = group.members;
        }

        void setSource_maxScan(const std::string &new_id, const std::vector<unsigned int> &values);

        void setSource_maxScan(const VieVS_parameterGroup &group, const std::vector<unsigned int> &values) {
            setSource_maxScan(group.name, values);
            group_source[group.name] = group.members;
        }

        void setSource_minScan(const std::string &new_id, const std::vector<unsigned int> &values);

        void setSource_minScan(const VieVS_parameterGroup &group, const std::vector<unsigned int> &values) {
            setSource_minScan(group.name, values);
            group_source[group.name] = group.members;
        }

        void setSource_weight(const std::string &new_id, const std::vector<double> &values);

        void setSource_weight(const VieVS_parameterGroup &group, const std::vector<double> &values) {
            setSource_weight(group.name, values);
            group_source[group.name] = group.members;
        }

        void setBaseline_maxScan(const std::string &new_id, const std::vector<unsigned int> &values);

        void setBaseline_maxScan(const VieVS_parameterGroup &group, const std::vector<unsigned int> &values) {
            setBaseline_maxScan(group.name, values);
            group_baseline[group.name] = group.members;
        }

        void setBaseline_minScan(const std::string &new_id, const std::vector<unsigned int> &values);

        void setBaseline_minScan(const VieVS_parameterGroup &group, const std::vector<unsigned int> &values) {
            setBaseline_minScan(group.name, values);
            group_baseline[group.name] = group.members;
        }

        void setBaseline_weight(const std::string &new_id, const std::vector<double> &values);

        void setBaseline_weight(const VieVS_parameterGroup &group, const std::vector<double> &values) {
            setBaseline_weight(group.name, values);
            group_baseline[group.name] = group.members;
        }

        std::vector<PARAMETERS> createMultiScheduleParameters();

        boost::property_tree::ptree createPropertyTree() const;

    private:
        std::unordered_map<std::string, std::vector<std::string> > group_station;
        std::unordered_map<std::string, std::vector<std::string> > group_source;
        std::unordered_map<std::string, std::vector<std::string> > group_baseline;


        std::vector<boost::posix_time::ptime> start;
        bool multisched_subnetting = false;
        bool multisched_fillinmode = false;


        std::vector<double> weight_skyCoverage;
        std::vector<double> weight_numberOfObservations;
        std::vector<double> weight_duration;
        std::vector<double> weight_averageSources;
        std::vector<double> weight_averageStations;


        std::vector<std::pair<std::string, std::vector<unsigned int> > > station_maxSlewtime;
        std::vector<std::pair<std::string, std::vector<unsigned int> > > station_maxWait;
        std::vector<std::pair<std::string, std::vector<unsigned int> > > station_maxScan;
        std::vector<std::pair<std::string, std::vector<unsigned int> > > station_minScan;
        std::vector<std::pair<std::string, std::vector<double> > > station_weight;


        std::vector<std::pair<std::string, std::vector<unsigned int> > > source_minNumberOfStations;
        std::vector<std::pair<std::string, std::vector<double> > > source_minFlux;
        std::vector<std::pair<std::string, std::vector<unsigned int> > > source_minRepeat;
        std::vector<std::pair<std::string, std::vector<unsigned int> > > source_maxScan;
        std::vector<std::pair<std::string, std::vector<unsigned int> > > source_minScan;
        std::vector<std::pair<std::string, std::vector<double> > > source_weight;


        std::vector<std::pair<std::string, std::vector<unsigned int> > > baseline_maxScan;
        std::vector<std::pair<std::string, std::vector<unsigned int> > > baseline_minScan;
        std::vector<std::pair<std::string, std::vector<double> > > baseline_weight;

    };
}


#endif //VLBI_MULTISCHED_H
