//
// Created by mschartner on 7/25/22.
//

#ifndef VIESCHEDPP_DIFFERENTIALPARALLACTICANGLEBLOCK_H
#define VIESCHEDPP_DIFFERENTIALPARALLACTICANGLEBLOCK_H

#include <algorithm>
#include <vector>

#include "VieVS_Object.h"

namespace VieVS {
/**
 * @class differential parallactic angle block
 * @brief define differential parallactic angle blocks
 *
 * @author Matthias Schartner
 * @date 25.07.2022
 */

class DifferentialParallacticAngleBlock : public VieVS_Object {
   public:
    static std::string intent_;
    static int cadence;
    static double distanceScaling;
    static std::vector<unsigned long> allowedSources;
    static std::vector<unsigned long> allowedBaseline;
    static unsigned int duration;
    static int nscans;

    static bool isAllowedSource( unsigned long srcid ) {
        return find( allowedSources.begin(), allowedSources.end(), srcid ) != allowedSources.end();
    }
    static bool isAllowedBaseline( unsigned long blid ) {
        return find( allowedBaseline.begin(), allowedBaseline.end(), blid ) != allowedBaseline.end();
    }
};
}  // namespace VieVS
#endif  // VIESCHEDPP_DIFFERENTIALPARALLACTICANGLEBLOCK_H
