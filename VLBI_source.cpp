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
    VLBI_source::VLBI_source(string src_name,int id, double src_ra_deg, double src_de_deg, VLBI_flux src_flux):
    name{src_name}, id{id}, ra{src_ra_deg*deg2rad}, de{src_de_deg*deg2rad}, flux{src_flux}{
//        STORAGE.sourceInCrs[0] = cos(de)*cos(ra);
//        STORAGE.sourceInCrs[1] = cos(de)*sin(ra);
//        STORAGE.sourceInCrs[2] = sin(de);
        
    }
    
    VLBI_source::~VLBI_source() {
    }
    
    double VLBI_source::angleDistance(VLBI_source other){
        return acos(cos(de)*cos(other.de) * cos(ra-other.ra) + sin(de)*sin(other.de));
    }
    
    void VLBI_source::setParameters(const string& group, boost::property_tree::ptree& PARA_station){
        PARA.parameterGroups.push_back(group);
        for (auto it: PARA_station){
            string name = it.first;
            if ( name == "<xmlattr>")
                continue;
            else if ( name == "weight")
                PARA.weight = PARA_station.get<double>("weight");
            else if ( name == "minRepeat")
                PARA.minRepeat = PARA_station.get<double>("minRepeat");
            else if ( name == "maxScan")
                PARA.maxScan = PARA_station.get<double>("maxScan");
            else if ( name == "minScan")
                PARA.minScan = PARA_station.get<double>("minScan");
            else if ( name == "minFlux")
                PARA.minFlux = PARA_station.get<double>("minFlux");
            else
                cerr << "Source " << this->name << ": parameter <" << name << "> not understood! (Ignored)\n";
        }
    }

    ostream& operator<<(ostream& out, const VLBI_source& src){
        cout << boost::format("%=36s\n") %src.name; 
        double ra_deg = src.ra*rad2deg;
        double de_deg = src.de*rad2deg;
        cout << "position:\n";
        cout << boost::format("  RA: %10.6f [deg]\n") % ra_deg;
        cout << boost::format("  DE: %10.6f [deg]\n") % de_deg;
        cout << src.flux;
        cout << "------------------------------------\n";
        return out;
    }
    
    
}
