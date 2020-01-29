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

#include "AstrometricCalibratorBlock.h"


using namespace std;
using namespace VieVS;

bool AstrometricCalibratorBlock::scheduleCalibrationBlocks = false;

unsigned int AstrometricCalibratorBlock::cadence = 0;
AstrometricCalibratorBlock::CadenceUnit AstrometricCalibratorBlock::cadenceUnit =
    AstrometricCalibratorBlock::CadenceUnit::seconds;

unsigned int AstrometricCalibratorBlock::nextBlock = 0;

std::vector<unsigned long> AstrometricCalibratorBlock::calibratorSourceIds{};

unsigned int AstrometricCalibratorBlock::nmaxScans = 0;

AstrometricCalibratorBlock::TargetScanLengthType AstrometricCalibratorBlock::targetScanLengthType =
    AstrometricCalibratorBlock::TargetScanLengthType::parameters;
std::unordered_map<std::string, double> AstrometricCalibratorBlock::minSNR{};
unsigned int AstrometricCalibratorBlock::scanLength = 0;

double AstrometricCalibratorBlock::lowElevationStartWeight = 0 * deg2rad;
double AstrometricCalibratorBlock::lowElevationFullWeight = 0 * deg2rad;

double AstrometricCalibratorBlock::highElevationStartWeight = 90 * deg2rad;
double AstrometricCalibratorBlock::highElevationFullWeight = 90 * deg2rad;
