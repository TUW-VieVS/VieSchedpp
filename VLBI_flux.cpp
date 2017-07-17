/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_flux.cpp
 * Author: mschartn
 * 
 * Created on June 28, 2017, 11:22 AM
 */

#include "VLBI_flux.h"

namespace VieVS{
    VLBI_flux::VLBI_flux() {
    }
    
    VLBI_flux::VLBI_flux(string stype){
        if (stype.compare("B")==0){
            type = fluxType::B;
        } else if(stype.compare("M")==0){
            type = fluxType::M;
        } else {
            type = fluxType::undefined;
        }
    }
        
    bool VLBI_flux::addFluxParameters(vector<string> parameters){
        
        if (this->type == fluxType::B){
            int npara = parameters.size();
            for(int i=0; i<npara; ++i){
                try{
                    if (i%2==0){
                        knots.push_back(boost::lexical_cast<double>(parameters[i]));
                    }
                    else {
                        values.push_back(boost::lexical_cast<double>(parameters[i]));
                    }
                }     
                catch(const std::exception& e){
                    cout << "*** ERROR: reading flux information ***\n";
                    cout << e.what();
                    return false;
                }
            }
        }else if (this->type == fluxType::M){
            int npara = parameters.size();
            
            int nmodels = npara/6;
            
            for(int i=0; i<nmodels; ++i){
                try{
                    flux.push_back(boost::lexical_cast<double>(parameters.at(i*6+0)));
                    majorAxis.push_back(boost::lexical_cast<double>(parameters.at(i*6+1))* flcon2);
                    axialRatio.push_back(boost::lexical_cast<double>(parameters.at(i*6+2)));
                    positionAngle.push_back(boost::lexical_cast<double>(parameters.at(i*6+3))*deg2rad);
                }     
                catch(const std::exception& e){
                    cout << "*** ERROR: " << parameters[0] << " " << parameters[1] << " " << e.what() << " reading flux information ***\n";
                    return false;
                }
            }
        }
        return true;
    }
    
    VLBI_flux::~VLBI_flux() {
    }
    
    double VLBI_flux::getMinimumFlux(){
        double minFlux = 9999;
        if (this->type == fluxType::B){
            for(auto flux_Jy: values){
                if(flux_Jy < minFlux){
                    minFlux = flux_Jy;
                }
            }
        } else if (this->type == fluxType::M){
            for(auto flux_Jy: flux){
                if(flux_Jy < minFlux){
                    minFlux = flux_Jy;
                }
            }
        } 
        return minFlux;
    }
    
    double VLBI_flux::getMaximumFlux(){
        double maxFlux = 0;
        if (this->type == fluxType::B){
            for(auto flux_Jy: values){
                if(flux_Jy > maxFlux){
                    maxFlux = flux_Jy;
                }
            }
        } else if (this->type == fluxType::M){
            for(auto flux_Jy: flux){
                if(flux_Jy > maxFlux){
                    maxFlux = flux_Jy;
                }
            }
        } 
        return maxFlux;
    }

    
    ostream& operator<<(ostream& out, const VLBI_flux& src){
        if (src.type == VLBI_flux::fluxType::B){
            cout << "Flux type: B\n";
        } else {
            cout << "Flux type: M\n";
        }
        cout << "\n";
        return out;
    }

    double VLBI_flux::getFlux(double u, double v) {
        double observedFlux = 0;

        if(type == fluxType::B){
            double pbase = sqrt(u*u + v*v) / 1000.0;

            for (int i = 1; i < knots.size(); ++i) {
                if(knots[i]>pbase){
                    observedFlux = values[i-1];
                }
            }

        }else if(type == fluxType::M){

            double u_w = u/wavelength;
            double v_w = v/wavelength;

            for (int i = 0; i < flux.size(); ++i) {

                double pa = positionAngle[i];
                double ucospa = u_w*cos(pa);
                double usinpa = u_w*sin(pa);
                double vcospa = v_w*cos(pa);
                double vsinpa = v_w*sin(pa);

                double arg1 = (vcospa + usinpa) * (vcospa + usinpa);
                double arg2 = (axialRatio[i] * (ucospa - vsinpa)) * (axialRatio[i] * (ucospa - vsinpa));
                double arg = -flcon1 * (arg1 + arg2) * majorAxis[i] * majorAxis[i];
                double f1 = flux[i] * exp(arg);
                observedFlux += f1;
            }
        }

        return observedFlux;
    }
}
