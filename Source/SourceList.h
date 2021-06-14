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

/**
 * @file SourceList.h
 * @brief class SourceList
 *
 * @author Matthias Schartner
 * @date 29.07.2020
 */

#ifndef VIESCHEDPP_SOURCELIST_H
#define VIESCHEDPP_SOURCELIST_H

#include "../Misc/VieVS_Object.h"
#include "AbstractSource.h"
#include "Quasar.h"
#include "Satellite.h"

namespace VieVS {

/**
 * @class SourceList
 * @brief representation of a SourceList
 *
 * @author Matthias Schartner
 * @date 29.07.2020
 */

class SourceList : public VieVS_Object {
   public:
    SourceList() : VieVS_Object( nextId++ ){};

    SourceList( const SourceList& rhs );

    SourceList( SourceList&& rhs ) noexcept;

    SourceList& operator=( SourceList rhs );

    void addQuasar( const std::shared_ptr<Quasar>& q ) {
        if ( q->getId() != sources_.size() ) {
            q->setId( sources_.size() );
        }
        sources_.push_back( q );
        quasars_.push_back( q );
        ++nquasars_;
        ++nsrc_;
    }

    void addSatellite( std::shared_ptr<Satellite> s ) {
        if ( s->getId() != sources_.size() ) {
            s->setId( sources_.size() );
        }
        sources_.push_back( s );
        satellites_.push_back( std::move( s ) );
        ++nsatellites_;
        ++nsrc_;
    }

    const std::vector<std::shared_ptr<AbstractSource>>& refSources() { return sources_; };

    std::vector<std::shared_ptr<const AbstractSource>> getSources() const {
        return std::vector<std::shared_ptr<const AbstractSource>>{ sources_.cbegin(), sources_.cend() };
    }

    const std::vector<std::shared_ptr<Quasar>>& refQuasars() { return quasars_; };

    std::vector<std::shared_ptr<const Quasar>> getQuasars() const {
        return std::vector<std::shared_ptr<const Quasar>>{ quasars_.cbegin(), quasars_.cend() };
    }

    const std::vector<std::shared_ptr<Satellite>>& refSatellites() { return satellites_; };

    std::vector<std::shared_ptr<const Satellite>> getSatellites() const {
        return std::vector<std::shared_ptr<const Satellite>>{ satellites_.cbegin(), satellites_.cend() };
    }

    unsigned long getNSrc() const { return nsrc_; };
    unsigned long getNQuasars() const { return nquasars_; };
    unsigned long getNSatellites() const { return nsatellites_; };

    bool empty() const { return nsrc_ == 0; }

    std::shared_ptr<const AbstractSource> getSource( unsigned long id ) const { return sources_[id]; }
    std::shared_ptr<AbstractSource> refSource( unsigned long id ) { return sources_[id]; }

    std::shared_ptr<const Quasar> getQuasar( unsigned long idx ) const { return quasars_[idx]; }
    std::shared_ptr<Quasar> refQuasar( unsigned long idx ) { return quasars_[idx]; }

    std::shared_ptr<const Satellite> getSatellite( unsigned long idx ) const { return satellites_[idx]; }
    std::shared_ptr<Satellite> refSatellite( unsigned long idx ) { return satellites_[idx]; }

    bool isQuasar( unsigned long id ) const { return id < nquasars_; }
    bool isSatellite( unsigned long id ) const { return id >= nquasars_ && id < nquasars_ + nsatellites_; }

   private:
    static unsigned long nextId;  ///< next id for this object type

    std::vector<std::shared_ptr<AbstractSource>> sources_;
    std::vector<std::shared_ptr<Quasar>> quasars_;
    std::vector<std::shared_ptr<Satellite>> satellites_;

    unsigned long nsrc_ = 0;
    unsigned long nquasars_ = 0;
    unsigned long nsatellites_ = 0;
};
}  // namespace VieVS

#endif  // VIESCHEDPP_SOURCELIST_H
