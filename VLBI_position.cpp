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

#include "VLBI_position.h"

namespace VieVS{
    VLBI_position::VLBI_position(){}
    
    VLBI_position::VLBI_position(double x_m, double y_m, double z_m):x{x_m},y{y_m},z{z_m} {
        double a = 6378136.6;
        double f = 1/298.25642;
        double e2 = 2*f-f*f;
        
        lon = atan2(y_m,x_m);
        double r = sqrt(x_m*x_m+y_m*y_m);
        lat = atan2(z_m,r);
        
        
        for(int i=0; i<6; ++i){
            double N=a/sqrt(1-e2*sin(lat)*sin(lat));
            h=r/cos(lat)-N;
            lat=atan2(z_m*(N+h),r*((1-e2)*N+h));
        }
        
    }
    
    ostream& operator<<(ostream& out, const VLBI_position& position){
        double latDeg = rad2deg*position.lat;
        double lonDeg = rad2deg*position.lon;
        cout << "Location: \n";
        cout << boost::format("    lat: %6.2f [deg]\n") % latDeg;
        cout << boost::format("    lon: %6.2f [deg]\n") % lonDeg;
        return out;
    }
    
    double VLBI_position::getDistance(const VLBI_position &other) const{
        double dx = x-other.x;
        double dy = y-other.y;
        double dz = z-other.z;
        return sqrt(dx*dx+dy*dy+dz*dz);
    }
    
}