/**
 * @file Subcon.h
 * @brief class Subcon
 *
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */

#ifndef SUBCON_H
#define SUBCON_H
#include <vector>
#include <utility>
#include <limits>
#include <queue>
#include <boost/optional.hpp>
#include <numeric>

#include "Station.h"
#include "Source.h"
#include "Scan.h"
#include "SkyCoverage.h"



namespace VieVS{
    /**
     * @class Subcon
     * @brief representation of a VLBI subcon
     *
     * @author Matthias Schartner
     * @date 29.06.2017
     */
    class Subcon {
    public:
        /**
         * @brief empty default constructor
         */
        Subcon();

        /**
         * @brief default copy constructor
         *
         * @param other other subcon
         */
        Subcon(const Subcon &other) = default;

        /**
         * @brief default move constructor
         *
         * @param other other subcon
         */
        Subcon(Subcon &&other) = default;

        /**
         * @brief default copy assignment operator
         *
         * @param other other subcon
         * @return copy of other subcon
         */
        Subcon &operator=(const Subcon &other) = default;

        /**
         * @brief default move assignment operator
         *
         * @param other other subcon
         * @return moved other subcon
         */
        Subcon &operator=(Subcon &&other) = default;

        /**
         * @brief destructor
         */
        virtual ~Subcon() {};

        /**
         * @brief add a single source scan to subcon
         *
         * @param scan scan which should be added
         */
        void addScan(const Scan &scan) noexcept;

        /**
         * @brief removes a scan from the subcon
         *
         * The index counts first through all single source scans and continues with all subnetting scans. If the index
         * is larger than the number of single scans both subnetting scans will be removed.
         *
         * @param idx index of scan which should be removed
         */
        void removeScan(unsigned long idx) noexcept;

        /**
         * @brief getter for number of possible single source scans
         *
         * @return number of possible single source scans
         */
        unsigned long getNumberSingleScans() const noexcept {
            return nSingleScans_;
        }

        /**
         * @brief getter for number of possible subnetting scans
         *
         * @return number of possible subnetting scans
         */
        unsigned long getNumberSubnettingScans() const noexcept {
            return nSubnettingScans_;
        }

        /**
         * @brief getter for a single source scan
         *
         * @param idx index
         * @return single source scan at this index
         */
        Scan &referenceSingleSourceScan(unsigned long idx) noexcept {
            return singleScans_[idx];
        }

        /**
         * @brief getter for subnettin scan
         *
         * @param idx index
         * @return subnetting scan at this index
         */
        std::pair<Scan, Scan> &referenceDoubleSourceScan(unsigned long idx) noexcept {
            return subnettingScans_[idx];
        }

        /**
         * @brief calculates the earliest possible start time for all single source scans in this subcon
         *
         * @param stations list of all stations
         * @param sources list of all sources
         */
        void calcStartTimes(const std::vector<Station> &stations, const std::vector<Source> &sources) noexcept;

        /**
         * @brief constructs all baselines for all single source scans in this subcon
         *
         * @param sources list of all sources
         */
        void constructAllBaselines(const std::vector<Source> &sources) noexcept;

        /**
         * @brief updates all azimuths and elevations of all pointing vectors for each single source scan in this subcon
         *
         * @param stations list of all stations
         * @param sources list of all sources
         */
        void updateAzEl(const std::vector<Station> &stations, const std::vector<Source> &sources) noexcept;

        /**
         * @brief calculates all baseline scan duration for all single source scans in this subcon
         *
         * @param stations list of all stations
         * @param sources list of all sources
         */
        void
        calcAllBaselineDurations(const std::vector<Station> &stations, const std::vector<Source> &sources) noexcept;

        /**
         * @brief calculates all scan duration of all single source scans in this subcon
         *
         * @param stations list of all stations
         * @param sources list of all sources
         */
        void calcAllScanDurations(const std::vector<Station> &stations, const std::vector<Source> &sources) noexcept;

        /**
         * @brief create all subnetting scans from possible single source scans
         *
         * @param subnettingSrcIds ids between all sources which could be used for subnetting
         * @param minStaPerSubcon  minimum number of stations per subconfiguration
         */
        void createSubcon2(const std::vector<std::vector<int> > &subnettingSrcIds, int minStaPerSubcon) noexcept;

        /**
         * @brief generate scores for all single source and subnetting scans
         *
         * @param stations list of all stations
         * @param skyCoverages list of all sky coverages
         * @param nsrc number of sources
         */
        void generateScore(const std::vector<Station> &stations,
                           const std::vector<SkyCoverage> &skyCoverages, unsigned long nsrc) noexcept;

        /**
         * @brief precalculate all necessary parameters to generate scores
         *
         * @param stations list of all stations
         * @param sources list of all sources
         */
        void precalcScore(const std::vector<Station> &stations, const std::vector<Source> &sources) noexcept;

        /**
         * @brief get minimum and maximum time required for a possible scan
         */
        void minMaxTime() noexcept;

        /**
         * @brief calculate the score for averaging out each station
         *
         * @param stations list of all stations
         */
        void average_station_score(const std::vector<Station> &stations) noexcept;

        /**
         * @brief calculate the score for averaging out each source
         * @param sources list of all sources
         */
        void average_source_score(const std::vector<Source> &sources) noexcept;

        /**
         * @brief rigorousely updates the best scans untill the best one is found
         *
         * @param stations list of all stations
         * @param sources list of all sources
         * @param skyCoverages list of all sky coverages
         * @return index of best scan
         */
<<<<<<< HEAD:VLBI_subcon.h
        boost::optional<unsigned long> rigorousScore(const vector<VLBI_station> &stations,
                                                     const vector<VLBI_source> &sources,
                                                     const vector<VLBI_skyCoverage> &skyCoverages) noexcept;

        void clearSubnettingScans();

    private:
        unsigned long n1scans; ///< number of single source scans
        vector<VLBI_scan> subnet1; ///< all single source scans
        vector<double> subnet1_score; ///< score for each single source scan
=======
        boost::optional<unsigned long> rigorousScore(const std::vector<Station> &stations,
                                                     const std::vector<Source> &sources,
                                                     const std::vector<SkyCoverage> &skyCoverages) noexcept;
>>>>>>> sandbox:Subcon.h

        /**
         * @brief clear all subnetting scans
         *
         * Usually unused
         */
        void clearSubnettingScans();

    private:
        unsigned long nSingleScans_; ///< number of single source scans
        std::vector<Scan> singleScans_; ///< all single source scans
        std::vector<double> singleScanScores_; ///< score for each single source scan

        unsigned long nSubnettingScans_; ///< number of subnetting scans
        std::vector<std::pair<Scan, Scan> > subnettingScans_; ///< all subnetting scans
        std::vector<double> subnettingScanScores_; ///< score for each subnetting scan

        unsigned long nMaxBaselines_; ///< maximum possible number of baselines
        unsigned int minRequiredTime_; ///< minimum time required for a scan
        unsigned int maxRequiredTime_; ///< maximum time required for a scan
        std::vector<double> astas_; ///< average station score for each station
        std::vector<double> asrcs_; ///< average source score for each source
    };
}
#endif /* SUBCON_H */

