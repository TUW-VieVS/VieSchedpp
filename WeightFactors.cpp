//
// Created by mschartn on 14.08.17.
//

#include "WeightFactors.h"

double thread_local VieVS::WeightFactors::weightSkyCoverage = 0;
double thread_local VieVS::WeightFactors::weightNumberOfObservations = 0;
double thread_local VieVS::WeightFactors::weightDuration = 0;
double thread_local VieVS::WeightFactors::weightAverageSources = 0;
double thread_local VieVS::WeightFactors::weightAverageStations = 0;

double thread_local VieVS::WeightFactors::weightDeclination = 0;
double thread_local VieVS::WeightFactors::declinationSlopeStart = 0;
double thread_local VieVS::WeightFactors::declinationSlopeEnd = 0;

double thread_local VieVS::WeightFactors::weightLowElevation = 0;
double thread_local VieVS::WeightFactors::lowElevationSlopeStart = 0;
double thread_local VieVS::WeightFactors::lowElevationSlopeEnd = 0;
