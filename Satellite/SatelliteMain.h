//
// Created by hwolf on 05.02.2019.
//

#ifndef VIESCHEDPP_SATELLITEMAIN_H
#define VIESCHEDPP_SATELLITEMAIN_H

#include <algorithm>
#include "../Scan/Scan.h"
#include "../Station/Baseline.h"
#include "../Station/Network.h"
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
    explicit SatelliteMain( const Network &network );
    void run();

   private:
    Network network_;
};
}  // namespace VieVS

#endif  // VIESCHEDPP_SATELLITEMAIN_H
