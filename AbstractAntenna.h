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
 * @file Antenna.h
 * @brief class Antenna
 *
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 */

#ifndef ANTENNA_H
#define ANTENNA_H
#include <iostream>
#include <boost/format.hpp>

#include "PointingVector.h"
#include "VieVS_Object.h"

namespace VieVS{

    /**
     * @class Antenna
     * @brief representation of an VLBI antenna
     *
     * @author Matthias Schartner
     * @date 27.06.2017
     */
    class AbstractAntenna : public VieVS_Object {
    public:

        /**
         * @brief constructor
         *
         * @param type axis type
         * @param offset_m offset of antenna axis intersection in meters
         * @param diam_m diameter of antenna dish in meters
         * @param rate1_deg_per_min slew rate of first axis in degrees/seconds
         * @param constantOverhead1_s constant overhead for first axis slew in seconds
         * @param rate2_deg_per_min slew rate of second axis in degrees/secondds
         * @param constantOverhead2_s constant overhead for second axis slew in seconds
         */
        AbstractAntenna(double offset_m, double diam_m, double rate1_deg_per_min,
                        unsigned int constantOverhead1_s, double rate2_deg_per_min, unsigned int constantOverhead2_s);

        double getDiam() const {
            return diam_;
        }

        double getOffset() const {
            return offset_;
        }

        double getRate1() const {
            return rate1_;
        }

        double getCon1() const {
            return con1_;
        }

        double getRate2() const {
            return rate2_;
        }

        double getCon2() const {
            return con2_;
        }

        /**
         * @brief calculates the slewtime between azimuth and elevation of two pointing vectors
         * @param old_pointingVector start azimuth and elevation
         * @param new_pointingVector end azimuth and elevation
         * @return slewtime in seconds
         */
        virtual unsigned int
        slewTime(const PointingVector &old_pointingVector,
                 const PointingVector &new_pointingVector) const noexcept = 0;

    protected:
        enum class Axis {
            axis1,
            axis2,
        };

        /**
         * @brief calculates slew time per axis
         *
         * @param delta distance to slew
         * @param rate velocity
         * @param acc acceleration
         * @return slew time in seconds
         */
        unsigned int slewTimePerAxis(double delta, Axis axis) const noexcept ;

    private:
        static unsigned long nextId;

        double offset_; ///< offset of the antenna axis intersection in meters
        double diam_; ///< diameter of the antenna dish in meters
        double rate1_; ///< slew rate of first axis in radians/second
        unsigned int con1_; ///< constant overhead for first axis slew in seconds
        double rate2_; ///< slew rate of second axis in radians/second
        unsigned int con2_; ///< constant overhead for second axis slew in seconds
    };
}
#endif /* ANTENNA_H */

