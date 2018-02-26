//
// Created by mschartn on 31.07.17.
//

#include "Nutation.h"

thread_local std::vector<double> VieVS::Nutation::nutX = {};
thread_local std::vector<double> VieVS::Nutation::nutY = {};
thread_local std::vector<double> VieVS::Nutation::nutS = {};
thread_local std::vector<unsigned int> VieVS::Nutation::nutTime = {};
