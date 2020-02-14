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
 * @file Observation.h
 * @brief class Observation
 *
 * @author Matthias Schartner
 * @date 28.06.2018
 */

#ifndef OBSERVATION_H
#define OBSERVATION_H


#include "../Misc/VieVS_Object.h"


namespace VieVS {

/**
 * @class Observation
 * @brief representation of a VLBI observation
 *
 * @author Matthias Schartner
 * @date 28.06.2018
 */
class Observation : public VieVS_Object {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param blid baseline id
     * @param staid1 first station id
     * @param staid2 second station id
     * @param srcid source id
     * @param startTime start time
     * @param observingTime observing time in seconds
     */
    Observation( unsigned long blid, unsigned long staid1, unsigned long staid2, unsigned long srcid,
                 unsigned int startTime, unsigned int observingTime = 0 );


    /**
     * @brief copy constructor
     * @author Matthias Schartner
     *
     * @param other reference observation
     */
    Observation( const Observation &other );


    /**
     * @brief get baseline id
     * @author Matthias Schartner
     *
     * @return baseline id
     */
    unsigned long getBlid() const { return blid_; }


    /**
     * @brief get first station id
     * @author Matthias Schartner
     *
     * @return first station id
     */
    unsigned long getStaid1() const { return staid1_; }


    /**
     * @brief get second station id
     * @author Matthias Schartner
     *
     * @return second station id
     */
    unsigned long getStaid2() const { return staid2_; }


    /**
     * @brief get source id
     * @author Matthias Schartner
     *
     * @return source id
     */
    unsigned long getSrcid() const { return srcid_; }


    /**
     * @brief get start time
     * @author Matthias Schartner
     *
     * @return start time
     */
    unsigned int getStartTime() const { return startTime_; }


    /**
     * @brief get observing time
     * @author Matthias Schartner
     *
     * @return observing time in seconds
     */
    unsigned int getObservingTime() const { return observingTime_; }


    /**
     * @brief check if observation is with this station
     * @author Matthias Schartner
     *
     * @param staid target station id
     * @return true if observation is between this station, otherwise false
     */
    bool containsStation( unsigned long staid ) const noexcept;


    /**
     * @brief set observing time for this observation
     * @author Matthias Schartner
     *
     * @param observingTime observing time in seconds
     */
    void setObservingTime( unsigned int observingTime ) { observingTime_ = observingTime; }


    /**
     * @brief set start time for this observation
     * @author Matthias Schartner
     *
     * @param startTime start time
     */
    void setStartTime( unsigned int startTime ) { startTime_ = startTime; }


    /**
     * @brief number of created observations
     * @author Matthias Schartner
     *
     * @return total number of created observations
     */
    static unsigned long numberOfCreatedObjects() { return nextId; }

    /**
     * @brief add noise to observed minus computed
     * @author Matthias Schartner
     *
     * @param noise noise in seconds
     */
    void addNoise( double noise ) { o_c += noise; }

   private:
    static unsigned long nextId;  ///< next id for this object type

    unsigned long blid_;    ///< baseline id
    unsigned long staid1_;  ///< first station id
    unsigned long staid2_;  ///< second station id
    unsigned long srcid_;   ///< source id

    double o_c = 0;
    double sigma = 0;

    unsigned int startTime_;      ///< observation start time
    unsigned int observingTime_;  ///< observation duration
};
}  // namespace VieVS

#endif  // OBSERVATION_H
