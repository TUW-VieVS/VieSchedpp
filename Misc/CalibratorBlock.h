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
 * @file CalibratorBlock.h
 * @brief class CalibratorBlock
 *
 * @author Matthias Schartner
 * @date 27.01.2020
 *
 */
#ifndef VIESCHEDPP_CALIBRATORBLOCK_H
#define VIESCHEDPP_CALIBRATORBLOCK_H

#include <vector>

#include "TimeSystem.h"
#include "VieVS_Object.h"

namespace VieVS {
/**
 * @class CalibratorBlock
 * @brief define calibrator blocks
 *
 * @author Matthias Schartner
 * @date 27.01.2020
 */

class CalibratorBlock : public VieVS_Object {
   public:
    CalibratorBlock( unsigned int startTime, unsigned int nScans, unsigned int duration,
                     std::string allowedSourceGroup );
    CalibratorBlock( unsigned int startTime, unsigned int nScans, unsigned int duration,
                     std::vector<std::string> allowedSources );

    bool isAllowedSource( const std::string &name ) const {
        return find( allowedSources.begin(), allowedSources.end(), name ) != allowedSources.end();
    }

    unsigned int getStartTime() const { return startTime; }
    unsigned int getNScans() const { return nScans; }
    unsigned int getDuration() const { return duration; }
    const std::vector<std::string> &getAllowedSources() const { return allowedSources; }
    const std::string &getAllowedSourceGroup() const { return allowedSourceGroup; }

   private:
    static unsigned long nextId;  ///< next id for this object type
    unsigned int startTime;
    unsigned int nScans;
    unsigned int duration;
    std::string allowedSourceGroup;
    std::vector<std::string> allowedSources;
};
}  // namespace VieVS


#endif  // VIESCHEDPP_CALIBRATORBLOCK_H
