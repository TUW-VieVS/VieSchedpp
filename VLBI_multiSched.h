//
// Created by matth on 03.09.2017.
//

#ifndef VLBI_MULTISCHED_H
#define VLBI_MULTISCHED_H

#include <boost/date_time.hpp>
#include <vector>
#include <boost/optional.hpp>

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

            std::vector<boost::optional<unsigned int> > station_maxSlewtime;
            std::vector<boost::optional<unsigned int> > station_maxWait;
            std::vector<boost::optional<unsigned int> > station_maxScan;
            std::vector<boost::optional<unsigned int> > station_minScan;
            std::vector<boost::optional<double> > station_weight;

            std::vector<boost::optional<unsigned int> > source_minNumberOfStations;
            std::vector<boost::optional<double> > source_minFlux;
            std::vector<boost::optional<unsigned int> > source_minRepeat;
            std::vector<boost::optional<unsigned int> > source_maxScan;
            std::vector<boost::optional<unsigned int> > source_minScan;
            std::vector<boost::optional<double> > source_weight;

            std::vector<std::vector<boost::optional<unsigned int> > > baseline_maxScan;
            std::vector<std::vector<boost::optional<unsigned int> > > baseline_minScan;
            std::vector<std::vector<boost::optional<double> > > baseline_weight;

            void output(std::ofstream &of) const {
                unsigned long nsta = station_weight.size();

                if (start.is_initialized()) {
                    of << "start: " << *start << std::endl;
                }
                if (multiSched_subnetting.is_initialized()) {
                    of << "multiSched_subnetting: " << *multiSched_subnetting << std::endl;
                }
                if (multisched_fillinmode.is_initialized()) {
                    of << "multisched_fillinmode: " << *multisched_fillinmode << std::endl;
                }

                if (weight_skyCoverage.is_initialized()) {
                    of << "weight_skyCoverage: " << *weight_skyCoverage << std::endl;
                }
                if (weight_numberOfObservations.is_initialized()) {
                    of << "weight_numberOfObservations: " << *weight_numberOfObservations << std::endl;
                }
                if (weight_duration.is_initialized()) {
                    of << "weight_duration: " << *weight_duration << std::endl;
                }
                if (weight_averageSources.is_initialized()) {
                    of << "weight_averageSources: " << *weight_averageSources << std::endl;
                }
                if (weight_averageStations.is_initialized()) {
                    of << "weight_averageStations: " << *weight_averageStations << std::endl;
                }

                int c;
                c = 0;
                for (const auto &any: station_maxSlewtime) {
                    if (any.is_initialized()) {
                        of << "station_maxSlewtime (id=" << c << "): " << *any << std::endl;
                    }
                    ++c;
                }
                c = 0;
                for (const auto &any: station_maxWait) {
                    if (any.is_initialized()) {
                        of << "station_maxWait (id=" << c << "): " << *any << std::endl;
                    }
                    ++c;
                }
                c = 0;
                for (const auto &any: station_maxScan) {
                    if (any.is_initialized()) {
                        of << "station_maxScan (id=" << c << "): " << *any << std::endl;
                    }
                    ++c;
                }
                c = 0;
                for (const auto &any: station_minScan) {
                    if (any.is_initialized()) {
                        of << "station_minScan (id=" << c << "): " << *any << std::endl;
                    }
                    ++c;
                }
                c = 0;
                for (const auto &any: station_weight) {
                    if (any.is_initialized()) {
                        of << "station_weight (id=" << c << "): " << *any << std::endl;
                    }
                    ++c;
                }

                c = 0;
                for (const auto &any: source_minNumberOfStations) {
                    if (any.is_initialized()) {
                        of << "source_minNumberOfStations (id=" << c << "): " << *any << std::endl;
                    }
                    ++c;
                }
                c = 0;
                for (const auto &any: source_minFlux) {
                    if (any.is_initialized()) {
                        of << "source_minFlux (id=" << c << "): " << *any << std::endl;
                    }
                    ++c;
                }
                c = 0;
                for (const auto &any: source_minRepeat) {
                    if (any.is_initialized()) {
                        of << "source_minRepeat (id=" << c << "): " << *any << std::endl;
                    }
                    ++c;
                }
                c = 0;
                for (const auto &any: source_maxScan) {
                    if (any.is_initialized()) {
                        of << "source_maxScan (id=" << c << "): " << *any << std::endl;
                    }
                    ++c;
                }
                c = 0;
                for (const auto &any: source_minScan) {
                    if (any.is_initialized()) {
                        of << "source_minScan (id=" << c << "): " << *any << std::endl;
                    }
                    ++c;
                }
                c = 0;
                for (const auto &any: source_weight) {
                    if (any.is_initialized()) {
                        of << "source_weight (id=" << c << "): " << *any << std::endl;
                    }
                    ++c;
                }

                for (int i = 0; i < nsta; ++i) {
                    for (int j = i + 1; j < nsta; ++j) {
                        if (baseline_maxScan[i][j].is_initialized()) {
                            of << "baseline_maxScan (id=" << i << "-" << j << "): " << *baseline_maxScan[i][j]
                               << std::endl;
                        }
                    }
                }
                for (int i = 0; i < nsta; ++i) {
                    for (int j = i + 1; j < nsta; ++j) {
                        if (baseline_minScan[i][j].is_initialized()) {
                            of << "baseline_minScan (id=" << i << "-" << j << "): " << *baseline_minScan[i][j]
                               << std::endl;
                        }
                    }
                }
                for (int i = 0; i < nsta; ++i) {
                    for (int j = i + 1; j < nsta; ++j) {
                        if (baseline_weight[i][j].is_initialized()) {
                            of << "baseline_weight (id=" << i << "-" << j << "): " << *baseline_weight[i][j]
                               << std::endl;
                        }
                    }
                }
            }
        };


        VLBI_multiSched(unsigned int nsta, unsigned int nsrc);

        void setStart(const std::vector<boost::posix_time::ptime> &start) {
            VLBI_multiSched::start = start;
        }

        void setMultiSched_subnetting(bool flag) {
            VLBI_multiSched::multiSched_subnetting = flag;
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

        void setStation_maxSlewtime(const std::vector<unsigned int> &new_id, const std::vector<unsigned int> &values);

        void setStation_maxWait(const std::vector<unsigned int> &new_id, const std::vector<unsigned int> &values);

        void setStation_maxScan(const std::vector<unsigned int> &new_id, const std::vector<unsigned int> &values);

        void setStation_minScan(const std::vector<unsigned int> &new_id, const std::vector<unsigned int> &values);

        void setStation_weight(const std::vector<unsigned int> &new_id, const std::vector<double> &values);

        void
        setSource_minNumberOfStations(const std::vector<unsigned int> &new_id, const std::vector<unsigned int> &values);

        void setSource_minFlux(const std::vector<unsigned int> &new_id, const std::vector<double> &values);

        void setSource_minRepeat(const std::vector<unsigned int> &new_id, const std::vector<unsigned int> &values);

        void setSource_maxScan(const std::vector<unsigned int> &new_id, const std::vector<unsigned int> &values);

        void setSource_minScan(const std::vector<unsigned int> &new_id, const std::vector<unsigned int> &values);

        void setSource_weight(const std::vector<unsigned int> &new_id, const std::vector<double> &values);

        void setBaseline_maxScan(const std::vector<std::pair<unsigned int, unsigned int> > &new_id,
                                 const std::vector<unsigned int> &values);

        void setBaseline_minScan(const std::vector<std::pair<unsigned int, unsigned int> > &new_id,
                                 const std::vector<unsigned int> &values);

        void setBaseline_weight(const std::vector<std::pair<unsigned int, unsigned int> > &new_id,
                                const std::vector<double> &values);

        std::vector<PARAMETERS> createMultiScheduleParameters();

    private:
        unsigned int nsta;
        unsigned int nsrc;

        std::vector<boost::posix_time::ptime> start;
        bool multiSched_subnetting = false;
        bool multisched_fillinmode = false;


        std::vector<double> weight_skyCoverage;
        std::vector<double> weight_numberOfObservations;
        std::vector<double> weight_duration;
        std::vector<double> weight_averageSources;
        std::vector<double> weight_averageStations;


        std::vector<std::pair<std::vector<unsigned int>, std::vector<unsigned int> > > station_maxSlewtime;
        std::vector<std::pair<std::vector<unsigned int>, std::vector<unsigned int> > > station_maxWait;
        std::vector<std::pair<std::vector<unsigned int>, std::vector<unsigned int> > > station_maxScan;
        std::vector<std::pair<std::vector<unsigned int>, std::vector<unsigned int> > > station_minScan;
        std::vector<std::pair<std::vector<unsigned int>, std::vector<double> > > station_weight;


        std::vector<std::pair<std::vector<unsigned int>, std::vector<unsigned int> > > source_minNumberOfStations;
        std::vector<std::pair<std::vector<unsigned int>, std::vector<double> > > source_minFlux;
        std::vector<std::pair<std::vector<unsigned int>, std::vector<unsigned int> > > source_minRepeat;
        std::vector<std::pair<std::vector<unsigned int>, std::vector<unsigned int> > > source_maxScan;
        std::vector<std::pair<std::vector<unsigned int>, std::vector<unsigned int> > > source_minScan;
        std::vector<std::pair<std::vector<unsigned int>, std::vector<double> > > source_weight;


        std::vector<std::pair<std::vector<std::pair<unsigned int, unsigned int> >, std::vector<unsigned int> > > baseline_maxScan;
        std::vector<std::pair<std::vector<std::pair<unsigned int, unsigned int> >, std::vector<unsigned int> > > baseline_minScan;
        std::vector<std::pair<std::vector<std::pair<unsigned int, unsigned int> >, std::vector<double> > > baseline_weight;

    };
}


#endif //VLBI_MULTISCHED_H
