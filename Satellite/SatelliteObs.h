//
// Created by hwolf on 2/13/19.
//

#ifndef VIESCHEDPP_SATELLITEOBS_H
#define VIESCHEDPP_SATELLITEOBS_H

#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <functional>  // std::minus
#include <iostream>
#include <numeric>
#include <vector>
#include "../Misc/TimeSystem.h"
#include "../Scan/PointingVector.h"
#include "../Scan/Scan.h"
#include "../Station/Network.h"
#include "Satellite.h"
#include "libsgp4/CoordGeodetic.h"

namespace VieVS {
class SatelliteObs : public VieVS_NamedObject {
   public:
    /**
     * @brief constructor
     * @author Helene Wolf
     */
    SatelliteObs();

    /**
     * @brief constructor
     * @author Helene Wolf
     *
     * @param startTime start time of satellite observation
     * @param endTime end time of satellite observation
     */
    SatelliteObs( DateTime startTime, DateTime endTime );

    /**
     * @brief destructor
     * @author Helene Wolf
     */
    ~SatelliteObs();

    /**
     * @brief Time Point of satellite observation start or end
     * @author Helene Wolf
     */
    struct TimePoint {
        DateTime time;              ///< time of observation start or end
        Timestamp ts;               ///< type of time point (start, end)
        unsigned long StationID;    ///< id of observing station
        unsigned long SatelliteID;  ///< id of observed satellite
    };

    /**
     * @brief getter for start time
     * @author Helene Wolf
     *
     * @return start time of stallite observation
     */
    const DateTime getStart() const noexcept;

    /**
     * @brief getter for end time
     * @author Helene Wolf
     *
     * @return end time of stallite observation
     */
    const DateTime getEnd() const noexcept;

    /**
     * @brief getter for List of IDs of observing Stations
     * @author Helene Wolf
     *
     * @return List of Station IDs
     */
    const std::vector<unsigned long> getStationIDList() const noexcept;

    /**
     * @brief getter for number of observing stations
     * @author Helene Wolf
     *
     * @return number of observing stations
     */
    const unsigned long getNumberofStations() const noexcept;

    /**
     * @brief converts the pass list to overlaps
     * @author Helene Wolf
     *
     * @param PassList 2D vector containing the possible observing times for the stations
     * @return vector with overlap times
     */
    static std::vector<SatelliteObs> passList2Overlap( std::vector<std::vector<Satellite::SatPass>> PassList );

    /**
     * @brief compares timepoints
     * @author Helene Wolf
     *
     * @param i1 time of TimePoint2
     * @param i2 time of TimePoint1
     * @return boolean i1 < i2
     */
    static bool compareTimePoint( TimePoint i1, TimePoint i2 );

    /**
     * @brief removes Station ID
     * @author Helene Wolf
     *
     * @param StationIDs list of stations
     * @param reStation station to be removed
     */
    static void removeStationID( std::vector<unsigned long> &StationIDs, unsigned long reStation );


    /**
     * @brief calculation of right ascension, declination and local hour angle of satellite at specific time
     * This function calculates the ra and dec and ha for the satellite to the given current time and sets the variables
     * in the pointing vector
     * @author Helene Wolf
     *
     * @param current_time time at which ra, dec and lha are calculated
     * @param start_time start time of session
     * @param eci earth center inertial system coordinates of satellite at current time
     * @param obs station position as object observer
     * @param pv pointing vector which is filled
     */
    static void calcRaDeHa( DateTime current_time, DateTime start_time, Eci eci, Observer obs, PointingVector *pv );

    /**
     * @brief calculation of observations
     * @author Helene Wolf
     *
     * @param scan scan for which observations should be created
     * @param network station network
     * @param sat satellite
     * @param PV_start vector of pointing vectors for each observing station at start time
     */
    static void calcObservations( Scan *scan, Network &network, Satellite sat, std::vector<PointingVector> PV_start );


    /**
     * @brief create Pointing Vectors
     * @author Helene Wolf
     *
     * @param PassList_
     * @param sat
     * @param network
     * @param SessionStartTime
     *
     * @return list of pointing Vectors for each Station
     */
    static std::vector<std::vector<std::vector<VieVS::PointingVector>>> PassList2pv(
        std::vector<std::vector<Satellite::SatPass>> PassList_, Satellite sat, VieVS::Network network,
        DateTime SessionStartTime );
    /**
     * @brief create scan list, convert PassList to scans
     * @author Helene Wolf
     *
     * @param PassList list of satellite passes
     * @param network station network
     * @param sat satellite
     * @param sessionStartTime start time of session
     *
     * @return List of scans
     */
    std::vector<Scan> static createScanList( std::vector<std::vector<Satellite::SatPass>> PassList, Network network,
                                             Satellite sat, DateTime sessionStartTime );

    /**
     * @brief create pointing vector
     * @author Helene Wolf
     *
     * @param station station
     * @param sat satellite
     * @param StartTime session start time
     * @param TimePoint start or end time of station
     *
     * @return pointing Vector
     */
    VieVS::PointingVector static createPV( VieVS::Station station, Satellite sat, DateTime sessionStartTime,
                                           TimePoint TimePoint );

    /**
     * @brief converts the PassList to a sorted list of Timepoints
     * @author Helene Wolf
     *
     * @param PassList list of stallite observations
     *
     * @return pointing Vector
     */
    static std::vector<SatelliteObs::TimePoint> createSortedTimePoints(
        std::vector<std::vector<Satellite::SatPass>> &PassList );


    /**
     * @brief compares two pointing Vectors by time
     * @author Helene Wolf
     *
     * @param pv1 Pointing Vector
     * @param pv2 PointingVector
     *
     * @return bool
     */
    static bool comparePV( PointingVector pv1, PointingVector pv2 );

    /**
     * @brief creates Output of observation windows and the time
     * @author Helene Wolf
     *
     * @param pv1 PassList
     * @param overlap
     * @param network
     *
     */
    void static createOutput( std::vector<std::vector<Satellite::SatPass>> PassList_, std::vector<SatelliteObs> overlap,
                              Network network );

   private:
    static unsigned long nextId;               ///< next id for this object type
    DateTime start;                            ///< start time of satellite observation
    DateTime end;                              ///< end time of satellite observation
    std::vector<unsigned long> StationIDList;  ///< list of ids of observing stations
    unsigned long SatelliteID;                 //< id of observed satellite
};
}  // namespace VieVS

#endif  // VIESCHEDPP_SATELLITEOBS_H
