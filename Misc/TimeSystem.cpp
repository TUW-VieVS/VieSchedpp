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

#include "TimeSystem.h"

double VieVS::TimeSystem::mjdStart = 0;
boost::posix_time::ptime VieVS::TimeSystem::startTime = boost::posix_time::ptime();
boost::posix_time::ptime VieVS::TimeSystem::endTime = boost::posix_time::ptime();
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

std::string VieVS::TimeSystem::time2string(const boost::posix_time::ptime &ptime) {
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
    return {boost::gregorian::date(year,month,day),boost::posix_time::time_duration(hour,minute,second)};
}

std::string VieVS::TimeSystem::time2date(const boost::posix_time::ptime &ptime) {
    std::string dateStr = (boost::format("%04d%3s%02d")
                           % ptime.date().year() %ptime.date().month() %ptime.date().day()).str();
    dateStr = boost::to_upper_copy(dateStr);
    return dateStr;
}

std::string VieVS::TimeSystem::time2string_doy(const boost::posix_time::ptime &ptime) {
    std::string dateStr = (boost::format("%02d%03d%02d%02d%02d") % (ptime.date().year()%100) % ptime.date().day_of_year() %
                            ptime.time_of_day().hours() % ptime.time_of_day().minutes() % ptime.time_of_day().seconds()).str();
    return dateStr;
}

std::string VieVS::TimeSystem::time2string_doy_minus(const boost::posix_time::ptime &ptime){
    std::string dateStr = (boost::format("%02d%03d-%02d%02d%02d") % (ptime.date().year()%100) % ptime.date().day_of_year() %
                           ptime.time_of_day().hours() % ptime.time_of_day().minutes() % ptime.time_of_day().seconds()).str();
    return dateStr;
}

std::string VieVS::TimeSystem::time2string_doySkdDowntime(const boost::posix_time::ptime &ptime) {
    std::string dateStr = (boost::format("%04d-%03d-%02d:%02d:%02d") % (ptime.date().year()) % ptime.date().day_of_year() %
                           ptime.time_of_day().hours() % ptime.time_of_day().minutes() % ptime.time_of_day().seconds()).str();
    return dateStr;
}

std::string VieVS::TimeSystem::time2string_ast(const boost::posix_time::ptime &ptime) {
    int month = ptime.date().month();
    std::string dateStr = (boost::format("%04d.%02d.%02d-%02d:%02d:%02d.0")
                           % ptime.date().year() %month %ptime.date().day()
                           % ptime.time_of_day().hours() %ptime.time_of_day().minutes() %ptime.time_of_day().seconds()).str();
    return dateStr;
}

boost::posix_time::ptime VieVS::TimeSystem::string_doy2ptime(std::string timeStr){
    unsigned long i =0;
    if(timeStr.length() == 13){
        i = 2;
    }
    auto year = boost::lexical_cast<unsigned short>(timeStr.substr(0,2+i));
    if(i==0){
        year +=2000;
    }
    auto doy = boost::lexical_cast<unsigned short>(timeStr.substr(2+i,3));
    auto hour = boost::lexical_cast<int>(timeStr.substr(5+i,2));
    auto minute = boost::lexical_cast<int>(timeStr.substr(7+i,2));
    auto second = boost::lexical_cast<int>(timeStr.substr(9+i,2));
    return {boost::gregorian::date(year,1,1)+boost::gregorian::days(doy-1),boost::posix_time::time_duration(hour,minute,second)};
}

std::string VieVS::TimeSystem::time2string_units(const boost::posix_time::ptime &ptime) {
    int month = ptime.date().month();
    std::string dateStr = (boost::format("%04dy%02md%02dd%02dh%02dm%02ds")
                           % ptime.date().year() %month %ptime.date().day()
                           % ptime.time_of_day().hours() %ptime.time_of_day().minutes() %ptime.time_of_day().seconds()).str();
    return dateStr;
}

std::string VieVS::TimeSystem::time2string_doy_units(const boost::posix_time::ptime &ptime) {
    std::string dateStr = (boost::format("%04dy%03dd%02dh%02dm%02ds") % ptime.date().year() % ptime.date().day_of_year() %
                           ptime.time_of_day().hours() % ptime.time_of_day().minutes() % ptime.time_of_day().seconds()).str();
    return dateStr;
}

boost::posix_time::ptime VieVS::TimeSystem::internalTime2PosixTime(unsigned int time) {
    return TimeSystem::startTime + boost::posix_time::seconds(time);
}

unsigned int VieVS::TimeSystem::posixTime2InternalTime(const boost::posix_time::ptime &ptime) {
    int sec = util::duration(TimeSystem::startTime, ptime);

    return static_cast<unsigned int>(sec);
}

std::string VieVS::TimeSystem::time2timeOfDay(const boost::posix_time::ptime &ptime) {
    std::string str = (boost::format("%02d:%02d:%02d") %ptime.time_of_day().hours()
                       %ptime.time_of_day().minutes() % ptime.time_of_day().seconds()).str();

    return str;
}

