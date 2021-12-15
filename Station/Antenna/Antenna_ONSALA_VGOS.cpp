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
    unsigned int t_1 = az.slew_time( old_pointingVector.getAz(), new_pointingVector.getAz() ) +
                       static_cast<unsigned int>( lround( getCon1() ) );
    unsigned int t_2 = el.slew_time( old_pointingVector.getEl(), new_pointingVector.getEl() ) +
                       static_cast<unsigned int>( lround( getCon2() ) );

    return t_1 > t_2 ? t_1 : t_2;
}
unsigned int Antenna_ONSALA_VGOS::slewTimeTracking( const PointingVector& old_pointingVector,
                                                    const PointingVector& new_pointingVector ) const noexcept {
    unsigned int t_1 = az.slew_time( old_pointingVector.getAz(), new_pointingVector.getAz() );
    unsigned int t_2 = el.slew_time( old_pointingVector.getEl(), new_pointingVector.getEl() );
    return t_1 > t_2 ? t_1 : t_2;
}

std::string Antenna_ONSALA_VGOS::toVex( AbstractAntenna::Axis axis ) const {
    string str;
    if ( axis == Axis::axis1 ) {
        str = ( boost::format( "        antenna_motion = %3s: %3.0f deg/min: %3d sec: %5.2f deg/sec^2; * reduced slew "
                               "rate close to wrap limits\n" ) %
                "az" % ( getRate1() * rad2deg * 60 ) % ( getCon1() ) % ( getRate1() * rad2deg ) )
                  .str();
    }
    if ( axis == Axis::axis2 ) {
        str += ( boost::format( "        antenna_motion = %3s: %3.0f deg/min: %3d sec: %5.2f deg/sec^2; * reduced slew "
                                "rate close to horizon and zenith\n" ) %
                 "el" % ( getRate2() * rad2deg * 60 ) % ( getCon2() ) % ( getRate2() * rad2deg ) )
                   .str();
    }
    return str;
}

unsigned int Antenna_ONSALA_VGOS::rate::slew_time( double start, double end ) const {
    double A = slew_time_A( start, end );
    double B = slew_time_B( start, end );
    double C = slew_time_C( start, end );
    double D = slew_time_D( start, end );
    double E = slew_time_E( start, end );
    double sum = A + B + C + D + E;
    return static_cast<unsigned int>( sum );
}

double Antenna_ONSALA_VGOS::rate::slew_time_A( double start, double end ) const {
    if ( start <= very_slow_lower && end <= very_slow_lower ) {
        double delta = abs( end - start );
        if ( end < start ) {
            return delta / very_slow_rate;
        } else {
            return delta / normal_rate;
        }
    }
    if ( start < very_slow_lower ) {
        double delta = very_slow_lower - start;
        return delta / normal_rate;
    }
    if ( end < very_slow_lower ) {
        double delta = very_slow_lower - end;
        return delta / very_slow_rate;
    }
    return 0.0;
}

double Antenna_ONSALA_VGOS::rate::slew_time_E( double start, double end ) const {
    if ( start >= very_slow_upper && end >= very_slow_upper ) {
        double delta = abs( end - start );
        if ( start < end ) {
            return delta / very_slow_rate;
        } else {
            return delta / normal_rate;
        }
    }
    if ( start >= very_slow_upper ) {
        double delta = start - very_slow_upper;
        return delta / normal_rate;
    }
    if ( end >= very_slow_upper ) {
        double delta = end - very_slow_upper;
        return delta / very_slow_rate;
    }
    return 0.0;
}

double Antenna_ONSALA_VGOS::rate::slew_time_B( double start, double end ) const {
    if ( ( start >= slow_lower && end >= slow_lower ) || ( start <= very_slow_lower && end <= very_slow_lower ) ) {
        return 0.0;
    }

    if ( end < start ) {
        double rate = slow_rate;
        double delta = min( start, slow_lower ) - max( very_slow_lower, end );
        return delta / rate;
    } else {
        double rate = normal_rate;
        double delta = min( slow_lower, end ) - max( start, very_slow_lower );
        return delta / rate;
    }
}

double Antenna_ONSALA_VGOS::rate::slew_time_D( double start, double end ) const {
    if ( ( start <= slow_upper && end <= slow_upper ) || ( start >= very_slow_upper && end >= very_slow_upper ) ) {
        return 0.0;
    }

    if ( end > start ) {
        double rate = slow_rate;
        double delta = min( very_slow_upper, end ) - max( start, slow_upper );
        return delta / rate;
    } else {
        double rate = normal_rate;
        double delta = min( very_slow_upper, start ) - max( end, slow_upper );
        return delta / rate;
    }
}

double Antenna_ONSALA_VGOS::rate::slew_time_C( double start, double end ) const {
    if ( ( start <= slow_lower && end <= slow_lower ) || ( start >= slow_upper && end >= slow_upper ) ) {
        return 0.0;
    }

    // full slew through C section (unlikely)
    if ( ( start < slow_lower && end > slow_upper ) || ( start > slow_upper && end < slow_lower ) ) {
        double delta = slow_upper - slow_lower;
        return delta / normal_rate;
    } else {
        if ( start > end ) {
            swap( start, end );
        }
        double delta = min( end, slow_upper ) - max( start, slow_lower );
        return delta / normal_rate;
    }
}
