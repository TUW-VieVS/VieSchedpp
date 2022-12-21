//
// Created by mschartner on 7/22/22.
//

#include "AvoidSatellites.h"

using namespace VieVS;
using namespace std;

std::vector<std::shared_ptr<VieVS::Satellite>> AvoidSatellites::satellitesToAvoid =
    std::vector<std::shared_ptr<VieVS::Satellite>>();  ///< list of satellites to be avoided during scheduling

double AvoidSatellites::angular_distance = 0.5 * deg2rad;  /// set to one degree as a default
int AvoidSatellites::frequency = 10;                       /// by default, check satellite position every 30 seconds
