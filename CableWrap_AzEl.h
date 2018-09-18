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

#ifndef CABLEWRAP_AZEL_H
#define CABLEWRAP_AZEL_H

#include "AbstractCableWrap.h"
namespace VieVS{
    class CableWrap_AzEl : public AbstractCableWrap {
    public:
        CableWrap_AzEl(double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg);

        bool anglesInside(const PointingVector &p) const noexcept override;

        void unwrapAzNearAz(PointingVector &new_pointingVector, double az_old) const noexcept override;

        bool unwrapAzInSection(PointingVector &pv, char section) const noexcept override;

        AbstractCableWrap::CableWrapFlag cableWrapFlag(double unaz) const noexcept override;

        std::pair<std::string, std::string> getMotions() const noexcept override;

        std::string vexPointingSectors() const noexcept override;

    };
}


#endif //VLBI_SCHEDULER_CABLEWRAP_AZEL_H
