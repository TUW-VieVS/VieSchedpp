/*
 * Copyright (C) 2017 mschartn
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * File:   pos.cpp
 * Author: mschartn
 * 
 * Created on June 23, 2017, 10:28 AM
 */

#include "Position.h"
using namespace std;
using namespace VieVS;
unsigned long Position::nextId = 0;

Position::Position(double x_m, double y_m, double z_m):VieVS_Object(nextId++), x_{x_m},y_{y_m},z_{z_m} {
    double a = 6378136.6;
    double f = 1/298.25642;
    double e2 = 2*f-f*f;

    lon_ = atan2(y_m,x_m);
    double r = sqrt(x_m*x_m+y_m*y_m);
    lat_ = atan2(z_m,r);


    for(int i=0; i<6; ++i){
        double N=a/sqrt(1-e2*sin(lat_)*sin(lat_));
        h_=r/cos(lat_)-N;
        lat_=atan2(z_m*(N+h_),r*((1-e2)*N+h_));
    }

}

namespace VieVS{
    ostream &operator<<(ostream &out, const Position &position) noexcept {
        double latDeg = rad2deg*position.lat_;
        double lonDeg = rad2deg*position.lon_;
        cout << "Location: \n";
        cout << boost::format("    lat: %6.2f [deg]\n") % latDeg;
        cout << boost::format("    lon: %6.2f [deg]\n") % lonDeg;
        return out;
    }
}

double Position::getDistance(const Position &other) const noexcept {
    double dx = x_-other.x_;
    double dy = y_-other.y_;
    double dz = z_-other.z_;
    return sqrt(dx*dx+dy*dy+dz*dz);
}

