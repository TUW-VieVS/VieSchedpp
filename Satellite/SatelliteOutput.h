//
// Created by hwolf on 8/1/19.
//

#include "../Misc/VieVS_NamedObject.h"
#include "../Scan/Scan.h"
#include "SatelliteMain.h"
#include "SatelliteObs.h"

#ifndef VIESCHEDPP_SATELLITEOUTPUT_H
#define VIESCHEDPP_SATELLITEOUTPUT_H

namespace VieVS {
/**
 * @class Satellite Output
 * @brief this is the Satellite Output class which creates all kind of output
 *
 * @author Helene Wolf
 * @date 01.08.2019
 */
class SatelliteOutput : public VieVS_NamedObject {
   public:
    /**
     * @brief constructor
     * @author Helene Wolf
     *
     * @param network_ station network
     * @param sat satellite
     * @param start start time of session
     * @param end end time of session
     */
    SatelliteOutput( Network &network_, Satellite sat, DateTime start, DateTime end );

    /**
     * @brief destructor
     * @author Helene Wolf
     */
    ~SatelliteOutput();
    /**
     * @brief adds the scanlist to the output object
     * @param scanList list of scans
     * @author Helene Wolf
     */
    void addScans( std::vector<Scan> scanList );

    /**
     * @brief print the satellite passes
     * @author Helene Wolf
     */
    void printPassList( std::vector<std::vector<Satellite::SatPass>> PassList );

    /**
     * @brief print the scans
     * @author Helene Wolf
     */
    void printScan();

    /**
     * @brief print participating stations
     * @author Helene Wolf
     */
    void printStations();

    /**
     * @brief print start and end time of session
     * @author Helene Wolf
     */
    void printStart();

    /**
     * @brief print the overlaps
     * @author Helene Wolf
     */
    void printOverlaps( std::vector<SatelliteObs> overlaps );

    /**
     * @brief print pointing vectors
     * @author Helene Wolf
     */
    void printPV( std::vector<std::vector<std::vector<VieVS::PointingVector>>> pvRes );

   private:
    static unsigned long nextId;  ///< next id for this object type
    Network network_;             ///< network
    Satellite satellite;          ///< all sources
    std::vector<Scan> scans_;     ///< all scans in schedule
    DateTime start_date;          ///< session start date
    DateTime end_date;            ///< session end date
};
}  // namespace VieVS
#endif  // VIESCHEDPP_SATELLITEOUTPUT_H
