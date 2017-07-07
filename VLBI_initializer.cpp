/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VLBI_initializer.cpp
 * Author: mschartn
 * 
 * Created on June 28, 2017, 12:38 PM
 */

#include "VLBI_initializer.h"
namespace VieVS{
    VLBI_initializer::VLBI_initializer() {
    }

    VLBI_initializer::~VLBI_initializer() {
    }
    
    map<string,vector<string>> VLBI_initializer::readCatalog(string catalogPath, catalog type){
        map<string,vector<string>> all;
        int indexOfKey;
        string filepath;
        
       // switch between four available catalogs
        switch (type) {
            case catalog::antenna:{
                indexOfKey = 1;
                filepath = catalogPath + "/antenna.cat";
                break;
            }
            case catalog::position:{
                indexOfKey = 0;
                filepath = catalogPath + "/position.cat";
                break;
            }
            case catalog::equip:{
                indexOfKey = 1;
                filepath = catalogPath + "/equip.cat";
                break;
            }
            case catalog::mask:{
                indexOfKey = 2;
                filepath = catalogPath + "/mask.cat";
                break;
            }
            case catalog::source:{
                indexOfKey = 0;
                filepath = catalogPath + "/source.cat";
                break;
            }
            case catalog::flux:{
                indexOfKey = 0;
                filepath = catalogPath + "/flux.cat";
            }
        }
        
        // read in catalog. antenna, position and equip use the same routine
        switch (type) {
            case catalog::antenna:
            case catalog::position:
            case catalog::equip:
            case catalog::source:{
                
                // open file
                ifstream fid (filepath);                
                if (!fid.is_open()){
                    cerr << "    Unable to open " << filepath << " file!\n";
                } else {
                    string line;
                    
                    // loop through file
                    while ( getline (fid,line) ){
                        if(line.length()>0 && line.at(0)!='*'){
                            
                            // trim leading and trailing blanks
                            boost::trim(line);
                            
                            // split vector
                            vector<string> splitVector;
                            boost::split( splitVector, line, boost::is_space(),boost::token_compress_on);
                            
                            // get key and convert it to upper case for case insensitivity
                            string key =  boost::algorithm::to_upper_copy(splitVector[indexOfKey]);
                            
                            // add station name to key if you look at equip.cat because id alone is not unique in catalogs
                            if (type == catalog::equip){
                                key = boost::algorithm::to_upper_copy(key + "|" + splitVector[indexOfKey-1]);
                            }
                            
                            // look if a key already exists, if not add it.
                            if(all.find(key) == all.end()){
                                all.insert( pair<string,vector<string>>(key,splitVector) );
                            } else {
                                cout << "    Duplicated element of '" << key << "' in " << filepath << "\n";
                            }
                        }
                    }
                }
                // close file
                fid.close();
                break;
            }
            
            case catalog::mask:{
                
                // open file
                ifstream fid (filepath);
                if (!fid.is_open()){
                    cerr << "    Unable to open " << filepath << " file!\n";
                }
                else{
                    string line;
                    vector<string> splitVector_total;
                    
                    // loop through catalog
                    while ( getline (fid,line) ){
                        if(line.length()>0 && line.at(0)!='*'){
                            
                            // trim leading and trailing blanks
                            boost::trim(line);

                            // if first element is not an '-' this line belongs to new station mask
                            if (line.at(0) != '-' && splitVector_total.size()>0){
                                
                                // get key and convert it to upper case for case insensitivity
                                string key = boost::algorithm::to_upper_copy(splitVector_total[indexOfKey]);
                                
                                // previous mask is finished, add it to map
                                all.insert( pair<string,vector<string>>(key,splitVector_total) );
                                splitVector_total.clear();
                            }
                            
                            // split vector 
                            vector<string> splitVector;
                            boost::split( splitVector, line, boost::is_space(),boost::token_compress_on);
                            
                            // if it is a new mask add all elements to vector, if not start at the 2nd element (ignore '-')
                            if(splitVector_total.size() == 0){
                                splitVector_total.insert(splitVector_total.end(), splitVector.begin(), splitVector.end());
                            } else {
                                splitVector_total.insert(splitVector_total.end(), splitVector.begin()+1, splitVector.end());
                            }
                        }
                    }
                }
                break;
            }
            
            case catalog::flux:{
                // open file
                ifstream fid (filepath);
                if (!fid.is_open()){
                    cerr << "    Unable to open " << filepath << " file!\n";
                }
                else{
                    string line;
                    vector<string> splitVector_total;
                    string station;
                    
                    // get first entry
                    while (true){
                        getline (fid,line);
                        boost::trim(line);
                        if(line.length()>0 && line.at(0)!='*'){
                            boost::split( splitVector_total, line, boost::is_space(),boost::token_compress_on);
                            station = splitVector_total[indexOfKey];
                            break;
                        }
                    }
                    
                    // loop through catalog
                    while (getline (fid,line)){
                        // trim leading and trailing blanks
                        boost::trim(line);

                        if(line.length()>0 && line.at(0)!='*'){
                            vector<string> splitVector;
                            boost::split( splitVector, line, boost::is_space(),boost::token_compress_on);
                            string newStation = splitVector[indexOfKey];

                            if(newStation.compare(station) == 0){
                                splitVector_total.insert(splitVector_total.end(), splitVector.begin(), splitVector.end());
                            } else {
                                all.insert( pair<string,vector<string>>(station,splitVector_total) );
                                station = newStation;
                                splitVector_total = splitVector;
                            }

                        }
                    }
                    all.insert( pair<string,vector<string>>(station,splitVector_total) );   
                }
                break;
            }
        }
        
        return all;
    }
    
    void VLBI_initializer::createStationsFromCatalogs(string catalogPath) {
        cout << "Creating stations from catalog files:\n";
        cout << "  reading antenna.cat:\n";
        map<string,vector<string>> antennaCatalog  =  readCatalog(catalogPath,catalog::antenna);
        cout << "  reading position.cat:\n";
        map<string,vector<string>> positionCatalog =  readCatalog(catalogPath,catalog::position);
        cout << "  reading equip.cat:\n";
        map<string,vector<string>> equipCatalog    =  readCatalog(catalogPath,catalog::equip);
        cout << "  reading mask.cat:\n";
        map<string,vector<string>> maskCatalog     =  readCatalog(catalogPath,catalog::mask);
        
        int nant;
        int counter = 0;
        
        if (!PARA.selectedStations.empty()){
            nant = PARA.selectedStations.size();
        } else {
            nant = antennaCatalog.size();
        }
        
        int created = 0;
        // loop through all antennas in antennaCatalog
        for (auto any : antennaCatalog){
            counter ++;
            // get antenna name
            string name = any.first;
            
            // check if station is in PARA.selectedStations or if PARA.selectedStations is not empty
            if (!PARA.selectedStations.empty() && (find(PARA.selectedStations.begin(), PARA.selectedStations.end(), name) == PARA.selectedStations.end())){
                continue;
            }
                        
            // look if vector in antenna.cat is long enough. Otherwise not all information is available!
            if (any.second.size() < 16){
                cout <<"*** ERROR: "<< any.first << ": antenna.cat to small ***\n";
                continue;
            }
            
            // convert all IDs to upper case for case insensitivity
            string id_PO = boost::algorithm::to_upper_copy(any.second.at(13));
            string id_EQ = boost::algorithm::to_upper_copy(any.second.at(14));
            string id_MS = boost::algorithm::to_upper_copy(any.second.at(15));
            
            // check if corresponding position and equip catalog exists. 
            if (positionCatalog.find(id_PO) == positionCatalog.end()){
                cout << "*** ERROR: position catalog not found ***\n";
                continue;
            }
            if (equipCatalog.find(id_EQ+"|"+name) == equipCatalog.end()){
                cout << "*** ERROR: equip catalog not found ***\n";
                continue;
            }
            
            // convert all items from antenna.cat
            string type = any.second.at(2);
            double offset,rate1,con1,lim1_low,lim1_up,rate2,con2,lim2_low,lim2_up,diam;
            try{
                offset = boost::lexical_cast<double>(any.second.at(3));
                rate1 = boost::lexical_cast<double>(any.second.at(4));
                con1 = boost::lexical_cast<double>(any.second.at(5));
                lim1_low = boost::lexical_cast<double>(any.second.at(6));
                lim1_up = boost::lexical_cast<double>(any.second.at(7));
                rate2 = boost::lexical_cast<double>(any.second.at(8));
                con2 = boost::lexical_cast<double>(any.second.at(9));
                lim2_low = boost::lexical_cast<double>(any.second.at(10));
                lim2_up = boost::lexical_cast<double>(any.second.at(11));
                diam = boost::lexical_cast<double>(any.second.at(12));
            }
            catch(const std::exception& e){
                cout << "*** ERROR: "<< e.what() << " ***\n";
            }
            
            // check if position.cat is long enough. Otherwise not all information is available. 
            vector<string> po_cat = positionCatalog.at(id_PO);
            if (po_cat.size()<5){
                cout <<"*** ERROR: "<< any.first << ": positon.cat to small ***\n";
                continue;
            }
            
            // convert all items from position.cat
            double x,y,z;
            try{
                x = boost::lexical_cast<double>(po_cat.at(2));
                y = boost::lexical_cast<double>(po_cat.at(3));
                z = boost::lexical_cast<double>(po_cat.at(4));
            }
            catch(const std::exception& e){
                cout <<"*** ERROR: "<< e.what() << " ***\n";
            }
            
            // check if equip.cat is long enough. Otherwise not all information is available. 
            vector<string> eq_cat = equipCatalog.at(id_EQ+"|"+name);
            if (eq_cat.size()<9){
                cout <<"*** ERROR: " << any.first << ": equip.cat to small ***\n";
                continue;
            }
            // check if SEFD information is in X and S band
            if (eq_cat[5].compare("X") != 0 || eq_cat[7].compare("S") != 0){
                cout <<"*** ERROR: "<< any.first << ": we only support SX equipment ***\n";
                continue;
            }
            // convert all items from equip.cat
            double SEFD_X,SEFD_S;
            try{
                SEFD_X = boost::lexical_cast<double>(eq_cat.at(6));
                SEFD_S = boost::lexical_cast<double>(eq_cat.at(8));
            }
            catch(const std::exception& e){
                cout << "*** ERROR: "<< e.what() << "\n";
            }
            
            // check if an horizontal mask exists
            vector<double> hmask;
            if (maskCatalog.find(id_MS) != maskCatalog.end()){
                vector<string> mask_cat = maskCatalog.at(id_MS);
                
                // loop through all element and convert them 
                for(int i=3; i<mask_cat.size(); ++i){
                    try{
                        hmask.push_back(boost::lexical_cast<double>(mask_cat.at(i)));
                    }
                    catch(const std::exception& e){
                        cout << "*** ERROR: "<< e.what() << " ***\n";
                        cout << mask_cat.at(i) << "\n";
                    }
                }
            } else {
                if (!id_MS.compare("--")==0){
                    cout << "*** ERROR: mask catalog not found ***\n";
                }
            }
            stations.push_back(VLBI_station(name,
                                            created,
                                            VLBI_antenna(offset,diam,rate1,con1,rate2,con2), 
                                            VLBI_cableWrap(lim1_low,lim1_up,lim2_low,lim2_up),
                                            VLBI_position(x,y,z),
                                            VLBI_equip(vector<string>{"X","S"},vector<double>{SEFD_X,SEFD_S}),
                                            VLBI_mask(hmask),
                                            type));
            created++;
            cout << boost::format("  %-8s added\n") %name;
            
        }
        cout <<"Finished! "<< created <<" of " << nant << " stations created\n\n" << endl;
    }
    
    void VLBI_initializer::createSourcesFromCatalogs(string catalogPath){
        cout << "Creating sources from catalog files:\n";
        cout << "  reading source.cat:\n";
        map<string,vector<string>> sourceCatalog  =  readCatalog(catalogPath,catalog::source);
        cout << "  reading flux.cat:\n";
        map<string,vector<string>> fluxCatalog =  readCatalog(catalogPath,catalog::flux);
        
        int counter = 0;
        int nsrc = sourceCatalog.size();
        int created = 0;
        vector<string> tooWeak;
        vector<double> tooWeak_Jy;
        
        for (auto any: sourceCatalog){
            counter ++;
            string name = any.first;
            
            if (any.second.size() < 8){
                cout <<"*** ERROR: "<< any.first << ": source.cat to small ***\n";
                continue;
            }
            if (!any.second.at(1).compare("$")==0){
                name = any.second.at(1);
            }
            
            if (fluxCatalog.find(name) == fluxCatalog.end()){
                cout << "*** ERROR: source " << name << ": flux information not found ***\n";
                continue;
            }

            double ra_h, ra_m, ra_s, de_deg, de_m, de_s;
            char sign =any.second.at(5).at(0);
            try{
                ra_h   = boost::lexical_cast<double>(any.second.at(2));
                ra_m   = boost::lexical_cast<double>(any.second.at(3));
                ra_s   = boost::lexical_cast<double>(any.second.at(4));
                de_deg = boost::lexical_cast<double>(any.second.at(5));
                de_m   = boost::lexical_cast<double>(any.second.at(6));
                de_s   = boost::lexical_cast<double>(any.second.at(7));
            }
            catch(const std::exception& e){
                cout << "*** ERROR: "<< e.what() << " ***\n";
            }
            double ra = 15*(ra_h + ra_m/60 + ra_s/3600);
            double de =    (abs(de_deg) + de_m/60 + de_s/3600);
            if (sign == '-'){
                de = -1*de;
            }
            
            vector<string> flux_cat = fluxCatalog.at(name);
            if (flux_cat.size() < 6){
                cout <<"*** ERROR: "<< name << ": flux.cat to small ***\n";
                continue;
            }
            string fluxType = flux_cat.at(2);
            VLBI_flux srcFlux(fluxType);
            
            bool fluxAdded = srcFlux.addFluxParameters(flux_cat);
            if(! fluxAdded){
                cout <<"*** ERROR: "<< name << ": flux could not be read ***\n";
                continue;
            }
            
            double minFlux = srcFlux.getMinimalFlux();
            if(minFlux < PARA.minimumFlux){
                tooWeak.push_back(name);
                tooWeak_Jy.push_back(minFlux);
                continue;
            }
            sources.push_back(VLBI_source(name,created,ra,de,srcFlux));
            
            created++;
            cout << boost::format("  %-8s added\n") %name;
        }
        
        cout <<"Finished! "<< created <<" of " << nsrc << " sources created\n\n" << endl;
        
        if (!tooWeak.empty()){
            cout << "weak sources:\n";
            for (int i=0; i<tooWeak.size(); ++i){
                string name = tooWeak[i];
                double jy = tooWeak_Jy[i];
                cout << boost::format("  %-8s: %4.2f [Jy]\n") %name %jy;
            }
        }
        
    }
    void VLBI_initializer::createSkyCoverages(){
        int nsta = stations.size();
        vector<bool> alreadyConsidered(nsta,false);
        int skyCoverageId = 0;
        vector<int> stationsPerId(nsta,0);
        
        for(int i=0; i<nsta; ++i){
            if(!alreadyConsidered[i]){
                stations[i].setSkyCoverageId(skyCoverageId);
                stationsPerId[skyCoverageId]++;
                alreadyConsidered[i] = true;
                for(int j=i+1; j<nsta; ++j){
                    if(!alreadyConsidered[j] && (stations[i].distance(stations[j])<PARA.maxDistanceTwinTeleskopes)){
                        stations[j].setSkyCoverageId(skyCoverageId);
                        stationsPerId[skyCoverageId]++;
                        alreadyConsidered[j] = true;
                    }
                }
                skyCoverageId++;
            }
        }
        
        for (int i=0; i<skyCoverageId; ++i){
            skyCoverages.push_back(VLBI_skyCoverage(stationsPerId[i]));
        }
        
    }
    
    void VLBI_initializer::displaySummary(){
        
        cout << "List of all sources:\n";
        cout << "------------------------------------\n";
        for(auto& any:stations){
            cout << any;
        }
        cout << "List of all sources:\n";
        cout << "------------------------------------\n";
        for(auto& any:sources){
            cout << any;
        }
    }
}
