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
#include <vector>
#include <iostream>
#include <cmath>
#include <limits>

#include "../Scan/PointingVector.h"
#include "../Misc/LookupTable.h"
#include "Station.h"

namespace VieVS{
    /**
     * @class SkyCoverage
     * @brief representation of the VLBI sky coverage object
     *
     * @author Matthias Schartner
     * @date 29.06.2017
     */
    class SkyCoverage:public VieVS_Object {
    public:

        /**
         * @brief sky coverage functions
         * @author Matthias Schartner
         */
        enum class Interpolation {
            constant, ///< constant function
            linear, ///< linear function
            cosine, ///< cosine function
        };

        static thread_local double maxInfluenceTime; ///< maximum angular distance of influence on the sky coverage
        static thread_local double maxInfluenceDistance; ///< maximum time influence on the sky coverage
        static thread_local Interpolation interpolationDistance; ///< function for distance
        static thread_local Interpolation interpolationTime; ///< function for time

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
        double calcScore(const PointingVector &pv) const;

        /**
         * @brief calculates the influence of the score between two pointing vectors
         * @author Matthias Schartner
         *
         * @param pv_new new observation pointing vector
         * @param pv_old already observed pointing vector
         * @return score
         */
        double
        scorePerPointingVector(const PointingVector &pv_new, const PointingVector &pv_old) const noexcept;

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
        void update(const PointingVector &pv) noexcept;

        /**
         * @brief get all pointing vectors
         * @author Matthias Schartner
         *
         * @return all pointing vectors
         */
        const std::vector<PointingVector> & getPointingVectors() const noexcept {
            return pointingVectors_;
        }

    private:
        static unsigned long nextId; ///< next id for this object type

        std::vector<PointingVector> pointingVectors_; ///< all pointing vectors
    };
}

#endif /* SKYCOVERAGE_H */

