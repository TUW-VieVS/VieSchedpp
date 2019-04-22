/*
 *  VieSched++ Very Long Baseline Interferometry (VLBI) Scheduling Software
 *  Copyright (C) 2018  Matthias Schartner
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Observation.h"


using namespace VieVS;
using namespace std;

unsigned long VieVS::Observation::nextId = 0;


Observation::Observation( unsigned long blid, unsigned long staid1, unsigned long staid2, unsigned long srcid,
                          unsigned int startTime, unsigned int observingTime )
    : VieVS_Object( nextId++ ),
      blid_{blid},
      staid1_{staid1},
      staid2_{staid2},
      srcid_{srcid},
      startTime_{startTime},
      observingTime_{observingTime} {}


Observation::Observation( const Observation &other )
    : VieVS_Object{nextId++},
      blid_{other.blid_},
      staid1_{other.staid1_},
      staid2_{other.staid2_},
      srcid_{other.srcid_},
      startTime_{other.startTime_},
      observingTime_{other.observingTime_} {}


bool Observation::containsStation( unsigned long staid ) const noexcept { return staid == staid1_ || staid == staid2_; }
