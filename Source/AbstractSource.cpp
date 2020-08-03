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

#include "AbstractSource.h"


using namespace std;
using namespace VieVS;
unsigned long VieVS::AbstractSource::nextId = 0;
unsigned long VieVS::AbstractSource::Parameters::nextId = 0;


void AbstractSource::Parameters::setParameters( const AbstractSource::Parameters &other ) {
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


AbstractSource::AbstractSource( const string &src_name, const string &src_name2,
                                unordered_map<string, unique_ptr<AbstractFlux>> &src_flux )
    : VieVS_NamedObject( src_name, src_name2, nextId++ ), parameters_{ Parameters( "empty" ) } {
    flux_ = std::make_shared<std::unordered_map<std::string, std::unique_ptr<AbstractFlux>>>( std::move( src_flux ) );

    condition_ = make_shared<Optimization>( Optimization() );
}


double AbstractSource::getMaxFlux() const noexcept {
    double maxFlux = 0;

    for ( auto &any : *flux_ ) {
        double thisFlux = any.second->getMaximumFlux();
        if ( thisFlux > maxFlux ) {
            maxFlux = thisFlux;
        }
    }
    return maxFlux;
}


double AbstractSource::getSunDistance( unsigned int time,
                                       const std::shared_ptr<const Position> &sta_pos ) const noexcept {
    unsigned int precalc_idx = 0;
    while ( AstronomicalParameters::sun_time[precalc_idx + 1] < time ) {
        ++precalc_idx;
    }
    unsigned int delta = AstronomicalParameters::sun_time[1] - AstronomicalParameters::sun_time[0];

    unsigned int deltaTime = time - AstronomicalParameters::sun_time[precalc_idx];

    double sunRa = AstronomicalParameters::sun_ra[precalc_idx] +
                   ( AstronomicalParameters::sun_ra[precalc_idx + 1] - AstronomicalParameters::sun_ra[precalc_idx] ) /
                       delta * deltaTime;
    double sunDe = AstronomicalParameters::sun_dec[precalc_idx] +
                   ( AstronomicalParameters::sun_dec[precalc_idx + 1] - AstronomicalParameters::sun_dec[precalc_idx] ) /
                       delta * deltaTime;

    auto srcRaDe = getRaDe( time, sta_pos );
    double tmp =
        sin( sunDe ) * sin( srcRaDe.second ) + cos( sunDe ) * cos( srcRaDe.second ) * cos( sunRa - srcRaDe.first );
    tmp = acos( tmp );
    return tmp;
}


double AbstractSource::observedFlux( const string &band, unsigned int time, double gmst,
                                     const std::vector<double> &dxyz ) const noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "source " << this->getName() << " get observed flux density";
#endif

    std::pair<double, double> uv = calcUV( time, gmst, dxyz );
    double flux = flux_->at( band )->observedFlux( uv.first, uv.second );
    return flux;
}


std::pair<double, double> AbstractSource::calcUV( unsigned int time, double gmst,
                                                  const std::vector<double> &dxyz ) const noexcept {
    auto srcRaDe = getRaDe( time, nullptr );
    double ha = gmst - srcRaDe.first;
    double de = srcRaDe.second;

    double sinHa = sin( ha );
    double cosHa = cos( ha );
    double cosDe = cos( de );
    double sinDe = sin( de );


    double u = dxyz[0] * sinHa + dxyz[1] * cosHa;
    double v = dxyz[2] * cosDe + sinDe * ( -dxyz[0] * cosHa + dxyz[1] * sinHa );
    return { u, v };
};


void AbstractSource::update( unsigned long nbl, unsigned int time, bool addToStatistics ) noexcept {
    if ( addToStatistics ) {
        ++nScans_;
        nObs_ += nbl;
        lastScan_ = time;
    }
    ++nTotalScans_;
}


bool AbstractSource::checkForNewEvent( unsigned int time, bool &hardBreak ) noexcept {
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

        flag = true;
    }
    return flag;
}


std::string AbstractSource::getRaString( double ang ) const noexcept {
    double h = rad2deg * ang / 15;
    auto m = fmod( h, 1. );
    m *= 60;
    auto s = fmod( m, 1. );
    s *= 60;
    string str = ( boost::format( "%02dh%02dm%08.5fs" ) % static_cast<int>( h ) % static_cast<int>( m ) % s ).str();
    return str;
}


std::string AbstractSource::getDeString( double ang ) const noexcept {
    double d = rad2deg * ang;
    auto m = abs( fmod( d, 1. ) );
    m *= 60;
    auto s = fmod( m, 1. );
    s *= 60;
    string str = ( boost::format( "%+03dd%02d'%08.5f\"" ) % static_cast<int>( d ) % static_cast<int>( m ) % s ).str();
    return str;
}


void AbstractSource::clearObservations() {
    lastScan_ = 0;
    nScans_ = 0;
    nTotalScans_ = 0;
    nObs_ = 0;

    bool hardBreak = false;
    nextEvent_ = 0;
    checkForNewEvent( 0, hardBreak );
}


double AbstractSource::observedFlux_model( double wavelength, unsigned int time, double gmst,
                                           const std::vector<double> &dxyz ) const {
    std::pair<double, double> uv = calcUV( time, gmst, dxyz );

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