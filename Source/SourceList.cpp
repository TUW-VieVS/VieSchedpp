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


#include "SourceList.h"
using namespace std;
using namespace VieVS;

unsigned long VieVS::SourceList::nextId = 0;

SourceList::SourceList( const SourceList& rhs ) : VieVS_Object( nextId++ ) {
    for ( const auto& any : rhs.getQuasars() ) {
        shared_ptr<Quasar> n = make_shared<Quasar>( *any );
        addQuasar( n );
    }
    for ( const auto& any : rhs.getSatellites() ) {
        shared_ptr<Satellite> n = make_shared<Satellite>( *any );
        addSatellite( n );
    }
    for ( const auto& any : rhs.getSpacecrafts() ) {
        shared_ptr<Spacecraft> n = make_shared<Spacecraft>( *any );
        addSpacecraft( n );
    }



    nsrc_ = rhs.nsrc_;
    nquasars_ = rhs.nquasars_;
    nsatellites_ = rhs.nsatellites_;
    nspacecrafts_ = rhs.nspacecrafts_;
}

SourceList::SourceList( SourceList&& rhs ) noexcept
    : VieVS_Object( nextId++ ),
      quasars_{ move( rhs.quasars_ ) },
      satellites_{ move( rhs.satellites_ ) },
spacecrafts_{ move(rhs.spacecrafts_) },
      sources_{ move( rhs.sources_ ) } {
    nsrc_ = rhs.nsrc_;
    nquasars_ = rhs.nquasars_;
    nsatellites_ = rhs.nsatellites_;
    nspacecrafts_= rhs.nspacecrafts_;
}

SourceList& SourceList::operator=( SourceList rhs ) {
    quasars_ = rhs.quasars_;
    satellites_ = rhs.satellites_;
    spacecrafts_ = rhs.spacecrafts_;
    sources_ = rhs.sources_;

    nsrc_ = rhs.nsrc_;
    nquasars_ = rhs.nquasars_;
    nsatellites_ = rhs.nsatellites_;
    nspacecrafts_ = rhs.nspacecrafts_;

    return *this;
}
