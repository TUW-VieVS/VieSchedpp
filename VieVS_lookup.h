/**
 * @file VieVS_lookup.h
 * @brief class VieVS_lookup
 *
 *
 * @author Matthias Schartner
 * @date 02.08.2017
 */

#ifndef VIEVS_LOOKUP_H
#define VIEVS_LOOKUP_H

#include <unordered_map>

namespace VieVS {
    class VieVS_lookup {
    public:
        static std::unordered_map<int, double> sinLookup; ///< table for fast lookup for sine function with reduced accuracy. Key is fraction of .001 pi
        static std::unordered_map<int, double> cosLookup; ///< table for fast lookup for cosine function with reuced accuracy. Key is fraction of .001 pi
        static std::unordered_map<int, double> acosLookup; ///< table for fast lookup for inverse cosine function with reduced accuracy. Key is fraction of .001 pi
    };
}

#endif //VIEVS_LOOKUP_H
