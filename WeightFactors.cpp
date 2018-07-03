//
// Created by mschartn on 14.08.17.
//

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
double thread_local VieVS::WeightFactors::lowElevationStartWeight = 0;
double thread_local VieVS::WeightFactors::lowElevationFullWeight = 0;
