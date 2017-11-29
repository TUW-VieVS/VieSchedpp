//
// Created by matth on 24.09.2017.
//

#include "CalibratorBlock.h"
using namespace std;
using namespace VieVS;

bool CalibratorBlock::scheduleCalibrationBlocks = false;

unsigned int CalibratorBlock::cadence = 0;
CalibratorBlock::CadenceUnit CalibratorBlock::cadenceUnit = CalibratorBlock::CadenceUnit::seconds ;

unsigned int CalibratorBlock::nextBlock = 0;

std::vector<int> CalibratorBlock::calibratorSourceIds {};

unsigned int CalibratorBlock::nmaxScans = 0;

CalibratorBlock::TargetScanLengthType CalibratorBlock::targetScanLengthType = CalibratorBlock::TargetScanLengthType::parameters;
std::unordered_map<std::string,double> CalibratorBlock::minSNR {};
unsigned int CalibratorBlock::scanLength = 0;

double CalibratorBlock::lowElevationStartWeight = 0*deg2rad;
double CalibratorBlock::lowElevationFullWeight   = 0*deg2rad;

double CalibratorBlock::highElevationStartWeight = 90*deg2rad;
double CalibratorBlock::highElevationFullWeight   = 90*deg2rad;
