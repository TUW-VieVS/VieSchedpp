//
// Created by mschartn on 02.08.17.
//

#include "LookupTable.h"

using namespace VieVS;
using namespace std;

std::unordered_map<int, double> VieVS::LookupTable::sinLookupTable;
std::unordered_map<int, double> VieVS::LookupTable::cosLookupTable;
std::unordered_map<int, double> VieVS::LookupTable::acosLookupTable;
vector<vector<vector<float> > > VieVS::LookupTable::angularDistanceLookup = {};

void LookupTable::initialize() {

    double x = 0;
    int counter = 0;
    while (x < twopi + 0.001) {
        double val = sin(x);
        sinLookupTable.insert(make_pair(counter, val));
        x += .001;
        ++counter;
    }

    x = 0;
    counter = 0;
    while (x < twopi + 0.001) {
        double val = cos(x);
        cosLookupTable.insert(make_pair(counter, val));
        x += .001;
        ++counter;
    }

    x = -1;
    counter = -1000;
    while (x < 1) {
        double val = acos(x);
        acosLookupTable.insert(make_pair(counter, val));
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

    double pv_1_el = p1.getEl();
    double pv_2_el = p2.getEl();

    double pv_1_az = p1.getAz();
    double pv_2_az = p2.getAz();
    if (pv_2_az > pv_1_az) {
        swap(pv_1_az, pv_2_az);
    }
    double delta_az = (pv_1_az - pv_2_az) * rad2deg;
    while (delta_az > 180) {
        delta_az = delta_az - 360;
    }


    long pv_delta_az = abs(lround(delta_az)); // +.5 for rounding
    if (pv_1_el > pv_2_el) {
        swap(pv_1_el, pv_2_el);
    }

    auto thisEl = lround(pv_1_el * rad2deg); // +.5 for rounding
    auto pv_delta_el = lround((pv_2_el - pv_1_el) * rad2deg); // +.5 for rounding

    return angularDistanceLookup[thisEl][pv_delta_az][pv_delta_el];
}

double LookupTable::sinLookup(double x) {
    return sinLookupTable[lround(x * 1000)];
}

double LookupTable::cosLookup(double x) {
    return cosLookupTable[lround(x * 1000)];
}

double LookupTable::acosLookup(double x) {
    return acosLookupTable[lround(x * 1000)];
}


