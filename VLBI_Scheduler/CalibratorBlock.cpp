//
// Created by matth on 24.09.2017.
//

#include "CalibratorBlock.h"
using namespace std;
using namespace VieVS;

bool thread_local CalibratorBlock::scheduleCalibrationBlocks = false;

unsigned int thread_local CalibratorBlock::cadence = 0;
CalibratorBlock::CadenceUnit thread_local CalibratorBlock::cadenceUnit = CalibratorBlock::CadenceUnit::seconds ;

unsigned int thread_local CalibratorBlock::nextBlock = 0;

std::vector<int> thread_local CalibratorBlock::calibratorSourceIds {};

unsigned int thread_local CalibratorBlock::nmaxScans = 0;

CalibratorBlock::TargetScanLengthType thread_local CalibratorBlock::targetScanLengthType = CalibratorBlock::TargetScanLengthType::parameters;
std::unordered_map<std::string,double> thread_local CalibratorBlock::minSNR {};
unsigned int thread_local CalibratorBlock::scanLength = 0;

double thread_local CalibratorBlock::lowElevationStartWeight = 0*deg2rad;
double thread_local CalibratorBlock::lowElevationFullWeight   = 0*deg2rad;

double thread_local CalibratorBlock::highElevationStartWeight = 90*deg2rad;
double thread_local CalibratorBlock::highElevationFullWeight   = 90*deg2rad;
