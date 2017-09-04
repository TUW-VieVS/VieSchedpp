//
// Created by matth on 03.09.2017.
//

#include "VLBI_multiSched.h"

namespace VieVS {

    VLBI_multiSched::VLBI_multiSched(unsigned int nsta, unsigned int nsrc) : nsta{nsta}, nsrc{nsrc} {

    }

    void VLBI_multiSched::setStation_maxSlewtime(const std::vector<unsigned int> &new_id,
                                                 const std::vector<unsigned int> &values) {
        std::vector<char> alreadyFound(nsta, false);
        for (const auto &any: station_maxSlewtime) {
            for (const auto &old_id: any.first) {
                alreadyFound[old_id] = true;
            }
        }
        for (const auto &thisId: new_id) {
            if (alreadyFound[thisId] == true) {
                std::cout << "ERROR: multisched station_maxSlewtime: double use of ID " << thisId << std::endl;
                std::terminate();
            }
        }
        station_maxSlewtime.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setStation_maxWait(const std::vector<unsigned int> &new_id,
                                             const std::vector<unsigned int> &values) {
        std::vector<char> alreadyFound(nsta, false);
        for (const auto &any: station_maxWait) {
            for (const auto &old_id: any.first) {
                alreadyFound[old_id] = true;
            }
        }
        for (const auto &thisId: new_id) {
            if (alreadyFound[thisId] == true) {
                std::cout << "ERROR: multisched station_maxWait: double use of ID " << thisId << std::endl;
                std::terminate();
            }
        }
        station_maxWait.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setStation_maxScan(const std::vector<unsigned int> &new_id,
                                             const std::vector<unsigned int> &values) {
        std::vector<char> alreadyFound(nsta, false);
        for (const auto &any: station_maxScan) {
            for (const auto &old_id: any.first) {
                alreadyFound[old_id] = true;
            }
        }
        for (const auto &thisId: new_id) {
            if (alreadyFound[thisId] == true) {
                std::cout << "ERROR: multisched station_maxScan: double use of ID " << thisId << std::endl;
                std::terminate();
            }
        }
        station_maxScan.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setStation_minScan(const std::vector<unsigned int> &new_id,
                                             const std::vector<unsigned int> &values) {
        std::vector<char> alreadyFound(nsta, false);
        for (const auto &any: station_minScan) {
            for (const auto &old_id: any.first) {
                alreadyFound[old_id] = true;
            }
        }
        for (const auto &thisId: new_id) {
            if (alreadyFound[thisId] == true) {
                std::cout << "ERROR: multisched station_minScan: double use of ID " << thisId << std::endl;
                std::terminate();
            }
        }
        station_minScan.emplace_back(new_id, values);
    }

    void
    VLBI_multiSched::setStation_weight(const std::vector<unsigned int> &new_id, const std::vector<double> &values) {
        std::vector<char> alreadyFound(nsta, false);
        for (const auto &any: station_weight) {
            for (const auto &old_id: any.first) {
                alreadyFound[old_id] = true;
            }
        }
        for (const auto &thisId: new_id) {
            if (alreadyFound[thisId] == true) {
                std::cout << "ERROR: multisched station_weight: double use of ID " << thisId << std::endl;
                std::terminate();
            }
        }
        station_weight.emplace_back(new_id, values);
    }

    void
    VLBI_multiSched::setSource_minNumberOfStations(const std::vector<unsigned int> &new_id,
                                                   const std::vector<unsigned int> &values) {
        std::vector<char> alreadyFound(nsta, false);
        for (const auto &any: source_minNumberOfStations) {
            for (const auto &old_id: any.first) {
                alreadyFound[old_id] = true;
            }
        }
        for (const auto &thisId: new_id) {
            if (alreadyFound[thisId] == true) {
                std::cout << "ERROR: multisched station_weight: double use of ID " << thisId << std::endl;
                std::terminate();
            }
        }
        source_minNumberOfStations.emplace_back(new_id, values);
    }

    void
    VLBI_multiSched::setSource_minFlux(const std::vector<unsigned int> &new_id, const std::vector<double> &values) {
        std::vector<char> alreadyFound(nsta, false);
        for (const auto &any: source_minFlux) {
            for (const auto &old_id: any.first) {
                alreadyFound[old_id] = true;
            }
        }
        for (const auto &thisId: new_id) {
            if (alreadyFound[thisId] == true) {
                std::cout << "ERROR: multisched station_weight: double use of ID " << thisId << std::endl;
                std::terminate();
            }
        }
        source_minFlux.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setSource_minRepeat(const std::vector<unsigned int> &new_id,
                                              const std::vector<unsigned int> &values) {
        std::vector<char> alreadyFound(nsta, false);
        for (const auto &any: source_minRepeat) {
            for (const auto &old_id: any.first) {
                alreadyFound[old_id] = true;
            }
        }
        for (const auto &thisId: new_id) {
            if (alreadyFound[thisId] == true) {
                std::cout << "ERROR: multisched station_weight: double use of ID " << thisId << std::endl;
                std::terminate();
            }
        }
        source_minRepeat.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setSource_maxScan(const std::vector<unsigned int> &new_id,
                                            const std::vector<unsigned int> &values) {
        std::vector<char> alreadyFound(nsta, false);
        for (const auto &any: source_maxScan) {
            for (const auto &old_id: any.first) {
                alreadyFound[old_id] = true;
            }
        }
        for (const auto &thisId: new_id) {
            if (alreadyFound[thisId] == true) {
                std::cout << "ERROR: multisched station_weight: double use of ID " << thisId << std::endl;
                std::terminate();
            }
        }
        source_maxScan.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setSource_minScan(const std::vector<unsigned int> &new_id,
                                            const std::vector<unsigned int> &values) {
        std::vector<char> alreadyFound(nsta, false);
        for (const auto &any: source_minScan) {
            for (const auto &old_id: any.first) {
                alreadyFound[old_id] = true;
            }
        }
        for (const auto &thisId: new_id) {
            if (alreadyFound[thisId] == true) {
                std::cout << "ERROR: multisched station_weight: double use of ID " << thisId << std::endl;
                std::terminate();
            }
        }
        source_minScan.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setSource_weight(const std::vector<unsigned int> &new_id, const std::vector<double> &values) {
        std::vector<char> alreadyFound(nsta, false);
        for (const auto &any: source_weight) {
            for (const auto &old_id: any.first) {
                alreadyFound[old_id] = true;
            }
        }
        for (const auto &thisId: new_id) {
            if (alreadyFound[thisId] == true) {
                std::cout << "ERROR: multisched station_weight: double use of ID " << thisId << std::endl;
                std::terminate();
            }
        }
        source_weight.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setBaseline_maxScan(const std::vector<std::pair<unsigned int, unsigned int> > &new_id,
                                              const std::vector<unsigned int> &values) {

        std::vector<std::vector<char> > alreadyFound(nsta, std::vector<char>(nsta, false));
        for (const auto &any: baseline_maxScan) {
            for (const auto &old_id: any.first) {
                alreadyFound[old_id.first][old_id.second] = true;
            }
        }
        for (const auto &thisId: new_id) {
            if (alreadyFound[thisId.first][thisId.second] == true) {
                std::cout << "ERROR: multisched station_weight: double use of ID " << thisId.first << "-"
                          << thisId.second << std::endl;
                std::terminate();
            }
        }
        baseline_maxScan.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setBaseline_minScan(const std::vector<std::pair<unsigned int, unsigned int> > &new_id,
                                              const std::vector<unsigned int> &values) {

        std::vector<std::vector<char> > alreadyFound(nsta, std::vector<char>(nsta, false));
        for (const auto &any: baseline_minScan) {
            for (const auto &old_id: any.first) {
                alreadyFound[old_id.first][old_id.second] = true;
            }
        }
        for (const auto &thisId: new_id) {
            if (alreadyFound[thisId.first][thisId.second] == true) {
                std::cout << "ERROR: multisched station_weight: double use of ID " << thisId.first << "-"
                          << thisId.second << std::endl;
                std::terminate();
            }
        }
        baseline_minScan.emplace_back(new_id, values);
    }

    void VLBI_multiSched::setBaseline_weight(const std::vector<std::pair<unsigned int, unsigned int> > &new_id,
                                             const std::vector<double> &values) {

        std::vector<std::vector<char> > alreadyFound(nsta, std::vector<char>(nsta, false));
        for (const auto &any: baseline_weight) {
            for (const auto &old_id: any.first) {
                alreadyFound[old_id.first][old_id.second] = true;
            }
        }
        for (const auto &thisId: new_id) {
            if (alreadyFound[thisId.first][thisId.second] == true) {
                std::cout << "ERROR: multisched station_weight: double use of ID " << thisId.first << "-"
                          << thisId.second << std::endl;
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
        if (multiSched_subnetting) {
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
        thisPARA.station_minScan.resize(nsta);
        thisPARA.station_weight.resize(nsta);
        thisPARA.station_maxScan.resize(nsta);
        thisPARA.station_maxWait.resize(nsta);
        thisPARA.station_maxSlewtime.resize(nsta);

        thisPARA.source_maxScan.resize(nsrc);
        thisPARA.source_minScan.resize(nsrc);
        thisPARA.source_minNumberOfStations.resize(nsrc);
        thisPARA.source_minRepeat.resize(nsrc);
        thisPARA.source_weight.resize(nsrc);
        thisPARA.source_minFlux.resize(nsrc);

        thisPARA.baseline_weight.resize(nsta);
        thisPARA.baseline_minScan.resize(nsta);
        thisPARA.baseline_maxScan.resize(nsta);
        for (int i = 0; i < nsta; ++i) {
            thisPARA.baseline_weight[i].resize(nsta);
            thisPARA.baseline_minScan[i].resize(nsta);
            thisPARA.baseline_maxScan[i].resize(nsta);
        }

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

        if (multiSched_subnetting) {
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
                const std::vector<unsigned int> &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        for (unsigned int thisId:ids) {
                            allPARA[c].station_maxSlewtime[thisId] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!station_maxWait.empty()) {
            for (const auto &any: station_maxWait) {
                const std::vector<unsigned int> &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        for (unsigned int thisId:ids) {
                            allPARA[c].station_maxWait[thisId] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!station_maxScan.empty()) {
            for (const auto &any: station_maxScan) {
                const std::vector<unsigned int> &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        for (unsigned int thisId:ids) {
                            allPARA[c].station_maxScan[thisId] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!station_minScan.empty()) {
            for (const auto &any: station_minScan) {
                const std::vector<unsigned int> &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        for (unsigned int thisId:ids) {
                            allPARA[c].station_minScan[thisId] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!station_weight.empty()) {
            for (const auto &any: station_weight) {
                const std::vector<unsigned int> &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        for (unsigned int thisId:ids) {
                            allPARA[c].station_weight[thisId] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }


        if (!source_minNumberOfStations.empty()) {
            for (const auto &any: source_minNumberOfStations) {
                const std::vector<unsigned int> &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        for (unsigned int thisId:ids) {
                            allPARA[c].source_minNumberOfStations[thisId] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!source_minFlux.empty()) {
            for (const auto &any: source_minFlux) {
                const std::vector<unsigned int> &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        for (unsigned int thisId:ids) {
                            allPARA[c].source_minFlux[thisId] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!source_minRepeat.empty()) {
            for (const auto &any: source_minRepeat) {
                const std::vector<unsigned int> &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        for (unsigned int thisId:ids) {
                            allPARA[c].source_minRepeat[thisId] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!source_maxScan.empty()) {
            for (const auto &any: source_maxScan) {
                const std::vector<unsigned int> &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        for (unsigned int thisId:ids) {
                            allPARA[c].source_maxScan[thisId] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!source_minScan.empty()) {
            for (const auto &any: source_minScan) {
                const std::vector<unsigned int> &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        for (unsigned int thisId:ids) {
                            allPARA[c].source_minScan[thisId] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!source_weight.empty()) {
            for (const auto &any: source_weight) {
                const std::vector<unsigned int> &ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        for (unsigned int thisId:ids) {
                            allPARA[c].source_weight[thisId] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }

        if (!baseline_maxScan.empty()) {
            for (const auto &any: baseline_maxScan) {
                std::vector<std::pair<unsigned int, unsigned int>> ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        for (auto thisId:ids) {
                            allPARA[c].baseline_maxScan[thisId.first][thisId.second] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!baseline_minScan.empty()) {
            for (const auto &any: baseline_minScan) {
                std::vector<std::pair<unsigned int, unsigned int>> ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        for (auto thisId:ids) {
                            allPARA[c].baseline_minScan[thisId.first][thisId.second] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }
        if (!baseline_weight.empty()) {
            for (const auto &any: baseline_weight) {
                std::vector<std::pair<unsigned int, unsigned int>> ids = any.first;
                const auto &values = any.second;

                unsigned long n_this = values.size();
                unsigned long n_block = n_before * n_this;
                unsigned int n_items = n_total / n_block;
                unsigned int c = 0;
                for (int i_block = 0; i_block < n_block; ++i_block) {
                    auto thisValue = values[i_block % n_this];

                    for (int i_item = 0; i_item < n_items; ++i_item) {
                        for (auto thisId:ids) {
                            allPARA[c].baseline_weight[thisId.first][thisId.second] = thisValue;
                        }
                        ++c;
                    }
                }
                n_before = n_block;
            }
        }


        return allPARA;
    }


}