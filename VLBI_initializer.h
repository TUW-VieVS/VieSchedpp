/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_initializer.h
 * Author: mschartn
 *
 * Created on June 28, 2017, 12:38 PM
 */

#ifndef VLBI_INITIALIZER_H
#define VLBI_INITIALIZER_H
#include <vector>
#include <map>
#include <boost/date_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "VieVS_constants.h"
#include "VLBI_station.h"
#include "VLBI_source.h"
#include "VLBI_skyCoverage.h"

using namespace std;
namespace VieVS{
    
    class VLBI_initializer {
    
    public:
        struct PARAMETERS{            
            string experimentName;
            string experimentDescription;
            boost::posix_time::ptime startTime;
            boost::posix_time::ptime endTime;
            unsigned int duration;
            double mjdStart;
            
            double maxDistanceTwinTeleskopes = 0;
            vector<string> selectedStations;
            
        };
        
        enum class catalog {antenna,position,equip,mask,source,flux};
        
        VLBI_initializer();
        
        virtual ~VLBI_initializer();
        
        map<string,vector<string> > readCatalog(string path, catalog type);
        
        void createStationsFromCatalogs(string catalogPath);

        void createSourcesFromCatalogs(string catalogPath);

        void createSkyCoverages();

        void displaySummary();

        vector<VLBI_station> getStations(){
            return stations;
        }

        vector<VLBI_source> getSources(){
            return sources;
        }

        vector<VLBI_skyCoverage> getSkyCoverages(){
            return skyCoverages;
        }

        PARAMETERS getPARA(){
            return PARA;
        }

        void initializeStations();

        void initializeSources();

    private:
        boost::property_tree::ptree PARA_xml;
        vector<VLBI_station> stations;
        vector<VLBI_source> sources;
        vector<VLBI_skyCoverage> skyCoverages;
        PARAMETERS PARA;
    };
}
#endif /* VLBI_INITIALIZER_H */

