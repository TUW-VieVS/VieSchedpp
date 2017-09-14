/**
 * @file Position.h
 * @brief class Position
 *
 *
 * @author Matthias Schartner
 * @date 23.06.2017
 */

#ifndef POSITION_H
#define POSITION_H
#include <cmath>
#include <iostream>
#include <boost/format.hpp>
#include "Constants.h"

namespace VieVS{
    /**
     * @class Position
     * @brief representation of VLBI station position
     *
     * @author Matthias Schartner
     * @date 23.06.2017
     */
    class Position {
    public:
        /**
         * @brief empty default constructor
         */
        Position();

        /**
         * @brief constructor
         *
         * @param x_m x coordinate in meters
         * @param y_m y coordinate in meters
         * @param z_m z coordinate in meters
         */
        Position(double x_m, double y_m, double z_m);

        /**
         * @brief default copy constructor
         *
         * @param other other positon
         */
        Position(const Position &other) = default;

        /**
         * @brief default move constructor
         *
         * @param other other positon
         */
        Position(Position &&other) = default;

        /**
         * @brief default copy assignment operator
         *
         * @param other other positon
         * @return copy of other positon
         */
        Position &operator=(const Position &other) = default;

        /**
         * @brief default move assignment operator
         *
         * @param other other positon
         * @return moved other positon
         */
        Position &operator=(Position &&other) = default;

        /**
         * @brief destructor
         */
        virtual ~Position(){};

        /**
         * @brief getter for x coordinate
         *
         * @return x coordinate in meters
         */
        double getX() const noexcept {
            return x_;
        }

        /**
         * @brief getter for y coordinate
         *
         * @return y coordinate in meters
         */
        double getY() const noexcept {
            return y_;
        }

        /**
         * @brief getter for z coordinate
         *
         * @return z coordinate in meters
         */
        double getZ() const noexcept {
            return z_;
        }

        /**
         * @brief getter for latitude
         *
         * @return latitude in radians
         */
        double getLat() const noexcept {
            return lat_;
        }

        /**
         * @brief getter for longitude
         *
         * @return longitude in radians
         */
        double getLon() const noexcept {
            return lon_;
        }

        /**
         * @brief calculates distance between two stations
         *
         * @param other second station
         * @return distance between stations
         */
        double getDistance(const Position &other) const noexcept;

        /**
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param position position information that should be printed to stream
         * @return stream object
         */
        friend std::ostream &operator<<(std::ostream &out, const Position &position) noexcept;

    private:
        double x_; ///< x coordinate in meters
        double y_; ///< y coordinate in meters
        double z_; ///< z coordinate in meters
        double lat_; ///< latitude in radians
        double lon_; ///< longitude in radians
        double h_; ///< height in meters
    };
}
#endif /* POSITION_H */

