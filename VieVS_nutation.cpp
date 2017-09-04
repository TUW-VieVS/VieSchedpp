//
// Created by mschartn on 31.07.17.
//

#include "VieVS_nutation.h"

thread_local vector<double> VieVS::VieVS_nutation::nut_x = {};
thread_local vector<double> VieVS::VieVS_nutation::nut_y = {};
thread_local vector<double> VieVS::VieVS_nutation::nut_s = {};
thread_local vector<unsigned int> VieVS::VieVS_nutation::nut_time = {};
