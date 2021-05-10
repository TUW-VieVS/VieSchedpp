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

#include "Network.h"


using namespace VieVS;
using namespace std;

unsigned long VieVS::Network::nextId = 0;


Network::Network() : VieVS_Object( nextId++ ) {
    nsta_ = 0;
    nbls_ = 0;
    maxDistBetweenCorrespondingTelescopes_ = 0;
}


void Network::addStation( Station station ) {
    if ( station.getId() != stations_.size() ) {
        station.setId( stations_.size() );
    }

    bool skyCoveragFound = false;
    for ( const auto &any : stations_ ) {
        // create baseline
        string name = ( boost::format( "%s-%s" ) % any.getAlternativeName() % station.getAlternativeName() ).str();
        string alternativeName =
            ( boost::format( "%s-%s" ) % station.getAlternativeName() % any.getAlternativeName() ).str();
        Baseline bl( name, alternativeName, any.getId(), station.getId() );
        if ( bl.getId() != baselines_.size() ) {
            bl.setId( baselines_.size() );
        }
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug )
            BOOST_LOG_TRIVIAL( debug ) << "Baseline " << bl.getName() << " successfully created " << bl.printId();
#endif
        baselines_.push_back( std::move( bl ) );
        staids2blid_[{ any.getId(), station.getId() }] = baselines_.back().getId();

        // create delta xyz
        double dx = any.getPosition()->getX() - station.getPosition()->getX();
        double dy = any.getPosition()->getY() - station.getPosition()->getY();
        double dz = any.getPosition()->getZ() - station.getPosition()->getZ();
        staids2dxyz_[{ any.getId(), station.getId() }] = { dx, dy, dz };

        // add to existing sky coverage object if possible
        double dist = sqrt( dx * dx + dy * dy + dz * dz );
        if ( dist <= maxDistBetweenCorrespondingTelescopes_ ) {
            staids2skyCoverageId_[station.getId()] = staids2skyCoverageId_[any.getId()];
            skyCoveragFound = true;
        }
    }
    // if necessary create new sky coverage object
    if ( !skyCoveragFound ) {
        staids2skyCoverageId_[station.getId()] = skyCoverages_.size();
        SkyCoverage skyCoverage;

        if ( skyCoverage.getId() != skyCoverages_.size() ) {
            skyCoverage.setId( skyCoverages_.size() );
        }

        skyCoverages_.push_back( std::move( skyCoverage ) );
    }

    // finally push back station
    stations_.push_back( std::move( station ) );

    nsta_ = stations_.size();
    nclosures_max_ = (nsta_-1)*(nsta_-2)/2 + nsta_ * (nsta_ -3) / 2;
    nbls_ = ( nsta_ * ( nsta_ - 1 ) ) / 2;
}


const Station &Network::getStation( unsigned long id ) const noexcept { return stations_[id]; }


const Station &Network::getStation( const std::string &name ) const noexcept {
    for ( const auto &any : stations_ ) {
        if ( any.hasName( name ) ) {
            return any;
        }
    }
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(fatal) << "This code should never be reached! Network::getStation( const std::string &name )";
#else
    cout << "This code should never be reached! Network::getStation( const std::string &name )";
#endif
    terminate();
}


const std::vector<Station> &Network::getStations() const noexcept { return stations_; }


const Baseline &Network::getBaseline( unsigned long id ) const noexcept { return baselines_[id]; }


const Baseline &Network::getBaseline( unsigned long staid1, unsigned long staid2 ) const noexcept {
    if ( staid1 > staid2 ) {
        swap( staid1, staid2 );
    }

    unsigned long blid = staids2blid_.at( { staid1, staid2 } );
    return baselines_[blid];
}


const Baseline &Network::getBaseline( const std::pair<unsigned long, unsigned long> &staids ) const noexcept {
    return getBaseline( staids.first, staids.second );
}


const Baseline &Network::getBaseline( const std::string &name ) const noexcept {
    for ( const auto &any : baselines_ ) {
        if ( any.hasName( name ) ) {
            return any;
        }
    }
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(fatal) << "This code should never be reached! Network::getBaseline( const std::string &name )";
#else
    cout << "This code should never be reached! Network::getBaseline( const std::string &name )";
#endif
    terminate();
}


const std::vector<Baseline> &Network::getBaselines() const noexcept { return baselines_; }


const SkyCoverage &Network::getSkyCoverage( unsigned long id ) const noexcept { return skyCoverages_[id]; }


const std::vector<SkyCoverage> &Network::getSkyCoverages() const noexcept { return skyCoverages_; }


Station &Network::refStation( unsigned long id ) { return stations_[id]; }


Station &Network::refStation( const std::string &name ) {
    for ( auto &any : stations_ ) {
        if ( any.hasName( name ) ) {
            return any;
        }
    }
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(fatal) << "This code should never be reached! Network::refStation( const std::string &name )";
#else
    cout << "This code should never be reached! Network::refStation( const std::string &name )";
#endif
    terminate();
}


std::vector<Station> &Network::refStations() { return stations_; }


Baseline &Network::refBaseline( unsigned long id ) { return baselines_[id]; }


Baseline &Network::refBaseline( unsigned long staid1, unsigned long staid2 ) {
    if ( staid1 > staid2 ) {
        swap( staid1, staid2 );
    }

    return baselines_[staids2blid_[{ staid1, staid2 }]];
}


Baseline &Network::refBaseline( const std::pair<unsigned long, unsigned long> &staids ) {
    return refBaseline( staids.first, staids.second );
}


Baseline &Network::refBaseline( const std::string &name ) {
    for ( auto &any : baselines_ ) {
        if ( any.hasName( name ) ) {
            return any;
        }
    }
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(fatal) << "This code should never be reached! Network::refBaseline( const std::string &name )";
#else
    cout << "This code should never be reached! Network::refBaseline( const std::string &name )";
#endif
    terminate();
}


std::vector<Baseline> &Network::refBaselines() { return baselines_; }


unsigned long Network::getBlid( unsigned long staid1, unsigned long staid2 ) const noexcept {
    return getBaseline( staid1, staid2 ).getId();
}


unsigned long Network::getBlid( const std::pair<unsigned long, unsigned long> &staids ) const noexcept {
    return getBaseline( staids.first, staids.second ).getId();
}


SkyCoverage &Network::refSkyCoverage( unsigned long id ) { return skyCoverages_[id]; }


std::vector<SkyCoverage> &Network::refSkyCoverages() { return skyCoverages_; }


void Network::update( unsigned long nObs, const PointingVector &pointingVector, bool influence ) {
    unsigned long staid = pointingVector.getStaid();

    stations_[staid].update( nObs, pointingVector, influence );

    if ( influence ) {
        unsigned long skyCoverageId = staids2skyCoverageId_[staid];
        skyCoverages_[skyCoverageId].update( pointingVector );
    }
}


void Network::update( unsigned long blid, bool influence ) { baselines_[blid].update( influence ); }


const std::vector<double> &Network::getDxyz( unsigned long staid1, unsigned long staid2 ) const {
    if ( staid1 > staid2 ) {
        swap( staid1, staid2 );
    }

    return staids2dxyz_.at( { staid1, staid2 } );
}


double Network::calcScore_skyCoverage( const vector<PointingVector> &pvs ) const {
    double score = 0;

    for ( const auto &pv : pvs ) {
        unsigned long staid = pv.getStaid();
        unsigned long skyCovid = staids2skyCoverageId_.at( staid );
        const SkyCoverage &thisSkyCov = skyCoverages_.at( skyCovid );
        score += thisSkyCov.calcScore( pv );
    }

    return score / nsta_;
}


double Network::calcScore_skyCoverage( const vector<PointingVector> &pvs,
                                       unordered_map<unsigned long, double> &staids2skyCoverageScore ) const {
    double score = 0;

    for ( const auto &pv : pvs ) {
        unsigned long staid = pv.getStaid();
        unsigned long skyCovid = staids2skyCoverageId_.at( staid );
        const SkyCoverage &thisSkyCov = skyCoverages_.at( skyCovid );
        double thisScore = thisSkyCov.calcScore( pv );
        staids2skyCoverageScore[staid] = thisScore;
        score += thisScore;
    }

    return score / nsta_;
}


double Network::calcScore_skyCoverage_subnetting(
    const vector<PointingVector> &pvs, const unordered_map<unsigned long, double> &staids2skyCoverageScore ) const {
    double score = 0;

    for ( const auto &pv : pvs ) {
        unsigned long staid = pv.getStaid();
        score += staids2skyCoverageScore.at( staid );
    }

    return score / nsta_;
}


void Network::stationSummary( ofstream &of ) const {
    const auto &snr = ObservingMode::minSNR;
    vector<string> bands;
    for ( auto const &element : snr ) {
        bands.push_back( element.first );
    }


    of << boost::format( "%8s  %2s  %5s  %5s   " ) % "name" % "ID" % "Mount" % "Diam";
    for ( const auto &any : bands ) {
        of << boost::format( "%7s %7s %7s %7s   " ) % ( "SEFD_" + any ) % ( "y_" + any ) % ( "c0_" + any ) %
                  ( "c1_" + any );
    }
    of << boost::format( "   %7s %7s   %6s %6s %6s %6s\n" ) % "lat" % "lon" % "rate1" % "c1" % "rate2" % "c2";


    for ( const auto &sta : stations_ ) {
        of << boost::format( "%8s  %2s  %5s  %5.1f   " ) % sta.getName() % sta.getAlternativeName() %
                  sta.getAntenna().getMount() % sta.getAntenna().getDiam();
        for ( const auto &band : bands ) {
            of << sta.getEquip().shortSummary( band );
            of << "   ";
        }
        of << boost::format( "   %7.2f %7.2f   %6.0f %6.0f %6.0f %6.0f\n" ) %
                  ( sta.getPosition()->getLat() * rad2deg ) % ( sta.getPosition()->getLon() * rad2deg ) %
                  ( sta.getAntenna().getRate1() * rad2deg * 60 ) % sta.getAntenna().getCon1() %
                  ( sta.getAntenna().getRate2() * rad2deg * 60 ) % sta.getAntenna().getCon2();
    }
    of << "\n";
}
