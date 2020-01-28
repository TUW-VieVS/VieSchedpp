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
 * @file CalibratorScanDescriptor.h
 * @brief class CalibratorScanDescriptor
 *
 * @author Matthias Schartner
 * @date 27.01.2020
 *
 */
#ifndef VIESCHEDPP_CALIBRATORSCANDESCRIPTOR_H
#define VIESCHEDPP_CALIBRATORSCANDESCRIPTOR_H

#include <vector>

#include "TimeSystem.h"
#include "VieVS_Object.h"

namespace VieVS {
/**
 * @class CalibratorScanDescriptor
 * @brief define calibrator scans
 *
 * @author Matthias Schartner
 * @date 27.01.2020
 */

class CalibratorScanDescriptor : public VieVS_Object {
   public:
    struct CalibratorBlock {
        bool flag = false;
        unsigned int startTime;
        unsigned int nScans;
        unsigned int duration;
        std::vector<std::string> allowedSources;

        bool isAllowedSource( const std::string &name ) {
            return find( allowedSources.begin(), allowedSources.end(), name ) != allowedSources.end();
        }
    };
    CalibratorScanDescriptor( CalibratorBlock start, CalibratorBlock mid, CalibratorBlock end );

    bool startBlock() { return start_.flag; }

    bool midBlock() { return mid_.flag; }

    bool endBlock() { return end_.flag; }

    const CalibratorBlock &getStartBlock() const { return start_; }

    const CalibratorBlock &getMidBlock() const { return mid_; }

    const CalibratorBlock &getEndBlock() const { return end_; }


   private:
    static unsigned long nextId;  ///< next id for this object type


    CalibratorBlock start_;
    CalibratorBlock mid_;
    CalibratorBlock end_;
};
}  // namespace VieVS


#endif  // VIESCHEDPP_CALIBRATORSCANDESCRIPTOR_H
