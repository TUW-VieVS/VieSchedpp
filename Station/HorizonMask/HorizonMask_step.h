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

/**
 * @file HorizonMask_step.h
 * @brief class HorizonMask_step
 *
 * @author Matthias Schartner
 * @date 12.04.2018
 *
 */

#ifndef HORIZONMASK_STEP_H
#define HORIZONMASK_STEP_H

#include "AbstractHorizonMask.h"

namespace VieVS {

/**
 * @class HorizonMask_step
 * @brief step based horizon mask
 *
 * @author Matthias Schartner
 * @date 12.04.2018
 */
class HorizonMask_step : public AbstractHorizonMask {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param azimuths horizon mask step azimuths
     * @param elevations horizon mask step elevations
     */
    HorizonMask_step( const std::vector<double> &azimuths, const std::vector<double> &elevations );

    /**
     * @brief checks if a pointing vector is visible
     * @author Matthias Schartner
     *
     * @param pv pointing vector whose azimuth and elevation should be checked
     * @return true if target is visible, otherwise false
     */
    bool visible( const PointingVector &pv ) const noexcept override;

    /**
     * @brief horizon mask string in .vex format
     * @author Matthias Schartner
     *
     * @return horizon mask string in .vex format
     */
    std::string vexOutput() const noexcept override;

    /**
     * @brief getter for horizon mask
     * @author Matthias Schartner
     *
     * @return first vector is azimuth in radians, second vector is elevation in radians
     */
    std::pair<std::vector<double>, std::vector<double>> getHorizonMask() const noexcept override;

   private:
    std::vector<double> azimuth_;    ///< horizon mask knots in radians
    std::vector<double> elevation_;  ///< minimum elevation values in radians

    /**
     * @brief minimum elevation per azimuth
     * @author Matthias Schartner
     *
     * @param az azimuth in radians
     * @return minimum elevation in radians
     */
    double az2el( double az ) const noexcept;
};
}  // namespace VieVS

#endif  // VLBI_SCHEDULER_HORIZONMASK_STEP_H
