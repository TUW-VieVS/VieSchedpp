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
            parameters,
            minSNR,
            seconds,
        };


        static thread_local bool scheduleCalibrationBlocks;

        static thread_local unsigned int cadence;
        static thread_local CadenceUnit cadenceUnit;

        static thread_local unsigned int nextBlock;

        static thread_local std::vector<int> calibratorSourceIds;

        static thread_local unsigned int nmaxScans;

        static thread_local TargetScanLengthType targetScanLengthType;
        static thread_local std::unordered_map<std::string,double> minSNR;
        static thread_local unsigned int scanLength;


        static thread_local double lowElevationStartWeight;
        static thread_local double lowElevationFullWeight;

        static thread_local double highElevationStartWeight;
        static thread_local double highElevationFullWeight;

    };
}


#endif //CALIBRATORBLOCK_H
