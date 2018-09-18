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
 
#include "CalibratorBlock.h"
using namespace std;
using namespace VieVS;

bool CalibratorBlock::scheduleCalibrationBlocks = false;

unsigned int CalibratorBlock::cadence = 0;
CalibratorBlock::CadenceUnit CalibratorBlock::cadenceUnit = CalibratorBlock::CadenceUnit::seconds ;

unsigned int CalibratorBlock::nextBlock = 0;

std::vector<unsigned long> CalibratorBlock::calibratorSourceIds {};

unsigned int CalibratorBlock::nmaxScans = 0;

CalibratorBlock::TargetScanLengthType CalibratorBlock::targetScanLengthType = CalibratorBlock::TargetScanLengthType::parameters;
std::unordered_map<std::string,double> CalibratorBlock::minSNR {};
unsigned int CalibratorBlock::scanLength = 0;

double CalibratorBlock::lowElevationStartWeight = 0*deg2rad;
double CalibratorBlock::lowElevationFullWeight   = 0*deg2rad;

double CalibratorBlock::highElevationStartWeight = 90*deg2rad;
double CalibratorBlock::highElevationFullWeight   = 90*deg2rad;
