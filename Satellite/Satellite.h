//
// Created by hwolf on 2/7/19.
//

#ifndef VIESCHEDPP_SATELLITE_H
#define VIESCHEDPP_SATELLITE_H
#include <math.h>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <vector>
#include "../Satellite/libsgp4/CoordTopocentric.h"
#include "../Satellite/libsgp4/Observer.h"
#include "../Satellite/libsgp4/SGP4.h"
#include "../Satellite/libsgp4/Tle.h"
#include "../Scan/PointingVector.h"
#include "../Station/Network.h"
#include "../Station/Station.h"


namespace VieVS {
class Satellite : public VieVS_NamedObject {
   public:
    /**
     * @brief constructor
     * @author Helene Wolf
     */
    Satellite();

    /**
     * @brief constructor
     * @author Helene Wolf
     *
     * @param hdr header line of TLE data
     * @param l1 first line of TLE data
     * @param l2 second line of TLE data
     */
    Satellite( std::string hdr, std::string l1, std::string l2 );

    /**
     * @brief destructor
     * @author Helene Wolf
     */
    ~Satellite();

    /**
     * @brief getter for header of TLE data
     * @author Helene Wolf
     *
     * @return header line of TLE data
     */
    const std::string getHeader() const noexcept;

    /**
     * @brief getter for first line of TLE data
     * @author Helene Wolf
     *
     * @return first line of TLE data
     */
    const std::string getLine1() const noexcept;

    /**
     * @brief getter for second line of TLE data
     * @author Helene Wolf
     *
     * @return second line of TLE data
     */
    const std::string getLine2() const noexcept;

    /**
     * @brief getter SGP4 data
     * @author Helene Wolf
     *
     * @return SGP4 data of satellite
     */
    SGP4 *getSGP4Data();

    /**
     * @brief satellite pass for a station
     * @author Helene Wolf
     */
    struct SatPass {
        DateTime start;             ///< start time of satellite pass
        DateTime end;               ///< end time of satellite pass
        unsigned long StationID;    ///< id of observing station
        unsigned long SatelliteID;  ///< id of observed satellite
    };

    /**
     * @brief generates the list of satellite passes for one station
     * @author Helene Wolf
     *
     * @param network network which is observing
     * @param start_time start time of session
     * @param end_time end time of session
     * @param time_step time step
     *
     * @return pass list
     */
    std::vector<std::vector<SatPass>> GeneratePassList( const VieVS::Network &network, const DateTime &start_time,
                                                        const DateTime &end_time, const int time_step );

    /**
     * @brief find exact time when the satellite crosses the horizon of station
     * @author Helene Wolf
     *
     * @param station station for which the pass list is created
     * @param sgp4 sgp4 data of satellite
     * @param initial_time1 first time point of interval to find crossing point
     * @param initial_time2 second time point of interval to find crossing point
     * @param finding_aos boolean if signal is found
     *
     * @return Date and time of the satellite crossing horizon
     */
    DateTime FindCrossingPoint( const VieVS::Station &station, const SGP4 &sgp4, const DateTime &initial_time1,
                                const DateTime &initial_time2, bool finding_aos );

    /**
     * @brief reads the satellite file
     * @author Helene Wolf
     *
     * @param filename_ name of file
     *
     * @return vector of satellites
     */
    static std::vector<Satellite> readSatelliteFile( std::string filename_ );


   private:
    static unsigned long nextId;  ///< next id for this object type
    std::string header;           ///< header line of TLE Data
    std::string line1;            ///< first line of TLE Data
    std::string line2;            ///< second line of TLE Data
    Tle *pTleData;                ///< pointer to TLE Data
    SGP4 *pSGP4Data;              ///< pointer to SGP4 Data
};
}  // namespace VieVS
#endif  // VIESCHEDPP_SATELLITE_H
