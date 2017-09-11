//
// Created by matth on 03.09.2017.
//

#include "VLBI_multiSched.h"

namespace VieVS {

    VLBI_multiSched::VLBI_multiSched() {

    }

    void VLBI_multiSched::setStation_maxSlewtime(const std::string &new_id, const std::vector<unsigned int> &values) {
        std::vector<std::string> alreadyFound;

        for (const auto &any: station_maxSlewtime) {
            std::string name = any.first;
            if (group_station.find(name) != group_station.end()) {
                auto tmp = group_station[name];
                for (const auto &member:tmp) {
                    alreadyFound.push_back(member);
                }
            } else {
                alreadyFound.push_back(name);
            }
        }

        if (group_station.find(new_id) != group_station.end()) {
            auto tmp = group_station[new_id];
            for (const auto &member:tmp) {
                if (find(alreadyFound.begin(), alreadyFound.end(), member) != alreadyFound.end()) {
                    std::cout << "ERROR: multisched station_maxSlewtime: double use of ID " << member << std::endl;
                    std::terminate();
                }
            }
        } else {
            if (find(alreadyFound.begin(), alreadyFound.end(), new_id) != alreadyFound.end()) {
                std::cout << "ERROR: multisched station_maxSlewtime: double use of ID " << new_id << std::endl;
                std::terminate();
            }
        }
        station_maxSlewtime.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setStation_maxWait(const std::string &new_id, const std::vector<unsigned int> &values) {
        std::vector<std::string> alreadyFound;

        for (const auto &any: station_maxWait) {
            std::string name = any.first;
            if (group_station.find(name) != group_station.end()) {
                auto tmp = group_station[name];
                for (const auto &member:tmp) {
                    alreadyFound.push_back(member);
                }
            } else {
                alreadyFound.push_back(name);
            }
        }

        if (group_station.find(new_id) != group_station.end()) {
            auto tmp = group_station[new_id];
            for (const auto &member:tmp) {
                if (find(alreadyFound.begin(), alreadyFound.end(), member) != alreadyFound.end()) {
                    std::cout << "ERROR: multisched station_maxWait: double use of ID " << member << std::endl;
                    std::terminate();
                }
            }
        } else {
            if (find(alreadyFound.begin(), alreadyFound.end(), new_id) != alreadyFound.end()) {
                std::cout << "ERROR: multisched station_maxWait: double use of ID " << new_id << std::endl;
                std::terminate();
            }
        }
        station_maxWait.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setStation_maxScan(const std::string &new_id, const std::vector<unsigned int> &values) {
        std::vector<std::string> alreadyFound;

        for (const auto &any: station_maxScan) {
            std::string name = any.first;
            if (group_station.find(name) != group_station.end()) {
                auto tmp = group_station[name];
                for (const auto &member:tmp) {
                    alreadyFound.push_back(member);
                }
            } else {
                alreadyFound.push_back(name);
            }
        }

        if (group_station.find(new_id) != group_station.end()) {
            auto tmp = group_station[new_id];
            for (const auto &member:tmp) {
                if (find(alreadyFound.begin(), alreadyFound.end(), member) != alreadyFound.end()) {
                    std::cout << "ERROR: multisched station_maxScan: double use of ID " << member << std::endl;
                    std::terminate();
                }
            }
        } else {
            if (find(alreadyFound.begin(), alreadyFound.end(), new_id) != alreadyFound.end()) {
                std::cout << "ERROR: multisched station_maxScan: double use of ID " << new_id << std::endl;
                std::terminate();
            }
        }
        station_maxScan.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setStation_minScan(const std::string &new_id, const std::vector<unsigned int> &values) {
        std::vector<std::string> alreadyFound;

        for (const auto &any: station_minScan) {
            std::string name = any.first;
            if (group_station.find(name) != group_station.end()) {
                auto tmp = group_station[name];
                for (const auto &member:tmp) {
                    alreadyFound.push_back(member);
                }
            } else {
                alreadyFound.push_back(name);
            }
        }

        if (group_station.find(new_id) != group_station.end()) {
            auto tmp = group_station[new_id];
            for (const auto &member:tmp) {
                if (find(alreadyFound.begin(), alreadyFound.end(), member) != alreadyFound.end()) {
                    std::cout << "ERROR: multisched station_minScan: double use of ID " << member << std::endl;
                    std::terminate();
                }
            }
        } else {
            if (find(alreadyFound.begin(), alreadyFound.end(), new_id) != alreadyFound.end()) {
                std::cout << "ERROR: multisched station_minScan: double use of ID " << new_id << std::endl;
                std::terminate();
            }
        }
        station_minScan.emplace_back(new_id, values);
    }

    void
    VLBI_multiSched::setStation_weight(const std::string &new_id, const std::vector<double> &values) {
        std::vector<std::string> alreadyFound;

        for (const auto &any: station_weight) {
            std::string name = any.first;
            if (group_station.find(name) != group_station.end()) {
                auto tmp = group_station[name];
                for (const auto &member:tmp) {
                    alreadyFound.push_back(member);
                }
            } else {
                alreadyFound.push_back(name);
            }
        }

        if (group_station.find(new_id) != group_station.end()) {
            auto tmp = group_station[new_id];
            for (const auto &member:tmp) {
                if (find(alreadyFound.begin(), alreadyFound.end(), member) != alreadyFound.end()) {
                    std::cout << "ERROR: multisched station_weight: double use of ID " << member << std::endl;
                    std::terminate();
                }
            }
        } else {
            if (find(alreadyFound.begin(), alreadyFound.end(), new_id) != alreadyFound.end()) {
                std::cout << "ERROR: multisched station_weight: double use of ID " << new_id << std::endl;
                std::terminate();
            }
        }
        station_weight.emplace_back(new_id, values);
    }

    void
    VLBI_multiSched::setSource_minNumberOfStations(const std::string &new_id, const std::vector<unsigned int> &values) {
        std::vector<std::string> alreadyFound;

        for (const auto &any: source_minNumberOfStations) {
            std::string name = any.first;
            if (group_source.find(name) != group_source.end()) {
                auto tmp = group_source[name];
                for (const auto &member:tmp) {
                    alreadyFound.push_back(member);
                }
            } else {
                alreadyFound.push_back(name);
            }
        }

        if (group_source.find(new_id) != group_source.end()) {
            auto tmp = group_source[new_id];
            for (const auto &member:tmp) {
                if (find(alreadyFound.begin(), alreadyFound.end(), member) != alreadyFound.end()) {
                    std::cout << "ERROR: multisched source_minNumberOfStations: double use of ID " << member
                              << std::endl;
                    std::terminate();
                }
            }
        } else {
            if (find(alreadyFound.begin(), alreadyFound.end(), new_id) != alreadyFound.end()) {
                std::cout << "ERROR: multisched source_minNumberOfStations: double use of ID " << new_id << std::endl;
                std::terminate();
            }
        }
        source_minNumberOfStations.emplace_back(new_id, values);
    }

    void
    VLBI_multiSched::setSource_minFlux(const std::string &new_id, const std::vector<double> &values) {
        std::vector<std::string> alreadyFound;

        for (const auto &any: source_minFlux) {
            std::string name = any.first;
            if (group_source.find(name) != group_source.end()) {
                auto tmp = group_source[name];
                for (const auto &member:tmp) {
                    alreadyFound.push_back(member);
                }
            } else {
                alreadyFound.push_back(name);
            }
        }

        if (group_source.find(new_id) != group_source.end()) {
            auto tmp = group_source[new_id];
            for (const auto &member:tmp) {
                if (find(alreadyFound.begin(), alreadyFound.end(), member) != alreadyFound.end()) {
                    std::cout << "ERROR: multisched source_minFlux: double use of ID " << member << std::endl;
                    std::terminate();
                }
            }
        } else {
            if (find(alreadyFound.begin(), alreadyFound.end(), new_id) != alreadyFound.end()) {
                std::cout << "ERROR: multisched source_minFlux: double use of ID " << new_id << std::endl;
                std::terminate();
            }
        }
        source_minFlux.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setSource_minRepeat(const std::string &new_id, const std::vector<unsigned int> &values) {
        std::vector<std::string> alreadyFound;

        for (const auto &any: source_minRepeat) {
            std::string name = any.first;
            if (group_source.find(name) != group_source.end()) {
                auto tmp = group_source[name];
                for (const auto &member:tmp) {
                    alreadyFound.push_back(member);
                }
            } else {
                alreadyFound.push_back(name);
            }
        }

        if (group_source.find(new_id) != group_source.end()) {
            auto tmp = group_source[new_id];
            for (const auto &member:tmp) {
                if (find(alreadyFound.begin(), alreadyFound.end(), member) != alreadyFound.end()) {
                    std::cout << "ERROR: multisched source_minRepeat: double use of ID " << member << std::endl;
                    std::terminate();
                }
            }
        } else {
            if (find(alreadyFound.begin(), alreadyFound.end(), new_id) != alreadyFound.end()) {
                std::cout << "ERROR: multisched source_minRepeat: double use of ID " << new_id << std::endl;
                std::terminate();
            }
        }
        source_minRepeat.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setSource_maxScan(const std::string &new_id, const std::vector<unsigned int> &values) {
        std::vector<std::string> alreadyFound;

        for (const auto &any: source_maxScan) {
            std::string name = any.first;
            if (group_source.find(name) != group_source.end()) {
                auto tmp = group_source[name];
                for (const auto &member:tmp) {
                    alreadyFound.push_back(member);
                }
            } else {
                alreadyFound.push_back(name);
            }
        }

        if (group_source.find(new_id) != group_source.end()) {
            auto tmp = group_source[new_id];
            for (const auto &member:tmp) {
                if (find(alreadyFound.begin(), alreadyFound.end(), member) != alreadyFound.end()) {
                    std::cout << "ERROR: multisched source_maxScan: double use of ID " << member << std::endl;
                    std::terminate();
                }
            }
        } else {
            if (find(alreadyFound.begin(), alreadyFound.end(), new_id) != alreadyFound.end()) {
                std::cout << "ERROR: multisched source_maxScan: double use of ID " << new_id << std::endl;
                std::terminate();
            }
        }
        source_maxScan.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setSource_minScan(const std::string &new_id, const std::vector<unsigned int> &values) {
        std::vector<std::string> alreadyFound;

        for (const auto &any: source_minScan) {
            std::string name = any.first;
            if (group_source.find(name) != group_source.end()) {
                auto tmp = group_source[name];
                for (const auto &member:tmp) {
                    alreadyFound.push_back(member);
                }
            } else {
                alreadyFound.push_back(name);
            }
        }

        if (group_source.find(new_id) != group_source.end()) {
            auto tmp = group_source[new_id];
            for (const auto &member:tmp) {
                if (find(alreadyFound.begin(), alreadyFound.end(), member) != alreadyFound.end()) {
                    std::cout << "ERROR: multisched source_minScan: double use of ID " << member << std::endl;
                    std::terminate();
                }
            }
        } else {
            if (find(alreadyFound.begin(), alreadyFound.end(), new_id) != alreadyFound.end()) {
                std::cout << "ERROR: multisched source_minScan: double use of ID " << new_id << std::endl;
                std::terminate();
            }
        }
        source_minScan.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setSource_weight(const std::string &new_id, const std::vector<double> &values) {
        std::vector<std::string> alreadyFound;

        for (const auto &any: source_weight) {
            std::string name = any.first;
            if (group_source.find(name) != group_source.end()) {
                auto tmp = group_source[name];
                for (const auto &member:tmp) {
                    alreadyFound.push_back(member);
                }
            } else {
                alreadyFound.push_back(name);
            }
        }

        if (group_source.find(new_id) != group_source.end()) {
            auto tmp = group_source[new_id];
            for (const auto &member:tmp) {
                if (find(alreadyFound.begin(), alreadyFound.end(), member) != alreadyFound.end()) {
                    std::cout << "ERROR: multisched source_weight: double use of ID " << member << std::endl;
                    std::terminate();
                }
            }
        } else {
            if (find(alreadyFound.begin(), alreadyFound.end(), new_id) != alreadyFound.end()) {
                std::cout << "ERROR: multisched source_weight: double use of ID " << new_id << std::endl;
                std::terminate();
            }
        }
        source_weight.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setBaseline_maxScan(const std::string &new_id, const std::vector<unsigned int> &values) {
        std::vector<std::string> alreadyFound;

        for (const auto &any: baseline_maxScan) {
            std::string name = any.first;
            if (group_baseline.find(name) != group_baseline.end()) {
                auto tmp = group_baseline[name];
                for (const auto &member:tmp) {
                    alreadyFound.push_back(member);
                }
            } else {
                alreadyFound.push_back(name);
            }
        }

        if (group_baseline.find(new_id) != group_baseline.end()) {
            auto tmp = group_baseline[new_id];
            for (const auto &member:tmp) {
                if (find(alreadyFound.begin(), alreadyFound.end(), member) != alreadyFound.end()) {
                    std::cout << "ERROR: multisched baseline_maxScan: double use of ID " << member << std::endl;
                    std::terminate();
                }
            }
        } else {
            if (find(alreadyFound.begin(), alreadyFound.end(), new_id) != alreadyFound.end()) {
                std::cout << "ERROR: multisched baseline_maxScan: double use of ID " << new_id << std::endl;
                std::terminate();
            }
        }
        baseline_maxScan.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setBaseline_minScan(const std::string &new_id, const std::vector<unsigned int> &values) {
        std::vector<std::string> alreadyFound;

        for (const auto &any: baseline_minScan) {
            std::string name = any.first;
            if (group_baseline.find(name) != group_baseline.end()) {
                auto tmp = group_baseline[name];
                for (const auto &member:tmp) {
                    alreadyFound.push_back(member);
                }
            } else {
                alreadyFound.push_back(name);
            }
        }

        if (group_baseline.find(new_id) != group_baseline.end()) {
            auto tmp = group_baseline[new_id];
            for (const auto &member:tmp) {
                if (find(alreadyFound.begin(), alreadyFound.end(), member) != alreadyFound.end()) {
                    std::cout << "ERROR: multisched baseline_minScan: double use of ID " << member << std::endl;
                    std::terminate();
                }
            }
        } else {
            if (find(alreadyFound.begin(), alreadyFound.end(), new_id) != alreadyFound.end()) {
                std::cout << "ERROR: multisched baseline_minScan: double use of ID " << new_id << std::endl;
                std::terminate();
            }
        }
        baseline_minScan.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setBaseline_weight(const std::string &new_id, const std::vector<double> &values) {
        std::vector<std::string> alreadyFound;

        for (const auto &any: baseline_weight) {
            std::string name = any.first;
            if (group_baseline.find(name) != group_baseline.end()) {
                auto tmp = group_baseline[name];
                for (const auto &member:tmp) {
                    alreadyFound.push_back(member);
                }
            } else {
                alreadyFound.push_back(name);
            }
        }

        if (group_baseline.find(new_id) != group_baseline.end()) {
            auto tmp = group_baseline[new_id];
            for (const auto &member:tmp) {
                if (find(alreadyFound.begin(), alreadyFound.end(), member) != alreadyFound.end()) {
                    std::cout << "ERROR: multisched baseline_weight: double use of ID " << member << std::endl;
                    std::terminate();
                }
            }
        } else {
            if (find(alreadyFound.begin(), alreadyFound.end(), new_id) != alreadyFound.end()) {
                std::cout << "ERROR: multisched baseline_weight: double use of ID " << new_id << std::endl;
                std::terminate();
            }
        }
        baseline_weight.emplace_back(new_id, values);
    }

    std::vector<VLBI_multiSched::PARAMETERS> VLBI_multiSched::createMultiScheduleParameters() {
        std::vector<unsigned int> counter;

        if (!start.empty()) {
            counter.push_back(start.size());
        }
        if (multisched_subnetting) {
            counter.push_back(2);
        }
        if (multisched_fillinmode) {
            counter.push_back(2);
        }

        if (!weight_skyCoverage.empty()) {
            counter.push_back(weight_skyCoverage.size());
        }
        if (!weight_numberOfObservations.empty()) {
            counter.push_back(weight_numberOfObservations.size());
        }
        if (!weight_duration.empty()) {
            counter.push_back(weight_duration.size());
        }
        if (!weight_averageSources.empty()) {
            counter.push_back(weight_averageSources.size());
        }
        if (!weight_averageStations.empty()) {
            counter.push_back(weight_averageStations.size());
        }

        if (!station_maxSlewtime.empty()) {
            for (auto &i : station_maxSlewtime) {
                counter.push_back(i.second.size());
            }
        }
        if (!station_maxWait.empty()) {
            for (auto &i : station_maxWait) {
                counter.push_back(i.second.size());
            }
        }
        if (!station_maxScan.empty()) {
            for (auto &i : station_maxScan) {
                counter.push_back(i.second.size());
            }
        }
        if (!station_minScan.empty()) {
            for (auto &i : station_minScan) {
                counter.push_back(i.second.size());
            }
        }
        if (!station_weight.empty()) {
            for (auto &i : station_weight) {
                counter.push_back(i.second.size());
            }
        }


        if (!source_minNumberOfStations.empty()) {
            for (auto &i : source_minNumberOfStations) {
                counter.push_back(i.second.size());
            }
        }
        if (!source_minFlux.empty()) {
            for (auto &i : source_minFlux) {
                counter.push_back(i.second.size());
            }
        }
        if (!source_minRepeat.empty()) {
            for (auto &i : source_minRepeat) {
                counter.push_back(i.second.size());
            }
        }
        if (!source_maxScan.empty()) {
            for (auto &i : source_maxScan) {
                counter.push_back(i.second.size());
            }
        }
        if (!source_minScan.empty()) {
            for (auto &i : source_minScan) {
                counter.push_back(i.second.size());
            }
        }
        if (!source_weight.empty()) {
            for (auto &i : source_weight) {
                counter.push_back(i.second.size());
            }
        }

        if (!baseline_maxScan.empty()) {
            for (auto &i : baseline_maxScan) {
                counter.push_back(i.second.size());
            }
        }
        if (!baseline_minScan.empty()) {
            for (auto &i : baseline_minScan) {
                counter.push_back(i.second.size());
            }
        }
        if (!baseline_weight.empty()) {
            for (auto &i : baseline_weight) {
                counter.push_back(i.second.size());
            }
        }

        unsigned int n_total = 1;
        for (auto &i : counter) {
            n_total *= i;
        }

        PARAMETERS thisPARA;
        std::vector<PARAMETERS> allPARA(n_total, thisPARA);

        unsigned int n_before = 1;
        if (!start.empty()) {
            unsigned long n_this = start.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = start[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    allPARA[c].start = thisValue;
                    ++c;
                }
            }
            n_before = n_block;
        }

        if (multisched_subnetting) {
            unsigned long n_block = n_before * 2;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                bool thisValue = i_block % 2 == 0;

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    allPARA[c].multiSched_subnetting = thisValue;
                    ++c;
                }
            }
            n_before = n_block;
        }
        if (multisched_fillinmode) {
            unsigned long n_block = n_before * 2;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                bool thisValue = i_block % 2 == 0;

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    allPARA[c].multisched_fillinmode = thisValue;
                    ++c;
                }
            }
            n_before = n_block;
        }

        if (!weight_skyCoverage.empty()) {
            unsigned long n_this = weight_skyCoverage.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = weight_skyCoverage[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    allPARA[c].weight_skyCoverage = thisValue;
                    ++c;
                }
            }
            n_before = n_block;
        }
        if (!weight_numberOfObservations.empty()) {
            unsigned long n_this = weight_numberOfObservations.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = weight_numberOfObservations[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    allPARA[c].weight_numberOfObservations = thisValue;
                    ++c;
                }
            }
            n_before = n_block;
        }
        if (!weight_duration.empty()) {
            unsigned long n_this = weight_duration.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = weight_duration[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    allPARA[c].weight_duration = thisValue;
                    ++c;
                }
            }
            n_before = n_block;
        }
        if (!weight_averageSources.empty()) {
            unsigned long n_this = weight_averageSources.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = weight_averageSources[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    allPARA[c].weight_averageSources = thisValue;
                    ++c;
                }
            }
            n_before = n_block;
        }
        if (!weight_averageStations.empty()) {
            unsigned long n_this = weight_averageStations.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = weight_averageStations[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    allPARA[c].weight_averageStations = thisValue;
                    ++c;
                }
            }
            n_before = n_block;
        }


        if (!station_maxSlewtime.empty()) {
            for (const auto &any: station_maxSlewtime) {
                const std::string &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        if (group_station.find(ids) != group_station.end()) {
                            for (const auto &thisId: group_station[ids]) {
                                allPARA[c].station_maxSlewtime[thisId] = thisValue;
                            }
                        } else {
                            allPARA[c].station_maxSlewtime[ids] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!station_maxWait.empty()) {
            for (const auto &any: station_maxWait) {
                const std::string &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        if (group_station.find(ids) != group_station.end()) {
                            for (const auto &thisId: group_station[ids]) {
                                allPARA[c].station_maxWait[thisId] = thisValue;
                            }
                        } else {
                            allPARA[c].station_maxWait[ids] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!station_maxScan.empty()) {
            for (const auto &any: station_maxScan) {
                const std::string &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        if (group_station.find(ids) != group_station.end()) {
                            for (const auto &thisId: group_station[ids]) {
                                allPARA[c].station_maxScan[thisId] = thisValue;
                            }
                        } else {
                            allPARA[c].station_maxScan[ids] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!station_minScan.empty()) {
            for (const auto &any: station_minScan) {
                const std::string &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        if (group_station.find(ids) != group_station.end()) {
                            for (const auto &thisId: group_station[ids]) {
                                allPARA[c].station_minScan[thisId] = thisValue;
                            }
                        } else {
                            allPARA[c].station_minScan[ids] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!station_weight.empty()) {
            for (const auto &any: station_weight) {
                const std::string &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        if (group_station.find(ids) != group_station.end()) {
                            for (const auto &thisId: group_station[ids]) {
                                allPARA[c].station_weight[thisId] = thisValue;
                            }
                        } else {
                            allPARA[c].station_weight[ids] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }


        if (!source_minNumberOfStations.empty()) {
            for (const auto &any: source_minNumberOfStations) {
                const std::string &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        if (group_source.find(ids) != group_source.end()) {
                            for (const auto &thisId: group_source[ids]) {
                                allPARA[c].source_minNumberOfStations[thisId] = thisValue;
                            }
                        } else {
                            allPARA[c].source_minNumberOfStations[ids] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!source_minFlux.empty()) {
            for (const auto &any: source_minFlux) {
                const std::string &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        if (group_source.find(ids) != group_source.end()) {
                            for (const auto &thisId: group_source[ids]) {
                                allPARA[c].source_minFlux[thisId] = thisValue;
                            }
                        } else {
                            allPARA[c].source_minFlux[ids] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!source_minRepeat.empty()) {
            for (const auto &any: source_minRepeat) {
                const std::string &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        if (group_source.find(ids) != group_source.end()) {
                            for (const auto &thisId: group_source[ids]) {
                                allPARA[c].source_minRepeat[thisId] = thisValue;
                            }
                        } else {
                            allPARA[c].source_minRepeat[ids] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!source_maxScan.empty()) {
            for (const auto &any: source_maxScan) {
                const std::string &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        if (group_source.find(ids) != group_source.end()) {
                            for (const auto &thisId: group_source[ids]) {
                                allPARA[c].source_maxScan[thisId] = thisValue;
                            }
                        } else {
                            allPARA[c].source_maxScan[ids] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!source_minScan.empty()) {
            for (const auto &any: source_minScan) {
                const std::string &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        if (group_source.find(ids) != group_source.end()) {
                            for (const auto &thisId: group_source[ids]) {
                                allPARA[c].source_minScan[thisId] = thisValue;
                            }
                        } else {
                            allPARA[c].source_minScan[ids] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!source_weight.empty()) {
            for (const auto &any: source_weight) {
                const std::string &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        if (group_source.find(ids) != group_source.end()) {
                            for (const auto &thisId: group_source[ids]) {
                                allPARA[c].source_weight[thisId] = thisValue;
                            }
                        } else {
                            allPARA[c].source_weight[ids] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }

        if (!baseline_maxScan.empty()) {
            for (const auto &any: baseline_maxScan) {
                const std::string &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        if (group_baseline.find(ids) != group_baseline.end()) {
                            for (const auto &thisId: group_baseline[ids]) {
                                allPARA[c].baseline_maxScan[thisId] = thisValue;
                            }
                        } else {
                            allPARA[c].baseline_maxScan[ids] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!baseline_minScan.empty()) {
            for (const auto &any: baseline_minScan) {
                const std::string &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        if (group_baseline.find(ids) != group_baseline.end()) {
                            for (const auto &thisId: group_baseline[ids]) {
                                allPARA[c].baseline_minScan[thisId] = thisValue;
                            }
                        } else {
                            allPARA[c].baseline_minScan[ids] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!baseline_weight.empty()) {
            for (const auto &any: baseline_weight) {
                const std::string &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        if (group_baseline.find(ids) != group_baseline.end()) {
                            for (const auto &thisId: group_baseline[ids]) {
                                allPARA[c].baseline_weight[thisId] = thisValue;
                            }
                        } else {
                            allPARA[c].baseline_weight[ids] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        return allPARA;
    }

    boost::property_tree::ptree VLBI_multiSched::createPropertyTree() const {
        boost::property_tree::ptree pt;

        if (!start.empty()) {
            boost::property_tree::ptree pt_tmp;
            for (const auto &any:start) {
                boost::property_tree::ptree value;
                value.add("start.value", any);
                pt_tmp.add_child("start.value", value.get_child("start"));
            }
            pt.add_child("multisched.start", pt_tmp.get_child("start"));
        }

        if (multisched_subnetting) {
            boost::property_tree::ptree pt_tmp;
            std::vector<bool> tmp{true, false};
            for (const auto &any:tmp) {
                boost::property_tree::ptree value;
                value.add("subnetting.value", "true");
                pt_tmp.add_child("subnetting.value", value.get_child("subnetting"));
            }
            pt.add_child("multisched.subnetting", pt_tmp.get_child("subnetting"));
        }

        if (multisched_fillinmode) {
            boost::property_tree::ptree pt_tmp;
            std::vector<bool> tmp{true, false};
            for (const auto &any:tmp) {
                boost::property_tree::ptree value;
                value.add("fillinmode.value", "true");
                pt_tmp.add_child("fillinmode.value", value.get_child("fillinmode"));
            }
            pt.add_child("multisched.fillinmode", pt_tmp.get_child("fillinmode"));
        }

        if (!weight_skyCoverage.empty()) {
            boost::property_tree::ptree pt_tmp;
            for (const auto &any:weight_skyCoverage) {
                boost::property_tree::ptree value;
                value.add("weight_skyCoverage.value", any);
                pt_tmp.add_child("weight_skyCoverage.value", value.get_child("weight_skyCoverage.value"));
            }
            pt.add_child("multisched.weight_skyCoverage", pt_tmp.get_child("weight_skyCoverage"));
        }
        if (!weight_averageSources.empty()) {
            boost::property_tree::ptree pt_tmp;
            for (const auto &any:weight_averageSources) {
                boost::property_tree::ptree value;
                value.add("weight_averageSources.value", any);
                pt_tmp.add_child("weight_averageSources.value", value.get_child("weight_averageSources.value"));
            }
            pt.add_child("multisched.weight_averageSources", pt_tmp.get_child("weight_averageSources"));
        }
        if (!weight_averageStations.empty()) {
            boost::property_tree::ptree pt_tmp;
            for (const auto &any:weight_averageStations) {
                boost::property_tree::ptree value;
                value.add("weight_averageStations.value", any);
                pt_tmp.add_child("weight_averageStations.value", value.get_child("weight_averageStations.value"));
            }
            pt.add_child("multisched.weight_averageStations", pt_tmp.get_child("weight_averageStations"));
        }
        if (!weight_duration.empty()) {
            boost::property_tree::ptree pt_tmp;
            for (const auto &any:weight_duration) {
                boost::property_tree::ptree value;
                value.add("weight_duration.value", any);
                pt_tmp.add_child("weight_duration.value", value.get_child("weight_duration.value"));
            }
            pt.add_child("multisched.weight_duration", pt_tmp.get_child("weight_duration"));
        }
        if (!weight_numberOfObservations.empty()) {
            boost::property_tree::ptree pt_tmp;
            for (const auto &any:weight_numberOfObservations) {
                boost::property_tree::ptree value;
                value.add("weight_numberOfObservations.value", any);
                pt_tmp.add_child("weight_numberOfObservations.value",
                                 value.get_child("weight_numberOfObservations.value"));
            }
            pt.add_child("multisched.weight_numberOfObservations", pt_tmp.get_child("weight_numberOfObservations"));
        }

        if (!station_maxSlewtime.empty()) {
            for (const auto &any:station_maxSlewtime) {
                boost::property_tree::ptree pt_tmp;
                for (const auto &values:any.second) {
                    boost::property_tree::ptree value;
                    value.add("station_maxSlewtime.value", values);
                    pt_tmp.add_child("station_maxSlewtime.value", value.get_child("station_maxSlewtime.value"));
                }
                pt_tmp.add("station_maxSlewtime.<xmlattr>.member", any.first);

                pt.add_child("multisched.station_maxSlewtime", pt_tmp.get_child("station_maxSlewtime"));
            }
        }
        if (!station_maxWait.empty()) {
            for (const auto &any:station_maxWait) {
                boost::property_tree::ptree pt_tmp;
                for (const auto &values:any.second) {
                    boost::property_tree::ptree value;
                    value.add("station_maxWait.value", values);
                    pt_tmp.add_child("station_maxWait.value", value.get_child("station_maxWait.value"));
                }
                pt_tmp.add("station_maxWait.<xmlattr>.member", any.first);

                pt.add_child("multisched.station_maxWait", pt_tmp.get_child("station_maxWait"));
            }
        }
        if (!station_maxScan.empty()) {
            for (const auto &any:station_maxScan) {
                boost::property_tree::ptree pt_tmp;
                for (const auto &values:any.second) {
                    boost::property_tree::ptree value;
                    value.add("station_maxScan.value", values);
                    pt_tmp.add_child("station_maxScan.value", value.get_child("station_maxScan.value"));
                }
                pt_tmp.add("station_maxScan.<xmlattr>.member", any.first);

                pt.add_child("multisched.station_maxScan", pt_tmp.get_child("station_maxScan"));
            }
        }
        if (!station_minScan.empty()) {
            for (const auto &any:station_minScan) {
                boost::property_tree::ptree pt_tmp;
                for (const auto &values:any.second) {
                    boost::property_tree::ptree value;
                    value.add("station_minScan.value", values);
                    pt_tmp.add_child("station_minScan.value", value.get_child("station_minScan.value"));
                }
                pt_tmp.add("station_minScan.<xmlattr>.member", any.first);

                pt.add_child("multisched.station_minScan", pt_tmp.get_child("station_minScan"));
            }
        }
        if (!station_weight.empty()) {
            for (const auto &any:station_weight) {
                boost::property_tree::ptree pt_tmp;
                for (const auto &values:any.second) {
                    boost::property_tree::ptree value;
                    value.add("station_weight.value", values);
                    pt_tmp.add_child("station_weight.value", value.get_child("station_weight.value"));
                }
                pt_tmp.add("station_weight.<xmlattr>.member", any.first);

                pt.add_child("multisched.station_weight", pt_tmp.get_child("station_weight"));
            }
        }

        if (!source_minNumberOfStations.empty()) {
            for (const auto &any:source_minNumberOfStations) {
                boost::property_tree::ptree pt_tmp;
                for (const auto &values:any.second) {
                    boost::property_tree::ptree value;
                    value.add("source_minNumberOfStations.value", values);
                    pt_tmp.add_child("source_minNumberOfStations.value",
                                     value.get_child("source_minNumberOfStations.value"));
                }
                pt_tmp.add("source_minNumberOfStations.<xmlattr>.member", any.first);

                pt.add_child("multisched.source_minNumberOfStations", pt_tmp.get_child("source_minNumberOfStations"));
            }
        }
        if (!source_minFlux.empty()) {
            for (const auto &any:source_minFlux) {
                boost::property_tree::ptree pt_tmp;
                for (const auto &values:any.second) {
                    boost::property_tree::ptree value;
                    value.add("source_minFlux.value", values);
                    pt_tmp.add_child("source_minFlux.value", value.get_child("source_minFlux.value"));
                }
                pt_tmp.add("source_minFlux.<xmlattr>.member", any.first);

                pt.add_child("multisched.source_minFlux", pt_tmp.get_child("source_minFlux"));
            }
        }
        if (!source_weight.empty()) {
            for (const auto &any:source_weight) {
                boost::property_tree::ptree pt_tmp;
                for (const auto &values:any.second) {
                    boost::property_tree::ptree value;
                    value.add("source_weight.value", values);
                    pt_tmp.add_child("source_weight.value", value.get_child("source_weight.value"));
                }
                pt_tmp.add("source_weight.<xmlattr>.member", any.first);

                pt.add_child("multisched.source_weight", pt_tmp.get_child("source_weight"));
            }
        }
        if (!source_minRepeat.empty()) {
            for (const auto &any:source_minRepeat) {
                boost::property_tree::ptree pt_tmp;
                for (const auto &values:any.second) {
                    boost::property_tree::ptree value;
                    value.add("source_minRepeat.value", values);
                    pt_tmp.add_child("source_minRepeat.value", value.get_child("source_minRepeat.value"));
                }
                pt_tmp.add("source_minRepeat.<xmlattr>.member", any.first);

                pt.add_child("multisched.source_minRepeat", pt_tmp.get_child("source_minRepeat"));
            }
        }
        if (!source_minScan.empty()) {
            for (const auto &any:source_minScan) {
                boost::property_tree::ptree pt_tmp;
                for (const auto &values:any.second) {
                    boost::property_tree::ptree value;
                    value.add("source_minScan.value", values);
                    pt_tmp.add_child("source_minScan.value", value.get_child("source_minScan.value"));
                }
                pt_tmp.add("source_minScan.<xmlattr>.member", any.first);

                pt.add_child("multisched.source_minScan", pt_tmp.get_child("source_minScan"));
            }
        }
        if (!source_maxScan.empty()) {
            for (const auto &any:source_maxScan) {
                boost::property_tree::ptree pt_tmp;
                for (const auto &values:any.second) {
                    boost::property_tree::ptree value;
                    value.add("source_maxScan.value", values);
                    pt_tmp.add_child("source_maxScan.value", value.get_child("source_maxScan.value"));
                }
                pt_tmp.add("source_maxScan.<xmlattr>.member", any.first);

                pt.add_child("multisched.source_maxScan", pt_tmp.get_child("source_maxScan"));
            }
        }

        if (!baseline_maxScan.empty()) {
            for (const auto &any:baseline_maxScan) {
                boost::property_tree::ptree pt_tmp;
                for (const auto &values:any.second) {
                    boost::property_tree::ptree value;
                    value.add("baseline_maxScan.value", values);
                    pt_tmp.add_child("baseline_maxScan.value", value.get_child("baseline_maxScan.value"));
                }
                pt_tmp.add("baseline_maxScan.<xmlattr>.member", any.first);

                pt.add_child("multisched.baseline_maxScan", pt_tmp.get_child("baseline_maxScan"));
            }
        }
        if (!baseline_minScan.empty()) {
            for (const auto &any:baseline_minScan) {
                boost::property_tree::ptree pt_tmp;
                for (const auto &values:any.second) {
                    boost::property_tree::ptree value;
                    value.add("baseline_minScan.value", values);
                    pt_tmp.add_child("baseline_minScan.value", value.get_child("baseline_minScan.value"));
                }
                pt_tmp.add("baseline_minScan.<xmlattr>.member", any.first);

                pt.add_child("multisched.baseline_minScan", pt_tmp.get_child("baseline_minScan"));
            }
        }
        if (!baseline_weight.empty()) {
            for (const auto &any:baseline_weight) {
                boost::property_tree::ptree pt_tmp;
                for (const auto &values:any.second) {
                    boost::property_tree::ptree value;
                    value.add("baseline_weight.value", values);
                    pt_tmp.add_child("baseline_weight.value", value.get_child("baseline_weight.value"));
                }
                pt_tmp.add("baseline_weight.<xmlattr>.member", any.first);

                pt.add_child("multisched.baseline_weight", pt_tmp.get_child("baseline_weight"));
            }
        }
        return pt;
    }


}