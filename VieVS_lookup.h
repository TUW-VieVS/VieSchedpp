//
// Created by mschartn on 02.08.17.
//

#ifndef VIEVS_LOOKUP_H
#define VIEVS_LOOKUP_H

#include <unordered_map>

namespace VieVS {
    class VieVS_lookup {
    public:
        static std::unordered_map<int, double> sinLookup;
        static std::unordered_map<int, double> cosLookup;
        static std::unordered_map<int, double> acosLookup;
    };
}

#endif //VIEVS_LOOKUP_H
