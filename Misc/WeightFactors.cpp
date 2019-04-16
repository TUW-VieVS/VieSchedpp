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

#include "WeightFactors.h"

double thread_local VieVS::WeightFactors::weightSkyCoverage = 0;
double thread_local VieVS::WeightFactors::weightNumberOfObservations = 0;
double thread_local VieVS::WeightFactors::weightDuration = 0;

double thread_local VieVS::WeightFactors::weightAverageSources = 0;
double thread_local VieVS::WeightFactors::weightAverageStations = 0;
double thread_local VieVS::WeightFactors::weightAverageBaselines = 0;

double thread_local VieVS::WeightFactors::weightIdleTime = 0;
unsigned int thread_local VieVS::WeightFactors::idleTimeInterval = 0;

double thread_local VieVS::WeightFactors::weightDeclination = 0;
double thread_local VieVS::WeightFactors::declinationStartWeight = 0;
double thread_local VieVS::WeightFactors::declinationFullWeight = 0;

double thread_local VieVS::WeightFactors::weightLowElevation = 0;
double thread_local VieVS::WeightFactors::lowElevationStartWeight = 30;
double thread_local VieVS::WeightFactors::lowElevationFullWeight = 20;
