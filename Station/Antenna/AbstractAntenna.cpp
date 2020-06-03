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

#include "AbstractAntenna.h"


using namespace std;
using namespace VieVS;
unsigned long AbstractAntenna::nextId = 0;


AbstractAntenna::AbstractAntenna( double offset_m, double diam_m, double rate1_deg_per_min,
                                  unsigned int constantOverhead1_s, double rate2_deg_per_min,
                                  unsigned int constantOverhead2_s )
    : VieVS_Object( nextId++ ),
      offset_{ offset_m },
      diam_{ diam_m },
      rate1_{ rate1_deg_per_min * deg2rad / 60 },
      con1_{ constantOverhead1_s },
      rate2_{ rate2_deg_per_min * deg2rad / 60 },
      con2_{ constantOverhead2_s } {}


unsigned int AbstractAntenna::slewTimePerAxis( double delta, Axis axis ) const noexcept {
    double rate, acc;
    unsigned int constantOverhead;
    switch ( axis ) {
        case Axis::axis1: {
            rate = rate1_;
            acc = rate1_;
            constantOverhead = con1_;
            break;
        }
        case Axis::axis2: {
            rate = rate2_;
            acc = rate2_;
            constantOverhead = con2_;
            break;
        }
    }

    double t_acc = rate / acc;
    double s_acc = 2 * ( acc * t_acc * t_acc / 2 );
    double t;
    if ( delta < s_acc ) {
        t = 2 * sqrt( delta / acc );
    } else {
        t = 2 * t_acc + ( delta - s_acc ) / rate;
    }

    // add an extra second for slew times close to next integer time
    if ( fmod( t, 1.0 ) > 0.85 ) {
        ++t;
    }
    // add an extra second for very slow antennas
    if ( rate1_ < 0.015 || rate2_ < 0.010 ) {
        ++t;
    }

    return static_cast<unsigned int>( ceil( t ) ) + constantOverhead;
}
