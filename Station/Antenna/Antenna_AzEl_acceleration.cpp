//
// Created by mschartner on 12/3/21.
//

#include "Antenna_AzEl_acceleration.h"
using namespace VieVS;
using namespace std;

Antenna_AzEl_acceleration::Antenna_AzEl_acceleration( double offset_m, double diam_m, double az_rate_deg_per_sec,
                                                      double az_acceleration_deg_per_sec_sec,
                                                      double az_deceleration_deg_per_sec_sec, unsigned int az_settle_s,
                                                      double el_rate_deg_per_sec,
                                                      double el_acceleration_deg_per_sec_sec,
                                                      double el_deceleration_deg_per_sec_sec, unsigned int el_settle_s )
    : AbstractAntenna( offset_m, diam_m, az_rate_deg_per_sec * 60, az_settle_s, el_rate_deg_per_sec * 60, el_settle_s ),
      az_acelleration{ az_acceleration_deg_per_sec_sec * deg2rad },
      az_deceleration{ az_deceleration_deg_per_sec_sec * deg2rad },
      el_acelleration{ el_acceleration_deg_per_sec_sec * deg2rad },
      el_deceleration{ el_deceleration_deg_per_sec_sec * deg2rad } {}

unsigned int Antenna_AzEl_acceleration::slewTime( const VieVS::PointingVector& old_pointingVector,
                                                  const VieVS::PointingVector& new_pointingVector ) const noexcept {
    double delta1 = abs( old_pointingVector.getAz() - new_pointingVector.getAz() );
    double delta2 = abs( old_pointingVector.getEl() - new_pointingVector.getEl() );

    unsigned int t_1 = calc_slew_times( delta1, getRate1(), az_acelleration, az_deceleration, getCon1() );
    unsigned int t_2 = calc_slew_times( delta2, getRate2(), el_acelleration, el_deceleration, getCon2() );

    return t_1 > t_2 ? t_1 : t_2;
}


unsigned int Antenna_AzEl_acceleration::slewTimeTracking(
    const VieVS::PointingVector& old_pointingVector, const VieVS::PointingVector& new_pointingVector ) const noexcept {
    double delta1 = abs( old_pointingVector.getAz() - new_pointingVector.getAz() );
    double delta2 = abs( old_pointingVector.getEl() - new_pointingVector.getEl() );

    unsigned int t_1 = calc_slew_times( delta1, getRate1(), az_acelleration, az_deceleration, 0.0 );
    unsigned int t_2 = calc_slew_times( delta2, getRate2(), el_acelleration, el_deceleration, 0.0 );

    return t_1 > t_2 ? t_1 : t_2;
}


unsigned int Antenna_AzEl_acceleration::calc_slew_times( double delta, double rate, double acceleration,
                                                         double deceleration, double settle ) {
    double time_acceleration = rate / acceleration;
    double time_deceleration = rate / deceleration;
    double distance_acceleration = acceleration * time_acceleration * time_acceleration / 2;
    double distance_deceleration = deceleration * time_deceleration * time_deceleration / 2;

    double t;
    if ( delta < distance_acceleration + distance_deceleration ) {
        double t1 = ( sqrt( 2 ) * sqrt( deceleration ) * sqrt( delta ) ) /
                    ( sqrt( acceleration * ( acceleration + deceleration ) ) );
        double t2 = ( sqrt( 2 ) * acceleration * sqrt( delta ) ) /
                    ( sqrt( deceleration ) * sqrt( acceleration * ( acceleration + deceleration ) ) );
        //        double v1 = t1 * acceleration;
        //        double v2 = t2 * deceleration;
        //        double dist = acceleration * t1 * t1 / 2 + deceleration * t2 * t2 / 2;
        t = t1 + t2 + settle;
    } else {
        t = time_acceleration + time_deceleration + ( delta - distance_acceleration - distance_deceleration ) / rate +
            settle;
    }
    return static_cast<unsigned int>( ceil( t ) );
}

std::string Antenna_AzEl_acceleration::toVex( Axis axis ) const {
    string str;
    if ( axis == Axis::axis1 ) {
        str = ( boost::format( "        antenna_motion = %3s: %3.0f deg/min: %3d sec: %5.2f deg/sec^2; * deceleration "
                               "%5.2f deg/sec^2 \n" ) %
                "az" % ( getRate1() * rad2deg * 60 ) % ( getCon1() ) % ( az_acelleration * rad2deg ) %
                ( az_deceleration * rad2deg ) )
                  .str();
    }
    if ( axis == Axis::axis1 ) {
        str = ( boost::format( "        antenna_motion = %3s: %3.0f deg/min: %3d sec: %5.2f deg/sec^2; * deceleration "
                               "%5.2f deg/sec^2 \n" ) %
                "el" % ( getRate2() * rad2deg * 60 ) % ( getCon2() ) % ( el_acelleration * rad2deg ) %
                ( el_deceleration * rad2deg ) )
                  .str();
    }
    return str;
}
