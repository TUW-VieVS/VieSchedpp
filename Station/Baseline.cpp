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
 * File:   baseline.cpp
 * Author: mschartn
 *
 * Created on June 29, 2017, 3:25 PM
 */

#include "Baseline.h"


using namespace std;
using namespace VieVS;

unsigned long Baseline::nextId = 0;
unsigned long VieVS::Baseline::Parameters::nextId = 0;


void Baseline::Parameters::setParameters( const Baseline::Parameters &other ) {
    ignore = other.ignore;
    weight = other.weight;
    minSNR = other.minSNR;
}


Baseline::Baseline( std::string name, std::string alternativeName, unsigned long staid1, unsigned long staid2 )
    : VieVS_NamedObject( move( name ), move( alternativeName ), nextId++ ),
      staid1_{staid1},
      staid2_{staid2},
      parameters_{Parameters( "empty" )} {}


bool Baseline::hasStationIds( unsigned long staid1, unsigned long staid2 ) const noexcept {
    if ( staid1 > staid2 ) {
        swap( staid1, staid2 );
    }

    return staid1 == staid1_ && staid2 == staid2_;
}


bool Baseline::hasStationIds( const std::pair<unsigned long, unsigned long> &staids ) const noexcept {
    return hasStationIds( staids.first, staids.second );
}


bool Baseline::checkForNewEvent( unsigned int time, bool &hardBreak ) noexcept {
    bool flag = false;
    while ( nextEvent_ < events_->size() && events_->at( nextEvent_ ).time <= time ) {
        parameters_ = events_->at( nextEvent_ ).PARA;

        hardBreak = hardBreak || !events_->at( nextEvent_ ).smoothTransition;

        nextEvent_++;
        flag = true;
    }
    return flag;
}


void Baseline::update( bool influence ) noexcept {
    if ( influence ) {
        ++nObs_;
        ++nTotalObs_;
    } else {
        ++nTotalObs_;
    }
}
