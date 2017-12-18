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

std::string VieVS::TimeSystem::ptime2string(boost::posix_time::ptime ptime) {
    int month = ptime.date().month();
    std::string dateStr = (boost::format("%04d.%02d.%02d %02d:%02d:%02d")
                           % ptime.date().year() %month %ptime.date().day()
                           % ptime.time_of_day().hours() %ptime.time_of_day().minutes() %ptime.time_of_day().seconds()).str();
    return dateStr;
}

boost::posix_time::ptime VieVS::TimeSystem::string2ptime(std::string timeStr) {
    boost::posix_time::ptime ptime;
    auto year = boost::lexical_cast<unsigned short>(timeStr.substr(0,4));
    auto month = boost::lexical_cast<unsigned short>(timeStr.substr(5,2));
    auto day = boost::lexical_cast<unsigned short>(timeStr.substr(8,2));
    auto hour = boost::lexical_cast<int>(timeStr.substr(11,2));
    auto minute = boost::lexical_cast<int>(timeStr.substr(14,2));
    auto second = boost::lexical_cast<int>(timeStr.substr(17,2));
    ptime = boost::posix_time::ptime(boost::gregorian::date(year,month,day),boost::posix_time::time_duration(hour,minute,second));
    return ptime;
}

std::string VieVS::TimeSystem::date2string(boost::posix_time::ptime ptime) {
    std::string dateStr = (boost::format("%04d%3s%02d")
                           % ptime.date().year() %ptime.date().month() %ptime.date().day()).str();
    dateStr = boost::to_upper_copy(dateStr);
    return dateStr;
}

std::string VieVS::TimeSystem::ptime2string_doy(boost::posix_time::ptime ptime) {
    std::string dateStr = (boost::format("%04d%03d%02d%02d%02d") % ptime.date().year() % ptime.date().day_of_year() %
                            ptime.time_of_day().hours() % ptime.time_of_day().minutes() % ptime.time_of_day().seconds()).str();
    return dateStr;
}

