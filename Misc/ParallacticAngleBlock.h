//
// Created by mschartner on 7/25/22.
//

#ifndef VIESCHEDPP_PARALLACTICANGLEBLOCK_H
#define VIESCHEDPP_PARALLACTICANGLEBLOCK_H

#include <algorithm>
#include <vector>

#include "VieVS_Object.h"

namespace VieVS {
/**
 * @class parallactic angle block
 * @brief define parallactic angle blocks
 *
 * @author Matthias Schartner
 * @date 25.07.2022
 */

class ParallacticAngleBlock : public VieVS_Object {
   public:
    static std::string intent_;
    static int cadence;
    static double distanceScaling;
    static std::vector<unsigned long> allowedSources;
    static std::vector<unsigned long> allowedStations;
    static unsigned int duration;
    static int nscans;

    static bool isAllowedSource( unsigned long srcid ) {
        return find( allowedSources.begin(), allowedSources.end(), srcid ) != allowedSources.end();
    }
    static bool isAllowedStation( unsigned long staid ) {
        return find( allowedStations.begin(), allowedStations.end(), staid ) != allowedStations.end();
    }
};

}  // namespace VieVS
#endif  // VIESCHEDPP_PARALLACTICANGLEBLOCK_H
