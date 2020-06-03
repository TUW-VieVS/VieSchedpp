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
 * @file PointingVector.h
 * @brief class PointingVector
 *
 * @author Matthias Schartner
 * @date 06.07.2017
 */

#ifndef VLBI_POINTINGVECTOR_H
#define VLBI_POINTINGVECTOR_H


#include <boost/date_time.hpp>
#include <iostream>
#include <limits>

#include "../Misc/Constants.h"
#include "../Misc/VieVS_Object.h"


namespace VieVS {
/**
 * @class PointingVector
 * @brief representation of VLBI pointing vector
 *
 * @author Matthias Schartner
 * @date 06.07.2017
 */
class PointingVector : public VieVS_Object {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param staid id of station
     * @param srcid id of observed source
     */
    PointingVector( unsigned long staid, unsigned long srcid );


    /**
     * @brief copy constructor
     * @author Matthias Schartner
     *
     * @param other other pointing vector
     */
    PointingVector( const PointingVector &other );


    /**
     * @brief copy assignment operator
     * @author Matthias Schartner
     *
     * @param other other pointing vector
     * @return copy of other
     */
    PointingVector &operator=( const PointingVector &other ) = default;


    /**
     * @brief move constructor
     * @author Matthias Schartner
     *
     * @param other other pointing vector
     */
    PointingVector( PointingVector &&other ) = default;


    /**
     * @brief move assignment operator
     * @author Matthias Schartner
     *
     * @param other other pointing vector
     * @return move of other
     */
    PointingVector &operator=( PointingVector &&other ) = default;


    /**
     * @brief destructor
     * @author Matthias Schartner
     */
    ~PointingVector() = default;


    /**
     * @brief less then operator
     * @author Matthias Schartner
     *
     * @param other other pointing vector
     * @return true if time is less then other's time, otherwise false
     */
    bool operator<( const PointingVector &other ) const { return time_ < other.time_; }


    /**
     * @brief copy values from other pointing vector
     * @author Matthias Schartner
     *
     * @param other other pointing vector
     */
    void copyValuesFromOtherPv( const PointingVector &other );


    /**
     * @brief getter for station id
     * @author Matthias Schartner
     *
     * @return id of station
     */
    unsigned long getStaid() const noexcept { return staid_; }


    /**
     * @brief getter for source id
     * @author Matthias Schartner
     *
     * @return id of source
     */
    unsigned long getSrcid() const noexcept { return srcid_; }


    /**
     * @brief getter for azimuth
     * @author Matthias Schartner
     *
     * @return azimth in radians
     */
    double getAz() const noexcept { return az_; }


    /**
     * @brief getter for elevation
     * @author Matthias Schartner
     *
     * @return elevation in radians
     */
    double getEl() const noexcept { return el_; }


    /**
     * @brief getter for local hour angle
     * @author Matthias Schartner
     *
     * @return local hour angle
     */
    double getHa() const { return ha_; }


    /**
     * @brief getter for declination
     * @author Matthias Schartner
     *
     * @return declination
     */
    double getDc() const { return dc_; }


    /**
     * @brief getter for time for which azimuth and elevation is calculated
     * @author Matthias Schartner
     *
     * @return time in seconds since session start
     */
    unsigned int getTime() const noexcept { return time_; }


    /**
     * @brief sets azimuth
     * @author Matthias Schartner
     *
     * @param new_az new azimuth
     */
    void setAz( double new_az ) noexcept { az_ = new_az; };


    /**
     * @brief sets elevation
     * @author Matthias Schartner
     *
     * @param new_el new elevation
     */
    void setEl( double new_el ) noexcept { el_ = new_el; };


    /**
     * @brief sets local hour angle
     * @author Matthias Schartner
     *
     * @param ha new local hour angle
     */
    void setHa( double ha ) { PointingVector::ha_ = ha; }


    /**
     * @brief sets declination
     * @author Matthias Schartner
     *
     * @param dc new declination
     */
    void setDc( double dc ) { PointingVector::dc_ = dc; }


    /**
     * @brief sets time for which azimth and elevation is calculated
     * @author Matthias Schartner
     *
     * @param new_time new time
     */
    void setTime( unsigned int new_time ) noexcept { time_ = new_time; };


    /**
     * @brief total number of created pointing vectors
     * @author Matthias Schartner
     *
     * @return total nubmer of created pointing vectors
     */
    static unsigned long numberOfCreatedObjects() { return nextId - 1; }


   private:
    static unsigned long nextId;  ///< next id for this object type

    unsigned long staid_;  ///< station id
    unsigned long srcid_;  ///< source id
    double az_;            ///< azimth in radians
    double el_;            ///< elevation in radians
    double ha_{ 0 };       ///< declination in radians
    double dc_{ 0 };       ///< local hour angle in radians
    unsigned int time_;    ///< time in seconds since session start for which azimuth and elevation is calculated
};
}  // namespace VieVS
#endif /* VLBI_POINTINGVECTOR_H */
