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
#include <unordered_map>

#include "VLBI_flux.h"
#include "VieVS_constants.h"

using namespace std;
namespace VieVS{
    
    class VLBI_source {
    public:
        struct PARAMETERS{
            vector<string> parameterGroups;

            double weight = 1;
            
            unordered_map<string,double> minSNR;
            int minNumberOfStations = 2;
            double minFlux = .01;
            double minRepeat = 1800;
            double maxScan = 600;
            double minScan = 30;
        };
//        struct PRECALC{
//            double sourceInCrs[3];
//        };
        
        VLBI_source();
        VLBI_source(string src_name, int id, double src_ra_deg, double src_de_deg, unordered_map<string,VLBI_flux> src_flux);
//        double* getSourceInCrs(){ return STORAGE.sourceInCrs;}
        string getName() {return name;}
        double getRa() {return ra;}
        double getDe() {return de;}
        double getId() {return id;}
        int getMinNumberOfStations() {return 2;}
        double angleDistance(VLBI_source other);
        bool isStrongEnough(double& maxFlux);
        
        void setParameters(const string& group, boost::property_tree::ptree& PARA_station);

        unordered_map<string,double> observedFlux(double gmst, double dx, double dy, double dz);

        virtual ~VLBI_source();
        
        friend ostream& operator<<(ostream& out, const VLBI_source& src);
        
    private:
        string name;
        int id;
        double ra;
        double de;
        unordered_map<string,VLBI_flux> flux;
        
        PARAMETERS PARA;
//        PRECALC STORAGE;
                
        boost::posix_time::ptime lastScan;
        int nscans;
        int nbls;
    };
    
    
}
#endif /* VLBI_SOURCE_H */

