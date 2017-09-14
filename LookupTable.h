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
    public:
        static std::unordered_map<int, double> sinLookup; ///< table for fast lookup for sine function with reduced accuracy. Key is fraction of .001 pi
        static std::unordered_map<int, double> cosLookup; ///< table for fast lookup for cosine function with reuced accuracy. Key is fraction of .001 pi
        static std::unordered_map<int, double> acosLookup; ///< table for fast lookup for inverse cosine function with reduced accuracy. Key is fraction of .001 pi
    };
}

#endif //LOOKUPTABLE_H
