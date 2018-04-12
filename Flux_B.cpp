//
// Created by mschartn on 05.04.18.
//

#include "Flux_B.h"

VieVS::Flux_B::Flux_B(double wavelength, const std::vector<double> &knots, const std::vector<double> &values):
        Flux{wavelength}, knots_{knots}, values_{values} {
}

double VieVS::Flux_B::getMaximumFlux() const noexcept {
    double maxFlux = 0;

    for(auto flux_Jy: values_){
        if(flux_Jy > maxFlux){
            maxFlux = flux_Jy;
        }
    }

    return maxFlux;
}

double VieVS::Flux_B::observedFlux(double u, double v) const noexcept {
    double observedFlux = 0;
    double pbase = sqrt(u*u + v*v) / 1000.0;

    for (int i = 1; i < knots_.size(); ++i) {
        if (knots_[i] > pbase) {
            observedFlux = values_[i - 1];
            break;
        }
    }

    return observedFlux;
}