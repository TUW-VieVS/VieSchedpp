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
 * @author Matthias Schartner
 * @date 02.08.2017
 */

#ifndef LOOKUPTABLE_H
#define LOOKUPTABLE_H


#include <unordered_map>
#include <vector>

#include "Constants.h"

#include "../Scan/PointingVector.h"


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
    static std::vector<std::vector<std::vector<float>>>
        angularDistanceLookup;  ///< lookup table for angular distance between two points
    static std::vector<double>
        sinLookupTable;  ///< table for fast lookup for sine function with reduced accuracy. Key is fraction of .001 pi
    static std::vector<double>
        cosLookupTable;  ///< table for fast lookup for cosine function with reuced accuracy. Key is fraction of .001 pi
    static std::vector<double> acosLookupTable;  ///< table for fast lookup for inverse cosine function with reduced
                                                 ///< accuracy. Key is fraction of .001 pi

   public:
    /**
     * @brief initialize lookup tables
     * @author Matthias Schartner
     */
    static void initialize();


    /**
     * @brief sinus lookup table
     * @author Matthias Schartner
     *
     * @param x argument in radians
     * @return sinus of argument
     */
    static double sinLookup( double x );


    /**
     * @brief cosine lookup table
     * @author Matthias Schartner
     *
     * @param x argument in radians
     * @return cosine of argument
     */
    static double cosLookup( double x );


    /**
     * @brief arcus cosine lookup table
     * @author Matthias Schartner
     *
     * @param x argument in radians
     * @return arcus cosine of argument
     */
    static double acosLookup( double x );


    /**
     * @brief angular distance between two points
     * @author Matthias Schartner
     *
     * @param p1 start pointing vector
     * @param p2 end pointing vector
     * @return angular distance between two points in radians
     */
    static float angularDistance( const PointingVector &p1, const PointingVector &p2 ) noexcept;


    /**
     * @brief angular distance between two points
     * @author Matthias Schartner
     *
     * @param phi1 start phi in radians
     * @param theta1 start theta in radians
     * @param phi2 end phi in radians
     * @param theta2 end thatet in radians
     * @return angular distance between two points in radians
     */
    static float angularDistance( double phi1, double theta1, double phi2, double theta2 ) noexcept;
};
}  // namespace VieVS

#endif  // LOOKUPTABLE_H
