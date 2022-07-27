//
// Created by mschartner on 7/25/22.
//

#include "ParallacticAngleBlock.h"

using namespace std;
using namespace VieVS;

string ParallacticAngleBlock::intent_;
double ParallacticAngleBlock::distanceScaling = 10.0;
int ParallacticAngleBlock::cadence = 600;
unsigned int ParallacticAngleBlock::duration = 300;
int ParallacticAngleBlock::nscans = 0;
vector<unsigned long> ParallacticAngleBlock::allowedSources = vector<unsigned long>();
vector<unsigned long> ParallacticAngleBlock::allowedStations = vector<unsigned long>();
