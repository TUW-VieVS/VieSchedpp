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
        bool flag = false;
        
        if (this->type == fluxType::B){
            if (parameters.size()>11){
                int nbands = count(parameters.begin(),parameters.end(),parameters[0]);
                int npara = parameters.size()/nbands-3;
                for(int i=0; i<nbands; ++i){
                    band.push_back(parameters[(npara+3)*i+1]);
                    knots.push_back(vector<double>());
                    values.push_back(vector<double>());
                }
                
                for(int i=0; i<npara; ++i){
                    try{
                        if (i%2==0){
                            for(int j=0; j<nbands; ++j){
                                knots.at(j).push_back(boost::lexical_cast<double>(parameters.at((npara+3)*j+i+3)));
                            }
                        }
                        else {
                            for(int j=0; j<nbands; ++j){
                                values.at(j).push_back(boost::lexical_cast<double>(parameters.at((npara+3)*j+i+3)));
                            }
                        }
                        
                    }     
                    catch(const std::exception& e){
                        cout << "*** ERROR: " << parameters[0] << " " << parameters[1] << " " << e.what() << " reading flux information ***\n";
                        return false;
                    }
                }
                flag = true;
            }
        }else if (this->type == fluxType::M){
            if (parameters.size()>17){
                for(int i=1; i<parameters.size(); i+=9){

                    band.push_back(parameters.at(i));
                    try{
                        flux.push_back(boost::lexical_cast<double>(parameters.at(i+2)));
                        majorAxis.push_back(boost::lexical_cast<double>(parameters.at(i+3)));
                        axialRatio.push_back(boost::lexical_cast<double>(parameters.at(i+4)));
                        positionAngle.push_back(boost::lexical_cast<double>(parameters.at(i+5)));
                    }     
                    catch(const std::exception& e){
                        cout << "*** ERROR: " << parameters[0] << " " << parameters[1] << " " << e.what() << " reading flux information ***\n";
                        return false;
                    }
                }
                flag = true;
            }
        }
        return true;
    }
    
    VLBI_flux::~VLBI_flux() {
    }
    
    double VLBI_flux::getMinimalFlux(){
        double minFlux = 9999;
        if (this->type == fluxType::B){
            for(auto any_v: values){
                for(auto flux_Jy: any_v){
                    if(flux_Jy < minFlux){
                        minFlux = flux_Jy;
                    }
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
    
    ostream& operator<<(ostream& out, const VLBI_flux& src){
        if (src.type == VLBI_flux::fluxType::B){
            cout << "Flux type: B\n";
        } else {
            cout << "Flux type: M\n";
        }
        cout << "  Bands: ";
        for(string any:src.band){
            cout << any << " ";
        }
        cout << "\n";
        return out;
    }
}
