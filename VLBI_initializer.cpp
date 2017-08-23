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
#include "VLBI_baseline.h"

namespace VieVS{
    VLBI_initializer::VLBI_initializer() {
        ifstream is("parameters.xml");

        cout << "reading: parameters.xml \n";
        boost::property_tree::read_xml(is, PARA_xml);

        try{
            boost::posix_time::ptime startTime = PARA_xml.get<boost::posix_time::ptime>("master.general.start");
            cout << "start time:" << startTime << "\n";
            int sec_ = startTime.time_of_day().total_seconds();
            double mjdStart = startTime.date().modjulian_day() + sec_ / 86400;


            boost::posix_time::ptime endTime = PARA_xml.get<boost::posix_time::ptime>("master.general.end");
            cout << "end time:" << endTime << "\n";


            boost::posix_time::time_duration a = endTime - startTime;
            int sec = a.total_seconds();
            if(sec<0){
                cerr << "ERROR: duration is less than zero seconds!\n";
            }
            unsigned int duration = (unsigned int) sec;
            cout << "duration: " << duration << " [s]\n";

            VieVS_timeEvents::mjdStart = mjdStart;
            VieVS_timeEvents::startTime = startTime;
            VieVS_timeEvents::endTime = endTime;
            VieVS_timeEvents::duration = duration;

            vector<string> sel_stations;
            boost::property_tree::ptree stations = PARA_xml.get_child("master.general.stations");
            auto it = stations.begin();
            while (it != stations.end()) {
                auto item = it->second.data();
                sel_stations.push_back(item);
                ++it;
            }
            PARA.selectedStations = sel_stations;

            PARA.subnetting = PARA_xml.get<bool>("master.general.subnetting");
            PARA.fillinmode = PARA_xml.get<bool>("master.general.fillinmode");

        }catch(const boost::property_tree::ptree_error &e){
            cout << "ERROR: reading parameters.xml file!"<< endl;
            throw;
        }

        PARA.experimentName = PARA_xml.get<string>("master.general.experiment_name", "");
        PARA.experimentDescription = PARA_xml.get<string>("master.general.experiment_description", "");

        PARA.skyCoverageDistance = PARA_xml.get<double>("master.general.skyCoverageDistance", 30) * deg2rad;
        PARA.skyCoverageInterval = PARA_xml.get<double>("master.general.skyCoverageInterval", 3600);;

        try{
            PARA.maxDistanceTwinTeleskopes = PARA_xml.get<double>("master.general.maxDistanceTwinTeleskopes", 0);
        }catch(const boost::property_tree::ptree_error &e){
            cout << "ERROR: reading parameters.xml file!"<< endl;
            throw;
        }

        cout << "finished\n";
    }

    VLBI_initializer::~VLBI_initializer() {
    }
    
    map<string,vector<string>> VLBI_initializer::readCatalog(const string &catalogPath, catalog type){
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
            
            // TODO: split_Vector_total is now unnecessary
            case catalog::flux:{
                // open file
                ifstream fid (filepath);
                if (!fid.is_open()){
                    cerr << "    Unable to open " << filepath << " file!\n";
                }
                else{
                    string line;
                    vector<string> lines;
                    vector<string> splitVector_total;
                    string station;
                    
                    // get first entry
                    while (true){
                        getline (fid,line);
                        boost::trim(line);
                        if(line.length()>0 && line.at(0)!='*'){
                            boost::split( splitVector_total, line, boost::is_space(),boost::token_compress_on);
                            station = splitVector_total[indexOfKey];
                            lines.push_back(line);
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
                                lines.push_back(line);
                                splitVector_total.insert(splitVector_total.end(), splitVector.begin(), splitVector.end());
                            } else {
                                all.insert( pair<string,vector<string>>(station,lines) );
                                lines.clear();
                                lines.push_back(line);
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
        
        return std::move(all);
    }
    
    void VLBI_initializer::createStationsFromCatalogs(const string &catalogPath) {
        cout << "Creating stations from catalog files:\n";
        cout << "  reading antenna.cat:\n";
        map<string,vector<string>> antennaCatalog  =  readCatalog(catalogPath,catalog::antenna);
        cout << "  reading position.cat:\n";
        map<string,vector<string>> positionCatalog =  readCatalog(catalogPath,catalog::position);
        cout << "  reading equip.cat:\n";
        map<string,vector<string>> equipCatalog    =  readCatalog(catalogPath,catalog::equip);
        cout << "  reading mask.cat:\n";
        map<string,vector<string>> maskCatalog     =  readCatalog(catalogPath,catalog::mask);

        unsigned long nant;
        int counter = 0;
        
        if (!PARA.selectedStations.empty()){
            nant = PARA.selectedStations.size();
        } else {
            nant = antennaCatalog.size();
        }
        
        int created = 0;
        // loop through all antennas in antennaCatalog
        for (auto& any : antennaCatalog){
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
            string type = any.second[2];
            double offset,rate1,con1,axis1_low,axis1_up,rate2,con2,axis2_low,axis2_up,diam;
            try{
                offset = boost::lexical_cast<double>(any.second.at(3));
                rate1 = boost::lexical_cast<double>(any.second.at(4));
                con1 = boost::lexical_cast<double>(any.second.at(5));
                axis1_low = boost::lexical_cast<double>(any.second.at(6));
                axis1_up = boost::lexical_cast<double>(any.second.at(7));
                rate2 = boost::lexical_cast<double>(any.second.at(8));
                con2 = boost::lexical_cast<double>(any.second.at(9));
                axis2_low = boost::lexical_cast<double>(any.second.at(10));
                axis2_up = boost::lexical_cast<double>(any.second.at(11));
                diam = boost::lexical_cast<double>(any.second.at(12));
            }
            catch(const std::exception& e){
                cout << "*** ERROR: "<< e.what() << " ***\n";
                continue;
            }

            // check if position.cat is long enough. Otherwise not all information is available.
            vector<string> po_cat = positionCatalog[id_PO];
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
                continue;
            }

            // check if equip.cat is long enough. Otherwise not all information is available.
            vector<string> eq_cat = equipCatalog[id_EQ + "|" + name];
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
                continue;
            }
            
            // check if an horizontal mask exists
            vector<double> hmask;
            if (maskCatalog.find(id_MS) != maskCatalog.end()){
                vector<string> mask_cat = maskCatalog[id_MS];
                
                // loop through all element and convert them 
                for(size_t i=3; i<mask_cat.size(); ++i){
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
                                            VLBI_cableWrap(axis1_low,axis1_up,axis2_low,axis2_up),
                                            VLBI_position(x,y,z),
                                            VLBI_equip(vector<string>{"X","S"},vector<double>{SEFD_X,SEFD_S}),
                                            VLBI_mask(hmask),
                                            type));
            created++;
            cout << boost::format("  %-8s added\n") %name;
            
        }
        cout <<"Finished! "<< created <<" of " << nant << " stations created\n\n" << endl;
    }
    
    void VLBI_initializer::createSourcesFromCatalogs(const string &catalogPath){
        cout << "Creating sources from catalog files:\n";
        cout << "  reading source.cat:\n";
        map<string,vector<string>> sourceCatalog  =  readCatalog(catalogPath,catalog::source);
        cout << "  reading flux.cat:\n";
        map<string,vector<string>> fluxCatalog =  readCatalog(catalogPath,catalog::flux);
        
        int counter = 0;
        unsigned long nsrc = sourceCatalog.size();
        int created = 0;
        
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
                continue;
            }
            double ra = 15*(ra_h + ra_m/60 + ra_s/3600);
            double de =    (abs(de_deg) + de_m/60 + de_s/3600);
            if (sign == '-'){
                de = -1*de;
            }

            vector<string> flux_cat = fluxCatalog[name];
//            if (flux_cat.size() < 6){
//                cout <<"*** ERROR: "<< name << ": flux.cat to small ***\n";
//                continue;
//            }

            vector<pair<string, VLBI_flux> > flux;

            vector<vector<string> > flux_split;
            for (unsigned int i=0; i<flux_cat.size(); ++i){
                vector<string> splitVector;
                boost::split( splitVector, flux_cat[i], boost::is_space(),boost::token_compress_on);
                if(splitVector.size()>3){
                    flux_split.push_back(splitVector);
                }
            }
            
            vector<int> alreadyConsidered;
            int cflux = 0;
            while (cflux < flux_split.size()){
                if(find(alreadyConsidered.begin(),alreadyConsidered.end(),cflux) != alreadyConsidered.end()){
                    ++cflux;
                    continue;
                }
                vector<string> parameters;
                alreadyConsidered.push_back(cflux);
                
                string thisBand = flux_split[cflux][1];
                string thisType = flux_split[cflux][2];
                if (thisType == "M"){
                    bool flagAdd = false;
                    if (flux_split[cflux].size() == 5){
                        flux_split[cflux].push_back("0");
                        flagAdd = true;
                    }
                    if (flux_split[cflux].size() == 4){
                        flux_split[cflux].push_back("0");
                        flux_split[cflux].push_back("0");
                        flagAdd = true;
                    }
                    if (flagAdd){
                        cout << "*** WARNING: Flux of type M lacks elements! zeros added!\n";
                    }
                }
                
                parameters.insert(parameters.end(), flux_split[cflux].begin()+3, flux_split[cflux].end());
                
                for(int i=cflux+1; i<flux_split.size(); ++i){
                    if (flux_split[i][1] == thisBand){
                        if (flux_split[i][2] == thisType){
                            parameters.insert(parameters.end(), flux_split[i].begin()+3, flux_split[i].end());
                        }else {
                            cerr << "*** ERROR: Source:" << name << "Flux: " << thisBand << "\n";
                            cerr << "    You can not mix B and M flux information for one band!";
                        }
                    }
                }
                
                VLBI_flux srcFlux(thisType);
                bool flagFlux = srcFlux.addFluxParameters(parameters);

                if(thisBand == "X"){
                    double wavelength = VLBI_obsMode::wavelength["X"];
                    srcFlux.setWavelength(wavelength);
                }else if(thisBand == "S"){
                    double wavelength = VLBI_obsMode::wavelength["S"];
                    srcFlux.setWavelength(wavelength);
                }


                if (flagFlux){
                    flux.push_back(make_pair(thisBand, srcFlux));
                }else{
                    cerr << "error reading flux info of: "<< name << "\n";
                }
                ++cflux;
            }
            if (!flux.empty()){
                sources.push_back(VLBI_source(name, ra, de, flux));
                created++;
                cout << boost::format("  %-8s added\n") %name;
            }
        }
        cout <<"Finished! "<< created <<" of " << nsrc << " sources created\n\n" << endl;
    }

    void VLBI_initializer::createSkyCoverages(){
        unsigned long nsta = stations.size();
        std::deque<bool> alreadyConsidered(nsta, false);
        int skyCoverageId = 0;
        vector<vector<int> > stationsPerId(nsta);
        
        for(int i=0; i<nsta; ++i){
            if(!alreadyConsidered[i]){
                stations[i].setSkyCoverageId(skyCoverageId);
                stationsPerId[skyCoverageId].push_back(i);
                alreadyConsidered[i] = true;
                for(int j=i+1; j<nsta; ++j){
                    if(!alreadyConsidered[j] && (stations[i].distance(stations[j])<PARA.maxDistanceTwinTeleskopes)){
                        stations[j].setSkyCoverageId(skyCoverageId);
                        stationsPerId[skyCoverageId].push_back(i);
                        alreadyConsidered[j] = true;
                    }
                }
                skyCoverageId++;
            }
        }
        
        for (int i=0; i<skyCoverageId; ++i){
            skyCoverages.push_back(
                    VLBI_skyCoverage(stationsPerId[i], PARA.skyCoverageDistance, PARA.skyCoverageInterval, i));
        }

        vector<int> sta2sky_(nsta);

        for (int i = 0; i < skyCoverages.size(); ++i) {
            vector<int> sky2sta = skyCoverages[i].getStaids();
            for (int j = 0; j < sky2sta.size(); ++j) {
                sta2sky_[sky2sta[j]] = i;
            }
        }


    }
    
    void VLBI_initializer::initializeStations(){
        int c = 0;
        boost::property_tree::ptree PARA_station;
        try{
            PARA_station = PARA_xml.get_child("master.station");
        }catch(const boost::property_tree::ptree_error &e){
            cerr << "ERROR: reading parameters.xml file!"<<
                    "    probably missing <station> block?" << endl;
            throw;
        }
        for(auto& any: stations){
            VLBI_pointingVector pV(c, 0);
            pV.setAz(any.getCableWrap().neutralPoint(1));
            pV.setEl(any.getCableWrap().neutralPoint(2));
            pV.setTime(0);
            any.pushPointingVector(pV);
            for (auto &it: PARA_station) {
                string group = it.first;
                if (group != "group"){
                    cerr << "ERROR: reading parameters.xml file!\n"<<
                            "    couldn't understand <" << group << "> in <station>" << endl;
                    continue;
                }
                boost::property_tree::ptree current_PARA_station = it.second;
                try{
                    string name = it.second.get_child("<xmlattr>.name").data();
                    string members = it.second.get_child("<xmlattr>.members").data();

                    vector<string> splitSta;
                    boost::split( splitSta, members, boost::algorithm::is_any_of(","));
                    if (find(splitSta.begin(), splitSta.end(), any.getName()) != splitSta.end() || members.compare("*") == 0){
                        any.setParameters(name, current_PARA_station);
                    }

                }catch(const boost::property_tree::ptree_error &e){
                    cerr << "ERROR: reading parameters.xml file!\n"<<
                            "    Probably missing 'name' or 'members' attribute in <station> <group>" << endl;
                    throw;
                }
            }
            any.setCableWrapMinimumOffsets();
            ++c;

        }

        unsigned long nsta = stations.size();
        for (int i = 0; i < nsta; ++i) {
            vector<double> distance(nsta);
            vector<double> dx(nsta);
            vector<double> dy(nsta);
            vector<double> dz(nsta);
            for (int j = i+1; j<nsta; ++j) {
                distance[j] = stations[i].distance(stations[j]);
                dx[j] = stations[j].getPosition().getX() - stations[i].getPosition().getX();
                dy[j] = stations[j].getPosition().getY() - stations[i].getPosition().getY();
                dz[j] = stations[j].getPosition().getZ() - stations[i].getPosition().getZ();
            }

            stations[i].preCalc(distance, dx, dy, dz);


        }
    }
    void VLBI_initializer::initializeSources(){
        
        boost::property_tree::ptree PARA_source;
        try{
            PARA_source = PARA_xml.get_child("master.source");
        }catch(const boost::property_tree::ptree_error &e){
            cout << "ERROR: reading parameters.xml file!"<<
                    "    probably missing <station> block?" << endl;
            throw;
        }

        vector<string> tooWeak;
        vector<double> tooWeak_Jy;
        int c = 0;
        while(c < sources.size()){
            VLBI_source& any = sources[c];
            for (auto &it: PARA_source) {
                string group = it.first;
                if (group != "group") {
                    cout << "ERROR: reading parameters.xml file!\n" <<
                         "    couldn't understand <" << group << "> in <source>" << endl;
                    continue;
                }
                boost::property_tree::ptree current_PARA_source = it.second;
                try {
                    string name = it.second.get_child("<xmlattr>.name").data();
                    string members = it.second.get_child("<xmlattr>.members").data();

                    vector<string> splitSrc;
                    boost::split(splitSrc, members, boost::algorithm::is_any_of(","));
                    if (find(splitSrc.begin(), splitSrc.end(), any.getName()) != splitSrc.end() ||
                        members.compare("*") == 0) {
                        any.setParameters(name, current_PARA_source);
                    }

                } catch (const boost::property_tree::ptree_error &e) {
                    cout << "ERROR: reading parameters.xml file!\n" <<
                         "    Probably missing 'name' or 'members' attribute in <source> <group>" << endl;
                    throw;
                }
            }
            double maxJy;
            bool flag = any.isStrongEnough(maxJy);

            if(!flag){
                sources.erase(sources.begin()+c);
                tooWeak.push_back(any.getName());
                tooWeak_Jy.push_back(maxJy);
                continue;
            } else {
                ++c;
            }
        }

        if (!tooWeak.empty()){
            cout << tooWeak.size() << " weak sources:\n";
            for (size_t i=0; i<tooWeak.size(); ++i){
                string name = tooWeak[i];
                double jy = tooWeak_Jy[i];
                cout << boost::format("  %-8s: %4.2f [Jy]\n") %name %jy;
            }
            cout << sources.size() << " sources left\n";
        }

        // TODO TO CLOSE TO SUN

        for (int i = 0; i < sources.size(); ++i) {
            sources[i].setId(i);
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

    void VLBI_initializer::initializeNutation() {
        vector<unsigned int> nut_t;
        vector<double> nut_x;
        vector<double> nut_y;
        vector<double> nut_s;

        double date1 = 2400000.5;
        double date2 = VieVS_timeEvents::mjdStart;

        unsigned int counter = 0;
        unsigned int frequency = 3600;
        unsigned int refTime;

        do {
            refTime = counter * frequency;
            date2 = VieVS_timeEvents::mjdStart + (double) refTime / 86400;

            double x, y, s;
            iauXys06a(date1, date2, &x, &y, &s);
            nut_t.push_back(refTime);
            nut_x.push_back(x);
            nut_y.push_back(y);
            nut_s.push_back(s);
            ++counter;
        } while (refTime < VieVS_timeEvents::duration + 3600);

        VieVS_nutation::nut_x = nut_x;
        VieVS_nutation::nut_y = nut_y;
        VieVS_nutation::nut_s = nut_s;
        VieVS_nutation::nut_time = nut_t;
    }

    void VLBI_initializer::initializeEarth() {
        double date1 = 2400000.5;
        double date2 = VieVS_timeEvents::mjdStart;
        double pvh[2][3];
        double pvb[2][3];
        iauEpv00(date1, date2, pvh, pvb);
        double aud2ms = DAU / DAYSEC;
        double vearth[3] = {aud2ms * pvb[1][0],
                            aud2ms * pvb[1][1],
                            aud2ms * pvb[1][2]};
        VieVS_earth::velocity = {vearth[0], vearth[1], vearth[2]};
    }

    void VLBI_initializer::initializeLookup() {


        unordered_map<int, double> sinLookup;
        double x = 0;
        int counter = 0;
        while (x < pi + 0.001) {
            double val = sin(x);
            sinLookup.insert(make_pair(counter, val));
            x += .001;
            ++counter;
        }
        VieVS_lookup::sinLookup = sinLookup;


        unordered_map<int, double> cosLookup;
        x = 0;
        counter = 0;
        while (x < pi + 0.001) {
            double val = cos(x);
            cosLookup.insert(make_pair(counter, val));
            x += .001;
            ++counter;
        }
        VieVS_lookup::cosLookup = cosLookup;

        unordered_map<int, double> acosLookup;
        x = 0;
        counter = 0;
        while (x < 1) {
            double val = acos(x);
            acosLookup.insert(make_pair(counter, val));
            x += .001;
            ++counter;
        }
        VieVS_lookup::acosLookup = acosLookup;

    }

    void VLBI_initializer::initializeWeightFactors() {
        VLBI_weightFactors::weight_skyCoverage = PARA_xml.get<double>("master.weightFactor.skyCoverage", 0);
        VLBI_weightFactors::weight_numberOfObservations = PARA_xml.get<double>("master.weightFactor.numberOfObservations", 0);
        VLBI_weightFactors::weight_duration = PARA_xml.get<double>("master.weightFactor.duration", 0);
        VLBI_weightFactors::weight_averageSources = PARA_xml.get<double>("master.weightFactor.averageSources", 0);
        VLBI_weightFactors::weight_averageStations = PARA_xml.get<double>("master.weightFactor.averageStations", 0);
    }

    void VLBI_initializer::initializeSkyCoverages() {
        unsigned int maxEl = 91;
        unsigned int sizeAz = 181;
        vector<vector<vector<float> > > storage;

        for (unsigned int thisEl = 0; thisEl < maxEl; ++thisEl) {
            unsigned int sizeEl = maxEl-thisEl;

            vector<vector<float> > thisStorage(sizeAz,vector<float>(sizeEl,0));


            for (int deltaAz = 0; deltaAz < sizeAz; ++deltaAz) {
                for (int deltaEl = 0; deltaEl < sizeEl; ++deltaEl) {
                    float tmp = (float) (sin(thisEl*deg2rad) * sin(thisEl*deg2rad + deltaEl*deg2rad) + cos(thisEl*deg2rad) * cos(thisEl*deg2rad + deltaEl*deg2rad) * cos(deltaAz*deg2rad));
                    float angle = acos(tmp);
                    thisStorage[deltaAz][deltaEl] = angle;
                }
            }
//            cout << thisEl << " of " << maxEl << endl;
            storage.push_back(thisStorage);
        }
        VLBI_skyCoverage::angularDistanceLookup = storage;
    }



    void VLBI_initializer::initializeBaselines() {
        unsigned long nsta = stations.size();
        vector<vector <char> > ignore(nsta, vector< char>(nsta,false));

        vector< vector<unsigned int> > minScan(nsta, vector<unsigned int>(nsta,0));

        vector< vector<unsigned int> > maxScan(nsta, vector<unsigned int>(nsta,numeric_limits<unsigned int>::max()));

        vector< vector<double> > weight(nsta, vector<double>(nsta,1));

        vector< vector<double> > this_minSNR(nsta, vector<double>(nsta,0));
        unordered_map<string,vector< vector<double> >> minSNR;

        for (const auto &it: VLBI_obsMode::wavelength) {
            string name = it.first;
            minSNR[name]=this_minSNR;
        }

        vector<string> stationNames;
        for (const auto& any:stations){
            stationNames.push_back(any.getName());
        }

        auto PARA_bl = PARA_xml.get_child("master.baseline");
        for (const auto &it: PARA_bl) {
            string name = it.first;
            vector<string> splitVector;
            boost::split(splitVector,name,boost::is_any_of("-_"));

            string sta1 = splitVector[0];
            auto its1 = find(stationNames.begin(),stationNames.end(),sta1);
            if(its1 == stationNames.end()){
                cerr << "station " << sta1 << " in baseline block not found!\n";
            }
            long idx1 = distance(stationNames.begin(), its1);

            string sta2 = splitVector[1];
            auto its2 = find(stationNames.begin(),stationNames.end(),sta2);
            if(its2 == stationNames.end()){
                cerr << "station " << sta2 << " in baseline block not found!\n";
            }
            long idx2 = distance(stationNames.begin(), its2);

            for (const auto &itBl:it.second) {
                string thisName = itBl.first;
                if (thisName == "ignore"){
//                    bool value = itBl.second.data().data();
                    bool value = itBl.second.get_value<bool>();
                    ignore[idx1][idx2] = value;
                    ignore[idx2][idx1] = value;
                }else if (thisName == "minScan"){
//                    unsigned int value = boost::lexical_cast<unsigned int>(itBl.second.data().data());
                    unsigned int value = itBl.second.get_value<unsigned int>();
                    minScan[idx1][idx2] = value;
                    minScan[idx2][idx1] = value;

                }else if (thisName == "maxScan"){
//                    unsigned int value = boost::lexical_cast<unsigned int>(itBl.second.data().data());
                    unsigned int value = itBl.second.get_value<unsigned int>();
                    maxScan[idx1][idx2] = value;
                    maxScan[idx2][idx1] = value;

                }else if (thisName == "weight"){
//                    double value = boost::lexical_cast<double>(itBl.second.data().data());
                    double value = itBl.second.get_value<double>();
                    weight[idx1][idx2] = value;
                    weight[idx2][idx1] = value;

                }else if (thisName == "minSNR"){
                    string bandName = itBl.second.get_child("<xmlattr>.band").data();
                    double value = itBl.second.get_value<double>();
                    minSNR[bandName][idx1][idx2] = value;
                    minSNR[bandName][idx2][idx1] = value;
                }

            }
        }

        VLBI_baseline::PARA.ignore = ignore;
        VLBI_baseline::PARA.minScan = minScan;
        VLBI_baseline::PARA.maxScan = maxScan;
        VLBI_baseline::PARA.weight = weight;
        VLBI_baseline::PARA.minSNR = minSNR;
    }

    void VLBI_initializer::initializeObservingMode() {
        auto PARA_mode = PARA_xml.get_child("master.mode");
        for (const auto &it: PARA_mode) {
            if(it.first == "bandwith"){
                VLBI_obsMode::bandwith = it.second.get_value<unsigned int>();
            }else if(it.first == "sample_rate"){
                VLBI_obsMode::sampleRate = it.second.get_value<unsigned int>();
            }else if(it.first == "fanout"){
                VLBI_obsMode::fanout = it.second.get_value<unsigned int>();
            }else if(it.first == "bits"){
                VLBI_obsMode::bits = it.second.get_value<unsigned int>();
            }else if(it.first == "band"){
                double wavelength;
                unsigned int channels;
                VLBI_obsMode::PROPERTY property;
                string name;

                for (const auto &it_band:it.second){

                    if(it_band.first == "<xmlattr>"){
                        name = it_band.second.get_child("name").data();
                    }else if(it_band.first == "wavelength"){
                        wavelength = it_band.second.get_value<double>();
                    }else if(it_band.first == "property"){

                        if (it_band.second.data() == "required"){
                            property = VLBI_obsMode::PROPERTY::required;
                        } else if (it_band.second.data() == "optional"){
                            property = VLBI_obsMode::PROPERTY::optional;
                        }

                    }else if(it_band.first == "chanels"){
                        channels = it_band.second.get_value<unsigned int>();
                    }
                }
                VLBI_obsMode::num_channels[name] = channels;
                VLBI_obsMode::wavelength[name] = wavelength;
                VLBI_obsMode::property[name] = property;
            }
        }

    }

}
