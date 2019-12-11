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
 * File:   Source.cpp
 * Author: mschartn
 *
 * Created on June 28, 2017, 11:25 AM
 */

#include "Source.h"


using namespace std;
using namespace VieVS;
unsigned long VieVS::Source::nextId = 0;
unsigned long VieVS::Source::Parameters::nextId = 0;


void Source::Parameters::setParameters( const Source::Parameters &other ) {
    available = other.available;
    globalAvailable = other.globalAvailable;
    availableForFillinmode = other.availableForFillinmode;

    weight = other.weight;

    minSNR = other.minSNR;

    minNumberOfStations = other.minNumberOfStations;
    minFlux = other.minFlux;
    minRepeat = other.minRepeat;
    maxScan = other.maxScan;
    minScan = other.minScan;
    maxNumberOfScans = other.maxNumberOfScans;
    minElevation = other.minElevation;
    minSunDistance = other.minSunDistance;

    tryToFocusIfObservedOnce = other.tryToFocusIfObservedOnce;
    tryToFocusFactor = other.tryToFocusFactor;
    tryToFocusOccurrency = other.tryToFocusOccurrency;
    tryToFocusType = other.tryToFocusType;

    tryToObserveXTimesEvenlyDistributed = other.tryToObserveXTimesEvenlyDistributed;
    tryToObserveXTimesMinRepeat = other.tryToObserveXTimesMinRepeat;

    fixedScanDuration = other.fixedScanDuration;

    ignoreStations = other.ignoreStations;
    ignoreBaselines = other.ignoreBaselines;
    requiredStations = other.requiredStations;
}


Source::Source( const string &src_name, const string &src_name2, double src_ra_deg, double src_de_deg,
                unordered_map<string, unique_ptr<AbstractFlux>> &src_flux )
    : VieVS_NamedObject( src_name, src_name2, nextId++ ),
      ra_{src_ra_deg * deg2rad},
      de_{src_de_deg * deg2rad},
      parameters_{Parameters( "empty" )} {
    flux_ = std::make_shared<std::unordered_map<std::string, std::unique_ptr<AbstractFlux>>>( std::move( src_flux ) );

    PreCalculated preCalculated = PreCalculated();
    preCalculated.sourceInCrs.resize( 3 );
    sinDe_ = sin( de_ );
    cosDe_ = cos( de_ );
    preCalculated.sourceInCrs[0] = cosDe_ * cos( ra_ );
    preCalculated.sourceInCrs[1] = cosDe_ * sin( ra_ );
    preCalculated.sourceInCrs[2] = sinDe_;

    preCalculated_ = make_shared<PreCalculated>( move( preCalculated ) );
    condition_ = make_shared<Optimization>( Optimization() );
}


double Source::getMaxFlux() const noexcept {
    double maxFlux = 0;

    for ( auto &any : *flux_ ) {
        double thisFlux = any.second->getMaximumFlux();
        if ( thisFlux > maxFlux ) {
            maxFlux = thisFlux;
        }
    }
    return maxFlux;
}


double Source::getSunDistance() const noexcept {
    double d = numeric_limits<double>::max();

    for ( int i = 0; i < AstronomicalParameters::sun_time.size(); ++i ) {
        double sunRa = AstronomicalParameters::sun_ra[i];
        double sunDe = AstronomicalParameters::sun_dec[i];
        double tmp = sin( sunDe ) * sinDe_ + cos( sunDe ) * cosDe_ * cos( sunRa - ra_ );
        tmp = acos( tmp );
        if ( tmp < d ) {
            d = tmp;
        }
    }

    return d;
}


double Source::observedFlux( const string &band, double gmst, const std::vector<double> &dxyz ) const noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "source " << this->getName() << " get observed flux density";
#endif

    std::pair<double, double> uv = calcUV( gmst, dxyz );
    double flux = flux_->at( band )->observedFlux( uv.first, uv.second );
    return flux;
}


std::pair<double, double> Source::calcUV( double gmst, const std::vector<double> &dxyz ) const noexcept {
    double ha = gmst - ra_;

    double sinHa = sin( ha );
    double cosHa = cos( ha );

    double u = dxyz[0] * sinHa + dxyz[1] * cosHa;
    double v = dxyz[2] * cosDe_ + sinDe_ * ( -dxyz[0] * cosHa + dxyz[1] * sinHa );
    return {u, v};
};


void Source::update( unsigned long nbl, unsigned int time, bool addToStatistics ) noexcept {
    if ( addToStatistics ) {
        ++nScans_;
        nObs_ += nbl;
        lastScan_ = time;
    }
    ++nTotalScans_;
}


bool Source::checkForNewEvent( unsigned int time, bool &hardBreak ) noexcept {
    bool flag = false;
    while ( events_ != nullptr && nextEvent_ < events_->size() && events_->at( nextEvent_ ).time <= time ) {
        double oldMinFlux = parameters_.minFlux;
        bool oldGlobalAvailable = parameters_.globalAvailable;
        parameters_ = events_->at( nextEvent_ ).PARA;
        parameters_.globalAvailable = oldGlobalAvailable;

        hardBreak = hardBreak || !events_->at( nextEvent_ ).smoothTransition;
        nextEvent_++;

        if ( getMaxFlux() < parameters_.minFlux ) {
            parameters_.available = false;
        }

        if ( getSunDistance() < parameters_.minSunDistance ) {
            parameters_.available = false;
        }

        flag = true;
    }
    return flag;
}


std::string Source::getRaString() const noexcept {
    double h = rad2deg * ra_ / 15;
    auto m = fmod( h, 1. );
    m *= 60;
    auto s = fmod( m, 1. );
    s *= 60;
    string str = ( boost::format( "%02dh%02dm%08.5fs" ) % static_cast<int>( h ) % static_cast<int>( m ) % s ).str();
    return str;
}


std::string Source::getDeString() const noexcept {
    double d = rad2deg * de_;
    auto m = abs( fmod( d, 1. ) );
    m *= 60;
    auto s = fmod( m, 1. );
    s *= 60;
    string str = ( boost::format( "%+03dd%02d'%08.5f\"" ) % static_cast<int>( d ) % static_cast<int>( m ) % s ).str();
    return str;
}


void Source::clearObservations() {
    lastScan_ = 0;
    nScans_ = 0;
    nTotalScans_ = 0;
    nObs_ = 0;

    bool hardBreak = false;
    nextEvent_ = 0;
    checkForNewEvent( 0, hardBreak );
}

// Source Source::clone() const{
//    std::unordered_map<string,unique_ptr<Flux> > newFlux;
//
//    for(const auto &any: flux_){
//        newFlux[any.first] = any.second->clone();
//    }
//
//    return Source(name_,alternativeName_,ra_*rad2deg, de_*rad2deg, newFlux);
//}

double Source::observedFlux_model( double wavelength, double gmst, const std::vector<double> &dxyz ) const {
    std::pair<double, double> uv = calcUV( gmst, dxyz );

    // first flux
    auto it = flux_->begin();
    double wl1 = it->second->getWavelength();
    double flux1 = it->second->observedFlux( uv.first, uv.second );
    // second flux
    ++it;
    double wl2 = it->second->getWavelength();
    double flux2 = it->second->observedFlux( uv.first, uv.second );

    // solve for alpha and K
    double alpha = log( flux1 / flux2 ) / log( wl1 / wl2 );
    double K = flux1 / pow( wl1, alpha );

    // calculate observed flux density
    double observedFlux = K * pow( wavelength, alpha );
    return observedFlux;
}