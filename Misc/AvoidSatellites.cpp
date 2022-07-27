//
// Created by mschartner on 7/22/22.
//

#include "AvoidSatellites.h"

using namespace VieVS;
using namespace std;

std::vector<std::shared_ptr<VieVS::Satellite>> AvoidSatellites::satellitesToAvoid =
    std::vector<std::shared_ptr<VieVS::Satellite>>();  ///< list of satellites to be avoided during scheduling

double angular_distance = 1;  /// set to one degree as a default
