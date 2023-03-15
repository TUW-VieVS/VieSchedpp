//
// Created by mschartner on 7/22/22.
//

#ifndef VIESCHEDPP_AVOIDSATELLITES_H
#define VIESCHEDPP_AVOIDSATELLITES_H

#include "../Source/Satellite.h"

namespace VieVS {
/**
 * @class AvoidSatellites
 * @brief list of satellites to be avoided during schedule generation
 *
 * @author Matthias Schartner
 * @date 22.07.2022
 */

class AvoidSatellites {
   public:
    static std::vector<std::shared_ptr<Satellite>> satellitesToAvoid;
    static double extraMargin;
    static double orbitError;
    static double orbitErrorPerDay;
    static double minElevation;
    static int frequency;
    static double outputPercentage;

    static void initialize( Network &network );

    static std::unordered_map<int, std::unordered_map<int, std::vector<std::pair<int, int>>>> visible_;
};
}  // namespace VieVS

#endif  // VIESCHEDPP_AVOIDSATELLITES_H
