//
// Created by matth on 03.09.2017.
//

#include "MultiScheduling.h"
using namespace std;
using namespace VieVS;


MultiScheduling::MultiScheduling() {

}

void MultiScheduling::setStation_maxSlewtime(const std::string &new_id, const std::vector<unsigned int> &values) {
    std::vector<std::string> alreadyFound;

    for (const auto &any: stationMaxSlewtime_) {
        std::string name = any.first;
        if (stationGroups_.find(name) != stationGroups_.end()) {
            auto tmp = stationGroups_[name];
            for (const auto &member:tmp) {
                alreadyFound.push_back(member);
            }
        } else {
            alreadyFound.push_back(name);
        }
    }

    if (stationGroups_.find(new_id) != stationGroups_.end()) {
        auto tmp = stationGroups_[new_id];
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
    stationMaxSlewtime_.emplace_back(new_id, values);
}

void MultiScheduling::setStation_maxWait(const std::string &new_id, const std::vector<unsigned int> &values) {
    std::vector<std::string> alreadyFound;

    for (const auto &any: stationMaxWait_) {
        std::string name = any.first;
        if (stationGroups_.find(name) != stationGroups_.end()) {
            auto tmp = stationGroups_[name];
            for (const auto &member:tmp) {
                alreadyFound.push_back(member);
            }
        } else {
            alreadyFound.push_back(name);
        }
    }

    if (stationGroups_.find(new_id) != stationGroups_.end()) {
        auto tmp = stationGroups_[new_id];
        for (const auto &member:tmp) {
            if (find(alreadyFound.begin(), alreadyFound.end(), member) != alreadyFound.end()) {
                std::cout << "ERROR: multisched stationMaxWait_: double use of ID " << member << std::endl;
                std::terminate();
            }
        }
    } else {
        if (find(alreadyFound.begin(), alreadyFound.end(), new_id) != alreadyFound.end()) {
            std::cout << "ERROR: multisched stationMaxWait_: double use of ID " << new_id << std::endl;
            std::terminate();
        }
    }
    stationMaxWait_.emplace_back(new_id, values);
}

void MultiScheduling::setStation_maxScan(const std::string &new_id, const std::vector<unsigned int> &values) {
    std::vector<std::string> alreadyFound;

    for (const auto &any: stationMaxScan_) {
        std::string name = any.first;
        if (stationGroups_.find(name) != stationGroups_.end()) {
            auto tmp = stationGroups_[name];
            for (const auto &member:tmp) {
                alreadyFound.push_back(member);
            }
        } else {
            alreadyFound.push_back(name);
        }
    }

    if (stationGroups_.find(new_id) != stationGroups_.end()) {
        auto tmp = stationGroups_[new_id];
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
    stationMaxScan_.emplace_back(new_id, values);
}

void MultiScheduling::setStation_minScan(const std::string &new_id, const std::vector<unsigned int> &values) {
    std::vector<std::string> alreadyFound;

    for (const auto &any: stationMinScan_) {
        std::string name = any.first;
        if (stationGroups_.find(name) != stationGroups_.end()) {
            auto tmp = stationGroups_[name];
            for (const auto &member:tmp) {
                alreadyFound.push_back(member);
            }
        } else {
            alreadyFound.push_back(name);
        }
    }

    if (stationGroups_.find(new_id) != stationGroups_.end()) {
        auto tmp = stationGroups_[new_id];
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
    stationMinScan_.emplace_back(new_id, values);
}

void
MultiScheduling::setStation_weight(const std::string &new_id, const std::vector<double> &values) {
    std::vector<std::string> alreadyFound;

    for (const auto &any: stationWeight_) {
        std::string name = any.first;
        if (stationGroups_.find(name) != stationGroups_.end()) {
            auto tmp = stationGroups_[name];
            for (const auto &member:tmp) {
                alreadyFound.push_back(member);
            }
        } else {
            alreadyFound.push_back(name);
        }
    }

    if (stationGroups_.find(new_id) != stationGroups_.end()) {
        auto tmp = stationGroups_[new_id];
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
    stationWeight_.emplace_back(new_id, values);
}

void
MultiScheduling::setSource_minNumberOfStations(const std::string &new_id, const std::vector<unsigned int> &values) {
    std::vector<std::string> alreadyFound;

    for (const auto &any: sourceMinNumberOfStations_) {
        std::string name = any.first;
        if (sourceGroups_.find(name) != sourceGroups_.end()) {
            auto tmp = sourceGroups_[name];
            for (const auto &member:tmp) {
                alreadyFound.push_back(member);
            }
        } else {
            alreadyFound.push_back(name);
        }
    }

    if (sourceGroups_.find(new_id) != sourceGroups_.end()) {
        auto tmp = sourceGroups_[new_id];
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
    sourceMinNumberOfStations_.emplace_back(new_id, values);
}

void
MultiScheduling::setSource_minFlux(const std::string &new_id, const std::vector<double> &values) {
    std::vector<std::string> alreadyFound;

    for (const auto &any: sourceMinFlux_) {
        std::string name = any.first;
        if (sourceGroups_.find(name) != sourceGroups_.end()) {
            auto tmp = sourceGroups_[name];
            for (const auto &member:tmp) {
                alreadyFound.push_back(member);
            }
        } else {
            alreadyFound.push_back(name);
        }
    }

    if (sourceGroups_.find(new_id) != sourceGroups_.end()) {
        auto tmp = sourceGroups_[new_id];
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
    sourceMinFlux_.emplace_back(new_id, values);
}

void MultiScheduling::setSource_minRepeat(const std::string &new_id, const std::vector<unsigned int> &values) {
    std::vector<std::string> alreadyFound;

    for (const auto &any: sourceMinRepeatability_) {
        std::string name = any.first;
        if (sourceGroups_.find(name) != sourceGroups_.end()) {
            auto tmp = sourceGroups_[name];
            for (const auto &member:tmp) {
                alreadyFound.push_back(member);
            }
        } else {
            alreadyFound.push_back(name);
        }
    }

    if (sourceGroups_.find(new_id) != sourceGroups_.end()) {
        auto tmp = sourceGroups_[new_id];
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
    sourceMinRepeatability_.emplace_back(new_id, values);
}

void MultiScheduling::setSource_maxScan(const std::string &new_id, const std::vector<unsigned int> &values) {
    std::vector<std::string> alreadyFound;

    for (const auto &any: sourceMaxScan_) {
        std::string name = any.first;
        if (sourceGroups_.find(name) != sourceGroups_.end()) {
            auto tmp = sourceGroups_[name];
            for (const auto &member:tmp) {
                alreadyFound.push_back(member);
            }
        } else {
            alreadyFound.push_back(name);
        }
    }

    if (sourceGroups_.find(new_id) != sourceGroups_.end()) {
        auto tmp = sourceGroups_[new_id];
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
    sourceMaxScan_.emplace_back(new_id, values);
}

void MultiScheduling::setSource_minScan(const std::string &new_id, const std::vector<unsigned int> &values) {
    std::vector<std::string> alreadyFound;

    for (const auto &any: sourceMinScan_) {
        std::string name = any.first;
        if (sourceGroups_.find(name) != sourceGroups_.end()) {
            auto tmp = sourceGroups_[name];
            for (const auto &member:tmp) {
                alreadyFound.push_back(member);
            }
        } else {
            alreadyFound.push_back(name);
        }
    }

    if (sourceGroups_.find(new_id) != sourceGroups_.end()) {
        auto tmp = sourceGroups_[new_id];
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
    sourceMinScan_.emplace_back(new_id, values);
}

void MultiScheduling::setSource_weight(const std::string &new_id, const std::vector<double> &values) {
    std::vector<std::string> alreadyFound;

    for (const auto &any: sourceWeight_) {
        std::string name = any.first;
        if (sourceGroups_.find(name) != sourceGroups_.end()) {
            auto tmp = sourceGroups_[name];
            for (const auto &member:tmp) {
                alreadyFound.push_back(member);
            }
        } else {
            alreadyFound.push_back(name);
        }
    }

    if (sourceGroups_.find(new_id) != sourceGroups_.end()) {
        auto tmp = sourceGroups_[new_id];
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
    sourceWeight_.emplace_back(new_id, values);
}

void MultiScheduling::setBaseline_maxScan(const std::string &new_id, const std::vector<unsigned int> &values) {
    std::vector<std::string> alreadyFound;

    for (const auto &any: baselineMaxScan_) {
        std::string name = any.first;
        if (baselineGroups_.find(name) != baselineGroups_.end()) {
            auto tmp = baselineGroups_[name];
            for (const auto &member:tmp) {
                alreadyFound.push_back(member);
            }
        } else {
            alreadyFound.push_back(name);
        }
    }

    if (baselineGroups_.find(new_id) != baselineGroups_.end()) {
        auto tmp = baselineGroups_[new_id];
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
    baselineMaxScan_.emplace_back(new_id, values);
}

void MultiScheduling::setBaseline_minScan(const std::string &new_id, const std::vector<unsigned int> &values) {
    std::vector<std::string> alreadyFound;

    for (const auto &any: baselineMinScan_) {
        std::string name = any.first;
        if (baselineGroups_.find(name) != baselineGroups_.end()) {
            auto tmp = baselineGroups_[name];
            for (const auto &member:tmp) {
                alreadyFound.push_back(member);
            }
        } else {
            alreadyFound.push_back(name);
        }
    }

    if (baselineGroups_.find(new_id) != baselineGroups_.end()) {
        auto tmp = baselineGroups_[new_id];
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
    baselineMinScan_.emplace_back(new_id, values);
}

void MultiScheduling::setBaseline_weight(const std::string &new_id, const std::vector<double> &values) {
    std::vector<std::string> alreadyFound;

    for (const auto &any: baselineWeight_) {
        std::string name = any.first;
        if (baselineGroups_.find(name) != baselineGroups_.end()) {
            auto tmp = baselineGroups_[name];
            for (const auto &member:tmp) {
                alreadyFound.push_back(member);
            }
        } else {
            alreadyFound.push_back(name);
        }
    }

    if (baselineGroups_.find(new_id) != baselineGroups_.end()) {
        auto tmp = baselineGroups_[new_id];
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
    baselineWeight_.emplace_back(new_id, values);
}

std::vector<MultiScheduling::Parameters> MultiScheduling::createMultiScheduleParameters() {
    std::vector<unsigned int> counter;

    if (!start_.empty()) {
        counter.push_back(start_.size());
    }
    if (subnetting_) {
        counter.push_back(2);
    }
    if (fillinmode_) {
        counter.push_back(2);
    }

    if (!weightSkyCoverage_.empty()) {
        counter.push_back(weightSkyCoverage_.size());
    }
    if (!weightNumberOfObservations_.empty()) {
        counter.push_back(weightNumberOfObservations_.size());
    }
    if (!weightDuration_.empty()) {
        counter.push_back(weightDuration_.size());
    }
    if (!weightAverageSources_.empty()) {
        counter.push_back(weightAverageSources_.size());
    }
    if (!weightAverageStations_.empty()) {
        counter.push_back(weightAverageStations_.size());
    }

    if (!stationMaxSlewtime_.empty()) {
        for (auto &i : stationMaxSlewtime_) {
            counter.push_back(i.second.size());
        }
    }
    if (!stationMaxWait_.empty()) {
        for (auto &i : stationMaxWait_) {
            counter.push_back(i.second.size());
        }
    }
    if (!stationMaxScan_.empty()) {
        for (auto &i : stationMaxScan_) {
            counter.push_back(i.second.size());
        }
    }
    if (!stationMinScan_.empty()) {
        for (auto &i : stationMinScan_) {
            counter.push_back(i.second.size());
        }
    }
    if (!stationWeight_.empty()) {
        for (auto &i : stationWeight_) {
            counter.push_back(i.second.size());
        }
    }


    if (!sourceMinNumberOfStations_.empty()) {
        for (auto &i : sourceMinNumberOfStations_) {
            counter.push_back(i.second.size());
        }
    }
    if (!sourceMinFlux_.empty()) {
        for (auto &i : sourceMinFlux_) {
            counter.push_back(i.second.size());
        }
    }
    if (!sourceMinRepeatability_.empty()) {
        for (auto &i : sourceMinRepeatability_) {
            counter.push_back(i.second.size());
        }
    }
    if (!sourceMaxScan_.empty()) {
        for (auto &i : sourceMaxScan_) {
            counter.push_back(i.second.size());
        }
    }
    if (!sourceMinScan_.empty()) {
        for (auto &i : sourceMinScan_) {
            counter.push_back(i.second.size());
        }
    }
    if (!sourceWeight_.empty()) {
        for (auto &i : sourceWeight_) {
            counter.push_back(i.second.size());
        }
    }

    if (!baselineMaxScan_.empty()) {
        for (auto &i : baselineMaxScan_) {
            counter.push_back(i.second.size());
        }
    }
    if (!baselineMinScan_.empty()) {
        for (auto &i : baselineMinScan_) {
            counter.push_back(i.second.size());
        }
    }
    if (!baselineWeight_.empty()) {
        for (auto &i : baselineWeight_) {
            counter.push_back(i.second.size());
        }
    }

    unsigned int n_total = 1;
    for (auto &i : counter) {
        n_total *= i;
    }

    Parameters thisPARA;
    std::vector<Parameters> allPARA(n_total, thisPARA);

    unsigned int n_before = 1;
    if (!start_.empty()) {
        unsigned long n_this = start_.size();
        unsigned long n_block = n_before * n_this;
        unsigned int n_items = n_total / n_block;
        unsigned int c = 0;
        for (int i_block = 0; i_block < n_block; ++i_block) {
            auto thisValue = start_[i_block % n_this];

            for (int i_item = 0; i_item < n_items; ++i_item) {
                allPARA[c].start = thisValue;
                ++c;
            }
        }
        n_before = n_block;
    }

    if (subnetting_) {
        unsigned long n_block = n_before * 2;
        unsigned int n_items = n_total / n_block;
        unsigned int c = 0;
        for (int i_block = 0; i_block < n_block; ++i_block) {
            bool thisValue = i_block % 2 == 0;

            for (int i_item = 0; i_item < n_items; ++i_item) {
                allPARA[c].subnetting = thisValue;
                ++c;
            }
        }
        n_before = n_block;
    }
    if (fillinmode_) {
        unsigned long n_block = n_before * 2;
        unsigned int n_items = n_total / n_block;
        unsigned int c = 0;
        for (int i_block = 0; i_block < n_block; ++i_block) {
            bool thisValue = i_block % 2 == 0;

            for (int i_item = 0; i_item < n_items; ++i_item) {
                allPARA[c].fillinmode = thisValue;
                ++c;
            }
        }
        n_before = n_block;
    }

    if (!weightSkyCoverage_.empty()) {
        unsigned long n_this = weightSkyCoverage_.size();
        unsigned long n_block = n_before * n_this;
        unsigned int n_items = n_total / n_block;
        unsigned int c = 0;
        for (int i_block = 0; i_block < n_block; ++i_block) {
            auto thisValue = weightSkyCoverage_[i_block % n_this];

            for (int i_item = 0; i_item < n_items; ++i_item) {
                allPARA[c].weightSkyCoverage = thisValue;
                ++c;
            }
        }
        n_before = n_block;
    }
    if (!weightNumberOfObservations_.empty()) {
        unsigned long n_this = weightNumberOfObservations_.size();
        unsigned long n_block = n_before * n_this;
        unsigned int n_items = n_total / n_block;
        unsigned int c = 0;
        for (int i_block = 0; i_block < n_block; ++i_block) {
            auto thisValue = weightNumberOfObservations_[i_block % n_this];

            for (int i_item = 0; i_item < n_items; ++i_item) {
                allPARA[c].weightNumberOfObservations = thisValue;
                ++c;
            }
        }
        n_before = n_block;
    }
    if (!weightDuration_.empty()) {
        unsigned long n_this = weightDuration_.size();
        unsigned long n_block = n_before * n_this;
        unsigned int n_items = n_total / n_block;
        unsigned int c = 0;
        for (int i_block = 0; i_block < n_block; ++i_block) {
            auto thisValue = weightDuration_[i_block % n_this];

            for (int i_item = 0; i_item < n_items; ++i_item) {
                allPARA[c].weightDuration = thisValue;
                ++c;
            }
        }
        n_before = n_block;
    }
    if (!weightAverageSources_.empty()) {
        unsigned long n_this = weightAverageSources_.size();
        unsigned long n_block = n_before * n_this;
        unsigned int n_items = n_total / n_block;
        unsigned int c = 0;
        for (int i_block = 0; i_block < n_block; ++i_block) {
            auto thisValue = weightAverageSources_[i_block % n_this];

            for (int i_item = 0; i_item < n_items; ++i_item) {
                allPARA[c].weightAverageSources = thisValue;
                ++c;
            }
        }
        n_before = n_block;
    }
    if (!weightAverageStations_.empty()) {
        unsigned long n_this = weightAverageStations_.size();
        unsigned long n_block = n_before * n_this;
        unsigned int n_items = n_total / n_block;
        unsigned int c = 0;
        for (int i_block = 0; i_block < n_block; ++i_block) {
            auto thisValue = weightAverageStations_[i_block % n_this];

            for (int i_item = 0; i_item < n_items; ++i_item) {
                allPARA[c].weightAverageStations = thisValue;
                ++c;
            }
        }
        n_before = n_block;
    }


    if (!stationMaxSlewtime_.empty()) {
        for (const auto &any: stationMaxSlewtime_) {
            const std::string &ids = any.first;
            const auto &values = any.second;

            unsigned long n_this = values.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = values[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    if (stationGroups_.find(ids) != stationGroups_.end()) {
                        for (const auto &thisId: stationGroups_[ids]) {
                            allPARA[c].stationMaxSlewtime[thisId] = thisValue;
                        }
                    } else {
                        allPARA[c].stationMaxSlewtime[ids] = thisValue;
                    }
                    ++c;
                }
            }
            n_before = n_block;
        }
    }
    if (!stationMaxWait_.empty()) {
        for (const auto &any: stationMaxWait_) {
            const std::string &ids = any.first;
            const auto &values = any.second;

            unsigned long n_this = values.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = values[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    if (stationGroups_.find(ids) != stationGroups_.end()) {
                        for (const auto &thisId: stationGroups_[ids]) {
                            allPARA[c].stationMaxWait[thisId] = thisValue;
                        }
                    } else {
                        allPARA[c].stationMaxWait[ids] = thisValue;
                    }
                    ++c;
                }
            }
            n_before = n_block;
        }
    }
    if (!stationMaxScan_.empty()) {
        for (const auto &any: stationMaxScan_) {
            const std::string &ids = any.first;
            const auto &values = any.second;

            unsigned long n_this = values.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = values[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    if (stationGroups_.find(ids) != stationGroups_.end()) {
                        for (const auto &thisId: stationGroups_[ids]) {
                            allPARA[c].stationMaxScan[thisId] = thisValue;
                        }
                    } else {
                        allPARA[c].stationMaxScan[ids] = thisValue;
                    }
                    ++c;
                }
            }
            n_before = n_block;
        }
    }
    if (!stationMinScan_.empty()) {
        for (const auto &any: stationMinScan_) {
            const std::string &ids = any.first;
            const auto &values = any.second;

            unsigned long n_this = values.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = values[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    if (stationGroups_.find(ids) != stationGroups_.end()) {
                        for (const auto &thisId: stationGroups_[ids]) {
                            allPARA[c].stationMinScan[thisId] = thisValue;
                        }
                    } else {
                        allPARA[c].stationMinScan[ids] = thisValue;
                    }
                    ++c;
                }
            }
            n_before = n_block;
        }
    }
    if (!stationWeight_.empty()) {
        for (const auto &any: stationWeight_) {
            const std::string &ids = any.first;
            const auto &values = any.second;

            unsigned long n_this = values.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = values[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    if (stationGroups_.find(ids) != stationGroups_.end()) {
                        for (const auto &thisId: stationGroups_[ids]) {
                            allPARA[c].stationWeight[thisId] = thisValue;
                        }
                    } else {
                        allPARA[c].stationWeight[ids] = thisValue;
                    }
                    ++c;
                }
            }
            n_before = n_block;
        }
    }


    if (!sourceMinNumberOfStations_.empty()) {
        for (const auto &any: sourceMinNumberOfStations_) {
            const std::string &ids = any.first;
            const auto &values = any.second;

            unsigned long n_this = values.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = values[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    if (sourceGroups_.find(ids) != sourceGroups_.end()) {
                        for (const auto &thisId: sourceGroups_[ids]) {
                            allPARA[c].sourceMinNumberOfStations[thisId] = thisValue;
                        }
                    } else {
                        allPARA[c].sourceMinNumberOfStations[ids] = thisValue;
                    }
                    ++c;
                }
            }
            n_before = n_block;
        }
    }
    if (!sourceMinFlux_.empty()) {
        for (const auto &any: sourceMinFlux_) {
            const std::string &ids = any.first;
            const auto &values = any.second;

            unsigned long n_this = values.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = values[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    if (sourceGroups_.find(ids) != sourceGroups_.end()) {
                        for (const auto &thisId: sourceGroups_[ids]) {
                            allPARA[c].sourceMinFlux[thisId] = thisValue;
                        }
                    } else {
                        allPARA[c].sourceMinFlux[ids] = thisValue;
                    }
                    ++c;
                }
            }
            n_before = n_block;
        }
    }
    if (!sourceMinRepeatability_.empty()) {
        for (const auto &any: sourceMinRepeatability_) {
            const std::string &ids = any.first;
            const auto &values = any.second;

            unsigned long n_this = values.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = values[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    if (sourceGroups_.find(ids) != sourceGroups_.end()) {
                        for (const auto &thisId: sourceGroups_[ids]) {
                            allPARA[c].sourceMinRepeat[thisId] = thisValue;
                        }
                    } else {
                        allPARA[c].sourceMinRepeat[ids] = thisValue;
                    }
                    ++c;
                }
            }
            n_before = n_block;
        }
    }
    if (!sourceMaxScan_.empty()) {
        for (const auto &any: sourceMaxScan_) {
            const std::string &ids = any.first;
            const auto &values = any.second;

            unsigned long n_this = values.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = values[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    if (sourceGroups_.find(ids) != sourceGroups_.end()) {
                        for (const auto &thisId: sourceGroups_[ids]) {
                            allPARA[c].sourceMaxScan[thisId] = thisValue;
                        }
                    } else {
                        allPARA[c].sourceMaxScan[ids] = thisValue;
                    }
                    ++c;
                }
            }
            n_before = n_block;
        }
    }
    if (!sourceMinScan_.empty()) {
        for (const auto &any: sourceMinScan_) {
            const std::string &ids = any.first;
            const auto &values = any.second;

            unsigned long n_this = values.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = values[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    if (sourceGroups_.find(ids) != sourceGroups_.end()) {
                        for (const auto &thisId: sourceGroups_[ids]) {
                            allPARA[c].sourceMinScan[thisId] = thisValue;
                        }
                    } else {
                        allPARA[c].sourceMinScan[ids] = thisValue;
                    }
                    ++c;
                }
            }
            n_before = n_block;
        }
    }
    if (!sourceWeight_.empty()) {
        for (const auto &any: sourceWeight_) {
            const std::string &ids = any.first;
            const auto &values = any.second;

            unsigned long n_this = values.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = values[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    if (sourceGroups_.find(ids) != sourceGroups_.end()) {
                        for (const auto &thisId: sourceGroups_[ids]) {
                            allPARA[c].sourceWeight[thisId] = thisValue;
                        }
                    } else {
                        allPARA[c].sourceWeight[ids] = thisValue;
                    }
                    ++c;
                }
            }
            n_before = n_block;
        }
    }

    if (!baselineMaxScan_.empty()) {
        for (const auto &any: baselineMaxScan_) {
            const std::string &ids = any.first;
            const auto &values = any.second;

            unsigned long n_this = values.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = values[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    if (baselineGroups_.find(ids) != baselineGroups_.end()) {
                        for (const auto &thisId: baselineGroups_[ids]) {
                            allPARA[c].baselineMaxScan[thisId] = thisValue;
                        }
                    } else {
                        allPARA[c].baselineMaxScan[ids] = thisValue;
                    }
                    ++c;
                }
            }
            n_before = n_block;
        }
    }
    if (!baselineMinScan_.empty()) {
        for (const auto &any: baselineMinScan_) {
            const std::string &ids = any.first;
            const auto &values = any.second;

            unsigned long n_this = values.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = values[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    if (baselineGroups_.find(ids) != baselineGroups_.end()) {
                        for (const auto &thisId: baselineGroups_[ids]) {
                            allPARA[c].baselineMinScan[thisId] = thisValue;
                        }
                    } else {
                        allPARA[c].baselineMinScan[ids] = thisValue;
                    }
                    ++c;
                }
            }
            n_before = n_block;
        }
    }
    if (!baselineWeight_.empty()) {
        for (const auto &any: baselineWeight_) {
            const std::string &ids = any.first;
            const auto &values = any.second;

            unsigned long n_this = values.size();
            unsigned long n_block = n_before * n_this;
            unsigned int n_items = n_total / n_block;
            unsigned int c = 0;
            for (int i_block = 0; i_block < n_block; ++i_block) {
                auto thisValue = values[i_block % n_this];

                for (int i_item = 0; i_item < n_items; ++i_item) {
                    if (baselineGroups_.find(ids) != baselineGroups_.end()) {
                        for (const auto &thisId: baselineGroups_[ids]) {
                            allPARA[c].baselineWeight[thisId] = thisValue;
                        }
                    } else {
                        allPARA[c].baselineWeight[ids] = thisValue;
                    }
                    ++c;
                }
            }
            n_before = n_block;
        }
    }
    return allPARA;
}

boost::property_tree::ptree MultiScheduling::createPropertyTree() const {
    boost::property_tree::ptree pt;

    if (!start_.empty()) {
        boost::property_tree::ptree pt_tmp;
        for (const auto &any:start_) {
            boost::property_tree::ptree value;
            int month = any.date().month();
            std::string dateStr = (boost::format("%04d.%02d.%02d %02d:%02d:%02d")
                                   % any.date().year() %month %any.date().day()
                                   % any.time_of_day().hours() %any.time_of_day().minutes() %any.time_of_day().seconds()).str();
            value.add("start.value", dateStr);
            pt_tmp.add_child("start.value", value.get_child("start.value"));
        }
        pt.add_child("multisched.start", pt_tmp.get_child("start"));
    }

    if (subnetting_) {
        boost::property_tree::ptree pt_tmp;
        std::vector<bool> tmp{true, false};
        for (const auto &any:tmp) {
            boost::property_tree::ptree value;
            value.add("subnetting.value", any);
            pt_tmp.add_child("subnetting.value", value.get_child("subnetting.value"));
        }
        pt.add_child("multisched.subnetting", pt_tmp.get_child("subnetting"));
    }

    if (fillinmode_) {
        boost::property_tree::ptree pt_tmp;
        std::vector<bool> tmp{true, false};
        for (const auto &any:tmp) {
            boost::property_tree::ptree value;
            value.add("fillinmode.value", any);
            pt_tmp.add_child("fillinmode.value", value.get_child("fillinmode.value"));
        }
        pt.add_child("multisched.fillinmode", pt_tmp.get_child("fillinmode"));
    }

    if (!weightSkyCoverage_.empty()) {
        boost::property_tree::ptree pt_tmp;
        for (const auto &any:weightSkyCoverage_) {
            boost::property_tree::ptree value;
            value.add("weight_skyCoverage.value", any);
            pt_tmp.add_child("weight_skyCoverage.value", value.get_child("weight_skyCoverage.value"));
        }
        pt.add_child("multisched.weight_skyCoverage", pt_tmp.get_child("weight_skyCoverage"));
    }
    if (!weightAverageSources_.empty()) {
        boost::property_tree::ptree pt_tmp;
        for (const auto &any:weightAverageSources_) {
            boost::property_tree::ptree value;
            value.add("weight_averageSources.value", any);
            pt_tmp.add_child("weight_averageSources.value", value.get_child("weight_averageSources.value"));
        }
        pt.add_child("multisched.weight_averageSources", pt_tmp.get_child("weight_averageSources"));
    }
    if (!weightAverageStations_.empty()) {
        boost::property_tree::ptree pt_tmp;
        for (const auto &any:weightAverageStations_) {
            boost::property_tree::ptree value;
            value.add("weight_averageStations.value", any);
            pt_tmp.add_child("weight_averageStations.value", value.get_child("weight_averageStations.value"));
        }
        pt.add_child("multisched.weight_averageStations", pt_tmp.get_child("weight_averageStations"));
    }
    if (!weightDuration_.empty()) {
        boost::property_tree::ptree pt_tmp;
        for (const auto &any:weightDuration_) {
            boost::property_tree::ptree value;
            value.add("weight_duration.value", any);
            pt_tmp.add_child("weight_duration.value", value.get_child("weight_duration.value"));
        }
        pt.add_child("multisched.weight_duration", pt_tmp.get_child("weight_duration"));
    }
    if (!weightNumberOfObservations_.empty()) {
        boost::property_tree::ptree pt_tmp;
        for (const auto &any:weightNumberOfObservations_) {
            boost::property_tree::ptree value;
            value.add("weight_numberOfObservations.value", any);
            pt_tmp.add_child("weight_numberOfObservations.value",
                             value.get_child("weight_numberOfObservations.value"));
        }
        pt.add_child("multisched.weight_numberOfObservations", pt_tmp.get_child("weight_numberOfObservations"));
    }

    if (!stationMaxSlewtime_.empty()) {
        for (const auto &any:stationMaxSlewtime_) {
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
    if (!stationMaxWait_.empty()) {
        for (const auto &any:stationMaxWait_) {
            boost::property_tree::ptree pt_tmp;
            for (const auto &values:any.second) {
                boost::property_tree::ptree value;
                value.add("stationMaxWait_.value", values);
                pt_tmp.add_child("stationMaxWait_.value", value.get_child("stationMaxWait_.value"));
            }
            pt_tmp.add("stationMaxWait_.<xmlattr>.member", any.first);

            pt.add_child("multisched.stationMaxWait_", pt_tmp.get_child("stationMaxWait_"));
        }
    }
    if (!stationMaxScan_.empty()) {
        for (const auto &any:stationMaxScan_) {
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
    if (!stationMinScan_.empty()) {
        for (const auto &any:stationMinScan_) {
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
    if (!stationWeight_.empty()) {
        for (const auto &any:stationWeight_) {
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

    if (!sourceMinNumberOfStations_.empty()) {
        for (const auto &any:sourceMinNumberOfStations_) {
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
    if (!sourceMinFlux_.empty()) {
        for (const auto &any:sourceMinFlux_) {
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
    if (!sourceWeight_.empty()) {
        for (const auto &any:sourceWeight_) {
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
    if (!sourceMinRepeatability_.empty()) {
        for (const auto &any:sourceMinRepeatability_) {
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
    if (!sourceMinScan_.empty()) {
        for (const auto &any:sourceMinScan_) {
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
    if (!sourceMaxScan_.empty()) {
        for (const auto &any:sourceMaxScan_) {
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

    if (!baselineMaxScan_.empty()) {
        for (const auto &any:baselineMaxScan_) {
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
    if (!baselineMinScan_.empty()) {
        for (const auto &any:baselineMinScan_) {
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
    if (!baselineWeight_.empty()) {
        for (const auto &any:baselineWeight_) {
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


