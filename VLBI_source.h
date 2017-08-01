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
#include <utility>

#include "VLBI_flux.h"
#include "VieVS_constants.h"

using namespace std;
namespace VieVS{
    
    class VLBI_source {
    public:
        struct PARAMETERS{
            vector<string> parameterGroups;

            double weight = 1;

            vector<pair<string, double> > minSNR;
            int minNumberOfStations = 2;
            double minFlux = .01;
            double minRepeat = 1800;
            unsigned int maxScan = 600;
            unsigned int minScan = 30;
        };
//        struct PRECALC{
//            double sourceInCrs[3];
//        };
        
        VLBI_source();

        VLBI_source(string src_name, double src_ra_deg, double src_de_deg, vector<pair<string, VLBI_flux> > src_flux);
//        double* getSourceInCrs(){ return STORAGE.sourceInCrs;}
        string getName() {return name;}

        double getRa() const {
            return ra;
        }

        double getDe() const {
            return de;
        }

        int getId() const {
            return id;
        }

        int getNbls() const {
            return nbls;
        }

        unsigned int lastScanTime() const {
            return lastScan;
        }

        unsigned int minRepeatTime() const {
            return PARA.minRepeat;
        }

        vector<pair<string, double> > getMinSNR() {
            return PARA.minSNR;
        }

        unsigned int getMinScanTime(){
            return PARA.minScan;
        }

        unsigned int getMaxScanTime(){
            return PARA.maxScan;
        }

        void setId(int id) {
            VLBI_source::id = id;
        }

        // todo: minimum number of stations
        int getMinNumberOfStations() {return 2;}
        double angleDistance(VLBI_source other);
        bool isStrongEnough(double& maxFlux);
        
        void setParameters(const string& group, boost::property_tree::ptree& PARA_station);

        vector<pair<string, double> > observedFlux(double gmst, double dx, double dy, double dz);

        virtual ~VLBI_source();

        void update(unsigned long nbl, unsigned int time);

        friend ostream& operator<<(ostream& out, const VLBI_source& src);
        
    private:
        string name;
        int id;
        double ra;
        double de;
        vector<pair<string, VLBI_flux> > flux;
        
        PARAMETERS PARA;
//        PRECALC STORAGE;

        unsigned int lastScan;
        int nscans;
        unsigned long nbls;
    };
    
    
}
#endif /* VLBI_SOURCE_H */

