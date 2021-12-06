//
// Created by mschartner on 12/6/21.
//
#include "Flux_constant.h"

using namespace std;
using namespace VieVS;

Flux_constant::Flux_constant( double wavelength, double flux ) : AbstractFlux( wavelength ), flux_{ flux } {}
