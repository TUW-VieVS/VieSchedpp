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


#ifndef HORIZONMASK_STEP_H
#define HORIZONMASK_STEP_H

#include "AbstractHorizonMask.h"

namespace VieVS{
    class HorizonMask_step : public AbstractHorizonMask {
    public:
        HorizonMask_step(const std::vector<double> &azimuths, const std::vector<double> &elevations);

        bool visible(const PointingVector &pv) const noexcept override;

        std::string vexOutput() const noexcept override;

        std::pair<std::vector<double>, std::vector<double>> getHorizonMask() const noexcept override;

    private:
        std::vector<double> azimuth_; ///< horizon mask knots in radians
        std::vector<double> elevation_; ///< minimum elevation values in radians

        double az2el(double az) const noexcept;

    };
}


#endif //VLBI_SCHEDULER_HORIZONMASK_STEP_H
