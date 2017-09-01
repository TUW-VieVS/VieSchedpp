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
        ifstream is("parameters.xml");

        cout << "reading: parameters.xml \n";
        boost::property_tree::read_xml(is, PARA_xml);

        try{
            boost::posix_time::ptime startTime = PARA_xml.get<boost::posix_time::ptime>("master.general.startTime");
            cout << "start time:" << startTime << "\n";
            int sec_ = startTime.time_of_day().total_seconds();
            double mjdStart = startTime.date().modjulian_day() + sec_ / 86400;


            boost::posix_time::ptime endTime = PARA_xml.get<boost::posix_time::ptime>("master.general.endTime");
            cout << "end time:" << endTime << "\n";


            boost::posix_time::time_duration a = endTime - startTime;
            int sec = a.total_seconds();
            if(sec<0){
                cerr << "ERROR: duration is less than zero seconds!\n";
            }
            unsigned int duration = (unsigned int) sec;
            cout << "duration: " << duration << " [s]\n";

            VieVS_time::mjdStart = mjdStart;
            VieVS_time::startTime = startTime;
            VieVS_time::endTime = endTime;
            VieVS_time::duration = duration;

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

    map<string, vector<string>> VLBI_initializer::readCatalog(const string &catalogPath, catalog type) noexcept {
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

    void VLBI_initializer::createStationsFromCatalogs(const string &catalogPath) noexcept {
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

    void VLBI_initializer::createSourcesFromCatalogs(const string &catalogPath) noexcept {
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

    void VLBI_initializer::createSkyCoverages() noexcept {
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

    void VLBI_initializer::initializeStations() noexcept {
        boost::property_tree::ptree PARA_station;
        try{
            PARA_station = PARA_xml.get_child("master.station");
        }catch(const boost::property_tree::ptree_error &e){
            cerr << "ERROR: reading parameters.xml file!"<<
                    "    probably missing <station> block?" << endl;
            throw;
        }

        unordered_map<std::string, std::vector<std::string> > groups = readGroups(PARA_station);

        unordered_map<std::string, VLBI_station::PARAMETERS> parameters;
        for (auto &it: PARA_station) {
            string name = it.first;
            if (name == "parameters") {
                string parameterName = it.second.get_child("<xmlattr>.name").data();

                VLBI_station::PARAMETERS PARA;

                for (auto &it2: it.second) {
                    string paraName = it2.first;
                    if (paraName == "<xmlattr>") {
                        continue;
                    } else if (paraName == "available") {
                        PARA.available = it2.second.get_value<bool>();
                    } else if (paraName == "firstScan") {
                        PARA.firstScan = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "weight") {
                        PARA.weight = it2.second.get_value<double>();
                    } else if (paraName == "minScan") {
                        PARA.minScan = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "maxScan") {
                        PARA.maxScan = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "maxSlewtime") {
                        PARA.maxSlewtime = it2.second.get_value < unsigned
                        int > ();
                    } else if (name == "maxWait") {
                        PARA.maxWait = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "wait_calibration") {
                        PARA.wait_calibration = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "wait_corsynch") {
                        PARA.wait_corsynch = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "wait_setup") {
                        PARA.wait_setup = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "wait_source") {
                        PARA.wait_source = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "wait_tape") {
                        PARA.wait_tape = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "minSNR") {
                        string bandName = it2.second.get_child("<xmlattr>.band").data();
                        double value = it2.second.get_value<double>();
                        PARA.minSNR[bandName] = value;
                    } else {
                        cerr << "<station> <parameter>: " << parameterName << ": parameter <" << name
                             << "> not understood! (Ignored)\n";
                    }
                }
                parameters[parameterName] = PARA;
            }
        }

        vector<vector<VLBI_station::EVENT> > events(stations.size());

        VLBI_station::PARAMETERS parentPARA;
        parentPARA.firstScan = false;
        parentPARA.available = true;

        parentPARA.axis1_low_offset = 5;
        parentPARA.axis1_up_offset = 5;
        parentPARA.axis2_low_offset = 1;
        parentPARA.axis2_up_offset = 1;

        parentPARA.wait_setup = 0;
        parentPARA.wait_source = 5;
        parentPARA.wait_tape = 1;
        parentPARA.wait_calibration = 10;
        parentPARA.wait_corsynch = 3;
        parentPARA.maxSlewtime = 9999;
        parentPARA.maxWait = 9999;
        parentPARA.maxScan = 600;
        parentPARA.minScan = 30;
        for (const auto &any:VLBI_obsMode::property) {
            string name = any.first;
            parentPARA.minSNR[name] = 0;
        }

        parentPARA.weight = 1;


        for (int i = 0; i < stations.size(); ++i) {
            VLBI_station::EVENT newEvent_start;
            newEvent_start.time = 0;
            newEvent_start.softTransition = false;
            newEvent_start.PARA = parentPARA;

            events[i].push_back(newEvent_start);

            VLBI_station::EVENT newEvent_end;
            newEvent_end.time = VieVS_time::duration;
            newEvent_end.softTransition = true;
            newEvent_end.PARA = parentPARA;

            events[i].push_back(newEvent_end);
        }

        for (auto &it: PARA_station) {
            string name = it.first;
            if (name == "setup") {
                stationSetup(events, it.second, parameters, groups, parentPARA);
            }
        }

        unsigned long nsta = stations.size();
        for (int i = 0; i < nsta; ++i) {
            VLBI_station &thisStation = stations[i];

            VLBI_pointingVector pV(i, 0);
            pV.setAz(thisStation.getCableWrap().neutralPoint(1));
            pV.setEl(thisStation.getCableWrap().neutralPoint(2));
            pV.setTime(0);

            thisStation.setCurrentPointingVector(pV);
            thisStation.setEVENTS(events[i]);
            thisStation.checkForNewEvent(0);
            thisStation.setCableWrapMinimumOffsets();

            vector<double> distance(nsta);
            vector<double> dx(nsta);
            vector<double> dy(nsta);
            vector<double> dz(nsta);
            for (int j = i+1; j<nsta; ++j) {
                VLBI_station &otherStation = stations[j];

                distance[j] = thisStation.distance(otherStation);
                dx[j] = otherStation.getPosition().getX() - thisStation.getPosition().getX();
                dy[j] = otherStation.getPosition().getY() - thisStation.getPosition().getY();
                dz[j] = otherStation.getPosition().getZ() - thisStation.getPosition().getZ();
            }

            thisStation.preCalc(distance, dx, dy, dz);
            thisStation.setFirstScan(true);
        }

    }

    void VLBI_initializer::stationSetup(vector<vector<VLBI_station::EVENT> > &events,
                                        const boost::property_tree::ptree &tree,
                                        const unordered_map<std::string, VLBI_station::PARAMETERS> &parameters,
                                        const unordered_map<std::string, std::vector<std::string> > &groups,
                                        const VLBI_station::PARAMETERS &parentPARA) noexcept {

        vector<string> members;
        VLBI_station::PARAMETERS combinedPARA = parentPARA;
        unsigned int start;
        unsigned int end;
        bool softTransition = true;

        for (auto &it: tree) {
            string paraName = it.first;
            if (paraName == "group") {
                string tmp = it.second.data();
                members = groups.at(tmp);
            } else if (paraName == "member") {
                string tmp = it.second.data();
                if (tmp == "__all__") {
                    for (const auto &any:stations) {
                        members.push_back(any.getName());
                    }
                } else {
                    members.push_back(tmp);
                }
            } else if (paraName == "parameter") {
                string tmp = it.second.data();
                VLBI_station::PARAMETERS newPARA = parameters.at(tmp);
                if (newPARA.available.is_initialized()) {
                    combinedPARA.available = *newPARA.available;
                }
                if (newPARA.firstScan.is_initialized()) {
                    combinedPARA.firstScan = *newPARA.firstScan;
                }

                if (newPARA.weight.is_initialized()) {
                    combinedPARA.weight = *newPARA.weight;
                }

                if (newPARA.axis1_up_offset.is_initialized()) {
                    combinedPARA.axis1_up_offset = *newPARA.axis1_up_offset;
                }
                if (newPARA.axis1_low_offset.is_initialized()) {
                    combinedPARA.axis1_low_offset = *newPARA.axis1_low_offset;
                }
                if (newPARA.axis2_up_offset.is_initialized()) {
                    combinedPARA.axis2_up_offset = *newPARA.axis2_up_offset;
                }
                if (newPARA.axis2_low_offset.is_initialized()) {
                    combinedPARA.axis2_low_offset = *newPARA.axis2_low_offset;
                }

                if (newPARA.minScan.is_initialized()) {
                    combinedPARA.minScan = *newPARA.minScan;
                }
                if (newPARA.maxScan.is_initialized()) {
                    combinedPARA.maxScan = *newPARA.maxScan;
                }
                if (newPARA.maxSlewtime.is_initialized()) {
                    combinedPARA.maxSlewtime = *newPARA.maxSlewtime;
                }
                if (newPARA.maxWait.is_initialized()) {
                    combinedPARA.maxWait = *newPARA.maxWait;
                }

                if (newPARA.wait_calibration.is_initialized()) {
                    combinedPARA.wait_calibration = *newPARA.wait_calibration;
                }
                if (newPARA.wait_corsynch.is_initialized()) {
                    combinedPARA.wait_corsynch = *newPARA.wait_corsynch;
                }
                if (newPARA.wait_setup.is_initialized()) {
                    combinedPARA.wait_setup = *newPARA.wait_setup;
                }
                if (newPARA.wait_source.is_initialized()) {
                    combinedPARA.wait_source = *newPARA.wait_source;
                }
                if (newPARA.wait_tape.is_initialized()) {
                    combinedPARA.wait_tape = *newPARA.wait_tape;
                }
                if (!newPARA.minSNR.empty()) {
                    for (const auto &any:newPARA.minSNR) {
                        string name = any.first;
                        double value = any.second;
                        combinedPARA.minSNR[name] = value;
                    }
                }

            } else if (paraName == "start") {
                start = it.second.get_value < unsigned
                int > ();
            } else if (paraName == "end") {
                end = it.second.get_value < unsigned
                int > ();
            } else if (paraName == "transition") {
                string tmp = it.second.data();
                if (tmp == "hard") {
                    softTransition = false;
                } else if (tmp == "soft") {
                    softTransition = true;
                } else {
                    cout << "ERROR: unknown transition type in <setup> block: " << tmp << "\n";
                }
            }
        }

        vector<string> staNames;
        for (const auto &any:stations) {
            staNames.push_back(any.getName());
        }

        for (const auto &any:members) {

            auto it = find(staNames.begin(), staNames.end(), any);
            int id = it - staNames.begin();
            auto &thisEvents = events[id];


            VLBI_station::EVENT newEvent_start;
            newEvent_start.time = start;
            newEvent_start.softTransition = softTransition;
            newEvent_start.PARA = combinedPARA;

            for (auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit) {
                if (iit->time > newEvent_start.time) {
                    thisEvents.insert(iit, newEvent_start);
                    break;
                }
            }

            VLBI_station::EVENT newEvent_end;
            newEvent_end.time = end;
            newEvent_end.softTransition = true;
            newEvent_end.PARA = parentPARA;
            for (auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit) {
                if (iit->time >= newEvent_end.time) {
                    thisEvents.insert(iit, newEvent_end);
                    break;
                }
            }
        }

        for (auto &it: tree) {
            string paraName = it.first;
            if (paraName == "setup") {
                stationSetup(events, it.second, parameters, groups, combinedPARA);
            }
        }
    }

    void VLBI_initializer::initializeSources() noexcept {
        
        boost::property_tree::ptree PARA_source;
        try{
            PARA_source = PARA_xml.get_child("master.source");
        }catch(const boost::property_tree::ptree_error &e){
            cout << "ERROR: reading parameters.xml file!" <<
                 "    probably missing <source> block?" << endl;
            throw;
        }

        unordered_map<std::string, std::vector<std::string> > groups = readGroups(PARA_source);

        unordered_map<std::string, VLBI_source::PARAMETERS> parameters;
        for (auto &it: PARA_source) {
            string name = it.first;
            if (name == "parameters") {
                string parameterName = it.second.get_child("<xmlattr>.name").data();

                VLBI_source::PARAMETERS PARA;

                for (auto &it2: it.second) {
                    string paraName = it2.first;
                    if (paraName == "<xmlattr>") {
                        continue;
                    } else if (paraName == "available") {
                        PARA.available = it2.second.get_value<bool>();
                    } else if (paraName == "weight") {
                        PARA.weight = it2.second.get_value<double>();
                    } else if (paraName == "minScan") {
                        PARA.minScan = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "maxScan") {
                        PARA.maxScan = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "minNumberOfStations") {
                        PARA.minNumberOfStations = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "minRepeat") {
                        PARA.minRepeat = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "minFlux") {
                        PARA.minFlux = it2.second.get_value<double>();
                    } else if (paraName == "fixedScanDuration") {
                        PARA.fixedScanDuration = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "minSNR") {
                        string bandName = it2.second.get_child("<xmlattr>.band").data();
                        double value = it2.second.get_value<double>();
                        PARA.minSNR[bandName] = value;
                    } else {
                        cerr << "<source> <parameter>: " << parameterName << ": parameter <" << name
                             << "> not understood! (Ignored)\n";
                    }
                }
                parameters[parameterName] = PARA;
            }
        }

        VLBI_source::PARAMETERS parentPARA;
        parentPARA.available = true;

        parentPARA.weight = 1; ///< multiplicative factor of score for scans to this source

        parentPARA.minNumberOfStations = 2; ///< minimum number of stations for a scan
        parentPARA.minFlux = .01; ///< minimum flux density required for this source in jansky
        parentPARA.minRepeat = 1800; ///< minimum time between two observations of this source in seconds
        parentPARA.maxScan = 600; ///< maximum allowed scan time in seconds
        parentPARA.minScan = 30; ///< minimum required scan time in seconds

        for (const auto &any:VLBI_obsMode::property) {
            string name = any.first;
            parentPARA.minSNR[name] = 0;
        }

        vector<vector<VLBI_source::EVENT> > events(sources.size());

        for (int i = 0; i < sources.size(); ++i) {
            VLBI_source::EVENT newEvent_start;
            newEvent_start.time = 0;
            newEvent_start.softTransition = false;
            newEvent_start.PARA = parentPARA;

            events[i].push_back(newEvent_start);

            VLBI_source::EVENT newEvent_end;
            newEvent_end.time = VieVS_time::duration;
            newEvent_end.softTransition = true;
            newEvent_end.PARA = parentPARA;

            events[i].push_back(newEvent_end);
        }

        for (auto &it: PARA_source) {
            string name = it.first;
            if (name == "setup") {
                sourceSetup(events, it.second, parameters, groups, parentPARA);
            }
        }
        for (int i = 0; i < sources.size(); ++i) {
            sources[i].setEVENTS(events[i]);
            sources[i].setId(i);
        }

        for (auto &any:sources) {
            any.checkForNewEvent(0);
        }


    }


    void VLBI_initializer::sourceSetup(vector<vector<VLBI_source::EVENT> > &events,
                                       const boost::property_tree::ptree &tree,
                                       const unordered_map<std::string, VLBI_source::PARAMETERS> &parameters,
                                       const unordered_map<std::string, std::vector<std::string> > &groups,
                                       const VLBI_source::PARAMETERS &parentPARA) noexcept {

        vector<string> members;
        VLBI_source::PARAMETERS combinedPARA = parentPARA;
        unsigned int start;
        unsigned int end;
        bool softTransition = true;

        for (auto &it: tree) {
            string paraName = it.first;
            if (paraName == "group") {
                string tmp = it.second.data();
                members = groups.at(tmp);
            } else if (paraName == "member") {
                string tmp = it.second.data();
                if (tmp == "__all__") {
                    for (const auto &any:sources) {
                        members.push_back(any.getName());
                    }
                } else {
                    members.push_back(tmp);
                }
            } else if (paraName == "parameter") {
                string tmp = it.second.data();
                VLBI_source::PARAMETERS newPARA = parameters.at(tmp);
                if (newPARA.available.is_initialized()) {
                    combinedPARA.available = *newPARA.available;
                }

                if (newPARA.weight.is_initialized()) {
                    combinedPARA.weight = *newPARA.weight;
                }

                if (newPARA.minNumberOfStations.is_initialized()) {
                    combinedPARA.minNumberOfStations = *newPARA.minNumberOfStations;
                }
                if (newPARA.minFlux.is_initialized()) {
                    combinedPARA.minFlux = *newPARA.minFlux;
                }
                if (newPARA.minRepeat.is_initialized()) {
                    combinedPARA.minRepeat = *newPARA.minRepeat;
                }
                if (newPARA.minScan.is_initialized()) {
                    combinedPARA.minScan = *newPARA.minScan;
                }
                if (newPARA.maxScan.is_initialized()) {
                    combinedPARA.maxScan = *newPARA.maxScan;
                }

                if (newPARA.fixedScanDuration.is_initialized()) {
                    combinedPARA.fixedScanDuration = *newPARA.fixedScanDuration;
                }


                if (!newPARA.minSNR.empty()) {
                    for (const auto &any:newPARA.minSNR) {
                        string name = any.first;
                        double value = any.second;
                        combinedPARA.minSNR[name] = value;
                    }
                }

            } else if (paraName == "start") {
                start = it.second.get_value < unsigned
                int > ();
            } else if (paraName == "end") {
                end = it.second.get_value < unsigned
                int > ();
            } else if (paraName == "transition") {
                string tmp = it.second.data();
                if (tmp == "hard") {
                    softTransition = false;
                } else if (tmp == "soft") {
                    softTransition = true;
                } else {
                    cout << "ERROR: unknown transition type in <setup> block: " << tmp << "\n";
                }
            }
        }

        vector<string> staNames;
        for (const auto &any:sources) {
            staNames.push_back(any.getName());
        }

        for (const auto &any:members) {

            auto it = find(staNames.begin(), staNames.end(), any);
            if (it == staNames.end()) {
                continue;
            }
            int id = it - staNames.begin();
            auto &thisEvents = events[id];


            VLBI_source::EVENT newEvent_start;
            newEvent_start.time = start;
            newEvent_start.softTransition = softTransition;
            newEvent_start.PARA = combinedPARA;

            for (auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit) {
                if (iit->time > newEvent_start.time) {
                    thisEvents.insert(iit, newEvent_start);
                    break;
                }
            }

            VLBI_source::EVENT newEvent_end;
            newEvent_end.time = end;
            newEvent_end.softTransition = true;
            newEvent_end.PARA = parentPARA;
            for (auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit) {
                if (iit->time >= newEvent_end.time) {
                    thisEvents.insert(iit, newEvent_end);
                    break;
                }
            }
        }

        for (auto &it: tree) {
            string paraName = it.first;
            if (paraName == "setup") {
                sourceSetup(events, it.second, parameters, groups, combinedPARA);
            }
        }

    }


    void VLBI_initializer::displaySummary() noexcept {
        
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

    void VLBI_initializer::initializeNutation() noexcept {
        vector<unsigned int> nut_t;
        vector<double> nut_x;
        vector<double> nut_y;
        vector<double> nut_s;

        double date1 = 2400000.5;
        double date2 = VieVS_time::mjdStart;

        unsigned int counter = 0;
        unsigned int frequency = 3600;
        unsigned int refTime;

        do {
            refTime = counter * frequency;
            date2 = VieVS_time::mjdStart + (double) refTime / 86400;

            double x, y, s;
            iauXys06a(date1, date2, &x, &y, &s);
            nut_t.push_back(refTime);
            nut_x.push_back(x);
            nut_y.push_back(y);
            nut_s.push_back(s);
            ++counter;
        } while (refTime < VieVS_time::duration + 3600);

        VieVS_nutation::nut_x = nut_x;
        VieVS_nutation::nut_y = nut_y;
        VieVS_nutation::nut_s = nut_s;
        VieVS_nutation::nut_time = nut_t;
    }

    void VLBI_initializer::initializeEarth() noexcept {
        double date1 = 2400000.5;
        double date2 = VieVS_time::mjdStart;
        double pvh[2][3];
        double pvb[2][3];
        iauEpv00(date1, date2, pvh, pvb);
        double aud2ms = DAU / DAYSEC;
        double vearth[3] = {aud2ms * pvb[1][0],
                            aud2ms * pvb[1][1],
                            aud2ms * pvb[1][2]};
        VieVS_earth::velocity = {vearth[0], vearth[1], vearth[2]};
    }

    void VLBI_initializer::initializeLookup() noexcept {


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

    void VLBI_initializer::initializeWeightFactors() noexcept {
        VLBI_weightFactors::weight_skyCoverage = PARA_xml.get<double>("master.weightFactor.skyCoverage", 0);
        VLBI_weightFactors::weight_numberOfObservations = PARA_xml.get<double>("master.weightFactor.numberOfObservations", 0);
        VLBI_weightFactors::weight_duration = PARA_xml.get<double>("master.weightFactor.duration", 0);
        VLBI_weightFactors::weight_averageSources = PARA_xml.get<double>("master.weightFactor.averageSources", 0);
        VLBI_weightFactors::weight_averageStations = PARA_xml.get<double>("master.weightFactor.averageStations", 0);
    }

    void VLBI_initializer::initializeSkyCoverages() noexcept {
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
            storage.push_back(thisStorage);
        }
        VLBI_skyCoverage::angularDistanceLookup = storage;
    }


    void VLBI_initializer::initializeBaselines() noexcept {

        boost::property_tree::ptree PARA_baseline;
        try {
            PARA_baseline = PARA_xml.get_child("master.baseline");
        } catch (const boost::property_tree::ptree_error &e) {
            cout << "ERROR: reading parameters.xml file!" <<
                 "    probably missing <baseline> block?" << endl;
            throw;
        }

        unordered_map<std::string, std::vector<std::string> > groups = readGroups(PARA_baseline);

        unordered_map<std::string, VLBI_baseline::PARAMETERS> parameters;
        for (auto &it: PARA_baseline) {
            string name = it.first;
            if (name == "parameters") {
                string parameterName = it.second.get_child("<xmlattr>.name").data();

                VLBI_baseline::PARAMETERS PARA;

                for (auto &it2: it.second) {
                    string paraName = it2.first;
                    if (paraName == "<xmlattr>") {
                        continue;
                    } else if (paraName == "ignore") {
                        PARA.ignore = it2.second.get_value<bool>();
                    } else if (paraName == "weight") {
                        PARA.weight = it2.second.get_value<double>();
                    } else if (paraName == "minScan") {
                        PARA.minScan = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "maxScan") {
                        PARA.maxScan = it2.second.get_value < unsigned
                        int > ();
                    } else if (paraName == "minSNR") {
                        string bandName = it2.second.get_child("<xmlattr>.band").data();
                        double value = it2.second.get_value<double>();
                        PARA.minSNR[bandName] = value;
                    } else {
                        cerr << "<baseline> <parameter>: " << parameterName << ": parameter <" << name
                             << "> not understood! (Ignored)\n";
                    }
                }
                parameters[parameterName] = PARA;
            }
        }

        unsigned long nsta = stations.size();
        vector<vector <char> > ignore(nsta, vector< char>(nsta,false));
        vector< vector<unsigned int> > minScan(nsta, vector<unsigned int>(nsta,0));
        vector< vector<unsigned int> > maxScan(nsta, vector<unsigned int>(nsta,numeric_limits<unsigned int>::max()));
        vector< vector<double> > weight(nsta, vector<double>(nsta,1));
        unordered_map<string,vector< vector<double> >> minSNR;

        for (const auto &any:VLBI_obsMode::property) {
            string name = any.first;
            vector<vector<double> > tmp(nsta, vector<double>(nsta, 0));
            minSNR[name] = tmp;
        }

        VLBI_baseline::PARA.ignore = ignore;
        VLBI_baseline::PARA.minScan = minScan;
        VLBI_baseline::PARA.maxScan = maxScan;
        VLBI_baseline::PARA.weight = weight;
        VLBI_baseline::PARA.minSNR = minSNR;

        VLBI_baseline::PARAMETERS parentPARA;
        parentPARA.ignore = false;
        parentPARA.weight = 1; ///< multiplicative factor of score for scans to this source
        parentPARA.maxScan = 9999; ///< maximum allowed scan time in seconds
        parentPARA.minScan = 0; ///< minimum required scan time in seconds
        for (const auto &any:VLBI_obsMode::property) {
            string name = any.first;
            parentPARA.minSNR[name] = 0;
        }

        vector<vector<vector<VLBI_baseline::EVENT> > > events(nsta, vector<vector<VLBI_baseline::EVENT> >(nsta));
        vector<vector<unsigned int> > nextEvent(nsta, vector<unsigned int>(nsta, 0));

        for (int i = 0; i < nsta; ++i) {
            for (int j = i + 1; j < nsta; ++j) {
                VLBI_baseline::EVENT newEvent_start;
                newEvent_start.time = 0;
                newEvent_start.softTransition = false;
                newEvent_start.PARA = parentPARA;

                events[i][j].push_back(newEvent_start);

                VLBI_baseline::EVENT newEvent_end;
                newEvent_end.time = VieVS_time::duration;
                newEvent_end.softTransition = true;
                newEvent_end.PARA = parentPARA;

                events[i][j].push_back(newEvent_end);
            }
        }


        for (auto &it: PARA_baseline) {
            string name = it.first;
            if (name == "setup") {
                baselineSetup(events, it.second, parameters, groups, parentPARA);
            }
        }

        VLBI_baseline::EVENTS = events;
        VLBI_baseline::nextEvent = nextEvent;

        VLBI_baseline::checkForNewEvent(0);

    }

    void VLBI_initializer::baselineSetup(vector<vector<vector<VLBI_baseline::EVENT> > > &events,
                                         const boost::property_tree::ptree &tree,
                                         const unordered_map<std::string, VLBI_baseline::PARAMETERS> &parameters,
                                         const unordered_map<std::string, std::vector<std::string> > &groups,
                                         const VLBI_baseline::PARAMETERS &parentPARA) noexcept {

        vector<string> members;
        VLBI_baseline::PARAMETERS combinedPARA = parentPARA;
        unsigned int start;
        unsigned int end;
        bool softTransition = true;

        for (auto &it: tree) {
            string paraName = it.first;
            if (paraName == "group") {
                string tmp = it.second.data();
                members = groups.at(tmp);
            } else if (paraName == "member") {
                string tmp = it.second.data();
                if (tmp == "__all__") {
                    for (const auto &any:sources) {
                        members.push_back(any.getName());
                    }
                } else {
                    members.push_back(tmp);
                }
            } else if (paraName == "parameter") {
                string tmp = it.second.data();
                VLBI_baseline::PARAMETERS newPARA = parameters.at(tmp);
                if (newPARA.ignore.is_initialized()) {
                    combinedPARA.ignore = *newPARA.ignore;
                }

                if (newPARA.weight.is_initialized()) {
                    combinedPARA.weight = *newPARA.weight;
                }

                if (newPARA.minScan.is_initialized()) {
                    combinedPARA.minScan = *newPARA.minScan;
                }
                if (newPARA.maxScan.is_initialized()) {
                    combinedPARA.maxScan = *newPARA.maxScan;
                }
                if (!newPARA.minSNR.empty()) {
                    for (const auto &any:newPARA.minSNR) {
                        string name = any.first;
                        double value = any.second;
                        combinedPARA.minSNR[name] = value;
                    }
                }

            } else if (paraName == "start") {
                start = it.second.get_value < unsigned
                int > ();
            } else if (paraName == "end") {
                end = it.second.get_value < unsigned
                int > ();
            } else if (paraName == "transition") {
                string tmp = it.second.data();
                if (tmp == "hard") {
                    softTransition = false;
                } else if (tmp == "soft") {
                    softTransition = true;
                } else {
                    cout << "ERROR: unknown transition type in <setup> block: " << tmp << "\n";
                }
            }
        }

        vector<string> staNames;
        for (const auto &any:stations) {
            staNames.push_back(any.getName());
        }

        for (const auto &any:members) {

            vector<string> splitVector;
            boost::split(splitVector, any, boost::is_any_of("-"));

            auto it1 = find(staNames.begin(), staNames.end(), splitVector[0]);
            int id1 = it1 - staNames.begin();
            auto it2 = find(staNames.begin(), staNames.end(), splitVector[1]);
            int id2 = it2 - staNames.begin();


            if (id1 > id2) {
                std::swap(id1, id2);
            }

            VLBI_baseline::EVENT newEvent_start;
            newEvent_start.time = start;
            newEvent_start.softTransition = softTransition;
            newEvent_start.PARA = combinedPARA;

            for (auto iit = events[id1][id2].begin(); iit < events[id1][id2].end(); ++iit) {
                if (iit->time > newEvent_start.time) {
                    events[id1][id2].insert(iit, newEvent_start);
                    break;
                }
            }

            VLBI_baseline::EVENT newEvent_end;
            newEvent_end.time = end;
            newEvent_end.softTransition = true;
            newEvent_end.PARA = parentPARA;
            for (auto iit = events[id1][id2].begin(); iit < events[id1][id2].end(); ++iit) {
                if (iit->time >= newEvent_end.time) {
                    events[id1][id2].insert(iit, newEvent_end);
                    break;
                }
            }
        }

        for (auto &it: tree) {
            string paraName = it.first;
            if (paraName == "setup") {
                baselineSetup(events, it.second, parameters, groups, combinedPARA);
            }
        }
    }


    void VLBI_initializer::initializeObservingMode() noexcept {
        auto PARA_mode = PARA_xml.get_child("master.mode");
        for (const auto &it: PARA_mode) {
            if(it.first == "bandwith"){
                VLBI_obsMode::bandwith = it.second.get_value<unsigned int>();
            } else if (it.first == "sampleRate") {
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

    unordered_map<string, vector<string> > VLBI_initializer::readGroups(boost::property_tree::ptree root) noexcept {
        unordered_map<std::string, std::vector<std::string> > groups;
        for (auto &it: root) {
            string name = it.first;
            if (name == "group") {
                string groupName = it.second.get_child("<xmlattr>.name").data();
                std::vector<std::string> members;
                for (auto &it2: it.second) {
                    if (it2.first == "member") {
                        members.push_back(it2.second.data());
                    }
                }
                groups[groupName] = members;
            }
        }
        return groups;
    }

}
