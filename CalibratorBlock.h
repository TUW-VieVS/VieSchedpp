//
// Created by matth on 24.09.2017.
//

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
            minSNR,
            seconds,
        };


        static bool scheduleCalibrationBlocks;

        static unsigned int cadence;
        static CadenceUnit cadenceUnit;

        static unsigned int nextBlock;

        static std::vector<int> calibratorSourceIds;

        static unsigned int nmaxScans;

        static TargetScanLengthType targetScanLengthType;
        static std::unordered_map<std::string,double> minSNR;
        static unsigned int scanLength;


        static double lowElevationSlopeStart;
        static double lowElevationSlopeEnd;

        static double highElevationSlopeStart;
        static double highElevationSlopeEnd;

    };
}


#endif //CALIBRATORBLOCK_H
