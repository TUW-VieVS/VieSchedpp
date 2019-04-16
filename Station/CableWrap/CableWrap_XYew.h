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
 * @file CableWrap_XYew.h
 * @brief class X, Y east west cable wrap
 *
 * @author Matthias Schartner
 * @date 12.04.2018
 *
 */

#ifndef CABLEWRAP_XYEW_H
#define CABLEWRAP_XYEW_H

#include "AbstractCableWrap.h"

namespace VieVS {
/**
 * @class CableWrap_XYew
 * @brief X, Y east west cable wrap
 *
 * @author Matthias Schartner
 * @date 12.04.2018
 */
class CableWrap_XYew : public AbstractCableWrap {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param axis1_low_deg X lower limit in degrees
     * @param axis1_up_deg X upper limit in degrees
     * @param axis2_low_deg Y limit in degrees
     * @param axis2_up_deg Y limit in degrees
     */
    CableWrap_XYew( double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg );

    /**
     * @brief checks if this pointing vectors azimuth and elevation are inside the axis limits
     * @author Matthias Schartner
     *
     * @param p pointing vector which should be tested
     * @return true if inside, otherwise false
     */
    bool anglesInside( const PointingVector &p ) const noexcept override;

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
    void unwrapAzNearAz( PointingVector &new_pointingVector, double az_old ) const noexcept override;

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
    bool unwrapAzInSection( PointingVector &pv, char section ) const noexcept override;

    /**
     * @brief cable wrap section based on unwrapped azimuth
     * @author Matthias Schartner
     *
     * @param unaz unwrapped azimuth in radians
     * @return cable wrap section
     */
    AbstractCableWrap::CableWrapFlag cableWrapFlag( double unaz ) const noexcept override;

    /**
     * @brief antenna motion names in .vex format
     * @author Matthias Schartner
     *
     * @return antenna motion name for first and second axis (e.g.: {"x", "yew"})
     */
    std::pair<std::string, std::string> getMotions() const noexcept override;

    /**
     * @brief cable wrap sections in .vex format
     * @author Matthias Schartner
     *
     * @return all cable wrap sections in .vex format
     */
    std::string vexPointingSectors() const noexcept override;
};
}  // namespace VieVS

#endif  // CABLEWRAP_XYEW_H
