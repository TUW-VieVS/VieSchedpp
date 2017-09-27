//
// Created by mschartn on 22.08.17.
//

#include "TimeSystem.h"

thread_local double VieVS::TimeSystem::mjdStart = 0;
thread_local boost::posix_time::ptime VieVS::TimeSystem::startTime = boost::posix_time::ptime();
thread_local boost::posix_time::ptime VieVS::TimeSystem::endTime = boost::posix_time::ptime();
unsigned int VieVS::TimeSystem::duration = 0;

double VieVS::TimeSystem::mjd2gmst(double mjd) {

    double fmjd = floor(mjd);

    double T = (fmjd - 51544.5)/36525;

    double hhs = (mjd - fmjd) * 24;
    constexpr double p1 = 6*3600 + 41*60 + 50.54841;
    double p2 = 8640184.812866;
    double p3 = 0.093104;

    double theta = ( p1 + (p2 + p3*T )*T ) / 3600;

    theta = fmod(theta, 24.0);

    theta = theta * 15 * deg2rad;

    constexpr double tmp = 366.2422/365.2422*15*deg2rad;
    double st = hhs * tmp;

    theta = theta + st;

    return fmod(theta,twopi);
}

