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
 * @file StationEndposition.h
 * @brief class StationEndposition
 *
 * @author Matthias Schartner
 * @date 12.08.2017
 */

#ifndef FILLINMODEENDPOSITION_H
#define FILLINMODEENDPOSITION_H


#include <limits>
#include <set>
#include <vector>

#include "../Source/SourceList.h"
#include "../Station/Station.h"


namespace VieVS {

/**
 * @class StationEndposition
 * @brief Information about which station can be used for fillin mode and how much time is available
 *
 * The available time is the time between the end of the slew time to the next source and the start of the scan of
 * the next scan. If a station is not participating in all following next scans the available time is the time
 * between the end of the last scan the station was participating and the earliest start of the next scan.
 *
 * @author Matthias Schartner
 * @date 12.08.2017
 */

class StationEndposition : public VieVS_Object {
   public:
    /**
     * @brief
     * @author Matthias Schartner
     */
    enum class change {
        start,  ///< start
        end     ///< end
    };


    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param nsta number of stations
     */
    explicit StationEndposition( unsigned long nsta );


    /**
     * @brief add new endposition
     * @author Matthias Schartner
     *
     * @param pv endposition pointing vector
     */
    void addPointingVectorAsEndposition( const PointingVector &pv );


    /**
     * @brief check if scan is possible for this station
     * @author Matthias Schartner
     *
     * @param station station
     */
    void checkStationPossibility( const Station &station );


    /**
     * @brief check if scan is possible for all stations
     * @author Matthias Schartner
     *
     * @param station all stations
     * @return true if scan is possible
     */
    bool checkStationPossibility( const std::vector<Station> &station );


    /**
     * @brief get flag if scan is possible with this station
     * @author Matthias Schartner
     *
     * @param staid station id
     * @return flag if scan is possible
     */
    bool getStationPossible( unsigned long staid ) const noexcept {
        bool possible = stationPossible_[staid];
        return possible;
    }


    /**
     * @brief check if station has endposition
     * @author Matthias Schartner
     *
     * @param staid station id
     * @return flag if station has endposition
     */
    bool hasEndposition( unsigned long staid ) const noexcept { return finalPosition_[staid].is_initialized(); }


    /**
     * @brief get required scan end time
     * @author Matthias Schartner
     *
     * if rigorous check is false: use earliest_scan_start_time instead of final_position_time in case the difference is
     * larger than 20 minutes
     *
     * @param staid station id
     * @param flag_rigorous rigorous check
     * @return latest possible scan end time
     */
    unsigned int requiredEndpositionTime( unsigned long staid, bool flag_rigorous = true ) const;


    /**
     *
     * @param staid
     * @return
     */
    bool hugeOffset( unsigned long staid ) const;


    /**
     * @brief getter for all desired positions at the end of the fillin scans.
     *
     * This end positions are the start positions of the following scan. If a station is not participating in the
     * following scans a default pointing vector is used. This information is than also stored in the
     * getStationUnused() function.
     *
     * @return vector of desired end positions per station
     */
    const std::vector<boost::optional<PointingVector>> &getFinalPosition() const noexcept { return finalPosition_; }


    /**
     * @brief get final position of station
     * @author Matthias Schartner
     *
     * @param staid station id
     * @return required position of station
     */
    const boost::optional<PointingVector> &getFinalPosition( unsigned long staid ) const noexcept {
        return finalPosition_[staid];
    }


    /**
     * @brief get flag if station is available
     *
     * @param staid station id
     * @return flag if station is available
     */
    bool getStationAvailable( unsigned long staid ) const noexcept { return stationAvailable_[staid]; }


    /**
     * @brief check if every station is initialized
     * @author Matthias Schartner
     *
     * @return flag if all stations were initialized
     */
    bool everyStationInitialized() const noexcept {
        return std::all_of( finalPosition_.begin(), finalPosition_.end(),
                            []( const boost::optional<PointingVector> &p ) { return p.is_initialized(); } );
    }


    /**
     * @brief get observed sources
     * @author Matthias Schartner
     *
     * @param time current scheduling time
     * @param sourceList list of all sources
     * @return all next observed source ids
     */
    std::set<unsigned long> getObservedSources( unsigned int time, const SourceList &sourceList ) const noexcept;

    /**
     * @brief get earliest possible scan start
     * @author Matthias Schartner
     *
     * @return earliest possible scan start
     */
    unsigned int getEarliestScanStart() const noexcept { return earliestScanStart_; }


    /**
     * @brief set station available flags
     * @author Matthias Schartner
     *
     * @param stations list of stations
     */
    void setStationAvailable( const std::vector<Station> &stations );


   private:
    static unsigned long nextId;  ///< next id for this object type

    std::vector<char> stationAvailable_;                          ///< saves available state of station
    std::vector<char> stationPossible_;                           ///< true if it is possible to use this station
    std::vector<boost::optional<PointingVector>> finalPosition_;  ///< final required end position for all stations
    unsigned int earliestScanStart_ = TimeSystem::duration;       ///< earliest scan start
};
}  // namespace VieVS

#endif  // FILLINMODEENDPOSITION_H
