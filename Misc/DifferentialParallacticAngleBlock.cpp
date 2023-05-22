//
// Created by mschartner on 7/25/22.
//

#include "DifferentialParallacticAngleBlock.h"

using namespace std;
using namespace VieVS;

string DifferentialParallacticAngleBlock::intent_;
int DifferentialParallacticAngleBlock::cadence = 600;
double DifferentialParallacticAngleBlock::distanceScaling = 2.0;
unsigned int DifferentialParallacticAngleBlock::duration = 300;
int DifferentialParallacticAngleBlock::nscans = 0;
int thread_local DifferentialParallacticAngleBlock::iScan = 0;
vector<unsigned long> DifferentialParallacticAngleBlock::allowedSources = vector<unsigned long>();
vector<unsigned long> DifferentialParallacticAngleBlock::allowedBaseline = vector<unsigned long>();
vector<double> DifferentialParallacticAngleBlock::angles = vector<double>();
