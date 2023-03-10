//
// Created by mschartner on 7/22/22.
//

#include "AvoidSatellites.h"

using namespace VieVS;
using namespace std;

std::vector<std::shared_ptr<VieVS::Satellite>> AvoidSatellites::satellitesToAvoid =
    std::vector<std::shared_ptr<VieVS::Satellite>>();  ///< list of satellites to be avoided during scheduling

double AvoidSatellites::extraMargin = 0.2 * deg2rad;  /// set to 0.2 degree as a default
double AvoidSatellites::orbitError = 2000;            /// set to 2 km by default
double AvoidSatellites::orbitErrorPerDay = 2000;      /// set to 2 km by default
double AvoidSatellites::minElevation = 20 * deg2rad;  /// set minimum elevation to 20 degrees
int AvoidSatellites::frequency = 10;                  /// by default, check satellite position every 10 seconds
