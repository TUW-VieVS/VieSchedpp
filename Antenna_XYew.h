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

#ifndef ANTENNA_XYEW_H
#define ANTENNA_XYEW_H

#include "AbstractAntenna.h"

namespace VieVS{
    class Antenna_XYew : public AbstractAntenna {
    public:
        Antenna_XYew(double offset_m, double diam_m, double rateX_deg_per_min, unsigned int constantOverheadX_s,
                     double rateY_deg_per_min, unsigned int constantOverheadY_s);

        unsigned int slewTime(const PointingVector &old_pointingVector,
                              const PointingVector &new_pointingVector) const noexcept override;

    };
}



#endif //ANTENNA_XYEW_H
