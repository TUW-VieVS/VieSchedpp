/*
 *  VieSched++ Very Long Baseline Interferometry (VLBI) Scheduling Software
 *  Copyright (C) 2018  Matthias Schartner
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file SkyCoverage.h
 * @brief class SkyCoverage
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */

#ifndef SKYCOVERAGE_H
#define SKYCOVERAGE_H
#include <cmath>
#include <iostream>
#include <limits>
#include <set>
#include <vector>

#include "../Misc/LookupTable.h"
#include "../Scan/PointingVector.h"
#include "Station.h"

namespace VieVS {
/**
 * @class SkyCoverage
 * @brief representation of the VLBI sky coverage object
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */
class SkyCoverage : public VieVS_Object {
   public:
    /**
     * @brief sky coverage functions
     * @author Matthias Schartner
     */
    enum class Interpolation {
        constant,  ///< constant function
        linear,    ///< linear function
        cosine,    ///< cosine function
    };

    static thread_local double maxInfluenceTime;      ///< maximum angular distance of influence on the sky coverage
    static thread_local double maxInfluenceDistance;  ///< maximum time influence on the sky coverage
    static thread_local Interpolation interpolationDistance;  ///< function for distance
    static thread_local Interpolation interpolationTime;      ///< function for time

    /**
     * @brief constructor
     * @author Matthias Schartner
     */
    SkyCoverage();

    /**
     * @brief calc score per station
     * @author Matthias Schartner
     *
     * @param pv pointing vector
     * @return score
     */
    double calcScore( const PointingVector &pv ) const;

    /**
     * @brief calculates the influence of the score between two pointing vectors
     * @author Matthias Schartner
     *
     * @param pv_new new observation pointing vector
     * @param pv_old already observed pointing vector
     * @return score
     */
    double scorePerPointingVector( const PointingVector &pv_new, const PointingVector &pv_old ) const noexcept;

    /**
     * @brief clear all observations
     * @author Matthias Schartner
     */
    void clearObservations();

    /**
     * @brief updates the pointing vectors
     * @author Matthias Schartner
     *
     * @param pv pointing vector at start of scan
     */
    void update( const PointingVector &pv ) noexcept;

    /**
     * @brief get all pointing vectors
     * @author Matthias Schartner
     *
     * @return all pointing vectors
     */
    const std::vector<PointingVector> &getPointingVectors() const noexcept { return pointingVectors_; }

    /**
     * @brief calculate sky coverage scores
     * @author Matthias Schartner
     *
     * with 13, 25 and 37 areas over 30 and 60 minutes.
     */
    void calculateSkyCoverageScores();

    /**
     * @brief get total sky coverage score
     * @author Matthias Schartner
     *
     * @return sky coverage score with 13 areas over 30 minutes
     */
    double getSkyCoverageScore_a13m30() const { return a13m30_; }

    /**
     * @brief get total sky coverage score
     * @author Matthias Schartner
     *
     * @return sky coverage score with 25 areas over 30 minutes
     */
    double getSkyCoverageScore_a25m30() const { return a25m30_; }

    /**
     * @brief get total sky coverage score
     * @author Matthias Schartner
     *
     * @return sky coverage score with 37 areas over 30 minutes
     */
    double getSkyCoverageScore_a37m30() const { return a37m30_; }

    /**
     * @brief get total sky coverage score
     * @author Matthias Schartner
     *
     * @return sky coverage score with 13 areas over 60 minutes
     */
    double getSkyCoverageScore_a13m60() const { return a13m60_; }

    /**
     * @brief get total sky coverage score
     * @author Matthias Schartner
     *
     * @return sky coverage score with 25 areas over 60 minutes
     */
    double getSkyCoverageScore_a25m60() const { return a25m60_; }

    /**
     * @brief get total sky coverage score
     * @author Matthias Schartner
     *
     * @return sky coverage score with 37 areas over 60 minutes
     */
    double getSkyCoverageScore_a37m60() const { return a37m60_; }

   private:
    static unsigned long nextId;  ///< next id for this object type

    std::vector<PointingVector> pointingVectors_;  ///< all pointing vectors
    double a13m30_{0};                             ///< sky coverage score with 13 areas over 30 minutes
    double a25m30_{0};                             ///< sky coverage score with 25 areas over 30 minutes
    double a37m30_{0};                             ///< sky coverage score with 37 areas over 30 minutes
    double a13m60_{0};                             ///< sky coverage score with 13 areas over 60 minutes
    double a25m60_{0};                             ///< sky coverage score with 25 areas over 60 minutes
    double a37m60_{0};                             ///< sky coverage score with 37 areas over 60 minutes

    /**
     * @brief calculate total sky coverage score of all observations over schedule session
     * @author Matthias Schartner
     *
     * The sky is distributed in 13 areas
     *
     * @param deltaTime time increment
     * @return sky coverage score. Maximum score is 1, minimum score is 0.
     */
    double skyCoverageScore_13( unsigned int deltaTime ) const;

    /**
     * @brief calculate total sky coverage score of all observations over schedule session
     * @author Matthias Schartner
     *
     * The sky is distributed in 25 areas
     *
     * @param deltaTime time increment
     * @return sky coverage score. Maximum score is 1, minimum score is 0.
     */
    double skyCoverageScore_25( unsigned int deltaTime ) const;

    /**
     * @brief calculate total sky coverage score of all observations over schedule session
     * @author Matthias Schartner
     *
     * The sky is distributed in 37 areas
     *
     * @param deltaTime time increment
     * @return sky coverage score. Maximum score is 1, minimum score is 0.
     */
    double skyCoverageScore_37( unsigned int deltaTime ) const;

    /**
     * @brief area index of observation
     * @author Matthias Schartner
     *
     * Sky is distributed in 13 areas.
     * An index is given to each area.
     * This function returns the index of the area where the observation is located.
     *
     * @param pv pointing vector containing azimuth and elevation
     * @return area index
     */
    static int areaIndex13_v1( const PointingVector &pv ) noexcept;

    /**
     * @brief area index of observation
     * @author Matthias Schartner
     *
     * Sky is distributed in 13 areas.
     * An index is given to each area.
     * This function returns the index of the area where the observation is located.
     *
     * @param pv pointing vector containing azimuth and elevation
     * @return area index
     */
    static int areaIndex13_v2( const PointingVector &pv ) noexcept;

    /**
     * @brief area index of observation
     * @author Matthias Schartner
     *
     * Sky is distributed in 25 areas.
     * An index is given to each area.
     * This function returns the index of the area where the observation is located.
     *
     * @param pv pointing vector containing azimuth and elevation
     * @return area index
     */
    static int areaIndex25_v1( const PointingVector &pv ) noexcept;

    /**
     * @brief area index of observation
     * @author Matthias Schartner
     *
     * Sky is distributed in 25 areas.
     * An index is given to each area.
     * This function returns the index of the area where the observation is located.
     *
     * @param pv pointing vector containing azimuth and elevation
     * @return area index
     */
    static int areaIndex25_v2( const PointingVector &pv ) noexcept;

    /**
     * @brief area index of observation
     * @author Matthias Schartner
     *
     * Sky is distributed in 37 areas.
     * An index is given to each area.
     * This function returns the index of the area where the observation is located.
     *
     * @param pv pointing vector containing azimuth and elevation
     * @return area index
     */
    static int areaIndex37_v1( const PointingVector &pv ) noexcept;

    /**
     * @brief area index of observation
     * @author Matthias Schartner
     *
     * Sky is distributed in 37 areas.
     * An index is given to each area.
     * This function returns the index of the area where the observation is located.
     *
     * @param pv pointing vector containing azimuth and elevation
     * @return area index
     */
    static int areaIndex37_v2( const PointingVector &pv ) noexcept;
};
}  // namespace VieVS

#endif /* SKYCOVERAGE_H */
