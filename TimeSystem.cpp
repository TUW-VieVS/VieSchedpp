//
// Created by mschartn on 22.08.17.
//

#include "TimeSystem.h"

thread_local double VieVS::TimeSystem::mjdStart = 0;
thread_local boost::posix_time::ptime VieVS::TimeSystem::startTime = boost::posix_time::ptime();
thread_local boost::posix_time::ptime VieVS::TimeSystem::endTime = boost::posix_time::ptime();
unsigned int VieVS::TimeSystem::duration = 0;

