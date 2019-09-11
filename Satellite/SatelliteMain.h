//
// Created by hwolf on 05.02.2019.
//

#ifndef VIESCHEDPP_SATELLITEMAIN_H
#define VIESCHEDPP_SATELLITEMAIN_H

#include <algorithm>
#include "../Scan/Scan.h"
#include "../Station/Baseline.h"
#include "../Station/Network.h"
#include "../Initializer.h"
#include "Satellite.h"
#include "SatelliteObs.h"
#include "SatelliteOutput.h"
#include "libsgp4/CoordGeodetic.h"
#include "libsgp4/CoordTopocentric.h"
#include "libsgp4/Eci.h"
#include "libsgp4/Observer.h"
#include "libsgp4/SGP4.h"
#include "libsgp4/Tle.h"


namespace VieVS {
class SatelliteMain {
   public:

    SatelliteMain() = default;

    explicit SatelliteMain( const Network &network );

    void run();

    void initialize( const std::string &pathAntenna, const std::string &pathEquip,
        const std::string &pathPosition, const std::string &pathMask,
        boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
        const std::vector<std::string> &stations);

    std::vector<Satellite> readSatelliteFile( const std::string &pathToTLE ) const;

    std::vector<Scan> generateScanList( const std::vector<Satellite> &satellites ) const;

   private:
    Network network_;
    DateTime startDate_;
    DateTime endDate_;
};
}  // namespace VieVS

#endif  // VIESCHEDPP_SATELLITEMAIN_H
