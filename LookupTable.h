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
