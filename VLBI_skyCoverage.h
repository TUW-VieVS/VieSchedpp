/**
 * @file VLBI_skyCoverage.h
 * @brief class VLBI_skyCoverage
 *
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */

#ifndef SKYCOVERAGE_H
#define SKYCOVERAGE_H
#include <vector>
#include <iostream>
#include <cmath>
#include <limits>

#include "VLBI_pointingVector.h"
#include "VieVS_lookup.h"
#include "VLBI_station.h"

using namespace std;
namespace VieVS{
    class VLBI_skyCoverage {
    public:
        static vector<vector<vector<float> > > angularDistanceLookup; ///< lookup table for angular distance between two points

        /**
         * @brief empty default constructor
         */
        VLBI_skyCoverage();

        /**
         * @brief constructor
         *
         * @param staids station ids which belong to this sky coverage
         * @param skyCoverageDistance maximum angular distance of influence of an scan in radians
         * @param skyCoverageInterval maximum influence time of a scan in seconds
         * @param id sky coverage id
         */
        VLBI_skyCoverage(const vector<int> &staids, double skyCoverageDistance, double skyCoverageInterval, int id);

        /**
         * @brief getter for all station ids which belong to this sky coverage
         *
         * @return all station ids
         */
        const vector<int> &getStaids() const {
            return staids;
        }

        /**
         * @brief calculates the score of pointing vectors on the sky Coverage
         *
         * @param pvs pointing vectors
         * @return score
         */
        double calcScore(const vector<VLBI_pointingVector> &pvs, const vector<VLBI_station> &stations) const;

        /**
         * @brief calculates the score of pointing vectors on the sky Coverage
         *
         * !!! This function changes firstScorePerPv !!!
         *
         * @param pvs pointing vectors
         * @param firstScorePerPv stores the score of each pointing vector without twin station influences
         * @return score
         */
        double calcScore(const vector<VLBI_pointingVector> &pvs, const vector<VLBI_station> &stations,
                         vector<double> &firstScorePerPv) const;

        /**
         * @brief calculates the score of pointing vectors on the sky Coverage
         *
         * @param pvs pointing vectors
         * @param firstScorePerPv stored score for each pointing vector without twin station influences
         * @return score
         */
        double calcScore_subcon(const vector<VLBI_pointingVector> &pvs,
                                const vector<VLBI_station> &stations,
                                const vector<double> &firstScorePerPv) const;

        /**
         * @brief calculates the influence of the score between two pointing vectors
         *
         * @param pv_new new observation pointing vector
         * @param pv_old already observed pointing vector
         * @return score
         */
        double scorePerPointingVector(const VLBI_pointingVector &pv_new, const VLBI_pointingVector &pv_old) const;

        /**
         * @brief updates the pointing vectors
         *
         * @param start pointing vector at start of scan
         * @param end pointing vector at end of scan
         */
        void update(const VLBI_pointingVector &start, const VLBI_pointingVector &end);

        /**
         * @brief destructor
         */
        virtual ~VLBI_skyCoverage();

    private:
        unsigned long nStations; ///< number of stations that belong to this sky coverage
        vector<int> staids; ///< ids of the stations that belong to this sky coverage

        vector<VLBI_pointingVector> pv_start; ///< all pointing vectors at start of a scan
        vector<VLBI_pointingVector> pv_end; ///< all pointing vectors at end of a scan

        double maxDistTime; ///< maximum angular distance of influence on the sky coverage
        double maxDistDistance; ///< maximum time influence on the sky coverage

        int id; ///< sky coverage id
    };
}

#endif /* SKYCOVERAGE_H */

