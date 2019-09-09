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
 * @file Position.h
 * @brief class Position
 *
 * @author Matthias Schartner
 * @date 23.06.2017
 */

#ifndef POSITION_H
#define POSITION_H


#include <boost/format.hpp>
#include <cmath>
#include <iostream>
#include "../Misc/Constants.h"
#include "../Misc/VieVS_Object.h"
#include "../Misc/sofa.h"


namespace VieVS {
/**
 * @class Position
 * @brief representation of VLBI station position
 *
 * @author Matthias Schartner
 * @date 23.06.2017
 */
class Position : public VieVS_Object {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param x_m x coordinate in meters
     * @param y_m y coordinate in meters
     * @param z_m z coordinate in meters
     */
    Position( double x_m, double y_m, double z_m );


    /**
     * @brief getter for x coordinate
     * @author Matthias Schartner
     *
     * @return x coordinate in meters
     */
    double getX() const noexcept { return x_; }


    /**
     * @brief getter for y coordinate
     * @author Matthias Schartner
     *
     * @return y coordinate in meters
     */
    double getY() const noexcept { return y_; }


    /**
     * @brief getter for z coordinate
     * @author Matthias Schartner
     *
     * @return z coordinate in meters
     */
    double getZ() const noexcept { return z_; }


    /**
     * @brief getter for latitude
     * @author Matthias Schartner
     *
     * @return latitude in radians
     */
    double getLat() const noexcept { return lat_; }


    /**
     * @brief getter for longitude
     * @author Matthias Schartner
     *
     * @return longitude in radians
     */
    double getLon() const noexcept { return lon_; }

    /**
     * @brief getter for height
     * @author Helene Wolf
     *
     * @return height in meters
     */
    double getHeight() const noexcept { return h_; }

    /**
     * @brief calculates distance between two stations
     * @author Matthias Schartner
     *
     * @param other second station
     * @return distance between stations
     */
    double getDistance( const Position &other ) const noexcept;


    /**
     * @brief get geodetic to local transformation matrix
     * @author Matthias Schartner
     *
     * @param g2l geodetic to local transformation matrix target
     */
    void geodetic2Local( double g2l[3][3] ) { g2l = g2l_; }


    /**
     * @brief get geodetic to local transformation matrix
     * @author Matthias Schartner
     *
     * @return geodetic to local transformation matrix
     */
    const std::vector<std::vector<double>> getGeodetic2Local() const { return g2l_2; }


   private:
    static unsigned long nextId;  ///< next id for this object type

    double x_;    ///< x coordinate in meters
    double y_;    ///< y coordinate in meters
    double z_;    ///< z coordinate in meters
    double lat_;  ///< latitude in radians
    double lon_;  ///< longitude in radians
    double h_;    ///< height in meters

    double g2l_[3][3];  ///< geocentric to local transformation matrix

    std::vector<std::vector<double>> g2l_2;  ///< geocentric to local transformation matrix
};
}  // namespace VieVS
#endif /* POSITION_H */
