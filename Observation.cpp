//
// Created by matth on 23.06.2018.
//

#include "Observation.h"
using namespace VieVS;
using namespace std;

unsigned long VieVS::Observation::nextId = 0;

Observation::Observation(unsigned long blid, unsigned long staid1, unsigned long staid2, unsigned long srcid, unsigned int startTime,
                         unsigned int observingTime): VieVS_Object(nextId++),
                                                      blid_{blid},
                                                      staid1_{staid1},
                                                      staid2_{staid2},
                                                      srcid_{srcid},
                                                      startTime_{startTime},
                                                      observingTime_{observingTime}{

}

bool Observation::containsStation(unsigned long staid) const noexcept {
    return staid == staid1_ || staid == staid2_;
}
