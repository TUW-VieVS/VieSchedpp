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


#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "unknown"
#endif

#ifndef UTILITY_H
#define UTILITY_H

#include <boost/format.hpp>
#include <boost/date_time.hpp>
#include <cmath>
#include <vector>
#include <numeric>

#include "Constants.h"

namespace VieVS{
    enum class Timestamp {
        start,
        end
    };
}


namespace VieVS{
namespace util{



    std::string ra2dms(double angle);

    std::string dc2hms(double angle);

    double wrapToPi(double angle);

    int duration(const boost::posix_time::ptime &start, const boost::posix_time::ptime &end);

    std::string version();

    template<typename T>
    T absDiff(const T&a, const T&b) {
        return (a > b) ? (a - b) : (b - a);
    }

    template <typename T>
    std::vector<int> sortIndexes(const std::vector<T> &v){
        // initialize original index locations
        std::vector<int> idx(v.size());
        std::iota(idx.begin(), idx.end(), 0);

        // sort indexes based on comparing values in v
        std::sort(idx.begin(), idx.end(), [&v](int i1, int i2) {
            return v[i1] < v[i2];}
        );

        return idx;
    }

    template<typename K, typename V>
    bool valueExists(std::map<K, V> mapOfElemen, V value) {
        auto it = mapOfElemen.begin();
        // Iterate through the map
        while(it != mapOfElemen.end()) {
            // Check if value of this entry matches with given value
            if(it->second == value) {
                return true;
            }
            // Go to next entry in map
            it++;
        }
        return false;
    }

    void outputObjectList(const std::string &title, const std::vector<std::string> &names, std::ofstream &of);


}
}


#endif //UTILITY_H
