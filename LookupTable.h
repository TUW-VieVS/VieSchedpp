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
 * @file LookupTable.h
 * @brief class LookupTable
 *
 *
 * @author Matthias Schartner
 * @date 02.08.2017
 */

#ifndef LOOKUPTABLE_H
#define LOOKUPTABLE_H

#include <unordered_map>
#include <vector>

#include "Constants.h"

#include "PointingVector.h"

namespace VieVS {
    /**
     * @class LookupTable
     * @brief fast lookup tables with reduced accuracy
     *
     * the key is always an integer that counts in .001 intervals (using th sinLookup with a key of 327 would mean you
     * get sin(0.327) as a return value)
     *
     * @author Matthias Schartner
     * @date 02.08.2017
     */
    class LookupTable {
    private:
        static std::vector<std::vector<std::vector<float> > > angularDistanceLookup; ///< lookup table for angular distance between two points
        static std::vector<double> sinLookupTable; ///< table for fast lookup for sine function with reduced accuracy. Key is fraction of .001 pi
        static std::vector<double> cosLookupTable; ///< table for fast lookup for cosine function with reuced accuracy. Key is fraction of .001 pi
        static std::vector<double> acosLookupTable; ///< table for fast lookup for inverse cosine function with reduced accuracy. Key is fraction of .001 pi

    public:
        static void initialize();

        static double sinLookup(double x);

        static double cosLookup(double x);

        static double acosLookup(double x);


        static float angularDistance(const PointingVector &p1, const PointingVector &p2) noexcept;

        static float angularDistance(double phi1, double theta1, double phi2, double theta2) noexcept;
    };
}

#endif //LOOKUPTABLE_H
