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
 * @file util.h
 * @brief utillity functions for VieSched++
 *
 * @author Matthias Schartner
 * @date 07.05.2018
 */

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "unknown"
#endif

#ifndef UTILITY_H
#define UTILITY_H


#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cmath>
#include <numeric>
#include <vector>

#include "Constants.h"


namespace VieVS {
/**
 * @brief
 * @author Matthias Schartner
 */
enum class Timestamp {
    start,  ///< start point
    end     ///< end point
};
}  // namespace VieVS

namespace VieVS {
/**
 * @namespace util
 * @brief namespace util for utility functions.
 * @author Matthias Schartner
 */
namespace util {

/**
 * @brief get number of stations from xml file
 * @author Matthias Schartner
 *
 * @param xml property tree
 * @return number of stations
 */
unsigned long getNumberOfStations( const boost::property_tree::ptree &xml );


/**
 * @brief get station names from xml file
 * @author Matthias Schartner
 *
 * @param xml property tree
 * @return vector of station names
 */
std::vector<std::string> getStationNames( const boost::property_tree::ptree &xml );


/**
 * @brief convert right ascension to string
 * @author Matthias Schartner
 *
 * example output: 00h 00m 00.00s
 *
 * @param angle right ascension angle in radians
 * @return output string
 */
std::string ra2dms( double angle );


/**
 * @brief convert right ascension to string
 * @author Matthias Schartner
 *
 * example output: 00:00:00.000
 *
 * @param angle right ascension angle in radians
 * @return output string
 */
std::string ra2dms_astFormat( double angle );


/**
 * @brief convert declination to string
 * @author Matthias Schartner
 *
 * example output: +00° 00' 00.00"
 *
 * @param angle declination angle in radians
 * @return output string
 */
std::string dc2hms( double angle );


/**
 * @brief convert declination to string
 * @author Matthias Schartner
 *
 * example output: +00° 00' 00.00"
 *
 * @param angle declination angle in radians
 * @return output string
 */
std::string dc2hms_astFormat( double angle );


/**
 * @brief transforms frequency to wavelength
 * @author Matthias Schartner
 *
 * assuming speed of light wave velocity
 *
 * @param frequency frequency in Hz
 * @return wavelength in m
 */
double freqency2wavelenth( double frequency );


/**
 * @brief transforms wavelength to frequency
 * @author Matthias Schartner
 *
 * assuming speed of light wave velocity
 *
 * @param wavelength wavelength in m
 * @return frequency in Hz
 */
double wavelength2frequency( double wavelength );


/**
 * @brief wrap angle to interval [0, 2*pi)
 * @author Matthias Schartner
 *
 * @param angle input angle in radians
 * @return output angle
 */
double wrap2twoPi( double angle );


/**
 * @brief wrap angle to interval [-pi, pi)
 * @author Matthias Schartner
 *
 * @param angle input angle in radians
 * @return output angle
 */
double wrap2pi( double angle );


/**
 * @brief calculate duration between two time points
 * @author Matthias Schartner
 *
 * @param start start time point
 * @param end end time point
 * @return duration between time points in seconds
 */
int duration( const boost::posix_time::ptime &start, const boost::posix_time::ptime &end );


/**
 * @brief software version number
 * @author Matthias Schartner
 *
 * @return software version number
 */
std::string version();


/**
 * @brief absolute difference between two points
 * @author Matthias Schartner
 *
 * prevents underflow errors for unsigned types
 *
 * @tparam T template type
 * @param a first value
 * @param b second value
 * @return absolute difference
 */
template <typename T>
T absDiff( const T &a, const T &b ) {
    return ( a > b ) ? ( a - b ) : ( b - a );
}


/**
 * @brief indices of sorted vector elements
 * @author Matthias Schartner
 *
 * Example: input {2.1, 1.3, 0.3, 5.3} would return {2, 1, 0, 3}
 *
 * @tparam T vector type
 * @param v vector
 * @return indices of sorted vector elements
 */
template <typename T>
std::vector<int> sortIndexes( const std::vector<T> &v ) {
    // initialize original index locations
    std::vector<int> idx( v.size() );
    std::iota( idx.begin(), idx.end(), 0 );

    // sort indexes based on comparing values in v
    std::sort( idx.begin(), idx.end(), [&v]( int i1, int i2 ) { return v[i1] < v[i2]; } );

    return idx;
}


/**
 * @brief checks if a value already exists in a map
 * @author Matthias Schartner
 *
 * @tparam K key type
 * @tparam V value type
 * @param mapOfElemen current map
 * @param value target value
 * @return true if target value exists, otherwise false
 */
template <typename K, typename V>
bool valueExists( std::map<K, V> mapOfElemen, V value ) {
    auto it = mapOfElemen.begin();
    // Iterate through the map
    while ( it != mapOfElemen.end() ) {
        // Check if value of this entry matches with given value
        if ( it->second == value ) {
            return true;
        }
        // Go to next entry in map
        it++;
    }
    return false;
}


/**
 * @brief outputs list of objects
 * @author Matthias Schartner
 *
 * @param title output title
 * @param names object names
 * @param of outstream object
 * @param indents number of indents
 */
void outputObjectList( const std::string &title, const std::vector<std::string> &names, std::ofstream &of,
                       unsigned long indents = 4 );


/**
 * @brief convert weekday integer to string
 * @author Matthias Schartner
 *
 * 0 = SUN
 * 1 = MON
 * 2 = TUE...
 *
 * @param weekday weekday in integer format
 * @return weekday in string format
 */
std::string weekDay2string( int weekday );


/**
 * @brief convert month integer to string
 * @author Matthias Schartner
 *
 * 1 = JAN
 * 2 = FEB...
 *
 * @param month month in integer format
 * @return month in string format
 */
std::string month2string( int month );

/**
 * @brief flip pair
 * @author Matthias Schartner
 *
 * @param p pair to flip
 * @return flipped pair
 */
    template<typename A, typename B>
    std::pair<B, A> flip_pair(const std::pair<A, B> &p) {
        return std::pair<B, A>(p.second, p.first);
    }

/**
 * @brief flip map
 * @author Matthias Schartner
 *
 * @param src map to flip
 * @return flipped map
 */
    template<typename A, typename B>
    std::map<B, A> flip_map(const std::map<A, B> &src) {
        std::map<B, A> dst;
        std::transform(src.begin(), src.end(), std::inserter(dst, dst.begin()),
                       flip_pair<A, B>);
        return dst;
    }

}  // namespace util
}  // namespace VieVS

#endif  // UTILITY_H
