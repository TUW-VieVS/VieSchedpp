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

#include "StationEndposition.h"


using namespace std;
using namespace VieVS;

unsigned long StationEndposition::nextId = 0;


StationEndposition::StationEndposition( unsigned long nsta ) : VieVS_Object( nextId++ ) {
    stationAvailable_ = vector<char>( nsta, false );
    stationPossible_ = std::vector<char>( nsta, false );
    finalPosition_ = vector<boost::optional<PointingVector>>( nsta );

    // if there is no subcon the earliest scan start is set to zero to be save
    earliestScanStart_ = TimeSystem::duration;
}


void StationEndposition::addPointingVectorAsEndposition( const PointingVector &pv ) {
    unsigned long staid = pv.getStaid();

    // check if there is already an earlier endposition
    if ( finalPosition_[staid].is_initialized() ) {
        if ( pv.getTime() < finalPosition_[staid]->getTime() ) {
#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace )
                BOOST_LOG_TRIVIAL( trace ) << "set required endposition for station " << pv.getStaid();
#endif
            finalPosition_[staid] = pv;
        }
    } else {
#ifdef VIESCHEDPP_LOG
        if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "set required endposition for station " << pv.getStaid();
#endif
        finalPosition_[staid] = pv;
    }

    if ( pv.getTime() < earliestScanStart_ ) {
        earliestScanStart_ = pv.getTime();
    }
}


void StationEndposition::checkStationPossibility( const Station &thisStation ) {
    unsigned long staid = thisStation.getId();

    if ( !thisStation.getPARA().available ) {
        stationPossible_[staid] = false;
        return;
    }

    // get start time of this station
    unsigned int staStarttime = thisStation.getCurrentTime();

    // estimate end time of this session, if there is a endposition then take this time, otherwise earliest scan start
    // time
    unsigned int staEndtime = 0;
    if ( finalPosition_[staid].is_initialized() ) {
        staEndtime = finalPosition_[staid]->getTime();
    } else {
        staEndtime = earliestScanStart_;
    }

    // calculate available time
    unsigned int availableTime;
    if ( staEndtime > staStarttime ) {
        availableTime = staEndtime - staStarttime;
    } else {
        stationPossible_[staid] = false;
        return;
    }

    // calculate minimum required time. Assumtion: only 5 sec slew time, min scan time and no idle time.
    unsigned int requiredTime =
        thisStation.getPARA().systemDelay + thisStation.getPARA().preob + 5 + thisStation.getPARA().minScan;

    // determine if station is possible
    stationPossible_[staid] = availableTime > requiredTime;
}


unsigned int StationEndposition::requiredEndpositionTime( unsigned long staid, bool flag_rigorous ) const {
    // check if station has a required endposition, otherwise use earliest scan start.
    if ( finalPosition_[staid].is_initialized() ) {
        unsigned int finalPositionTime = finalPosition_[staid]->getTime();
        if ( !flag_rigorous && hugeOffset( staid ) ) {
            return earliestScanStart_;
        } else {
            return finalPositionTime;
        }
    } else {
        return earliestScanStart_;
    }
}


bool StationEndposition::hugeOffset( unsigned long staid ) const {
    unsigned int finalPositionTime = finalPosition_[staid]->getTime();
    return finalPositionTime - earliestScanStart_ > 1800;
}


bool StationEndposition::checkStationPossibility( const std::vector<Station> &stations ) {
    for ( const auto &any : stations ) {
        checkStationPossibility( any );
    }
    return count( stationPossible_.begin(), stationPossible_.end(), true ) >= 2;
}


std::set<unsigned long> StationEndposition::getObservedSources( unsigned int time,
                                                                const SourceList &sourceList ) const noexcept {
    set<unsigned long> obsSrc;

    for ( auto pv : finalPosition_ ) {
        if ( pv.is_initialized() ) {
            unsigned long srcid = pv->getSrcid();
            const auto &src = sourceList.getSource( srcid );
            unsigned int minRep = src->getPARA().minRepeat;
            unsigned int dT = util::absDiff( pv->getTime(), time );
            if ( dT < minRep ) {
                obsSrc.insert( srcid );
            }
        }
    }

    return std::move( obsSrc );
}


void StationEndposition::setStationAvailable( const std::vector<Station> &stations ) {
    for ( int i = 0; i < stations.size(); ++i ) {
        stationAvailable_[i] = stations[i].getPARA().available;
    }
}
