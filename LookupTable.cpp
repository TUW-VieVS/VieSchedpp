//
// Created by mschartn on 02.08.17.
//

#include "LookupTable.h"

using namespace VieVS;
using namespace std;

std::vector<double> VieVS::LookupTable::sinLookupTable;
std::vector<double> VieVS::LookupTable::cosLookupTable;
std::vector<double> VieVS::LookupTable::acosLookupTable;
vector<vector<vector<float> > > VieVS::LookupTable::angularDistanceLookup = {};

void LookupTable::initialize() {

    double x = 0;
    int counter = 0;
    while (x < twopi + 0.001) {
        double val = sin(x);
        sinLookupTable.push_back(val);
        x += .001;
        ++counter;
    }

    x = 0;
    counter = 0;
    while (x < twopi + 0.001) {
        double val = cos(x);
        cosLookupTable.push_back(val);
        x += .001;
        ++counter;
    }

    x = -1;
    counter = -1000;
    while (x < 1) {
        double val = acos(x);
        acosLookupTable.push_back(val);
        x += .001;
        ++counter;
    }

    unsigned int maxEl = 91;
    unsigned int sizeAz = 181;
    vector<vector<vector<float> > > storage;

    for (unsigned int thisEl = 0; thisEl < maxEl; ++thisEl) {
        unsigned int sizeEl = maxEl - thisEl;
        double thisEl_rad = thisEl * deg2rad;

        vector<vector<float> > thisStorage(sizeAz, vector<float>(sizeEl, 0));


        for (int deltaAz = 0; deltaAz < sizeAz; ++deltaAz) {
            double deltaAz_rad = deltaAz * deg2rad;
            for (int deltaEl = 0; deltaEl < sizeEl; ++deltaEl) {
                double deltaEl_rad = deltaEl * deg2rad;

                double tmp = sinLookup(thisEl_rad) * sinLookup(thisEl_rad + deltaEl_rad) + cosLookup(thisEl_rad) *
                                                                                           cosLookup(thisEl_rad +
                                                                                                     deltaEl_rad) *
                                                                                           cosLookup(deltaAz_rad);
                double angle = acosLookup(tmp);

                thisStorage[deltaAz][deltaEl] = static_cast<float>(angle);
            }
        }
        angularDistanceLookup.push_back(move(thisStorage));
    }
}

float LookupTable::angularDistance(const PointingVector &p1, const PointingVector &p2) noexcept {
    return angularDistance(p1.getAz(), p1.getEl(), p2.getAz(), p2.getEl());
}

double LookupTable::sinLookup(double x) {
    return sinLookupTable[lround(x * 1000)];
}

double LookupTable::cosLookup(double x) {
    return cosLookupTable[lround(x * 1000)];
}

double LookupTable::acosLookup(double x) {
    return acosLookupTable[lround(x * 1000)+1000];
}

float LookupTable::angularDistance(double phi1, double theta1, double phi2, double theta2) {
    if (phi1 > phi2) {
        swap(phi1, phi2);
    }
    if (theta1 > theta2) {
        swap(theta1, theta2);
    }

    double deltaPhi_tmp = (phi2 - phi1) * rad2deg;
    while (deltaPhi_tmp > 180) {
        deltaPhi_tmp = deltaPhi_tmp - 360;
    }

    long deltaPhi = lround(deltaPhi_tmp);

    auto thisTheta = lround(theta1 * rad2deg);
    auto deltaTheta = lround((theta2 - theta1) * rad2deg);

    return angularDistanceLookup[thisTheta][deltaPhi][deltaTheta];
}


