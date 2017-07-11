/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_source.h
 * Author: mschartn
 *
 * Created on June 28, 2017, 11:25 AM
 */

#ifndef VLBI_SOURCE_H
#define VLBI_SOURCE_H
#include <iostream>
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <math.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "VLBI_flux.h"
#include "VieVS_constants.h"

using namespace std;
namespace VieVS{
    
    class VLBI_source {
    public:
        struct PARAMETERS{
            vector<string> parameterGroups;

            double weight = 1;
            
            vector<string> minSNR_band;
            vector<double> minSNR_value;

            double minFlux = .01;
            double minRepeat = 1800;
            double maxScan = 600;
            double minScan = 30;
        };
//        struct PRECALC{
//            double sourceInCrs[3];
//        };
        
        VLBI_source();
        VLBI_source(string src_name, int id, double src_ra_deg, double src_de_deg, VLBI_flux src_flux);
//        double* getSourceInCrs(){ return STORAGE.sourceInCrs;}
        string getName() {return name;}
        double getRa() {return ra;}
        double getDe() {return de;}
        double getId() {return id;}
        double angleDistance(VLBI_source other);

        void setParameters(const string& group, boost::property_tree::ptree& PARA_station);

        virtual ~VLBI_source();
        
        friend ostream& operator<<(ostream& out, const VLBI_source& src);
        
    private:
        string name;
        int id;
        double ra;
        double de;
        VLBI_flux flux;
        
        PARAMETERS PARA;
//        PRECALC STORAGE;
                
        boost::posix_time::ptime lastScan;
        int nscans;
        int nbls;
    };
    
    
}
#endif /* VLBI_SOURCE_H */

