/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Flux.cpp
 * Author: mschartn
 * 
 * Created on June 28, 2017, 11:22 AM
 */

#include "Flux.h"
using namespace std;
using namespace VieVS;

Flux::Flux() = default;

Flux::Flux(string stype){
    if (stype == "B"){
        type_ = FluxType::B;
    } else if(stype == "M"){
        type_ = FluxType::M;
    } else {
        type_ = FluxType::undefined;
    }
}

bool Flux::addFluxParameters(const vector<string> &parameters) noexcept {

    if (this->type_ == FluxType::B){
        int npara = parameters.size();
        for(int i=0; i<npara; ++i){
            try{
                if (i%2==0){
                    knots_.push_back(boost::lexical_cast<double>(parameters[i]));
                }
                else {
                    values_.push_back(boost::lexical_cast<double>(parameters[i]));
                }
            }
            catch(const std::exception& e){
                cout << "*** ERROR: reading flux information ***\n";
                cout << e.what();
                return false;
            }
        }
    }else if (this->type_ == FluxType::M){
        int npara = parameters.size();

        int nmodels = npara / 6;

        for(unsigned int i=0; i<nmodels; ++i){
            try{
                flux_.push_back(boost::lexical_cast<double>(parameters.at(i*6+0)));
                majorAxis_.push_back(boost::lexical_cast<double>(parameters.at(i*6+1))* flcon2_);
                axialRatio_.push_back(boost::lexical_cast<double>(parameters.at(i*6+2)));
                positionAngle_.push_back(boost::lexical_cast<double>(parameters.at(i*6+3))*deg2rad);
            }
            catch(const std::exception& e){
                cout << "*** ERROR: " << parameters[0] << " " << parameters[1] << " " << e.what() << " reading flux information ***\n";
                return false;
            }
        }
    }
    return true;
}

Flux::~Flux() = default;

double Flux::getMaximumFlux() const noexcept {
    double maxFlux = 0;
    if (this->type_ == FluxType::B){
        for(auto flux_Jy: values_){
            if(flux_Jy > maxFlux){
                maxFlux = flux_Jy;
            }
        }
    } else if (this->type_ == FluxType::M){
        for(auto flux_Jy: flux_){
            if(flux_Jy > maxFlux){
                maxFlux = flux_Jy;
            }
        }
    }
    return maxFlux;
}

namespace VieVS {
    ostream &operator<<(ostream &out, const Flux &src) noexcept {
        if (src.type_ == Flux::FluxType::B) {
            cout << "Flux type: B\n";
        } else {
            cout << "Flux type: M\n";
        }
        cout << "\n";
        return out;
    }
}

double Flux::getFlux(double u, double v) const noexcept {
    double observedFlux = 0;

    if(type_ == FluxType::B){
        double pbase = sqrt(u*u + v*v) / 1000.0;

        for (int i = 1; i < knots_.size(); ++i) {
            if(knots_[i]>pbase){
                observedFlux = values_[i-1];
            }
        }

    }else if(type_ == FluxType::M){

        double u_w = u/wavelength_;
        double v_w = v/wavelength_;

        for (int i = 0; i < flux_.size(); ++i) {

            double pa = positionAngle_[i];
            double ucospa = u_w*cos(pa);
            double usinpa = u_w*sin(pa);
            double vcospa = v_w*cos(pa);
            double vsinpa = v_w*sin(pa);

            double arg1 = (vcospa + usinpa) * (vcospa + usinpa);
            double arg2 = (axialRatio_[i] * (ucospa - vsinpa)) * (axialRatio_[i] * (ucospa - vsinpa));
            double arg = -flcon1_ * (arg1 + arg2) * majorAxis_[i] * majorAxis_[i];
            double f1 = flux_[i] * exp(arg);
            observedFlux += f1;
        }
    }

    return observedFlux;
}

