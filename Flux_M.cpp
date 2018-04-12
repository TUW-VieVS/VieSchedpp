//
// Created by mschartn on 05.04.18.
//

#include "Flux_M.h"

using namespace VieVS;
using namespace std;

double Flux_M::flcon1_{(pi * pi) / (4.0 * 0.6931471)}; ///< constant precalculated value for model M
double Flux_M::flcon2_{pi / (3600.0 * 180.0 * 1000.0)}; ///< constant precalculated value for model M

VieVS::Flux_M::Flux_M(double wavelength, const std::vector<double> &flux, const std::vector<double> &majorAxis,
                      const std::vector<double> &axialRatio, const std::vector<double> &positionAngle):
        Flux{wavelength}, flux_{flux}, majorAxis_{majorAxis}, axialRatio_{axialRatio}, positionAngle_{positionAngle} {
}

double Flux_M::getMaximumFlux() const noexcept {
    double maxFlux = 0;

    for(auto flux_Jy: flux_){
        if(flux_Jy > maxFlux){
            maxFlux = flux_Jy;
        }
    }

    return maxFlux;
}

double Flux_M::observedFlux(double u, double v) const noexcept {

    double observedFlux = 0;


    double u_w = u / Flux::getWavelength();
    double v_w = v / Flux::getWavelength();

    for (int i = 0; i < flux_.size(); ++i) {

        double pa = positionAngle_[i];
        double ucospa = u_w * cos(pa);
        double usinpa = u_w * sin(pa);
        double vcospa = v_w * cos(pa);
        double vsinpa = v_w * sin(pa);

        double arg1 = (vcospa + usinpa) * (vcospa + usinpa);
        double arg2 = (axialRatio_[i] * (ucospa - vsinpa)) * (axialRatio_[i] * (ucospa - vsinpa));
        double arg = -flcon1_ * (arg1 + arg2) * majorAxis_[i] * majorAxis_[i];
        double f1 = flux_[i] * exp(arg);
        observedFlux += f1;
    }

    return observedFlux;

}

