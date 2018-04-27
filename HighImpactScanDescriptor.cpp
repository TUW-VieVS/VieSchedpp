//
// Created by mschartn on 26.04.18.
//

#include "HighImpactScanDescriptor.h"

#include <utility>

using namespace std;
using namespace VieVS;

int HighImpactScanDescriptor::nextId = 0;
int HighImpactScanDescriptor::AzElDescriptor::nextId = 0;

HighImpactScanDescriptor::AzElDescriptor::AzElDescriptor(double az, double azMargin, double el, double elMargin, std::vector<int> staids):
        VieVS_Object(nextId++), az_{az}, azMargin_{azMargin}, el_{el}, elMargin_{elMargin}, staids_{std::move(staids)}{

}

HighImpactScanDescriptor::HighImpactScanDescriptor(unsigned int interval, unsigned int minTimeBetweenScans):
        VieVS_Object(nextId++), interval_{interval}, minTimeBetweenScans_{minTimeBetweenScans} {
}

void HighImpactScanDescriptor::addAzElDescriptor(double az, double azMargin, double el, double elMargin, const std::vector<int> &staids) {
    azElDescritors_.emplace_back(az,azMargin,el,elMargin,staids);
}

bool HighImpactScanDescriptor::isHighImpactScan(const Scan &scan) const {
    for(const auto &thisAzElDesc:azElDescritors_){
        for(int i=0; i<scan.getNSta(); ++i){
            const auto &pv = scan.getPointingVector(i);
            bool highImpact = thisAzElDesc.isHighImpactPv(pv);
            if(highImpact){
                return true;
            }
        }
    }
    return false;
}

bool HighImpactScanDescriptor::AzElDescriptor::isHighImpactPv(const PointingVector &pv) const {
    int staid = pv.getStaid();
    bool highImpact = false;
    if(std::find(staids_.begin(), staids_.end(), staid) != staids_.end()){
        double thisAz = Miscellaneous::wrapToPi(pv.getAz());
        double thisEl = pv.getEl();
        if(abs(thisAz-az_)<azMargin_ && abs(thisEl-el_)<elMargin_){
            highImpact = true;
        }
    }
    return highImpact;
}
