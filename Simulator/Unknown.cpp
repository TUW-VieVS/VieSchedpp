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
 * File:   Simulator.cpp
 * Author: mschartn
 *
 * 26.02.2020
 */

#include "Unknown.h"
using namespace std;
using namespace VieVS;

unsigned long VieVS::Unknown::nextId = 0;

std::string Unknown::typeString( Type t ) {
    switch ( t ) {
        case Type::XPO:
            return "PWL_XPO";
        case Type::YPO:
            return "PWL_YPO";
        case Type::dUT1:
            return "PWL_dUT1";
        case Type::NUTX:
            return "PWL_NUTX";
        case Type::NUTY:
            return "PWL_NUTY";
        case Type::ZWD:
            return "PWL_ZWD";
        case Type::NGR:
            return "PWL_NGR";
        case Type::EGR:
            return "PWL_EGR";
        case Type::CLK:
            return "PWL_CLK";
        case Type::CLK_linear:
            return "lin_CLK";
        case Type::CLK_quad:
            return "qua_CLK";
        case Type::COORD_X:
            return "STA_X";
        case Type::COORD_Y:
            return "STA_Y";
        case Type::COORD_Z:
            return "STA_Z";
        case Type::RA:
            return "SRC_RA";
        case Type::DEC:
            return "SRC_DE";
        case Type::datum_station:
            return "DAT_STA";
        case Type::datum_source:
            return "DAT_SRC";
        case Type::undefined:
            return "UNDEF!";
    }
}

std::string Unknown::getUnit( Type t ) {
    switch ( t ) {
        case Type::XPO:
            return "[mas]";
        case Type::YPO:
            return "[mas]";
        case Type::dUT1:
            return "[mas]";
        case Type::NUTX:
            return "[mas]";
        case Type::NUTY:
            return "[mas]";
        case Type::ZWD:
            return "[cm]";
        case Type::NGR:
            return "[cm]";
        case Type::EGR:
            return "[cm]";
        case Type::CLK:
            return "[cm]";
        case Type::CLK_linear:
            return "[cm/day]";
        case Type::CLK_quad:
            return "[cm/day^2]";
        case Type::COORD_X:
            return "[cm]";
        case Type::COORD_Y:
            return "[cm]";
        case Type::COORD_Z:
            return "[cm]";
        case Type::RA:
            return "[mas]";
        case Type::DEC:
            return "[mas]";
        case Type::datum_station:
            return "[]";
        case Type::datum_source:
            return "[]";
        case Type::undefined:
            return "[]";
    }
}

std::string Unknown::toString() const {
    string t = typeString( type );
    string m = member.empty() ? "--" : member;
    string e = refTime == std::numeric_limits<int>::min()
                   ? "--"
                   : TimeSystem::time2string( TimeSystem::startTime + boost::posix_time::seconds( refTime ) );
    return ( boost::format( "| %-8s | %-8s | %-19s |" ) % t % m % e ).str();
}
