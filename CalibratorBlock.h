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

#ifndef CALIBRATORBLOCK_H
#define CALIBRATORBLOCK_H

#include <vector>
#include <unordered_map>
#include "Constants.h"

namespace VieVS{
    class CalibratorBlock {
    public:
        enum class CadenceUnit{
            scans,
            seconds,
        };

        enum class TargetScanLengthType {
            parameters,
            minSNR,
            seconds,
        };


        static bool scheduleCalibrationBlocks;

        static unsigned int cadence;
        static CadenceUnit cadenceUnit;

        static unsigned int nextBlock;

        static std::vector<unsigned long> calibratorSourceIds;

        static unsigned int nmaxScans;

        static TargetScanLengthType targetScanLengthType;
        static std::unordered_map<std::string,double> minSNR;
        static unsigned int scanLength;


        static double lowElevationStartWeight;
        static double lowElevationFullWeight;

        static double highElevationStartWeight;
        static double highElevationFullWeight;

    };
}


#endif //CALIBRATORBLOCK_H
