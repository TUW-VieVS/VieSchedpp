//
// Created by mschartner on 12/8/21.
//

#include "Antenna_ONSALA_VGOS.h"

using namespace VieVS;
using namespace std;

Antenna_ONSALA_VGOS::Antenna_ONSALA_VGOS( double offset_m, double diam_m, double rateAz_deg_per_min,
                                          unsigned int constantOverheadAz_s, double rateEl_deg_per_min,
                                          unsigned int constantOverheadEl_s )
    : AbstractAntenna( offset_m, diam_m, rateAz_deg_per_min, constantOverheadAz_s, rateEl_deg_per_min,
                       constantOverheadEl_s ) {}

unsigned int Antenna_ONSALA_VGOS::slewTime( const PointingVector& old_pointingVector,
                                            const PointingVector& new_pointingVector ) const noexcept {
    double delta_el = abs( old_pointingVector.getEl() - new_pointingVector.getEl() );

    unsigned int t_1 = calcSlewTime( old_pointingVector.getAz(), new_pointingVector.getAz() );
    unsigned int t_2 = slewTimePerAxis( delta_el, Axis::axis2 );

    return t_1 > t_2 ? t_1 : t_2;
}
unsigned int Antenna_ONSALA_VGOS::slewTimeTracking( const PointingVector& old_pointingVector,
                                                    const PointingVector& new_pointingVector ) const noexcept {
    double delta_el = abs( old_pointingVector.getEl() - new_pointingVector.getEl() );

    unsigned int t_1 = calcSlewTime( old_pointingVector.getAz(), new_pointingVector.getAz() ) -
                       static_cast<unsigned int>( lround( getCon1() ) );
    unsigned int t_2 = slewTimePerAxis( delta_el, Axis::axis2 ) - static_cast<unsigned int>( lround( getCon2() ) );

    return t_1 > t_2 ? t_1 : t_2;
}

std::string Antenna_ONSALA_VGOS::toVex( AbstractAntenna::Axis axis ) const {
    string str;
    if ( axis == Axis::axis1 ) {
        str = ( boost::format( "        antenna_motion = %3s: %3.0f deg/min: %3d sec: %5.2f deg/sec^2; * rate %.0f "
                               "deg/min for unwrapped azimuth < %.0f deg or > %.0f deg\n" ) %
                "az" % ( getRate1() * rad2deg * 60 ) % ( getCon1() ) % ( getRate1() * rad2deg ) %
                ( rate_slow_ * rad2deg * 60 ) % ( slow_unaz_lower_as_ * rad2deg ) % ( slow_unaz_higher_as_ * rad2deg ) )
                  .str();
    }
    if ( axis == Axis::axis2 ) {
        str += ( boost::format( "        antenna_motion = %3s: %3.0f deg/min: %3d sec: %5.2f deg/sec^2; \n" ) % "el" %
                 ( getRate2() * rad2deg * 60 ) % ( getCon2() ) % ( getRate2() * rad2deg ) )
                   .str();
    }
    return str;
}

unsigned int Antenna_ONSALA_VGOS::calcSlewTime( double az_start, double az_end ) const {
    double rate = getRate1();
    double constantOverhead = getCon1();

    if ( az_start > az_end ) {
        swap( az_start, az_end );
    }

    double t;
    if ( az_end < slow_unaz_lower_as_ ) {
        t = ( az_end - az_start ) / rate_slow_;
    } else if ( az_start < slow_unaz_lower_as_ && az_end > slow_unaz_higher_as_ ) {
        t = ( slow_unaz_lower_as_ - az_start ) / rate_slow_ + ( slow_unaz_higher_as_ - slow_unaz_lower_as_ ) / rate +
            ( az_end - slow_unaz_higher_as_ ) / rate_slow_;
    } else if ( az_start < slow_unaz_lower_as_ && az_end < slow_unaz_higher_as_ ) {
        t = ( slow_unaz_lower_as_ - az_start ) / rate_slow_ + ( az_end - slow_unaz_lower_as_ ) / rate;
    } else if ( az_start > slow_unaz_lower_as_ && az_end < slow_unaz_higher_as_ ) {
        t = ( az_end - az_start ) / rate;
    } else if ( az_start < slow_unaz_higher_as_ && az_end > slow_unaz_higher_as_ ) {
        t = ( slow_unaz_higher_as_ - az_start ) / rate + ( az_end - slow_unaz_higher_as_ ) / rate_slow_;
    } else if ( az_start > slow_unaz_higher_as_ ) {
        t = ( az_end - az_start ) / rate_slow_;
    } else {
        t = 99999.9;
    }

    return static_cast<unsigned int>( ceil( t + constantOverhead ) );
}
