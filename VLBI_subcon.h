/**
 * @file VLBI_subcon.h
 * @brief class VLBI_subcon
 *
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */

#ifndef VLBI_SUBCON_H
#define VLBI_SUBCON_H
#include <vector>
#include <utility>
#include <limits>
#include <queue>
#include <boost/optional.hpp>

#include "VLBI_station.h"
#include "VLBI_source.h"
#include "VLBI_scan.h"
#include "VLBI_skyCoverage.h"


using namespace std;

namespace VieVS{
    class VLBI_subcon {
    public:
        /**
         * @brief empty default constructor
         */
        VLBI_subcon();

        /**
         * @brief destructor
         */
        virtual ~VLBI_subcon() {};

        /**
         * @brief add a single source scan to subcon
         *
         * @param scan scan which should be added
         */
        void addScan(VLBI_scan scan);

        /**
         * @brief removes a scan from the subcon
         *
         * The index counts first through all single source scans and continues with all subnetting scans. If the index
         * is larger than the number of single scans both subnetting scans will be removed.
         * @param idx index of scan which should be removed
         */
        void removeScan(unsigned long idx);

        /**
         * @brief getter for number of possible single source scans
         *
         * @return number of possible single source scans
         */
        unsigned long getNumberSingleScans() const {
            return n1scans;
        }

        /**
         * @brief getter for number of possible subnetting scans
         *
         * @return number of possible subnetting scans
         */
        unsigned long getNumberSubnettingScans() const {
            return n2scans;
        }

        /**
         * @brief getter for a single source scan
         *
         * @param idx index
         * @return single source scan at this index
         */
        VLBI_scan &getSingleSourceScan(unsigned long idx) {
            return *(subnet1.begin() + idx);
        }

        /**
         * @brief getter for subnettin scan
         *
         * @param idx index
         * @return subnetting scan at this index
         */
        pair<VLBI_scan, VLBI_scan> &getDoubleSourceScan(unsigned long idx) {
            return *(subnet2.begin() + idx);
        }

        /**
         * @brief calculates the earliest possible start time of all single source scans in this subcon
         *
         * @param stations list of all stations
         * @param sources list of all sources
         */
        void calcStartTimes(vector<VLBI_station> &stations, vector<VLBI_source> &sources);

        /**
         * @brief constructs all baselines for all single source scans in this subcon
         */
        void constructAllBaselines();

        /**
         * @brief updates all azimuths and elevations of all pointing vectors for each single source scan in this subcon
         *
         * @param stations list of all stations
         * @param sources list of all sources
         */
        void updateAzEl(vector<VLBI_station> &stations, vector<VLBI_source> &sources);

        /**
         * @brief calculates all baseline scan duration for all single source scans in this subcon
         *
         * @param stations list of all stations
         * @param sources list of all sources
         * @param mjdStart modified julian date of session start
         */
        void calcAllBaselineDurations(vector<VLBI_station> &stations, vector<VLBI_source> &sources, double mjdStart);

        /**
         * @brief calculates all scan duration of all single source scans in this subcon
         *
         * @param stations list of all stations
         * @param sources list of all sources
         */
        void calcAllScanDurations(vector<VLBI_station> &stations, vector<VLBI_source> &sources);

        /**
         * @brief create all subnetting scans from possible single source scans
         *
         * @param subnettingSrcIds ids between all sources which could be used for subnetting
         * @param minStaPerSubcon  minimum number of stations per subconfiguration
         */
        void createSubcon2(vector<vector<int> > &subnettingSrcIds, int minStaPerSubcon);

        /**
         * @brief generate scores for all single source and subnetting scans
         *
         * @param stations list of all stations
         * @param skyCoverages list of all sky coverages
         */
        void generateScore(vector<VLBI_station> &stations,
                           vector<VLBI_skyCoverage> &skyCoverages, unsigned long nsrc);

        /**
         * @brief precalculate all necessary parameters to generate scores
         *
         * @param stations list of all stations
         * @param sources list of all sources
         */
        void precalcScore(vector<VLBI_station> &stations, vector<VLBI_source> &sources);

        /**
         * @brief get minimum and maximum time required for a possible scan
         */
        void minMaxTime();

        /**
         * @brief calculate the score for averaging out each station
         *
         * @param stations list of all stations
         */
        void average_station_score(const vector<VLBI_station> &stations);

        /**
         * @brief calculate the score for averaging out each source
         * @param sources list of all sources
         */
        void average_source_score(vector<VLBI_source> &sources);

        /**
         * @brief rigorousely updates the best scans untill the best one is found
         *
         * @param stations list of all stations
         * @param sources list of all sources
         * @param skyCoverages list of all sky coverages
         * @param mjdStart modified julian date of session start
         * @return index of best scan
         */
        boost::optional<unsigned long> rigorousScore(vector<VLBI_station> &stations, vector<VLBI_source> &sources,
                                                     vector<VLBI_skyCoverage> &skyCoverages, double mjdStart);

    private:
        unsigned long n1scans; ///< number of single source scans
        vector<VLBI_scan> subnet1; ///< all single source scans
        vector<double> subnet1_score; ///< score for each single source scan

        unsigned long n2scans; ///< number of subnetting scans
        vector<pair<VLBI_scan, VLBI_scan> > subnet2; ///< all subnetting scans
        vector<double> subnet2_score; ///< score for each subnetting scan

        unsigned long nmaxbl; ///< maximum possible number of baselines
        unsigned int minTime; ///< minimum time required for a scan
        unsigned int maxTime; ///< maximum time required for a scan
        vector<double> astas; ///< average station score for each station
        vector<double> asrcs; ///< average source score for each source

    };
}
#endif /* VLBI_SUBCON_H */

