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

/*
 * File:   PointingVector.cpp
 * Author: mschartn
 *
 * Created on July 6, 2017, 1:24 PM
 */

#include "PointingVector.h"


using namespace std;
using namespace VieVS;
unsigned long PointingVector::nextId = 0;

// PointingVector::PointingVector():VieVS_Object(nextId++), staid_{-1}, srcid_{-1}{
//}

PointingVector::PointingVector( unsigned long staid, unsigned long srcid )
    : VieVS_Object( nextId++ ), staid_{ staid }, srcid_{ srcid } {}


PointingVector::PointingVector( const PointingVector &other )
    : VieVS_Object( nextId++ ),
      staid_{ other.staid_ },
      srcid_{ other.srcid_ },
      az_{ other.az_ },
      el_{ other.el_ },
      ha_{ other.ha_ },
      dc_{ other.dc_ },
      time_{ other.time_ } {}


void PointingVector::copyValuesFromOtherPv( const PointingVector &other ) {
    staid_ = other.staid_;
    srcid_ = other.srcid_;
    az_ = other.az_;
    el_ = other.el_;
    ha_ = other.ha_;
    dc_ = other.dc_;
    time_ = other.time_;
}
