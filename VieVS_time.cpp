//
// Created by mschartn on 22.08.17.
//

#include "VieVS_time.h"

thread_local double VieVS::VieVS_time::mjdStart = 0;
thread_local boost::posix_time::ptime VieVS::VieVS_time::startTime = boost::posix_time::ptime();
thread_local boost::posix_time::ptime VieVS::VieVS_time::endTime = boost::posix_time::ptime();
unsigned int VieVS::VieVS_time::duration = 0;

namespace VieVS {

}