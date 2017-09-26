/**
 * @file SkyCoverage.h
 * @brief class SkyCoverage
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

#include "PointingVector.h"
#include "LookupTable.h"
#include "Station.h"

namespace VieVS{
    /**
     * @class SkyCoverage
     * @brief representation of the VLBI sky coverage object
     *
     * @author Matthias Schartner
     * @date 29.06.2017
     */
    class SkyCoverage {
    public:
        static std::vector<std::vector<std::vector<float> > > angularDistanceLookup; ///< lookup table for angular distance between two points
        static double maxInfluenceTime; ///< maximum angular distance of influence on the sky coverage
        static double maxInfluenceDistance; ///< maximum time influence on the sky coverage
        static double maxTwinTelecopeDistance; ///< maximum distance between corresponding telescopes

        /**
         * @brief empty default constructor
         */
        SkyCoverage();

        /**
         * @brief constructor
         *
         * @param staids station ids which belong to this sky coverage
         * @param skyCoverageDistance maximum angular distance of influence of an scan in radians
         * @param skyCoverageInterval maximum influence time of a scan in seconds
         * @param id sky coverage id
         */
        SkyCoverage(const std::vector<int> &staids, int id);

        /**
         * @brief default copy constructor
         *
         * @param other other sky coverage
         */
        SkyCoverage(const SkyCoverage &other) = default;

        /**
         * @brief default move constructor
         *
         * @param other other sky coverage
         */
        SkyCoverage(SkyCoverage &&other) = default;

        /**
         * @brief default copy assignment operator
         *
         * @param other other sky coverage
         * @return copy of other sky coverage
         */
        SkyCoverage &operator=(const SkyCoverage &other) = default;

        /**
         * @brief default move assignment operator
         *
         * @param other other sky coverage
         * @return moved other sky coverage
         */
        SkyCoverage &operator=(SkyCoverage &&other) = default;

        /**
         * @brief destructor
         */
        virtual ~SkyCoverage();


        /**
         * @brief getter for all station ids which belong to this sky coverage
         *
         * @return all station ids
         */
        const std::vector<int> &getStaids() const noexcept {
            return staids_;
        }

        /**
         * @brief calculates the score of pointing vectors on the sky Coverage
         *
         * @param pvs pointing vectors
         * @param stations list of all stations
         * @return score
         */
        double calcScore(const std::vector<PointingVector> &pvs, const std::vector<Station> &stations) const noexcept;

        /**
         * @brief calculates the score of pointing vectors on the sky Coverage
         *
         * !!! This function changes firstScorePerPv !!!
         *
         * @param pvs pointing vectors
         * @param stations list of all stations
         * @param firstScorePerPv stores the score of each pointing vector without twin station influences
         * @return score
         */
        double calcScore(const std::vector<PointingVector> &pvs, const std::vector<Station> &stations,
                         std::vector<double> &firstScorePerPv) const noexcept;

        /**
         * @brief calculates the score of pointing vectors on the sky Coverage
         *
         * @param pvs pointing vectors
         * @param stations list of all stations
         * @param firstScorePerPv stored score for each pointing vector without twin station influences
         * @return score
         */
        double calcScore_subcon(const std::vector<PointingVector> &pvs,
                                const std::vector<Station> &stations,
                                const std::vector<double> &firstScorePerPv) const noexcept;

        /**
         * @brief calculates the influence of the score between two pointing vectors
         *
         * @param pv_new new observation pointing vector
         * @param pv_old already observed pointing vector
         * @return score
         */
        double
        scorePerPointingVector(const PointingVector &pv_new, const PointingVector &pv_old) const noexcept;

        /**
         * @brief updates the pointing vectors
         *
         * @param start pointing vector at start of scan
         * @param end pointing vector at end of scan
         */
        void update(const PointingVector &start, const PointingVector &end) noexcept;

    private:
        unsigned long nStations_; ///< number of stations that belong to this sky coverage
        std::vector<int> staids_; ///< ids of the stations that belong to this sky coverage

        std::vector<PointingVector> pointingVectorsStart_; ///< all pointing vectors at start of a scan
        std::vector<PointingVector> pointingVectorsEnd_; ///< all pointing vectors at end of a scan

        int id_; ///< sky coverage id
    };
}

#endif /* SKYCOVERAGE_H */

