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
  * @file AbstractAntenna.h
  * @brief class AbstractAntenna
  *
  * @author Matthias Schartner
  * @date 27.06.2017
  *
  * This class serves as the base class for all antenna implementations.
  */

#ifndef ANTENNA_H
#define ANTENNA_H
#include <iostream>
#include <boost/format.hpp>

#include "../../Scan/PointingVector.h"
#include "../../Misc/VieVS_Object.h"

namespace VieVS{

    /**
     * @class AbstractAntenna
     * @brief representation of a VLBI antenna
     *
     * @author Matthias Schartner
     * @date 27.06.2017
     */
    class AbstractAntenna : public VieVS_Object {
    public:

        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param offset_m offset of antenna axis intersection in meters
         * @param diam_m diameter of antenna dish in meters
         * @param rate1_deg_per_min slew rate of first axis in degrees/seconds
         * @param constantOverhead1_s constant overhead for first axis slew time in seconds
         * @param rate2_deg_per_min slew rate of second axis in degrees/secondds
         * @param constantOverhead2_s constant overhead for second axis slew time in seconds
         */
        AbstractAntenna(double offset_m, double diam_m, double rate1_deg_per_min,
                        unsigned int constantOverhead1_s, double rate2_deg_per_min, unsigned int constantOverhead2_s);

        /**
         * @brief getter for antenna diameter
         * @author Matthias Schartner
         *
         * @return diameter of antenna dish in meters
         */
        double getDiam() const {
            return diam_;
        }

        /**
         * @brief getter for antenna offset
         * @author Matthias Schartner
         *
         * @return offset of antenna axis intersection in meters
         */
        double getOffset() const {
            return offset_;
        }

       /**
        * @brief slew rate of first axis in rad/seconds
        * @author Matthias Schartner
        *
        * @return slew rate of first antenna axis in rad/seconds
        */
        double getRate1() const {
            return rate1_;
        }

        /**
         * @brief getter for constant overhead for first axis slew time in seconds
         * @author Matthias Schartner
         *
         * @return constant overhead for first axis slew in seconds
         */
        double getCon1() const {
            return con1_;
        }

        /**
         * @brief slew rate of second axis in rad/seconds
         * @author Matthias Schartner
         *
         * @return slew rate of first antenna axis in rad/seconds
         */
        double getRate2() const {
            return rate2_;
        }

        /**
         * @brief getter for constant overhead for second axis slew time in seconds
         * @author Matthias Schartner
         *
         * @return constant overhead for first axis slew in seconds
         */
        double getCon2() const {
            return con2_;
        }

        /**
         * @brief calculates the slewtime between azimuth and elevation of two pointing vectors
         * @author Matthias Schartner
         *
         * @param old_pointingVector start pointing vector
         * @param new_pointingVector end pointing vector
         * @return slewtime between start pointing vector and end pointing vector in seconds
         */
        virtual unsigned int
        slewTime(const PointingVector &old_pointingVector,
                 const PointingVector &new_pointingVector) const noexcept = 0;

        /**
         * @brief get mount name
         * @author Matthias Schartner
         *
         * @return mount name
         */
        virtual std::string getMount() const noexcept = 0;

    protected:

        /**
         * @brief enum to distinguish antenna axis
         * @author Matthias Schartner
         */
        enum class Axis {
            axis1, ///< first antenna axis
            axis2, ///< second antenna axis
        };

        /**
         * @brief calculates slew time per axis
         * @author Matthias Schartner
         *
         * @param delta distance to slew in radians
         * @param axis antenna axis
         * @return slew time in seconds
         */
        unsigned int slewTimePerAxis(double delta, Axis axis) const noexcept ;

    private:
        static unsigned long nextId; ///< next id for this object type

        double offset_; ///< offset of the antenna axis intersection in meters
        double diam_; ///< diameter of the antenna dish in meters
        double rate1_; ///< slew rate of first axis in radians/second
        unsigned int con1_; ///< constant overhead for first axis slew in seconds
        double rate2_; ///< slew rate of second axis in radians/second
        unsigned int con2_; ///< constant overhead for second axis slew in seconds
    };
}
#endif /* ANTENNA_H */

