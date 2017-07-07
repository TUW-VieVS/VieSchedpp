/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_flux.h
 * Author: mschartn
 *
 * Created on June 28, 2017, 11:22 AM
 */

#ifndef VLBI_FLUX_H
#define VLBI_FLUX_H
#include <vector>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include "VieVS_constants.h"

using namespace std;
namespace VieVS{
    class VLBI_flux {
    public:
        enum class fluxType{B,M,undefined};
        
        VLBI_flux();
        VLBI_flux(string stype);
        bool addFluxParameters(vector<string> parameters);
        double getMinimalFlux();
        
        VLBI_flux(vector<string> cat_lines);
        virtual ~VLBI_flux();
        
        friend ostream& operator<<(ostream& out, const VLBI_flux& src);
    private:
        fluxType type;
        vector<string> band;
        
        vector<vector<double> > knots;
        vector<vector<double> > values;
        
        vector<double> flux;
        vector<double> majorAxis;
        vector<double> axialRatio;
        vector<double> positionAngle;
    };
}
#endif /* VLBI_FLUX_H */

