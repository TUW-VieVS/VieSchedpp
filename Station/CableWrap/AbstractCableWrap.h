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
 * @file AbstractCableWrap.h
 * @brief class AbstractCableWrap
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 */

#ifndef CABLEWRAP_H
#define CABLEWRAP_H


#include <boost/format.hpp>
#include <cmath>
#include <iostream>

#include "../../Misc/Constants.h"
#include "../../Misc/VieVS_Object.h"
#include "../../Scan/PointingVector.h"
#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#endif


namespace VieVS {
/**
 * @class AbstractCableWrap
 * @brief representation of an VLBI cable wrap
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 *
 * This class serves as the base class for all cable wrap implementations.
 */
class AbstractCableWrap : public VieVS_Object {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param axis1_low_deg lower limit for first axis in degrees
     * @param axis1_up_deg upper limit for first axis in degrees
     * @param axis2_low_deg lower limit for second axis in degrees
     * @param axis2_up_deg upper limit for second axis in degrees
     */
    AbstractCableWrap( double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg );

    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param axis1_low_deg lower limit for first axis in degrees
     * @param axis1_c_low_deg azimuth lower limit of center part in degrees
     * @param axis1_c_up_deg azimuth upper limit of center part in degrees
     * @param axis1_up_deg upper limit for first axis in degrees
     * @param axis2_low_deg lower limit for second axis in degrees
     * @param axis2_up_deg upper limit for second axis in degrees
     */
    AbstractCableWrap( double axis1_low_deg, double axis1_c_low_deg, double axis1_c_up_deg, double axis1_up_deg,
                       double axis2_low_deg, double axis2_up_deg );


    /**
     * @brief enum to distinguish cable wraps
     * @author Matthias Schartner
     */
    enum class CableWrapFlag {
        ccw,  ///< counter clock wise
        n,    ///< neutral
        cw    ///< clock wise
    };


    /**
     * @brief sets safety margins for axis limits
     *
     * @param axis1_low_offset safety margin for lower limit for first axis in degrees
     * @param axis1_up_offset safety margin for upper limit for first axis in degrees
     * @param axis2_low_offset safety margin for lower limit for second axis in degrees
     * @param axis2_up_offset safety margin for upper limit for second axis in degrees
     */
    void setMinimumOffsets( double axis1_low_offset, double axis1_up_offset, double axis2_low_offset,
                            double axis2_up_offset ) noexcept;


    /**
     * @brief getter for lower neutral cable wrap limit in radians
     * @author Matthias Schartner
     *
     * @return lower neutral cable wrap limit in radians
     */
    double getNLow() const { return nLow_; }


    /**
     * @brief getter for upper neutral cable wrap limit in radians
     * @author Matthias Schartner
     *
     * @return upper neutral cable wrap limit in radians
     */
    double getNUp() const { return nUp_; }


    /**
     * @brief getter for lower clock wise cable wrap limit in radians
     * @author Matthias Schartner
     *
     * @return lower clock wise cable wrap limit in radians
     */
    double getCLow() const { return cLow_; }


    /**
     * @brief getter for upper clock wise cable wrap limit in radians
     * @author Matthias Schartner
     *
     * @return upper clock wise cable wrap limit in radians
     */
    double getCUp() const { return cUp_; }


    /**
     * @brief getter for lower counter clock wise cable wrap limit in radians
     * @author Matthias Schartner
     *
     * @return lower counter clock wise cable wrap limit in radians
     */
    double getWLow() const { return wLow_; }


    /**
     * @brief getter for upper counter clock wise cable wrap limit in radians
     * @author Matthias Schartner
     *
     * @return upper counter clock wise cable wrap limit in radians
     */
    double getWUp() const { return wUp_; }


    /**
     * @brief getter for limits of cable wrap section in radians
     * @author Matthias Schartner
     *
     * This function takes cable wrap section in sked format.
     * - 'C': clock wise
     * - '-': neutral
     * - 'W': counter clock wise
     *
     * @param section cable wrap section in sked format
     * @return pair with lower and upper limit for cable wrap section in radians
     */
    std::pair<double, double> getLimits( char section ) const;


    /**
     * @brief checks if this pointing vectors azimuth and elevation are inside the axis limits
     * @author Matthias Schartner
     *
     * @param p pointing vector which should be tested
     * @return true if inside, otherwise false
     */
    virtual bool anglesInside( const PointingVector &p ) const noexcept = 0;


    /**
     * @brief unwraps the current azimuth of pointing vector
     * @author Matthias Schartner
     *
     * The azimuth of one pointing vector is first calculated in the range between [-pi,pi]. This function
     * adds an factor of 2*pi so that the azimuth is inside the axis limits. If there are possible ambiguities,
     * for example if the azimuth axis range is bigger than 360 degrees, the value, which is closest to the second
     * input parameter value (az_old) is used.
     *
     * @param new_pointingVector pointing vector whose azimuth should be unwrapped
     * @param az_old closest target antenna azimuth in radians
     */
    virtual void unwrapAzNearAz( PointingVector &new_pointingVector, double az_old ) const noexcept = 0;


    /**
     * @brief unwraps the current azimuth of pointing vector
     * @author Matthias Schartner
     *
     * The azimuth of one pointing vector is first calculated in the range between [-pi,pi]. This function
     * adds an factor of 2*pi so that the azimuth is inside the axis limits. If there are possible ambiguities,
     * for example if the azimuth axis range is bigger than 360 degrees, the value, which is closest to the old
     * pointing vector is used.
     *
     * @param old_pointingVector closest target antenna pointing vector
     * @param new_pointingVector pointing vector whose azimuth should be unwrapped
     */
    void calcUnwrappedAz( const PointingVector &old_pointingVector, PointingVector &new_pointingVector ) const noexcept;


    /**
     * @brief unwraps the current azimuth of pointing vector in specific cable wrap section
     * @author Matthias Schartner
     *
     * The azimuth of one pointing vector is first calculated in the range between [-pi,pi]. This function
     * adds an factor of 2*pi so that the azimuth is inside the cable wrap section limits.
     *
     * This function takes cable wrap section in sked format.
     * - 'C': clock wise
     * - '-': neutral
     * - 'W': counter clock wise
     *
     * @param pv pointing vector whose azimuth should be unwrapped
     * @param section cable wrap section in sked format
     * @return true if unwrapping was possible, otherwise false
     */
    virtual bool unwrapAzInSection( PointingVector &pv, char section ) const noexcept = 0;


    /**
     * @brief cable wrap section based on unwrapped azimuth from pointing vector
     * @author Matthias Schartner
     *
     * @param pointingVector pointing vector whose azimuth is taken for calculation
     * @return cable wrap section
     */
    CableWrapFlag cableWrapFlag( const PointingVector &pointingVector ) const noexcept {
        return cableWrapFlag( pointingVector.getAz() );
    }


    /**
     * @brief cable wrap section based on unwrapped azimuth
     * @author Matthias Schartner
     *
     * @param unaz unwrapped azimuth in radians
     * @return cable wrap section
     */
    virtual CableWrapFlag cableWrapFlag( double unaz ) const noexcept = 0;


    /**
     * @brief antenna motion names in .vex format
     * @author Matthias Schartner
     *
     * @return antenna motion name for first and second axis (e.g.: {"az", "el"})
     */
    virtual std::pair<std::string, std::string> getMotions() const noexcept = 0;


    /**
     * @brief cable wrap sections in .vex format
     * @author Matthias Schartner
     *
     * @return all cable wrap sections in .vex format
     */
    virtual std::string vexPointingSectors() const noexcept = 0;


    /**
     * @brief getter for lowest possible value of 2nd axis
     * @author Matthias Schartner
     *
     * @return lowest possible value of 2nd axis
     */
    double getAxis2Low() const { return axis2Low_; }


    /**
     * @brief getter for highest possible value of 2nd axis
     * @author Matthias Schartner
     *
     * @return highest possible value of 2nd axis
     */
    double getAxis2Up() const { return axis2Up_; }


   protected:
    /**
     * @brief enum to distinguish antenna axis
     * @author Matthias Schartner
     */
    enum class Axis {
        axis1,  ///< first antenna axis
        axis2,  ///< second antenna axis
    };


    /**
     * @brief checks if axis values are inside of possible value range
     * @author Matthias Schartner
     *
     * checks if value ax1 is inside the allowed first axis range and if value ax2 is inside the allowed second axis
     * range. basically checks if target at postion (ax1, ax2) can be observed.
     *
     * @param ax1 value for first axis
     * @param ax2 value for second axis
     * @return all cable wrap sections in .vex format
     */
    bool axisInsideCableWrap( double ax1, double ax2 ) const noexcept;


    /**
     * @brief pointing sector in vex format for one axis
     * @author Matthias Schartner
     *
     * This function takes cable wrap section in sked format.
     * - 'C': clock wise
     * - '-': neutral
     * - 'W': counter clock wise
     *
     * @param motion1 motion type name for first axis (e.g.: "az")
     * @param motion2 motion type name for second axis (e.g.: "el")
     * @param section section name in sked format
     * @return pointing sector string in vex format for one axis
     */
    std::string pointingSector( const std::string &motion1, const std::string &motion2, char section ) const noexcept;


    /**
     * @brief getter for lowest possible value for this antenna axis
     * @author Matthias Schartner
     *
     * @param axis antenna axis
     * @return lowest possible value for this antenna axis in radians
     */
    double minLow( Axis axis ) const;


    /**
     * @brief getter for uppermost possible value for this antenna axis
     * @author Matthias Schartner
     *
     * @param axis antenna axis
     * @return uppermost possible value for this antenna axis in radians
     */
    double maxUp( Axis axis ) const;


   private:
    static unsigned long nextId;  ///< next id for this object type

    double axis1Low_;  ///< lower limit of first axis in radians
    double axis1Up_;   ///< upper limit of first axis in radians
    double axis2Low_;  ///< lower limit of second axis in radians
    double axis2Up_;   ///< upper limit of second axis in radians

    double axis1LowOffset_ = 0;  ///< safety margin for lower limit for first axis in radians
    double axis1UpOffset_ = 0;   ///< safety margin for upper limit for first axis in radians
    double axis2LowOffset_ = 0;  ///< safety margin for lower limit for second axis in radians
    double axis2UpOffset_ = 0;   ///< safety margin for upper limit for second axis in radians

    double nLow_;  ///< lower limit for neutral cable wrap range
    double nUp_;   ///< upper limit for neutral cable wrap range
    double cLow_;  ///< lower limit for clockwise cable wrap range
    double cUp_;   ///< upper limit for clockwise cable wrap range
    double wLow_;  ///< lower limit for counter clockwise cable wrap range
    double wUp_;   ///< upper limit for counter clockwise cable wrap range
};
}  // namespace VieVS
#endif /* CABLEWRAP_H */
