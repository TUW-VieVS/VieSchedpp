/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_source.cpp
 * Author: mschartn
 * 
 * Created on June 28, 2017, 11:25 AM
 */

#include "VLBI_source.h"
namespace VieVS{
    VLBI_source::VLBI_source() {
    }

    VLBI_source::VLBI_source(string src_name, double src_ra_deg, double src_de_deg,
                             vector<pair<string, VLBI_flux> > src_flux) :
            name{src_name}, id{0}, ra{src_ra_deg * deg2rad}, de{src_de_deg * deg2rad}, flux{src_flux}, lastScan{0},
            nscans{0}, nbls{0} {

//        STORAGE.sourceInCrs[0] = cos(de)*cos(ra);
//        STORAGE.sourceInCrs[1] = cos(de)*sin(ra);
//        STORAGE.sourceInCrs[2] = sin(de);

    }
    
    VLBI_source::~VLBI_source() {
    }
    
    double VLBI_source::angleDistance(VLBI_source other){
        return acos(cos(de)*cos(other.de) * cos(ra-other.ra) + sin(de)*sin(other.de));
    }
    
    void VLBI_source::setParameters(const string& group, boost::property_tree::ptree& PARA_source){
        PARA.parameterGroups.push_back(group);
        for (auto& it: PARA_source){
            string name = it.first;
            if ( name == "<xmlattr>")
                continue;
            else if ( name == "weight")
                PARA.weight = PARA_source.get<double>("weight");
            else if ( name == "minRepeat")
                PARA.minRepeat = PARA_source.get<double>("minRepeat");
            else if ( name == "maxScan")
                PARA.maxScan = PARA_source.get<unsigned int>("maxScan");
            else if ( name == "minScan")
                PARA.minScan = PARA_source.get<unsigned int>("minScan");
            else if ( name == "minFlux")
                PARA.minFlux = PARA_source.get<double>("minFlux");
            else if ( name == "minSNR"){
                string bandName = it.second.get_child("<xmlattr>.band").data();
                double value = it.second.get_value<double>();
                PARA.minSNR.push_back(make_pair(bandName, value));
            } else
                cerr << "Source " << this->name << ": parameter <" << name << "> not understood! (Ignored)\n";
        }
    }
    
    bool VLBI_source::isStrongEnough(double& maxFlux){
        maxFlux = 0;
        
        for (auto& any: flux){
            double thisFlux = any.second.getMaximumFlux();
            if (thisFlux > maxFlux){
                maxFlux = thisFlux;
            }
        }
        if(maxFlux > PARA.minFlux){
            return true;
        }
        return false;
    }

    ostream& operator<<(ostream& out, const VLBI_source& src){
        cout << boost::format("%=36s\n") %src.name; 
        double ra_deg = src.ra*rad2deg;
        double de_deg = src.de*rad2deg;
        cout << "position:\n";
        cout << boost::format("  RA: %10.6f [deg]\n") % ra_deg;
        cout << boost::format("  DE: %10.6f [deg]\n") % de_deg;
//        cout << src.flux;
        cout << "------------------------------------\n";
        return out;
    }

    vector<pair<string, double> > VLBI_source::observedFlux(double gmst, double dx, double dy, double dz) {
        vector<pair<string, double> > fluxes;

        double cosdec = cos(de);
        double ha = gmst - ra;

        double u = dx * sin(ha) + dy * cos(ha);
        double v = dz*cos(de) + sin(de) * (-dx * cos(ha) + dy * sin(ha));

        for(auto& thisFlux: flux){
            double observedFlux = thisFlux.second.getFlux(u,v);

            fluxes.push_back(make_pair(thisFlux.first, observedFlux));
        }

        return fluxes;
    }

    void VLBI_source::update(unsigned long nbl, unsigned int time) {
        ++nscans;
        nbls += nbl;
        lastScan = time;
    }


}
