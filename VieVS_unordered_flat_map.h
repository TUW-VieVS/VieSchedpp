//
// Created by mschartn on 01.08.17.
//

#ifndef VIEVS_UNORDERED_FLAT_MAP_H
#define VIEVS_UNORDERED_FLAT_MAP_H

#include <vector>
#include <algorithm>

namespace VieVS {
    template<class keyType, class valueType>
    class VieVS_unordered_flat_map {
    public:
        void insert(keyType key, valueType value) {
            keys.push_back(key);
            values.push_back(value);
        }

        valueType get(keyType element) {
            auto it = std::find(keys.begin(), keys.end(), element);
            if (it != keys.end()) {
                long d = std::distance(keys.begin(), it);
                return values[d];
            }
        }

        valueType operator[](const keyType element) {
            return get(element);
        }

    private:
        std::vector<keyType> keys;
        std::vector<valueType> values;
    };
}


#endif //VIEVS_SCHEDULER_CLION_VIEVS_UNORDERED_FLAT_MAP_H
