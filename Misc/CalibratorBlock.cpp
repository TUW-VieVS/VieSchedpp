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

#include <utility>

using namespace std;
using namespace VieVS;

unsigned long CalibratorBlock::nextId = 0;
string CalibratorBlock::intent_;
bool CalibratorBlock::tryToIncludeAllStationFlag = false;
bool CalibratorBlock::subnetting = false;

double CalibratorBlock::tryToIncludeAllStations_factor = 3;
double CalibratorBlock::numberOfObservations_factor = 5.0;
double CalibratorBlock::numberOfObservations_offset = 0.0;
double CalibratorBlock::averageStations_factor = 100.0;
double CalibratorBlock::averageStations_offset = 1.0;
double CalibratorBlock::duration_factor = 0.2;
double CalibratorBlock::duration_offset = 1.0;
double CalibratorBlock::averageBaseline_factor = 0.0;
double CalibratorBlock::averageBaseline_offset = 1.0;


std::vector<char> thread_local CalibratorBlock::stationFlag = std::vector<char>();

CalibratorBlock::CalibratorBlock( unsigned int startTime, unsigned int nScans, unsigned int duration,
                                  std::string allowedSourceGroup )
    : VieVS_Object( nextId++ ),
      startTime{ startTime },
      nScans{ nScans },
      duration{ duration },
      allowedSourceGroup{ std::move( allowedSourceGroup ) } {}

CalibratorBlock::CalibratorBlock( unsigned int startTime, unsigned int nScans, unsigned int duration,
                                  std::vector<std::string> allowedSources )
    : VieVS_Object( nextId++ ),
      startTime{ startTime },
      nScans{ nScans },
      duration{ duration },
      allowedSources{ std::move( allowedSources ) } {}