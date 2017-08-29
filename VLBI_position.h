/**
 * @file VLBI_position.h
 * @brief class VLBI_position
 *
 *
 * @author Matthias Schartner
 * @date 23.06.2017
 */

#ifndef VLBI_POS_H
#define VLBI_POS_H
#include <cmath>
#include <iostream>
#include <boost/format.hpp>
#include "VieVS_constants.h"

using namespace std;
namespace VieVS{
    class VLBI_position {
    public:
        /**
         * @brief empty default constructor
         */
        VLBI_position();

        /**
         * @brief constructor
         *
         * @param x_m x coordinate in meters
         * @param y_m y coordinate in meters
         * @param z_m z coordinate in meters
         */
        VLBI_position(double x_m, double y_m, double z_m);

        /**
         * @brief default copy constructor
         *
         * @param other other positon
         */
        VLBI_position(const VLBI_position &other) = default;

        /**
         * @brief default move constructor
         *
         * @param other other positon
         */
        VLBI_position(VLBI_position &&other) = default;

        /**
         * @brief default copy assignment operator
         *
         * @param other other positon
         * @return copy of other positon
         */
        VLBI_position &operator=(const VLBI_position &other) = default;

        /**
         * @brief default move assignment operator
         *
         * @param other other positon
         * @return moved other positon
         */
        VLBI_position &operator=(VLBI_position &&other) = default;

        /**
         * @brief destructor
         */
        virtual ~VLBI_position(){};

        /**
         * @brief getter for x coordinate
         *
         * @return x coordinate in meters
         */
        double getX() const noexcept {
            return x;
        }

        /**
         * @brief getter for y coordinate
         *
         * @return y coordinate in meters
         */
        double getY() const noexcept {
            return y;
        }

        /**
         * @brief getter for z coordinate
         *
         * @return z coordinate in meters
         */
        double getZ() const noexcept {
            return z;
        }

        /**
         * @brief getter for latitude
         *
         * @return latitude in radians
         */
        double getLat() const noexcept {
            return lat;
        }

        /**
         * @brief getter for longitude
         *
         * @return longitude in radians
         */
        double getLon() const noexcept {
            return lon;
        }

        /**
         * @brief calculates distance between two stations
         *
         * @param other second station
         * @return distance between stations
         */
        double getDistance(const VLBI_position &other) const noexcept;

        /**
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param position position information that should be printed to stream
         * @return stream object
         */
        friend ostream &operator<<(ostream &out, const VLBI_position &position) noexcept;

    private:
        double x; ///< x coordinate in meters
        double y; ///< y coordinate in meters
        double z; ///< z coordinate in meters
        double lat; ///< latitude in radians
        double lon; ///< longitude in radians
        double h; ///< height in meters
    };
}
#endif /* VLBI_POS_H */

