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
 * @file AstrometricCalibratorBlock.h
 * @brief class for calibrator block
 *
 * @author Matthias Schartner
 * @date 26.09.2017
 *
 */

#ifndef CALIBRATORBLOCK_H
#define CALIBRATORBLOCK_H


#include <unordered_map>
#include <vector>
#include <string>

#include "Constants.h"


namespace VieVS {
/**
 * @class AstrometricCalibratorBlock
 * @brief calibrator block
 *
 * @author Matthias Schartner
 * @date 26.09.2017
 */
class AstrometricCalibratorBlock {
   public:
    /**
     * @brief calibrator block cadence unit
     * @author Matthias Schartner
     */
    enum class CadenceUnit {
        scans,    ///< based on number of scans
        seconds,  ///< based on seconds passed
    };

    /**
     * @brief calibrator block scan length type
     */
    enum class TargetScanLengthType {
        parameters,  ///< use time from parameters
        minSNR,      ///< until target SNR is reached
        seconds,     ///< fixed time in seconds
    };

    static bool scheduleCalibrationBlocks;  ///< flag if calibration block should be scheduled

    static unsigned int cadence;     ///< cadence
    static CadenceUnit cadenceUnit;  ///< cadence unit

    static unsigned int nextBlock;  ///< seconds/number of scans when next block should start

    static std::vector<unsigned long> calibratorSourceIds;  ///< list of calibrator sources

    static unsigned int nmaxScans;  ///< maximum number of scans per calibrator block

    static TargetScanLengthType targetScanLengthType;       ///< target scan length type
    static std::unordered_map<std::string, double> minSNR;  ///<  target minimum signal to noise ratio per band
    static unsigned int scanLength;                         ///< target scan length in seconds

    static double lowElevationStartWeight;  ///< low elevation start value in radians
    static double lowElevationFullWeight;   ///< low elevation full value in radians

    static double highElevationStartWeight;  ///< high elevation start value in radians
    static double highElevationFullWeight;   ///< high elevation full value in radians
};
}  // namespace VieVS

#endif  // CALIBRATORBLOCK_H
