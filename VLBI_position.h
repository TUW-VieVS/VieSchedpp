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
 * File:   pos.h
 * Author: mschartn
 *
 * Created on June 23, 2017, 10:27 AM
 */

#ifndef VLBI_POS_H
#define VLBI_POS_H
#include <cmath>
#include <iostream>
#include <boost/format.hpp>
#include "VieVS_constants.h"

using namespace std;
namespace VieVS{
    class VLBI_position {
    public:
        VLBI_position();
        
        VLBI_position(double x_m, double y_m, double z_m);
        
        virtual ~VLBI_position(){};

        double getX() const {
            return x;
        }

        double getY() const {
            return y;
        }

        double getZ() const {
            return z;
        }

        double getLat() const {
            return lat;
        }

        double getLon() const {
            return lon;
        }

        double getDistance(VLBI_position other);


        friend ostream& operator<<(ostream& out, const VLBI_position& position);

    private:
        double x;
        double y;
        double z;
        double lat;
        double lon;
        double h;
    };
}
#endif /* VLBI_POS_H */

