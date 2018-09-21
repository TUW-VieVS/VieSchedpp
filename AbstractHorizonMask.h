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
 * @file AbstractHorizonMask.h
 * @brief class AbstractHorizonMask
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 */

#ifndef HORIOZONMASK_H
#define HORIOZONMASK_H
#include <vector>
#include <cmath>

#include "Constants.h"
#include "PointingVector.h"
#include "VieVS_Object.h"

namespace VieVS{
    /**
     * @class AbstractHorizonMask
     * @brief representation of VLBI horizon mask
     *
     * @author Matthias Schartner
     * @date 27.06.2017
     *
     * This class serves as the base class for all horizon mask implementations.
     *
     */
    class AbstractHorizonMask : public VieVS_Object {
    public:

        /**
         * @brief constructor
         * @author Matthias Schartner
         */
        AbstractHorizonMask();

        /**
         * @brief checks if a pointing vector is visible
         * @author Matthias Schartner
         *
         * @param pv pointing vector whose azimuth and elevation should be checked
         * @return true if target is visible, otherwise false
         */
        virtual bool visible(const PointingVector &pv) const noexcept = 0;

        /**
         * @brief horizon mask string in .vex format
         * @author Matthias Schartner
         *
         * @return horizon mask string in .vex format
         */
        virtual std::string vexOutput() const noexcept = 0;

        /**
         * @brief getter for horizon mask
         * @author Matthias Schartner
         *
         * @return first vector is azimuth in radinas, second vector is elevation in radians
         */
        virtual std::pair<std::vector<double>, std::vector<double>> getHorizonMask() const noexcept  = 0;

    private:
        static unsigned long nextId;  ///< next id for this object type
    };
}
#endif /* HORIOZONMASK_H */

