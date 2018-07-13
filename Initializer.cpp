/*
 * File:   Initializer.cpp
 * Author: mschartn
 * 
 * Created on June 28, 2017, 12:38 PM
 */

#include "Initializer.h"

using namespace std;
using namespace VieVS;
unsigned long Initializer::nextId = 0;

Initializer::Initializer(): VieVS_Object(nextId++){

};

Initializer::Initializer(const std::string &path): VieVS_Object(nextId++) {
    ifstream is(path);
    boost::property_tree::read_xml(is, xml_,boost::property_tree::xml_parser::trim_whitespace);
    double maxDistCorrestpondingTelescopes = xml_.get("master.skyCoverage.maxTwinTelecopeDistance",0.0);
    network_.setMaxDistBetweenCorrespondingTelescopes(maxDistCorrestpondingTelescopes);
}

Initializer::Initializer(const boost::property_tree::ptree &xml): VieVS_Object(nextId++), xml_{xml} {
    double maxDistCorrestpondingTelescopes = xml_.get("master.skyCoverage.maxTwinTelecopeDistance",0.0);
    network_.setMaxDistBetweenCorrespondingTelescopes(maxDistCorrestpondingTelescopes);
}


void Initializer::precalcSubnettingSrcIds() noexcept {
    unsigned long nsrc = sources_.size();
    vector<vector<unsigned long> > subnettingSrcIds(nsrc);
    for (int i = 0; i < nsrc; ++i) {
        for (int j = i + 1; j < nsrc; ++j) {
            double tmp = sin(sources_[i].getDe()) * sin(sources_[j].getDe()) + cos(sources_[i].getDe()) *
                         cos(sources_[j].getDe()) * cos(sources_[i].getRa() - sources_[j].getRa());
            double dist = acos(tmp);

            if (dist > parameters_.subnettingMinAngle){
                subnettingSrcIds.at(i).push_back(j);
            }
        }
    }
    preCalculated_.subnettingSrcIds = subnettingSrcIds;
}

void Initializer::createStations(const SkdCatalogReader &reader, ofstream &headerLog) noexcept {

    const map<string, vector<string> > &antennaCatalog = reader.getAntennaCatalog();
    const map<string, vector<string> > &positionCatalog = reader.getPositionCatalog();
    const map<string, vector<string> > &equipCatalog = reader.getEquipCatalog();
    const map<string, vector<string> > &maskCatalog = reader.getMaskCatalog();

    headerLog << "Create Stations:\n";
    unsigned long nant;
    int counter = 0;

    const vector<string> &selectedStations = reader.getStaNames();


    if (!selectedStations.empty()) {
        nant = selectedStations.size();
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
        if (!selectedStations.empty() &&
            (find(selectedStations.begin(), selectedStations.end(), name) == selectedStations.end())) {
            continue;
        }

        // look if vector in antenna.cat is long enough. Otherwise not all information is available!
        if (any.second.size() < 16){
            headerLog << "*** ERROR: " << any.first << ": antenna.cat to small ***\n";
            continue;
        }

        // convert all IDs to upper case for case insensitivity
        const string &id_PO = reader.positionKey(name);
        const string &id_EQ = reader.equipKey(name);
        const string &id_MS = reader.maskKey(name);
        const string &tlc = reader.getTwoLetterCode().at(name);

        // check if corresponding position and equip CATALOG exists.
        if (positionCatalog.find(id_PO) == positionCatalog.end()){
            headerLog << "*** ERROR: creating station "<< name <<": position CATALOG not found ***\n";
            continue;
        }
        if (equipCatalog.find(id_EQ) == equipCatalog.end()){
            headerLog << "*** ERROR: creating station "<< name <<": equip CATALOG not found ***\n";
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
            headerLog << "*** ERROR: creating station "<< name <<": " << e.what() << " ***\n";
            continue;
        }

        // check if position.cat is long enough. Otherwise not all information is available.
        vector<string> po_cat = positionCatalog.at(id_PO);
        if (po_cat.size()<5){
            headerLog << "*** ERROR: creating station "<< name <<": " << any.first << ": positon.cat to small ***\n";
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
            headerLog << "*** ERROR: creating station "<< name <<": " << e.what() << " ***\n";
            continue;
        }

        // check if equip.cat is long enough. Otherwise not all information is available.
        vector<string> eq_cat = equipCatalog.at(id_EQ);
        if (eq_cat.size()<9){
            headerLog << "*** ERROR: creating station "<< name <<": " << any.first << ": equip.cat to small ***\n";
            continue;
        }
        // check if SEFD_ information is in X and S band
        if (eq_cat[5] != "X" || eq_cat[7] != "S") {
            headerLog << "*** ERROR: creating station "<< name <<": " << any.first << ": we only support SX equipment ***\n";
            continue;
        }

        unordered_map<std::string, double> SEFDs;
        // convert all items from equip.cat
        unordered_map<std::string, double> SEFD_found;
        try{
            SEFD_found[eq_cat.at(5)] = boost::lexical_cast<double>(eq_cat.at(6));
            SEFD_found[eq_cat.at(7)] = boost::lexical_cast<double>(eq_cat.at(8));
        }
        catch(const std::exception& e){
            headerLog << "*** ERROR: creating station "<< name <<": " << e.what() << "\n";
            continue;
        }
        bool everythingOkWithBands = true;
        for(const auto &bandName: ObservationMode::bands){
            if(SEFD_found.find(bandName) != SEFD_found.end()){
                SEFDs[bandName] = SEFD_found[bandName];
            } else {
                if(ObservationMode::stationProperty[bandName] == ObservationMode::Property::required){
                    everythingOkWithBands = false;
                } else if(ObservationMode::stationBackup[bandName] == ObservationMode::Backup::value){
                    SEFDs[bandName] = ObservationMode::stationBackupValue[bandName];
                }
            }
        }
        if(!everythingOkWithBands){
            cerr << "ERROR: creating station "<< name <<": required SEFD_ information missing!;\n";
            continue;
        }

        if(SEFDs.size() != ObservationMode::bands.size()){
            if(SEFDs.empty()){
                cerr << "ERROR: creating station "<< name <<": no SEFD_ information found to calculate backup value!;\n";
                continue;
            }
            double max = 0;
            double min = std::numeric_limits<double>::max();
            for(const auto &any2:SEFDs){
                if(any2.second<min){
                    min = any2.second;
                }
                if(any2.second>max){
                    max = any2.second;
                }
            }
            for(const auto &bandName:ObservationMode::bands){
                if(SEFDs.find(bandName) == SEFDs.end()){
                    if(ObservationMode::stationBackup[bandName] == ObservationMode::Backup::minValueTimes){
                        SEFDs[bandName] = min * ObservationMode::stationBackupValue[bandName];
                    }
                    if(ObservationMode::stationBackup[bandName] == ObservationMode::Backup::maxValueTimes){
                        SEFDs[bandName] = max * ObservationMode::stationBackupValue[bandName];
                    }
                }
            }
        }


        bool elSEFD = false;
        unordered_map<std::string,double> SEFD_y;
        unordered_map<std::string,double> SEFD_c0;
        unordered_map<std::string,double> SEFD_c1;

        if(eq_cat.size()>=16){
            if (eq_cat[9] == "X" || eq_cat[9] == "S"){
                elSEFD = true;
                try{
                    string band = eq_cat[9];
                    auto elSEFD_y = boost::lexical_cast<double>(eq_cat.at(10));
                    auto elSEFD_c0 = boost::lexical_cast<double>(eq_cat.at(11));
                    auto elSEFD_c1 = boost::lexical_cast<double>(eq_cat.at(12));

                    SEFD_y[band] = elSEFD_y;
                    SEFD_c0[band] = elSEFD_c0;
                    SEFD_c1[band] = elSEFD_c1;


                }catch (const std::exception& e){
                    cerr << "ERROR: creating station "<< name <<": elevation dependent SEFD value not understood - ignored!!;\n";
                    elSEFD = false;
                }
            }
            if (eq_cat[13] == "X" || eq_cat[13] == "S"){
                try{
                    string band = eq_cat[13];
                    auto elSEFD_y = boost::lexical_cast<double>(eq_cat.at(14));
                    auto elSEFD_c0 = boost::lexical_cast<double>(eq_cat.at(15));
                    auto elSEFD_c1 = boost::lexical_cast<double>(eq_cat.at(16));

                    SEFD_y[band] = elSEFD_y;
                    SEFD_c0[band] = elSEFD_c0;
                    SEFD_c1[band] = elSEFD_c1;


                }catch (const std::exception& e){
                    cerr << "ERROR: creating station "<< name <<": elevation dependent SEFD value not understood - ignored!!;\n";
                    elSEFD = false;
                }
            }
        }

            // check if an horizontal mask exists
        vector<double> hmask;
        if (maskCatalog.find(id_MS) != maskCatalog.end()){
            vector<string> mask_cat = maskCatalog.at(id_MS);

            // loop through all element and convert them
            for(size_t i=3; i<mask_cat.size(); ++i){
                try{
                    hmask.push_back(boost::lexical_cast<double>(mask_cat.at(i)));
                }
                catch(const std::exception& e){
                    headerLog << "*** ERROR: creating station "<< name <<": mask catalog entry "<< mask_cat.at(i) << " not understood \n";
                }
            }
        } else {
            if (id_MS != "--"){
                headerLog << "*** ERROR: creating station "<< name <<": mask CATALOG not found ***\n";
            }
        }

        std::vector<double> hmask_az;
        std::vector<double> hmask_el;
        for(unsigned long i=0; i < hmask.size(); ++i){
            if (i%2==0){
                hmask_az.push_back(hmask.at(i)*deg2rad);
            }else{
                hmask_el.push_back(hmask.at(i)*deg2rad);
            }
        }

        if (!hmask.empty() && hmask_az.back() != twopi) {
            hmask_az.push_back(twopi);
            hmask_el.push_back(*hmask_el.end());
        }


        shared_ptr<Antenna> antenna;
        shared_ptr<CableWrap> cableWrap;
        if(type == "AZEL"){
            antenna = make_shared<Antenna_AzEl>(offset, diam, rate1, con1, rate2, con2);
            cableWrap = make_shared<CableWrap_AzEl>(axis1_low,axis1_up,axis2_low,axis2_up);
        }else if(type == "HADC"){
            antenna = make_shared<Antenna_HaDc>(offset, diam, rate1, con1, rate2, con2);
            cableWrap = make_shared<CableWrap_HaDc>(axis1_low,axis1_up,axis2_low,axis2_up);
        }else if(type == "XYEW"){
            antenna = make_shared<Antenna_XYew>(offset, diam, rate1, con1, rate2, con2);
            cableWrap = make_shared<CableWrap_XYew>(axis1_low,axis1_up,axis2_low,axis2_up);
        }

        shared_ptr<Equipment> equipment;
        if(elSEFD){
            equipment = make_shared<Equipment_elDependent>(SEFDs, SEFD_y, SEFD_c0, SEFD_c1);
        }else{
            equipment = make_shared<Equipment>(SEFDs);
        }

        auto position = make_shared<Position>(x,y,z);

        shared_ptr<HorizonMask> horizonMask;
        if(!hmask_az.empty() && hmask_az.size() == hmask_el.size()){
            horizonMask = make_shared<HorizonMask_line>(hmask_az,hmask_el);
        } else if(!hmask_az.empty()) {
            horizonMask = make_shared<HorizonMask_step>(hmask_az,hmask_el);
        }

        network_.addStation(Station(name, tlc, antenna, cableWrap, position, equipment, horizonMask));

        created++;
        headerLog << boost::format("  %-8s added\n") % name;

    }
    headerLog << "Finished! " << created << " of " << nant << " stations created\n\n";
}

void Initializer::createSources(const SkdCatalogReader &reader, std::ofstream &headerLog) noexcept {

    double flcon2{pi / (3600.0 * 180.0 * 1000.0)};

    const map<string, vector<string> > &sourceCatalog = reader.getSourceCatalog();
    const map<string, vector<string> > &fluxCatalog = reader.getFluxCatalog();

    int counter = 0;
    unsigned long nsrc = sourceCatalog.size();
    int created = 0;
    headerLog << "Create Sources:\n";

    vector<string> src_created;
    vector<string> src_ignored;
    vector<string> src_fluxInformationNotFound;
    vector<string> src_failed;


    vector<string> sel_sources;
    const auto &ptree_useSources = xml_.get_child_optional("master.general.onlyUseListedSources");
    if(ptree_useSources.is_initialized()){
        auto it = ptree_useSources->begin();
        while (it != ptree_useSources->end()) {
            auto item = it->second.data();
            sel_sources.push_back(item);
            ++it;
        }
    }

    vector<string> ignore_sources;
    const auto &ptree_ignoreSources = xml_.get_child_optional("master.general.ignoreListedSources");
    if(ptree_ignoreSources.is_initialized()){
        auto it = ptree_ignoreSources->begin();
        while (it != ptree_ignoreSources->end()) {
            auto item = it->second.data();
            ignore_sources.push_back(item);
            ++it;
        }
    }

    for (auto any: sourceCatalog){
        counter ++;
        string name = any.first;

        if (any.second.size() < 8){
            headerLog << "*** ERROR: " << any.first << ": source.cat to small ***\n";
            src_failed.push_back(name);
            continue;
        }
        string commonname = any.second.at(1);
        if(commonname == "$"){
            commonname = "";
        }

        if(!sel_sources.empty()){
            if(find(sel_sources.begin(),sel_sources.end(),name) == sel_sources.end() &&
               find(sel_sources.begin(),sel_sources.end(),commonname) == sel_sources.end()){
                src_ignored.push_back(name);
                continue;
            }
        }else{
            if(find(ignore_sources.begin(),ignore_sources.end(),name) != ignore_sources.end() ||
               find(ignore_sources.begin(),ignore_sources.end(),commonname) != ignore_sources.end()){
                src_ignored.push_back(name);
                continue;
            }
        }


        bool foundName = fluxCatalog.find(name) != fluxCatalog.end();
        bool foundCommName = (!commonname.empty() && fluxCatalog.find(commonname) != fluxCatalog.end());

        if ( !foundName && !foundCommName){
            src_fluxInformationNotFound.push_back(name);
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
            src_failed.push_back(name);
            headerLog << "*** ERROR: reading right ascension and declination for " << name << " ***\n";
            continue;
        }
        double ra = 15*(ra_h + ra_m/60 + ra_s/3600);
        double de =    (abs(de_deg) + de_m/60 + de_s/3600);
        if (sign == '-'){
            de = -1*de;
        }

        vector<string> flux_cat;
        if(foundName){
            flux_cat = fluxCatalog.at(name);
        }else{
            flux_cat = fluxCatalog.at(commonname);
        }

        unordered_map<string, unique_ptr<Flux> > flux;

        vector<vector<string> > flux_split;
        for (auto &i : flux_cat) {
            vector<string> splitVector;
            boost::split(splitVector, i, boost::is_space(), boost::token_compress_on);
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
            if(std::find(ObservationMode::bands.begin(),ObservationMode::bands.end(),thisBand) == ObservationMode::bands.end()){
                continue;
            }

            string thisType = flux_split[cflux][2];
            if (thisType == "M"){
                bool flagAdd = false;
                if (flux_split[cflux].size() == 5){
                    flux_split[cflux].emplace_back("0");
                    flagAdd = true;
                }
                if (flux_split[cflux].size() == 4){
                    flux_split[cflux].emplace_back("0");
                    flux_split[cflux].emplace_back("0");
                    flagAdd = true;
                }
                if (flagAdd){
                    headerLog << "*** WARNING: Flux of type M lacks elements! zeros added!\n";
                }
            }

            parameters.insert(parameters.end(), flux_split[cflux].begin()+3, flux_split[cflux].end());

            for(int i=cflux+1; i<flux_split.size(); ++i){
                if (flux_split[i][1] == thisBand){
                    if (flux_split[i][2] == thisType){
                        parameters.insert(parameters.end(), flux_split[i].begin()+3, flux_split[i].end());
                        alreadyConsidered.push_back(i);
                    }else {
                        src_failed.push_back(name);
                        cerr << "ERROR: Source:" << name << "Flux: " << thisBand
                             << " You can not mix B and M flux information for one band!;\n";
                    }
                }
            }

            unique_ptr<Flux> srcFlux;
            bool errorWhileReadingFlux = false;

            if(thisType == "M"){
                std::vector<double> tflux;
                std::vector<double> tmajorAxis;
                std::vector<double> taxialRatio;
                std::vector<double> tpositionAngle;

                unsigned long npara = parameters.size();

                unsigned long nmodels = npara / 6;
                for(unsigned int i=0; i<nmodels; ++i){
                    try{
                        tflux.push_back(boost::lexical_cast<double>(parameters.at(i*6+0)));
                        tmajorAxis.push_back(boost::lexical_cast<double>(parameters.at(i*6+1))* flcon2);
                        taxialRatio.push_back(boost::lexical_cast<double>(parameters.at(i*6+2)));
                        tpositionAngle.push_back(boost::lexical_cast<double>(parameters.at(i*6+3))*deg2rad);
                    }
                    catch(const std::exception& e){
                        errorWhileReadingFlux = true;
                        src_failed.push_back(name);
                        cout << "ERROR: " << parameters[0] << " " << parameters[1] << " " << e.what() << " reading flux information;\n";
                        break;
                    }
                }

                if(!errorWhileReadingFlux){
                    srcFlux = make_unique<Flux_M>(ObservationMode::wavelength[thisBand],tflux,tmajorAxis,taxialRatio,tpositionAngle);
                }
            }else{
                std::vector<double> knots; ///< baseline length of flux information (type B)
                std::vector<double> values; ///< corresponding flux information for baseline length (type B)

                unsigned long npara = parameters.size();
                for(int i=0; i<npara; ++i){
                    try{
                        if (i%2==0){
                            knots.push_back(boost::lexical_cast<double>(parameters[i]));
                        }
                        else {
                            values.push_back(boost::lexical_cast<double>(parameters[i]));
                        }
                    }
                    catch(const std::exception& e){
                        errorWhileReadingFlux = true;
                        src_failed.push_back(name);
                        cout << "ERROR: reading flux information; \n";
                        break;
                    }
                }

                if(!errorWhileReadingFlux){
                    srcFlux = make_unique<Flux_B>(ObservationMode::wavelength[thisBand],knots,values);
                }

            }

            if(!errorWhileReadingFlux){
                flux[thisBand] = move(srcFlux);
                ++cflux;
            }
        }

        bool fluxBandInfoOk = true;
        for(const auto &bandName: ObservationMode::bands){
            if(flux.find(bandName) == flux.end()){
                if(ObservationMode::sourceProperty[bandName] == ObservationMode::Property::required){
                    fluxBandInfoOk = false;
                    break;
                }
                if(ObservationMode::sourceBackup[bandName] == ObservationMode::Backup::value){

                    flux[bandName] = make_unique<Flux_B>(ObservationMode::wavelength[bandName],
                                                          vector<double>{0,13000},
                                                          vector<double>{ObservationMode::stationBackupValue[bandName]});
                }
            }
        }
        if(!fluxBandInfoOk){
            cerr << "WARNING: source " << name << " required flux information missing!;\n";
        }

        if(flux.size() != ObservationMode::bands.size()){
            if(flux.empty()){
                src_failed.push_back(name);
                cerr << "ERROR: source " << name << " no flux information found to calculate backup value!;\n";
                continue;
            }
            double max = 0;
            double min = std::numeric_limits<double>::max();
            for(const auto &any2:flux){
                //TODO use something like .getMinimumFlux instead of .getMaximumFlux
                if(any2.second->getMaximumFlux()<min){
                    min = any2.second->getMaximumFlux();
                }
                if(any2.second->getMaximumFlux()>max){
                    max = any2.second->getMaximumFlux();
                }
            }
            for(const auto &bandName:ObservationMode::bands){
                if(flux.find(bandName) == flux.end()){
                    if(ObservationMode::stationBackup[bandName] == ObservationMode::Backup::minValueTimes){

                        flux[bandName] = make_unique<Flux_B>(ObservationMode::wavelength[bandName],
                                                              vector<double>{0,13000},
                                                              vector<double>{min * ObservationMode::stationBackupValue[bandName]});
                    }
                    if(ObservationMode::stationBackup[bandName] == ObservationMode::Backup::maxValueTimes){

                        flux[bandName] = make_unique<Flux_B>(ObservationMode::wavelength[bandName],
                                                              vector<double>{0,13000},
                                                              vector<double>{max * ObservationMode::stationBackupValue[bandName]});
                    }
                }
            }
        }



        if (!flux.empty()){
            string name1, name2;
            if(commonname.empty()){
                name1 = name;
                name2 = "";
            }else{
                name1 = commonname;
                name2 = name;
            }
            sources_.emplace_back(name1, name2, ra, de, flux);
            created++;
            src_created.push_back(name);
        }
    }
    headerLog << "Finished! " << created << " of " << nsrc << " sources created\n\n";

    util::outputObjectList("Created sources",src_created,headerLog);
    util::outputObjectList("ignored sources",src_ignored,headerLog);
    util::outputObjectList("failed because of missing flux information",src_fluxInformationNotFound,headerLog);
    util::outputObjectList("failed to create source",src_failed,headerLog);

}

void Initializer::initializeGeneral(ofstream &headerLog) noexcept {
    try {

        string startString = xml_.get<string>("master.general.startTime");
        boost::posix_time::ptime startTime = TimeSystem::string2ptime(startString);
        headerLog << "start time: " << TimeSystem::ptime2string(startTime) << "\n";
        int sec_ = startTime.time_of_day().total_seconds();
        double mjdStart = startTime.date().modjulian_day() + sec_ / 86400.0;


        string endString = xml_.get<string>("master.general.endTime");
        boost::posix_time::ptime endTime = TimeSystem::string2ptime(endString);
        headerLog << "end time:   " << TimeSystem::ptime2string(endTime) << "\n";


        int sec = util::duration(startTime,endTime);
        if (sec < 0) {
            cerr << "ERROR: duration is less than zero seconds!;\n";
        }
        auto duration = static_cast<unsigned int>(sec);
        headerLog << "duration: " << duration << " [s]\n";

        TimeSystem::mjdStart = mjdStart;
        TimeSystem::startTime = startTime;
        TimeSystem::endTime = endTime;
        TimeSystem::duration = duration;

        vector<string> sel_stations;
        boost::property_tree::ptree stations = xml_.get_child("master.general.stations");
        auto it = stations.begin();
        while (it != stations.end()) {
            auto item = it->second.data();
            sel_stations.push_back(item);
            ++it;
        }
        parameters_.selectedStations = sel_stations;

        parameters_.subnetting = xml_.get<bool>("master.general.subnetting");
        parameters_.subnettingMinAngle = xml_.get<double>("master.general.subnettingMinAngle",120.)*deg2rad;
        parameters_.subnettingMinNSta = xml_.get<double>("master.general.subnettingMinNSta",60.)/100;

        parameters_.fillinmodeDuringScanSelection = xml_.get<bool>("master.general.fillinmodeDuringScanSelection",false);
        parameters_.fillinmodeInfluenceOnSchedule = xml_.get<bool>("master.general.fillinmodeInfluenceOnSchedule",false);
        parameters_.fillinmodeAPosteriori = xml_.get<bool>("master.general.fillinmodeAPosteriori",false);
        parameters_.idleToObservingTime = xml_.get<bool>("master.general.idleToObservingTime",false);

        std::string anchor = xml_.get<std::string>("master.general.scanAlignment","start");
        if(anchor == "start"){
            ScanTimes::setAlignmentAnchor(ScanTimes::AlignmentAnchor::start);
        }else if(anchor == "end"){
            ScanTimes::setAlignmentAnchor(ScanTimes::AlignmentAnchor::end);
        }else if(anchor == "individual"){
            ScanTimes::setAlignmentAnchor(ScanTimes::AlignmentAnchor::individual);
        }else{
            headerLog << "ERROR: cannot read scan alignment type:" << anchor << endl;
        }

    } catch (const boost::property_tree::ptree_error &e) {
        headerLog << "ERROR: reading parameters.xml file!" << endl;
    }

    headerLog << "\n";
}


void Initializer::initializeStations() noexcept {
    // get station tree
    const auto & PARA_station_o = xml_.get_child_optional("master.station");
    if(PARA_station_o.is_initialized()) {
        const auto &PARA_station = PARA_station_o.get();

        // get all defined baseline group
        staGroups_ = readGroups(PARA_station, GroupType::station);

        // get all defined parameters
        unordered_map<std::string, ParameterSettings::ParametersStations> parameters;
        const auto &para_tree = PARA_station.get_child("parameters");
        for (auto &it: para_tree) {
            string name = it.first;
            if (name == "parameter") {
                string parameterName = it.second.get_child("<xmlattr>.name").data();
                ParameterSettings::ParametersStations PARA;
                auto PARA_ = ParameterSettings::ptree2parameterStation(it.second);
                PARA = PARA_.second;
                parameters[parameterName] = PARA;
            }
        }

        // change ignore source name to id
        for (auto &any:parameters) {
            auto &iss = any.second.ignoreSourcesString;
            for (const auto &iss_n:iss) {
                for (const auto &src:sources_) {
                    if (src.hasName(iss_n)) {
                        any.second.ignoreSources.push_back(src.getId());
                        break;
                    }
                }
            }
        }

        // define backup parameter
        Station::Parameters parentPARA("backup");

        // store events for each station
        vector<vector<Station::Event> > events(network_.getNSta());

        // set observation mode band names
        for (const auto &any:ObservationMode::bands) {
            const string &name = any;
            parentPARA.minSNR[name] = ObservationMode::minSNR[name];
        }

        // create default events at start and end
        for (int i = 0; i < network_.getNSta(); ++i) {
            Station::Event newEvent_start(0, false, parentPARA);
            events[i].push_back(newEvent_start);

            Station::Event newEvent_end(TimeSystem::duration, true, parentPARA);
            events[i].push_back(newEvent_end);
        }

        // add setup for all stations
        for (auto &it: PARA_station) {
            string name = it.first;
            if (name == "setup") {
                stationSetup(events, it.second, parameters, staGroups_, parentPARA);
            }
        }

        // set to start state
        for (unsigned long ista = 0; ista < network_.getNSta(); ++ista) {
            Station &thisStation = network_.refStation(ista);

            PointingVector pV(ista, 0);
            pV.setAz(0);
            pV.setEl(0);
            pV.setTime(0);

            thisStation.setCurrentPointingVector(pV);
            thisStation.setEVENTS(events[ista]);

            bool hardBreak = false;
            thisStation.checkForNewEvent(0, hardBreak);

            thisStation.referencePARA().firstScan = true;
        }

        // set wait times
        vector<string> waitTimesInitialized;
        auto waitTime_tree = PARA_station.get_child("waitTimes");
        for (auto &it: waitTime_tree) {
            string name = it.first;
            if (name == "waitTime") {
                vector<string> waitTimesNow;
                string memberName = it.second.get_child("<xmlattr>.member").data();
                if (staGroups_.find(memberName) != staGroups_.end()) {
                    waitTimesNow.insert(waitTimesNow.end(), staGroups_[memberName].begin(), staGroups_[memberName].end());
                } else if (memberName == "__all__") {
                    for (const auto &sta:network_.getStations()) {
                        waitTimesNow.push_back(sta.getName());
                    }
                } else {
                    waitTimesNow.push_back(memberName);
                }

                bool errorFlagWaitTime = false;
                for (const auto &any:waitTimesNow) {
                    if (find(waitTimesInitialized.begin(), waitTimesInitialized.end(), any) !=
                        waitTimesInitialized.end()) {
                        cerr << "ERROR: double use of station/group " << name
                             << " in wait times block! This whole block is ignored!;\n";
                        errorFlagWaitTime = true;
                    }
                }
                if (errorFlagWaitTime) {
                    continue;
                }


                for (const auto &any: waitTimesNow) {
                    for (auto &sta: network_.refStations()) {
                        if (sta.hasName(any)) {
                            Station::WaitTimes wtimes;
                            wtimes.fieldSystem = it.second.get<double>("fieldSystem");
                            wtimes.preob = it.second.get<double>("preob");
                            wtimes.midob = it.second.get<double>("midob");
                            wtimes.postob = it.second.get<double>("postob");
                            sta.setWaitTimes(wtimes);
                            break;
                        }
                    }
                }
                waitTimesInitialized.insert(waitTimesInitialized.end(), waitTimesNow.begin(), waitTimesNow.end());
            }
        }

        // set cable wrap buffers
        auto cableBuffer_tree = PARA_station.get_child("cableWrapBuffers");
        vector<string> cableInitialized;
        for (auto &it: cableBuffer_tree) {
            string name = it.first;
            if (name == "cableWrapBuffer") {
                vector<string> cableNow;
                string memberName = it.second.get_child("<xmlattr>.member").data();
                if (staGroups_.find(memberName) != staGroups_.end()) {
                    cableNow.insert(cableNow.end(), staGroups_[memberName].begin(), staGroups_[memberName].end());
                } else if (memberName == "__all__") {
                    for (const auto &sta: network_.getStations()) {
                        cableNow.push_back(sta.getName());
                    }
                } else {
                    cableNow.push_back(memberName);
                }

                bool errorFlagWaitTime = false;
                for (const auto &any:cableNow) {
                    if (find(cableInitialized.begin(), cableInitialized.end(), any) != cableInitialized.end()) {
                        cerr << "ERROR: double use of station/group " << name
                             << " in wait times block! This whole block is ignored!;\n";
                        errorFlagWaitTime = true;
                    }
                }
                if (errorFlagWaitTime) {
                    continue;
                }


                for (const auto &any: cableNow) {
                    for (auto &sta:network_.refStations()) {
                        if (sta.hasName(any)) {
                            auto axis1Low = it.second.get<double>("axis1LowOffset");
                            auto axis1Up = it.second.get<double>("axis1UpOffset");
                            auto axis2Low = it.second.get<double>("axis2LowOffset");
                            auto axis2Up = it.second.get<double>("axis2UpOffset");
                            sta.referenceCableWrap().setMinimumOffsets(axis1Low, axis1Up, axis2Low, axis2Up);
                            break;
                        }
                    }
                }
                cableInitialized.insert(cableInitialized.end(), cableNow.begin(), cableNow.end());
            }
        }
    }else{
        cout << "ERROR: reading parameters.xml file!" <<
             "    probably missing <station> block?;" << endl;
    }
}

void Initializer::stationSetup(vector<vector<Station::Event> > &events,
                               const boost::property_tree::ptree &tree,
                               const unordered_map<std::string, ParameterSettings::ParametersStations> &parameters,
                               const unordered_map<std::string, std::vector<std::string> > &groups,
                               const Station::Parameters &parentPARA) noexcept {

    vector<string> members;
    Station::Parameters combinedPARA = Station::Parameters(tree.get<string>("parameter"));
    combinedPARA.setParameters(parentPARA);
    unsigned int start = 0;
    unsigned int end = TimeSystem::duration;
    bool softTransition = true;

    for (auto &it: tree) {
        string paraName = it.first;
        if (paraName == "group") {
            string tmp = it.second.data();
            members = groups.at(tmp);
        } else if (paraName == "member") {
            string tmp = it.second.data();
            if (tmp == "__all__") {
                for (const auto &any:network_.getStations()) {
                    members.push_back(any.getName());
                }
            } else {
                members.push_back(tmp);
            }
        } else if (paraName == "parameter") {
            string tmp = it.second.data();

            ParameterSettings::ParametersStations newPARA = parameters.at(tmp);
            if (newPARA.available.is_initialized()) {
                combinedPARA.available = *newPARA.available;
            }
            if (newPARA.availableForFillinmode.is_initialized()) {
                combinedPARA.availableForFillinmode = *newPARA.availableForFillinmode;
            }

            if (newPARA.tagalong.is_initialized()) {
                combinedPARA.tagalong = *newPARA.tagalong;
            }
//            if (newPARA.firstScan.is_initialized()) {
//                combinedPARA.firstScan = *newPARA.firstScan;
//            }

            if (newPARA.weight.is_initialized()) {
                combinedPARA.weight = *newPARA.weight;
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
            if (newPARA.maxSlewDistance.is_initialized()) {
                combinedPARA.maxSlewDistance = *newPARA.maxSlewDistance * deg2rad;
            }
            if (newPARA.minSlewDistance.is_initialized()) {
                combinedPARA.minSlewDistance = *newPARA.minSlewDistance * deg2rad;
            }
            if (newPARA.maxWait.is_initialized()) {
                combinedPARA.maxWait = *newPARA.maxWait;
            }
            if (newPARA.minElevation.is_initialized()){
                combinedPARA.minElevation = *newPARA.minElevation*deg2rad;
            }
            if (newPARA.maxNumberOfScans.is_initialized()) {
                combinedPARA.maxNumberOfScans = *newPARA.maxNumberOfScans;
            }

            if (!newPARA.minSNR.empty()) {
                for (const auto &any:newPARA.minSNR) {
                    string name = any.first;
                    double value = any.second;
                    combinedPARA.minSNR[name] = value;
                }
            }
            if (!newPARA.ignoreSources.empty()) {
                combinedPARA.ignoreSources = newPARA.ignoreSources;
            }

        } else if (paraName == "start") {
            string t = it.second.get_value<string>();
            boost::posix_time::ptime thisStartTime = TimeSystem::string2ptime(t);
            start = TimeSystem::posixTime2InternalTime(thisStartTime);
        } else if (paraName == "end") {
            string t = it.second.get_value<string>();
            boost::posix_time::ptime thisEndTime = TimeSystem::string2ptime(t);
            end = TimeSystem::posixTime2InternalTime(thisEndTime);
        } else if (paraName == "transition") {
            string tmp = it.second.data();
            if (tmp == "hard") {
                softTransition = false;
            } else if (tmp == "soft") {
                softTransition = true;
            } else {
                cout << "ERROR: unknown transition type in <setup> block: " << tmp << ";\n";
            }
        }
    }

    vector<string> staNames;
    for (const auto &any: network_.getStations()) {
        staNames.push_back(any.getName());
    }

    for (const auto &any:members) {

        auto it = find(staNames.begin(), staNames.end(), any);
        long id = distance(staNames.begin(), it);
        auto &thisEvents = events[id];


        Station::Event newEvent_start(start,softTransition,combinedPARA);

        for (auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit) {
            if (iit->time > newEvent_start.time) {
                thisEvents.insert(iit, newEvent_start);
                break;
            }
        }

        Station::Event newEvent_end(end,true,parentPARA);
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

void Initializer::initializeSources() noexcept {

    // get source tree
    const auto & PARA_source_o = xml_.get_child_optional("master.source");
    if(PARA_source_o.is_initialized()) {
        const auto &PARA_source = PARA_source_o.get();

        // get all defined source groups
        srcGroups_ = readGroups(PARA_source, GroupType::source);

        // get all defined parameters
        const auto & para_tree = PARA_source.get_child("parameters");
        unordered_map<std::string, ParameterSettings::ParametersSources> parameters;
        for (auto &it: para_tree) {
            string name = it.first;
            if (name == "parameter") {
                string parameterName = it.second.get_child("<xmlattr>.name").data();

                ParameterSettings::ParametersSources PARA;

                auto PARA_ = ParameterSettings::ptree2parameterSource(it.second);
                PARA = PARA_.second;

                parameters[parameterName] = PARA;
            }
        }
        // change names in parameters to ids
        for (auto &any:parameters) {

            // get ignore station ids
            auto &ignoreStationsString = any.second.ignoreStationsString;
            for (const auto &ignoreStationName:ignoreStationsString) {
                unsigned long id = network_.getStation(ignoreStationName).getId();
                any.second.ignoreStations.push_back(id);
            }

            // get required station ids
            auto &requiredStationsString = any.second.requiredStationsString;
            for (const auto &requiredStationName:requiredStationsString) {
                unsigned long id = network_.getStation(requiredStationName).getId();
                any.second.requiredStations.push_back(id);
            }

            // get ignore baseline ids
            auto &ignoreBaselineString = any.second.ignoreBaselinesString;
            for (const auto &ignoreBaselineName: ignoreBaselineString) {
                unsigned long id = network_.getBaseline(ignoreBaselineName).getId();
                any.second.ignoreBaselines.push_back(id);
            }
        }

        // define backup parameter
        Source::Parameters parentPARA("backup");

        // set observation mode band names
        for (const auto &any:ObservationMode::bands) {
            const string &name = any;
            parentPARA.minSNR[name] = ObservationMode::minSNR[name];
        }

        // store events for each source
        vector<vector<Source::Event> > events(sources_.size());

        // create default events at start and end
        for (int i = 0; i < sources_.size(); ++i) {
            Source::Event newEvent_start(0, false, parentPARA);
            events[i].push_back(newEvent_start);
            Source::Event newEvent_end(TimeSystem::duration, true, parentPARA);
            events[i].push_back(newEvent_end);
        }

        // add setup for all sources
        for (auto &it: PARA_source) {
            string name = it.first;
            if (name == "setup") {
                sourceSetup(events, it.second, parameters, srcGroups_, parentPARA);
            }
        }

        // set events for all sources
        for (int i = 0; i < sources_.size(); ++i) {
            sources_[i].setEVENTS(events[i]);
        }

        // set to start event
        for (auto &any:sources_) {
            bool hardBreak = false;
            any.checkForNewEvent(0, hardBreak);
        }
    }else{
        cout << "ERROR: reading parameters.xml file!" <<
             "    probably missing <source> block?;" << endl;
    }
}


void Initializer::sourceSetup(vector<vector<Source::Event> > &events,
                              const boost::property_tree::ptree &tree,
                              const unordered_map<std::string, ParameterSettings::ParametersSources> &parameters,
                              const unordered_map<std::string, std::vector<std::string> > &groups,
                              const Source::Parameters &parentPARA) noexcept {

    vector<string> members;
    Source::Parameters combinedPARA = Source::Parameters( tree.get<string>("parameter"));
    combinedPARA.setParameters(parentPARA);
    unsigned int start = 0;
    unsigned int end = TimeSystem::duration;
    bool softTransition = true;

    for (auto &it: tree) {
        string paraName = it.first;
        if (paraName == "group") {
            string tmp = it.second.data();
            members = groups.at(tmp);
        } else if (paraName == "member") {
            string tmp = it.second.data();
            if (tmp == "__all__") {
                for (const auto &any:sources_) {
                    members.push_back(any.getName());
                }
            } else {
                members.push_back(tmp);
            }
        } else if (paraName == "parameter") {
            const string &tmp = it.second.data();

            ParameterSettings::ParametersSources newPARA = parameters.at(tmp);
            if (newPARA.available.is_initialized()) {
                combinedPARA.available = *newPARA.available;
            }
            if (newPARA.availableForFillinmode.is_initialized()) {
                combinedPARA.availableForFillinmode = *newPARA.availableForFillinmode;
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
            if (newPARA.maxNumberOfScans.is_initialized()) {
                combinedPARA.maxNumberOfScans = *newPARA.maxNumberOfScans;
            }
            if (newPARA.maxNumberOfScans.is_initialized()) {
                combinedPARA.maxNumberOfScans = *newPARA.maxNumberOfScans;
            }
            if (newPARA.tryToFocusIfObservedOnce.is_initialized()) {
                combinedPARA.tryToFocusIfObservedOnce = *newPARA.tryToFocusIfObservedOnce;
                combinedPARA.tryToFocusFactor = *newPARA.tryToFocusFactor;
                if(*newPARA.tryToFocusOccurrency == ParameterSettings::TryToFocusOccurrency::once){
                    combinedPARA.tryToFocusOccurrency = Source::TryToFocusOccurrency::once;
                }else{
                    combinedPARA.tryToFocusOccurrency = Source::TryToFocusOccurrency::perScan;
                }
                if(*newPARA.tryToFocusType == ParameterSettings::TryToFocusType::additive){
                    combinedPARA.tryToFocusType = Source::TryToFocusType::additive;
                }else{
                    combinedPARA.tryToFocusType = Source::TryToFocusType::multiplicative;
                }
            }
            if (newPARA.tryToObserveXTimesEvenlyDistributed.is_initialized()) {
                combinedPARA.tryToObserveXTimesEvenlyDistributed = *newPARA.tryToObserveXTimesEvenlyDistributed;
                combinedPARA.tryToObserveXTimesMinRepeat = *newPARA.tryToObserveXTimesMinRepeat;
            }
            if (newPARA.fixedScanDuration.is_initialized()) {
                combinedPARA.fixedScanDuration = *newPARA.fixedScanDuration;
            }
            if (newPARA.minElevation.is_initialized()){
                combinedPARA.minElevation = *newPARA.minElevation*deg2rad;
            }
            if (newPARA.minSunDistance.is_initialized()){
                combinedPARA.minSunDistance = *newPARA.minSunDistance*deg2rad;
            }

            if (!newPARA.ignoreStations.empty()) {
                combinedPARA.ignoreStations = newPARA.ignoreStations;
            }
            if (!newPARA.requiredStations.empty()) {
                combinedPARA.requiredStations = newPARA.requiredStations;
            }
            if (!newPARA.ignoreBaselines.empty()) {
                combinedPARA.ignoreBaselines = newPARA.ignoreBaselines;
            }

            if (!newPARA.minSNR.empty()) {
                for (const auto &any:newPARA.minSNR) {
                    string name = any.first;
                    double value = any.second;
                    combinedPARA.minSNR[name] = value;
                }
            }

        } else if (paraName == "start") {
            string t = it.second.get_value<string>();
            boost::posix_time::ptime thisStartTime = TimeSystem::string2ptime(t);
            start = TimeSystem::posixTime2InternalTime(thisStartTime);
        } else if (paraName == "end") {
            string t = it.second.get_value<string>();
            boost::posix_time::ptime thisEndTime = TimeSystem::string2ptime(t);
            end = TimeSystem::posixTime2InternalTime(thisEndTime);
        } else if (paraName == "transition") {
            string tmp = it.second.data();
            if (tmp == "hard") {
                softTransition = false;
            } else if (tmp == "soft") {
                softTransition = true;
            } else {
                cout << "ERROR: unknown transition type in <setup> block: " << tmp << ";\n";
            }
        }
    }


    vector<string> srcNames;
    for (const auto &any:sources_) {
        srcNames.push_back(any.getName());
    }

    bool tryToFocusIfObservedOnceBackup = combinedPARA.tryToFocusIfObservedOnce;
    unsigned int maxNumberOfScansBackup = combinedPARA.maxNumberOfScans;
    unsigned int minRepeatBackup = combinedPARA.minRepeat;

    for (const auto &any:members) {

        auto it = find(srcNames.begin(), srcNames.end(), any);
        if (it == srcNames.end()) {
            continue;
        }


        long id = distance(srcNames.begin(), it);


        if(combinedPARA.tryToObserveXTimesEvenlyDistributed.is_initialized() && *combinedPARA.tryToObserveXTimesEvenlyDistributed){
            const Source &thisSource = sources_[id];
            combinedPARA.tryToFocusIfObservedOnce = true;
            combinedPARA.maxNumberOfScans = *combinedPARA.tryToObserveXTimesEvenlyDistributed;


            unsigned int minutes = minutesVisible(thisSource,combinedPARA,start,end);
            unsigned int minRepeat = (60*minutes)/(combinedPARA.maxNumberOfScans);
            unsigned int minRepeatOther = *combinedPARA.tryToObserveXTimesMinRepeat;
            if(minRepeat < minRepeatOther){
                minRepeat = minRepeatOther;
            }
            combinedPARA.minRepeat = minRepeat;
        }
        auto &thisEvents = events[id];


        Source::Event newEvent_start(start, softTransition, combinedPARA);

        for (auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit) {
            if (iit->time > newEvent_start.time) {
                thisEvents.insert(iit, newEvent_start);
                break;
            }
        }

        Source::Event newEvent_end(end, true, parentPARA);
        for (auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit) {
            if (iit->time >= newEvent_end.time) {
                thisEvents.insert(iit, newEvent_end);
                break;
            }
        }

        combinedPARA.tryToFocusIfObservedOnce = tryToFocusIfObservedOnceBackup;
        combinedPARA.maxNumberOfScans = maxNumberOfScansBackup;
        combinedPARA.minRepeat = minRepeatBackup;

    }

    for (auto &it: tree) {
        string paraName = it.first;
        if (paraName == "setup") {
            sourceSetup(events, it.second, parameters, groups, combinedPARA);
        }
    }
}

void Initializer::initializeBaselines() noexcept {
    // get baseline tree
    const auto & PARA_baseline_o = xml_.get_child_optional("master.baseline");
    if(PARA_baseline_o.is_initialized()){
        const auto & PARA_baseline = PARA_baseline_o.get();

        // get all defined baseline groups
        blGroups_ = readGroups(PARA_baseline, GroupType::baseline);

        // get all defined parameters
        const auto &para_tree = PARA_baseline.get_child("parameters");
        unordered_map<std::string, ParameterSettings::ParametersBaselines> parameters;
        for (auto &it: para_tree) {
            string name = it.first;
            if (name == "parameter") {
                string parameterName = it.second.get_child("<xmlattr>.name").data();

                ParameterSettings::ParametersBaselines PARA;

                auto PARA_ = ParameterSettings::ptree2parameterBaseline(it.second);
                PARA = PARA_.second;

                parameters[parameterName] = PARA;
            }
        }

        // define backup parameter
        Baseline::Parameters parentPARA("backup");

        // set observation mode band names
        for (const auto &any:ObservationMode::bands) {
            const string &name = any;
            parentPARA.minSNR[name] = ObservationMode::minSNR[name];
        }


        // store events for each baseline
        vector<vector<Baseline::Event> > events(network_.getNBls());

        // create default events at start and end
        for (int i = 0; i < network_.getNBls(); ++i) {
            Baseline::Event newEvent_start(0,false,parentPARA);
            events[i].push_back(newEvent_start);
            Baseline::Event newEvent_end(TimeSystem::duration, true, parentPARA);
            events[i].push_back(newEvent_end);
        }

        // add setup for all baselines
        for (auto &it: PARA_baseline) {
            string name = it.first;
            if (name == "setup") {
                baselineSetup(events, it.second, parameters, blGroups_, parentPARA);
            }
        }
        // set events for all baselines
        for (unsigned int i = 0; i < network_.getNBls(); ++i) {
            Baseline &bl = network_.refBaseline(i);
            bl.setEVENTS(events[i]);
        }

        // set to start event
        for (auto &thisBl:network_.refBaselines()) {
            bool dummy = false;
            thisBl.checkForNewEvent(0, dummy);
        }

    }else{
        cout << "ERROR: reading parameters.xml file!" <<
             "    probably missing <baseline> block?;" << endl;
    }

}

void Initializer::baselineSetup(vector<vector<Baseline::Event> > &events,
                                const boost::property_tree::ptree &tree,
                                const unordered_map<std::string, ParameterSettings::ParametersBaselines> &parameters,
                                const unordered_map<std::string, std::vector<std::string> > &groups,
                                const Baseline::Parameters &parentPARA) noexcept {

    vector<string> members;
    Baseline::Parameters combinedPARA = Baseline::Parameters( tree.get<string>("parameter"));
    combinedPARA.setParameters(parentPARA);
    unsigned int start = 0;
    unsigned int end = TimeSystem::duration;
    bool softTransition = true;

    for (auto &it: tree) {
        string paraName = it.first;
        if (paraName == "group") {
            string tmp = it.second.data();
            members = groups.at(tmp);
        } else if (paraName == "member") {
            string tmp = it.second.data();
            if (tmp == "__all__") {
                for (const auto &any:network_.getBaselines()) {
                    members.push_back(any.getName());
                }
            } else {
                members.push_back(tmp);
            }
        } else if (paraName == "parameter") {
            const string &tmp = it.second.data();

            ParameterSettings::ParametersBaselines newPARA = parameters.at(tmp);
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
            string t = it.second.get_value<string>();
            boost::posix_time::ptime thisStartTime = TimeSystem::string2ptime(t);
            start = TimeSystem::posixTime2InternalTime(thisStartTime);
        } else if (paraName == "end") {
            string t = it.second.get_value<string>();
            boost::posix_time::ptime thisEndTime = TimeSystem::string2ptime(t);
            end = TimeSystem::posixTime2InternalTime(thisEndTime);
        } else if (paraName == "transition") {
            string tmp = it.second.data();
            if (tmp == "hard") {
                softTransition = false;
            } else if (tmp == "soft") {
                softTransition = true;
            } else {
                cout << "ERROR: unknown transition type in <setup> block: " << tmp << ";\n";
            }
        }
    }


    vector<string> blNames;
    for (const auto &any: network_.getBaselines()) {
        blNames.push_back(any.getName());
    }

    for (const auto &any:members) {

        auto it = find(blNames.begin(), blNames.end(), any);
        long id = distance(blNames.begin(), it);
        auto &thisEvents = events[id];


        Baseline::Event newEvent_start(start,softTransition,combinedPARA);

        for (auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit) {
            if (iit->time > newEvent_start.time) {
                thisEvents.insert(iit, newEvent_start);
                break;
            }
        }

        Baseline::Event newEvent_end(end,true,parentPARA);
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
            baselineSetup(events, it.second, parameters, groups, combinedPARA);
        }
    }
}


void Initializer::initializeAstronomicalParameteres() noexcept{
    // earth velocity
    double date1 = 2400000.5;
    double date2 = TimeSystem::mjdStart+static_cast<double>(TimeSystem::duration)/2/86400;
    double pvh[2][3];
    double pvb[2][3];
    iauEpv00(date1, date2, pvh, pvb);
    double aud2ms = DAU / DAYSEC;
    double vearth[3] = {aud2ms * pvb[1][0],
                        aud2ms * pvb[1][1],
                        aud2ms * pvb[1][2]};
    AstronomicalParameters::earth_velocity = {vearth[0], vearth[1], vearth[2]};

    // earth nutation
    vector<unsigned int> nut_t;
    vector<double> nut_x;
    vector<double> nut_y;
    vector<double> nut_s;

    date2 = TimeSystem::mjdStart;

    unsigned int counter = 0;
    unsigned int frequency = 3600;
    unsigned int refTime;

    do {
        refTime = counter * frequency;
        date2 = TimeSystem::mjdStart + static_cast<double>(refTime) / 86400;

        double x, y, s;
        iauXys06a(date1, date2, &x, &y, &s);
        nut_t.push_back(refTime);
        nut_x.push_back(x);
        nut_y.push_back(y);
        nut_s.push_back(s);
        ++counter;
    } while (refTime < TimeSystem::duration + 3600);

    AstronomicalParameters::earth_nutX = nut_x;
    AstronomicalParameters::earth_nutY = nut_y;
    AstronomicalParameters::earth_nutS = nut_s;
    AstronomicalParameters::earth_nutTime = nut_t;

    //sunPosition
    double mjd = TimeSystem::mjdStart+static_cast<double>(TimeSystem::duration)/2/86400;
    // NUMBER OF DAYS SINCE J2000.0
    double days = mjd - 51544.5;
    // MEAN SOLAR LONGITUDE
    double slon = 280.460 + 0.9856474 * days;
    slon = fmod(slon, 360);
    if (slon < 1.0d-3){
        slon = slon + 360.0;
    }
    // MEAN ANOMALY OF THE SUN
    double sanom = 357.528 + 0.9856003 * days;
    sanom = sanom * pi / 180.0;
    sanom = fmod(sanom, 2*pi);
    if (sanom < 1.0d-3){
        sanom = sanom + 2 * pi;
    }
    // ECLIPTIC LONGITUDE AND OBLIQUITY OF THE ECLIPTIC
    double ecllon = slon + 1.915 * sin(sanom) + 0.020 * sin(2*sanom);
    ecllon = ecllon * pi / 180.0;
    ecllon = fmod(ecllon, 2*pi);
    double quad = ecllon / (0.5*pi);
    double iquad = floor(1 + quad);
    double obliq = 23.439 - 0.0000004 * days;
    obliq = obliq * pi / 180.0;
    // RIGHT ASCENSION AND DECLINATION
    // (RA IS IN SAME QUADRANT AS ECLIPTIC LONGITUDE)
    double sunra = atan(cos(obliq)*tan(ecllon));
    if (iquad == 2){
        sunra = sunra + pi;
    }else if(iquad == 3){
        sunra = sunra + pi;
    }else if(iquad == 4){
        sunra = sunra + 2*pi;
    }
    double sunde = asin(sin(obliq) * sin(ecllon));
    AstronomicalParameters::sun_radc = {sunra, sunde};
}

void Initializer::initializeWeightFactors() noexcept {
    WeightFactors::weightSkyCoverage = xml_.get<double>("master.weightFactor.skyCoverage", 0);
    WeightFactors::weightNumberOfObservations = xml_.get<double>("master.weightFactor.numberOfObservations", 0);
    WeightFactors::weightDuration = xml_.get<double>("master.weightFactor.duration", 0);

    WeightFactors::weightAverageSources = xml_.get<double>("master.weightFactor.averageSources", 0);
    WeightFactors::weightAverageStations = xml_.get<double>("master.weightFactor.averageStations", 0);
    WeightFactors::weightAverageBaselines = xml_.get<double>("master.weightFactor.averageBaselines", 0);

    WeightFactors::weightIdleTime = xml_.get<double>("master.weightFactor.idleTime", 0);
    WeightFactors::idleTimeInterval = xml_.get<unsigned int>("master.weightFactor.idleTimeInterval", 0);

    WeightFactors::weightDeclination = xml_.get<double>("master.weightFactor.weightDeclination", 0);
    WeightFactors::declinationStartWeight = xml_.get<double>("master.weightFactor.declinationStartWeight", 0) * deg2rad;
    WeightFactors::declinationFullWeight = xml_.get<double>("master.weightFactor.declinationFullWeight", 0) * deg2rad;

    WeightFactors::weightLowElevation = xml_.get<double>("master.weightFactor.weightLowElevation", 0);
    WeightFactors::lowElevationStartWeight = xml_.get<double>("master.weightFactor.lowElevationStartWeight", 0) * deg2rad;
    WeightFactors::lowElevationFullWeight = xml_.get<double>("master.weightFactor.lowElevationFullWeight", 0) * deg2rad;
}

void Initializer::initializeSkyCoverages() noexcept {

    SkyCoverage::maxInfluenceDistance = xml_.get<double>("master.skyCoverage.skyCoverageDistance", 30) * deg2rad;
    SkyCoverage::maxInfluenceTime = xml_.get<double>("master.skyCoverage.skyCoverageInterval", 3600);

    string interpolationDistance = xml_.get<string>("master.skyCoverage.interpolationDistance", "linear");
    if (interpolationDistance == "constant") {
        SkyCoverage::interpolationDistance = SkyCoverage::Interpolation::constant;
    } else if (interpolationDistance == "linear") {
        SkyCoverage::interpolationDistance = SkyCoverage::Interpolation::linear;
    } else if (interpolationDistance == "cosine") {
        SkyCoverage::interpolationDistance = SkyCoverage::Interpolation::cosine;
    }

    string interpolationTime = xml_.get<string>("master.skyCoverage.interpolationTime", "linear");
    if (interpolationTime == "constant") {
        SkyCoverage::interpolationTime = SkyCoverage::Interpolation::constant;
    } else if (interpolationTime == "linear") {
        SkyCoverage::interpolationTime = SkyCoverage::Interpolation::linear;
    } else if (interpolationTime == "cosine") {
        SkyCoverage::interpolationTime = SkyCoverage::Interpolation::cosine;
    }

}

void Initializer::initializeObservingMode(const SkdCatalogReader &reader, ofstream &headerLog) noexcept {
    auto PARA_mode = xml_.get_child("master.mode");
    for (const auto &it: PARA_mode) {
        if (it.first == "skdMode"){

            const string &name = it.second.get_value<string>();
            ObservationMode::sampleRate = reader.getSampleRate();
            ObservationMode::bits = reader.getBits();
            ObservationMode::manual = false;

            const auto &chan2band = reader.getChannelNumber2band();
            vector<string> bands;
            unordered_map<string,unsigned int> band2nchan;
            for(const auto &any:chan2band){
                if(find(bands.begin(), bands.end(), any.second) == bands.end()){
                    bands.push_back(any.second);
                    // TODO: count lower and upper sideband somehow... (remove this if-else)
                    if(any.second == "X"){
                        band2nchan.insert(make_pair(any.second,3));
                    }else{
                        band2nchan.insert(make_pair(any.second,1));
                    }

                }else{
                    ++band2nchan[any.second];
                }
            }
            ObservationMode::bands = bands;
            ObservationMode::nChannels = band2nchan;


            unordered_map<string,vector<double> > band2skyFreqs;
            for(const auto &any:bands){
                band2skyFreqs.insert(make_pair(any,vector<double>{}));
            }

            for(const auto&any:reader.getChannelNumber2skyFreq()){
                int chanNr = any.first;
                const string &band = chan2band.at(chanNr);
                auto freq = boost::lexical_cast<double>(any.second);
                band2skyFreqs.at(band).push_back(freq);
            }

            unordered_map<string,double> band2wavelength;
            for(const auto &any:band2skyFreqs){
                const string &band = any.first;
                const auto & tmp = any.second;
                double meanFreq = std::accumulate(tmp.begin(),tmp.end(),0.0)/tmp.size();
                double wl = speedOfLight/(meanFreq*1e6);
                band2wavelength[band] = wl;
            }
            ObservationMode::wavelength = band2wavelength;

            unordered_map<string, ObservationMode::Property> stationProperty;
            unordered_map<string, ObservationMode::Backup> stationBackup;
            unordered_map<string, double> stationBackupValue;

            unordered_map<string, ObservationMode::Property> sourceProperty;
            unordered_map<string, ObservationMode::Backup> sourceBackup;
            unordered_map<string, double> sourceBackupValue;

            for(const auto &any:bands){
                stationProperty[any] = ObservationMode::Property::required;
                sourceProperty[any] = ObservationMode::Property::required;
                stationBackup[any] = ObservationMode::Backup::none;
                sourceBackup[any] = ObservationMode::Backup::none;
                stationBackupValue[any] = 0;
                sourceBackupValue[any] = 0;
            }

            ObservationMode::stationProperty = stationProperty;
            ObservationMode::stationBackup = stationBackup;
            ObservationMode::stationBackupValue = stationBackupValue;

            ObservationMode::sourceProperty = sourceProperty;
            ObservationMode::sourceBackup = sourceBackup;
            ObservationMode::sourceBackupValue = sourceBackupValue;

        } else if (it.first == "sampleRate") {
            ObservationMode::sampleRate = it.second.get_value<double>();
        } else if(it.first == "bits"){
            ObservationMode::bits = it.second.get_value<unsigned int>();
        } else if(it.first == "bands"){
            ObservationMode::manual = true;
            for(const auto &itt: it.second){
                double wavelength;
                unsigned int channels;
                string name;

                for (const auto &it_band:itt.second){

                    if(it_band.first == "<xmlattr>"){
                        name = it_band.second.get_child("name").data();
                    }else if(it_band.first == "wavelength"){
                        wavelength = it_band.second.get_value<double>();
                    }else if(it_band.first == "chanels"){
                        channels = it_band.second.get_value<unsigned int>();
                    }
                }
                ObservationMode::bands.push_back(name);

                ObservationMode::nChannels[name] = channels;
                ObservationMode::wavelength[name] = wavelength;
            }
        }else if(it.first == "bandPolicies"){

            for (const auto &it_bandPolicies:it.second){
                ObservationMode::Property station_property;
                ObservationMode::Backup station_backup = ObservationMode::Backup::none;
                double station_backupValue;
                ObservationMode::Property source_property;
                ObservationMode::Backup source_backup = ObservationMode::Backup::none;
                double source_backupValue;
                double minSNR;
                string name;

                for(const auto &it_bandPolicy:it_bandPolicies.second){
                    if(it_bandPolicy.first == "<xmlattr>"){
                        name = it_bandPolicy.second.get_child("name").data();
                    }else if(it_bandPolicy.first == "minSNR"){
                        minSNR = it_bandPolicy.second.get_value<double>();
                    }else if(it_bandPolicy.first == "station"){
                        for (const auto &it_band_station:it_bandPolicy.second){
                            string thisName = it_band_station.first;
                            if (thisName == "tag"){
                                if(it_band_station.second.get_value<std::string>() == "required") {
                                    station_property = ObservationMode::Property::required;
                                }else if(it_band_station.second.get_value<std::string>() == "optional"){
                                    station_property = ObservationMode::Property::optional;
                                }
                            } else if (thisName == "backup_maxValueTimes"){
                                station_backup = ObservationMode::Backup::maxValueTimes;
                                station_backupValue = it_band_station.second.get_value<double>();

                            } else if (thisName == "backup_minValueTimes"){
                                station_backup = ObservationMode::Backup::minValueTimes;
                                station_backupValue = it_band_station.second.get_value<double>();

                            } else if (thisName == "backup_value"){
                                station_backup = ObservationMode::Backup::value;
                                station_backupValue = it_band_station.second.get_value<double>();
                            }
                        }
                    }else if(it_bandPolicy.first == "source") {
                        for (const auto &it_band_source:it_bandPolicy.second) {
                            string thisName = it_band_source.first;
                            if (thisName == "tag") {
                                if (it_band_source.second.get_value<std::string>() == "required") {
                                    source_property = ObservationMode::Property::required;
                                } else if (it_band_source.second.get_value<std::string>() == "optional") {
                                    source_property = ObservationMode::Property::optional;
                                }
                            } else if (thisName == "backup_maxValueTimes") {
                                source_backup = ObservationMode::Backup::maxValueTimes;
                                source_backupValue = it_band_source.second.get_value<double>();

                            } else if (thisName == "backup_minValueTimes") {
                                source_backup = ObservationMode::Backup::minValueTimes;
                                source_backupValue = it_band_source.second.get_value<double>();

                            } else if (thisName == "backup_value") {
                                source_backup = ObservationMode::Backup::value;
                                source_backupValue = it_band_source.second.get_value<double>();
                            }
                        }
                    }
                }
                ObservationMode::minSNR[name] = minSNR;

                ObservationMode::stationProperty[name] = station_property;
                ObservationMode::stationBackup[name] = station_backup;
                ObservationMode::stationBackupValue[name] = station_backupValue;

                ObservationMode::sourceProperty[name] = source_property;
                ObservationMode::sourceBackup[name] = source_backup;
                ObservationMode::sourceBackupValue[name] = source_backupValue;
            }
        }
    }

    headerLog << "Observing Mode:\n";
    headerLog << "  sample rate:    " << ObservationMode::sampleRate << "\n";
    headerLog << "  recording bits: " << ObservationMode::bits << "\n";
    headerLog << "  Bands: \n";
    for(const auto &any:ObservationMode::bands){
        unsigned int channels = ObservationMode::nChannels.at(any);
        double wavelength = ObservationMode::wavelength.at(any);
        headerLog << boost::format("    %2s: channels: %2d wavelength: %5.3f\n") %any %channels %wavelength;
    }
    headerLog << "\n";
}

unordered_map<string, vector<string> > Initializer::readGroups(boost::property_tree::ptree root, GroupType type) noexcept {
    unordered_map<std::string, std::vector<std::string> > groups;
    auto groupTree = root.get_child_optional("groups");
    if(groupTree.is_initialized()){
        for (auto &it: *groupTree) {
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
    }

    switch(type){
        case GroupType::source:{
            std::vector<std::string> members;
            for(const auto&any:sources_){
                members.push_back(any.getName());
            }
            groups["__all__"] = members;
            break;
        }
        case GroupType::station:{
            std::vector<std::string> members;
            for(const auto&any:network_.getStations()){
                members.push_back(any.getName());
            }
            groups["__all__"] = members;
            break;
        }
        case GroupType::baseline:{
            std::vector<std::string> members;
            for(const auto&any: network_.getBaselines()){
                members.push_back(any.getName());
            }
            groups["__all__"] = members;
            break;
        }
    }

    return groups;
}

void Initializer::applyMultiSchedParameters(const VieVS::MultiScheduling::Parameters &parameters) {
//    parameters.output(bodyLog);

//    Initializer copyOfInit(*this);
    multiSchedulingParameters_ = parameters;

    unsigned long nsta = network_.getNSta();

    // GENERAL
    if (parameters.start.is_initialized()) {
//        boost::posix_time::ptime startTime = *parameters.start;
//        int sec_ = startTime.time_of_day().total_seconds();
//        double mjdStart = startTime.date().modjulian_day() + sec_ / 86400;
//
//        boost::posix_time::ptime endTime = startTime + boost::posix_time::seconds(
//                static_cast<long>(TimeSystem::duration));
//
//        TimeSystem::mjdStart = mjdStart;
//        TimeSystem::startTime = startTime;
//        TimeSystem::endTime = endTime;
    }
    if (parameters.subnetting.is_initialized()) {
        parameters_.subnetting = *parameters.subnetting;
    }
    if (parameters.subnetting_minSourceAngle.is_initialized()) {
        parameters_.subnettingMinAngle = *parameters.subnetting_minSourceAngle;
    }
    if (parameters.subnetting_minParticipatingStations.is_initialized()) {
        parameters_.subnettingMinNSta = *parameters.subnetting_minParticipatingStations;
    }
    if (parameters.fillinmode_duringScanSelection.is_initialized()) {
        parameters_.fillinmodeDuringScanSelection = *parameters.fillinmode_duringScanSelection;
    }
    if (parameters.fillinmode_influenceOnScanSelection.is_initialized()) {
        parameters_.fillinmodeDuringScanSelection = *parameters.fillinmode_influenceOnScanSelection;
    }
    if (parameters.fillinmode_aPosteriori.is_initialized()) {
        parameters_.fillinmodeAPosteriori = *parameters.fillinmode_aPosteriori;
    }

    // WEIGHT FACTORS
    if (parameters.weightSkyCoverage.is_initialized()) {
        WeightFactors::weightSkyCoverage = *parameters.weightSkyCoverage;
    }
    if (parameters.weightNumberOfObservations.is_initialized()) {
        WeightFactors::weightNumberOfObservations = *parameters.weightNumberOfObservations;
    }
    if (parameters.weightDuration.is_initialized()) {
        WeightFactors::weightDuration = *parameters.weightDuration;
    }
    if (parameters.weightAverageSources.is_initialized()) {
        WeightFactors::weightAverageSources = *parameters.weightAverageSources;
    }
    if (parameters.weightAverageStations.is_initialized()) {
        WeightFactors::weightAverageStations = *parameters.weightAverageStations;
    }
    if (parameters.weightAverageBaselines.is_initialized()) {
        WeightFactors::weightAverageBaselines = *parameters.weightAverageBaselines;
    }
    if (parameters.weightIdleTime.is_initialized()) {
        WeightFactors::weightIdleTime = *parameters.weightIdleTime;
    }
    if (parameters.weightIdleTime_interval.is_initialized()) {
        WeightFactors::idleTimeInterval = static_cast<unsigned int>(*parameters.weightIdleTime_interval);
    }
    if (parameters.weightLowDeclination.is_initialized()) {
        WeightFactors::weightDeclination = *parameters.weightLowDeclination;
    }
    if (parameters.weightLowDeclination_begin.is_initialized()) {
        WeightFactors::declinationStartWeight = *parameters.weightLowDeclination_begin;
    }
    if (parameters.weightLowDeclination_full.is_initialized()) {
        WeightFactors::declinationFullWeight = *parameters.weightLowDeclination_full;
    }
    if (parameters.weightLowElevation.is_initialized()) {
        WeightFactors::weightLowElevation = *parameters.weightLowElevation;
    }
    if (parameters.weightLowElevation_begin.is_initialized()) {
        WeightFactors::lowElevationStartWeight = *parameters.weightLowElevation_begin;
    }
    if (parameters.weightLowElevation_full.is_initialized()) {
        WeightFactors::lowElevationFullWeight = *parameters.weightLowElevation_full;
    }

    // SKY COVERAGE
    if(parameters.skyCoverageInfluenceDistance.is_initialized()){
        SkyCoverage::maxInfluenceDistance = *parameters.skyCoverageInfluenceDistance;
    }
    if(parameters.skyCoverageInfluenceTime.is_initialized()){
        SkyCoverage::maxInfluenceTime = *parameters.skyCoverageInfluenceTime;
    }

    // STATION
    if (!parameters.stationWeight.empty()) {
        for (const auto &any:parameters.stationWeight) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,network_.getStations());
            for(auto id : ids){
                network_.refStation(id).referencePARA().weight = any.second;
            }
        }
    }
    if (!parameters.stationMaxSlewtime.empty()) {
        for (const auto &any:parameters.stationMaxSlewtime) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,network_.getStations());
            for(auto id : ids){
                network_.refStation(id).referencePARA().maxSlewtime = any.second;
            }
        }
    }
    if (!parameters.stationMaxSlewDistance.empty()) {
        for (const auto &any:parameters.stationMaxSlewDistance) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,network_.getStations());
            for(auto id : ids){
                network_.refStation(id).referencePARA().maxSlewDistance = any.second;
            }
        }
    }
    if (!parameters.stationMinSlewDistance.empty()) {
        for (const auto &any:parameters.stationMinSlewDistance) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,network_.getStations());
            for(auto id : ids){
                network_.refStation(id).referencePARA().minSlewDistance = any.second;
            }
        }
    }
    if (!parameters.stationMaxWait.empty()) {
        for (const auto &any:parameters.stationMaxWait) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,network_.getStations());
            for(auto id : ids){
                network_.refStation(id).referencePARA().maxWait = any.second;
            }
        }
    }
    if (!parameters.stationMinElevation.empty()) {
        for (const auto &any:parameters.stationMinElevation) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,network_.getStations());
            for(auto id : ids){
                network_.refStation(id).referencePARA().minElevation = any.second;
            }
        }
    }
    if (!parameters.stationMaxNumberOfScans.empty()) {
        for (const auto &any:parameters.stationMaxNumberOfScans) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,network_.getStations());
            for(auto id : ids){
                network_.refStation(id).referencePARA().maxNumberOfScans = any.second;
            }
        }
    }
    if (!parameters.stationMaxScan.empty()) {
        for (const auto &any:parameters.stationMaxScan) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,network_.getStations());
            for(auto id : ids){
                network_.refStation(id).referencePARA().maxScan = any.second;
            }
        }
    }
    if (!parameters.stationMinScan.empty()) {
        for (const auto &any:parameters.stationMinScan) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,network_.getStations());
            for(auto id : ids){
                network_.refStation(id).referencePARA().minScan = any.second;
            }
        }
    }

    // SOURCE
    if (!parameters.sourceWeight.empty()) {
        for (const auto &any:parameters.sourceWeight) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,sources_);
            for(auto id : ids){
                sources_[id].referencePARA().weight = any.second;
            }
        }
    }
    if (!parameters.sourceMinNumberOfStations.empty()) {
        for (const auto &any:parameters.sourceMinNumberOfStations) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,sources_);
            for(auto id : ids){
                sources_[id].referencePARA().minNumberOfStations = any.second;
            }
        }
    }
    if (!parameters.sourceMinFlux.empty()) {
        for (const auto &any:parameters.sourceMinFlux) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,sources_);
            for(auto id : ids){
                sources_[id].referencePARA().minFlux = any.second;
            }
        }
    }
    if (!parameters.sourceMaxNumberOfScans.empty()) {
        for (const auto &any:parameters.sourceMaxNumberOfScans) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,sources_);
            for(auto id : ids){
                sources_[id].referencePARA().maxNumberOfScans = any.second;
            }
        }
    }
    if (!parameters.sourceMinElevation.empty()) {
        for (const auto &any:parameters.sourceMinElevation) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,sources_);
            for(auto id : ids){
                sources_[id].referencePARA().minElevation = any.second;
            }
        }
    }
    if (!parameters.sourceMinSunDistance.empty()) {
        for (const auto &any:parameters.sourceMinSunDistance) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,sources_);
            for(auto id : ids){
                sources_[id].referencePARA().minSunDistance = any.second;
            }
        }
    }
    if (!parameters.sourceMaxScan.empty()) {
        for (const auto &any:parameters.sourceMaxScan) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,sources_);
            for(auto id : ids){
                sources_[id].referencePARA().maxScan = any.second;
            }
        }
    }
    if (!parameters.sourceMinScan.empty()) {
        for (const auto &any:parameters.sourceMinScan) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,sources_);
            for(auto id : ids){
                sources_[id].referencePARA().minScan = any.second;
            }
        }
    }
    if (!parameters.sourceMinRepeat.empty()) {
        for (const auto &any:parameters.sourceMinRepeat) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,sources_);
            for(auto id : ids){
                sources_[id].referencePARA().minRepeat = any.second;
            }
        }
    }

    // BASELINES
    if (!parameters.baselineWeight.empty()) {
        for (const auto &any:parameters.baselineWeight) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,network_.getBaselines());
            for(auto id : ids){
                network_.refBaseline(id).refParameters().weight = any.second;
            }
        }
    }
    if (!parameters.baselineMinScan.empty()) {
        for (const auto &any:parameters.baselineMinScan) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,network_.getBaselines());
            for(auto id : ids){
                network_.refBaseline(id).refParameters().minScan = any.second;
            }
        }
    }
    if (!parameters.baselineMaxScan.empty()) {
        for (const auto &any:parameters.baselineMaxScan) {
            string name = any.first;
            vector<unsigned long> ids = getMembers(name,network_.getBaselines());
            for(auto id : ids){
                network_.refBaseline(id).refParameters().maxScan = any.second;
            }
        }
    }

}

vector<MultiScheduling::Parameters> Initializer::readMultiSched(std::ostream &out) {
    vector<MultiScheduling::Parameters> para;

    MultiScheduling ms;
    boost::optional<boost::property_tree::ptree &> mstree_o = xml_.get_child_optional("master.multisched");
    if(mstree_o.is_initialized()) {
        boost::property_tree::ptree mstree = *mstree_o;

        boost::property_tree::ptree PARA_station = xml_.get_child("master.station");
        unordered_map<std::string, std::vector<std::string> > group_station = readGroups(PARA_station,
                                                                                         GroupType::station);
        boost::property_tree::ptree PARA_source = xml_.get_child("master.source");
        unordered_map<std::string, std::vector<std::string> > group_source = readGroups(PARA_source, GroupType::source);

        boost::property_tree::ptree PARA_baseline = xml_.get_child("master.baseline");
        unordered_map<std::string, std::vector<std::string> > group_baseline = readGroups(PARA_baseline,
                                                                                          GroupType::baseline);

        unsigned int maxNumber = mstree.get("maxNumber",numeric_limits<unsigned int>::max());
        boost::optional<unsigned int> o_seed = mstree.get_optional<unsigned int>("seed");
        unsigned int seed;
        if(o_seed.is_initialized()){
            seed = *o_seed;
        }else{
            std::default_random_engine generator;
            generator.seed(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
            std::uniform_int_distribution<unsigned int> distribution(0,2147483647);
            seed = distribution(generator);
        }


        for (const auto &any:mstree) {
            std::string name = any.first;
            if(name == "maxNumber" || name == "seed"){
                continue;
            }
            if(name == "general_subnetting" || name == "general_fillinmode_during_scan_selection" ||
               name == "general_fillinmode_influence_on_scan_selection" || name == "general_fillinmode_a_posteriori"){
                ms.addParameters(name);
                continue;
            }

            vector<double> values;
            const auto &valueTree = any.second;
            std::string member;
            boost::optional<string> om = valueTree.get_optional<std::string>("<xmlattr>.member");
            if(om.is_initialized()){
                member = *om;
            }
            for(const auto &any2: valueTree){
                std::string name2 = any2.first;
                if(name2 == "value"){
                    values.push_back(any2.second.get_value<double>());
                }
            }
            if(member.empty()){
                ms.addParameters(name,values);
            }else{
                ms.addParameters(name,member,values);
            }
        }

        std::vector<MultiScheduling::Parameters> ans = ms.createMultiScheduleParameters(maxNumber,seed);

        if(ans.size() != maxNumber){
            out << "multi scheduling found ... creating " << ans.size() << " schedules!;\n";
        }else{
            out << "multi scheduling found ... creating " << ans.size() << " schedules using this seed: "<< seed << "!;\n";
        }
        return ans;
    }
    return std::vector<MultiScheduling::Parameters>{};
}

void Initializer::initializeSourceSequence() noexcept{
    boost::optional< boost::property_tree::ptree& > sq = xml_.get_child_optional( "master.rules.sourceSequence" );
    if( sq.is_initialized() )
    {
        boost::property_tree::ptree PARA_source = xml_.get_child("master.source");
        unordered_map<std::string, std::vector<std::string> > groups = readGroups(PARA_source, GroupType::source);

        Scan::scanSequence.customScanSequence = true;
        for(const auto &any:*sq){
            if(any.first == "cadence"){
                Scan::scanSequence.cadence = any.second.get_value<unsigned int>();
            }
            if(any.first == "sequence"){
                auto tmp = any.second;
                unsigned int modulo;
                vector<unsigned long> targetIds;

                for(const auto &any2:tmp){
                    if(any2.first == "modulo"){
                        modulo = any2.second.get_value<unsigned int>();
                    }
                    if(any2.first == "member"){
                        string member = any2.second.get_value<string>();
                        vector<string> targetSources;

                        if(groups.find(member) != groups.end()){
                            targetSources = groups[member];
                        } else if(member == "__all__"){
                            for(const auto &source:sources_){
                                const string &name = source.getName();
                                targetSources.push_back(name);
                            }
                        } else{
                            targetSources.push_back(member);
                        }




                        for(const auto &source:sources_){
                            const string &name = source.getName();
                            if(find(targetSources.begin(),targetSources.end(),name) != targetSources.end()){
                                targetIds.push_back(source.getId());
                            }
                        }

                    }
                }
                Scan::scanSequence.targetSources[modulo] = targetIds;
            }
        }
    }
}

void Initializer::initializeCalibrationBlocks(std::ofstream &headerLog) {
    boost::optional<boost::property_tree::ptree &> cb = xml_.get_child_optional("master.rules.calibratorBlock");
    if (cb.is_initialized()) {
        boost::property_tree::ptree PARA_source = xml_.get_child("master.source");
        unordered_map<std::string, std::vector<std::string> > groups = readGroups(PARA_source, GroupType::source);

        CalibratorBlock::scheduleCalibrationBlocks = true;

        headerLog << "Calibration Block found!\n";

        for(const auto &any:*cb) {
            if (any.first == "cadence_nScanSelections") {
                CalibratorBlock::cadenceUnit = CalibratorBlock::CadenceUnit::scans;
                CalibratorBlock::cadence = any.second.get_value<unsigned int>();
                CalibratorBlock::nextBlock = CalibratorBlock::cadence;
                headerLog << "  calibration block every "<< CalibratorBlock::cadence <<" scan selections\n";
            } else if (any.first == "cadence_seconds") {
                CalibratorBlock::cadenceUnit = CalibratorBlock::CadenceUnit::seconds;
                CalibratorBlock::cadence = any.second.get_value<unsigned int>();
                CalibratorBlock::nextBlock = CalibratorBlock::cadence;
                headerLog << "  calibration block every "<< CalibratorBlock::cadence <<" seconds\n";
            } else if (any.first == "member") {
                string member = any.second.get_value<string>();
                vector<string> targetSources;

                if (groups.find(member) != groups.end()) {
                    targetSources = groups[member];
                } else if(member == "__all__"){
                    for(const auto &source:sources_){
                        const string &name = source.getName();
                        targetSources.push_back(name);
                    }
                }  else {
                    targetSources.push_back(member);
                }

                headerLog << "  allowed calibratior sources: \n    ";
                vector<unsigned long> targetIds;
                int c = 0;
                for (const auto &source:sources_) {
                    const string &name = source.getName();
                    if (find(targetSources.begin(), targetSources.end(), name) != targetSources.end()) {
                        if(c==9){
                            headerLog << "\n    ";
                            c = 0;
                        }
                        headerLog << boost::format("%-8s ") % name;
                        targetIds.push_back(source.getId());
                        ++c;
                    }
                }
                headerLog << "\n";
                CalibratorBlock::calibratorSourceIds = std::move(targetIds);
            } else if (any.first == "nMaxScans") {
                CalibratorBlock::nmaxScans = any.second.get_value<unsigned int>();
                headerLog << "  maximum number of calibration block scans: "<< CalibratorBlock::nmaxScans <<"\n";

            } else if (any.first == "fixedScanTime") {
                CalibratorBlock::targetScanLengthType = CalibratorBlock::TargetScanLengthType::seconds;
                CalibratorBlock::scanLength = any.second.get_value<unsigned int>();

                headerLog << "  fixed scan length for calibrator scans: "<< CalibratorBlock::scanLength <<" seconds\n";

            } else if (any.first == "lowElevation"){
                CalibratorBlock::lowElevationStartWeight = any.second.get<double>("startWeight")*deg2rad;
                CalibratorBlock::lowElevationFullWeight = any.second.get<double>("fullWeight")*deg2rad;
            } else if (any.first == "highElevation"){
                CalibratorBlock::highElevationStartWeight = any.second.get<double>("startWeight")*deg2rad;
                CalibratorBlock::highElevationFullWeight = any.second.get<double>("fullWeight")*deg2rad;
            }

        }
    }
}

unsigned int Initializer::minutesVisible(const Source &source, const Source::Parameters &parameters, unsigned int start,
                                         unsigned int end) {
    unsigned int minutes = 0;
    unsigned int minVisible = parameters.minNumberOfStations;

    vector<unsigned long> reqSta = parameters.requiredStations;
    vector<unsigned long> ignSta = parameters.ignoreStations;

    for(unsigned int t = start; t<end; t+=60){
        unsigned int visible = 0;

        bool requiredStationNotVisible = false;
        for(unsigned long staid = 0; staid<network_.getNSta(); ++staid){

            const Station &thisSta = network_.getStation(staid);
            if(find(ignSta.begin(),ignSta.end(),staid) != ignSta.end()){
                continue;
            }

            PointingVector p(staid,source.getId());
            p.setTime(t);

            thisSta.calcAzEl(source, p, Station::AzelModel::simple);

            // check if source is up from station
            bool flag = thisSta.isVisible(p, source.getPARA().minElevation);
            if(flag){
                ++visible;
            }else{
                if(find(reqSta.begin(),reqSta.end(),staid) != reqSta.end()){
                    requiredStationNotVisible = true;
                    break;
                }
            }
        }
        if(requiredStationNotVisible){
            continue;
        }
        if(visible>=minVisible){
            ++minutes;
        }

    }
    return minutes;
}

#ifdef _OPENMP
void Initializer::initializeMultiCore(int& nThreads, std::string & jobScheduling, int& chunkSize, std::string & threadPlace) {

    std::string threads = xml_.get<std::string>("master.multiCore.threads","auto");

    if(threads == "manual"){
        nThreads = xml_.get<int>("master.multiCore.nThreads",1);
    } else if (threads == "single"){
        nThreads = 1;
    } else if (threads == "auto"){
        nThreads = 4;
    }
    //TODO: proper implementation of multi scheduling
//    nThreads = 4;

    jobScheduling = xml_.get<std::string>("master.multiCore.jobScheduling","auto");
    chunkSize = xml_.get<int>("master.multiCore.chunkSize",1);
    threadPlace = xml_.get<std::string>("master.multiCore.threadPlace","auto");
}
#endif

void Initializer::statisticsLogHeader(ofstream &ofstream) {

    ofstream << "version,n_scans,n_single_scans,n_subnetting_scans,n_fillinmode_scans,n_calibrator_scans,n_baselines,";
    ofstream << "n_stations,";
    for(const auto&any : network_.getStations()){
        ofstream << "n_scans_" << any.getName() << ",";
    }
    for(const auto&any : network_.getStations()){
        ofstream << "n_baselines_" << any.getName() << ",";
    }
    ofstream << "n_sources,\n";
}

void Initializer::initializeOptimization(std::ofstream &ofstream) {
    boost::optional<boost::property_tree::ptree &> ctree = xml_.get_child_optional("master.optimization");
    if (ctree.is_initialized()) {

        boost::property_tree::ptree PARA_source = xml_.get_child("master.source");
        unordered_map<std::string, std::vector<std::string> > groups = readGroups(PARA_source, GroupType::source);

        for(const auto &any: *ctree){
            if(any.first == "combination"){
                bool tmp = any.second.get_value<string>() == "and";
                parameters_.andAsConditionCombination = tmp;
            }else if(any.first == "maxNumberOfIterations"){
                parameters_.maxNumberOfIterations = any.second.get_value<unsigned int>();
            }else if(any.first == "numberOfGentleSourceReductions"){
                parameters_.numberOfGentleSourceReductions = any.second.get_value<unsigned int>();
            }else if(any.first == "minNumberOfSourcesToReduce"){
                parameters_.minNumberOfSourcesToReduce = any.second.get_value<unsigned int>();
            }else if(any.first == "condition"){
                string member = any.second.get<string>("members");
                auto scans = any.second.get<unsigned int>("minScans");
                auto bls = any.second.get<unsigned int>("minBaselines");

                if (groups.find(member) != groups.end()) {
                    const vector<string> &group = groups.at(member);
                    for(auto &source:sources_){
                        if(find(group.begin(),group.end(),source.getName()) != group.end()){
                            source.referenceCondition().minNumScans = scans;
                            source.referenceCondition().minNumBaselines = bls;
                        }
                    }
                } else {
                    for(auto &source:sources_){
                        if(source.hasName(member)){
                            source.referenceCondition().minNumScans = scans;
                            source.referenceCondition().minNumBaselines = bls;
                        }
                    }
                }
            }
        }
    }
}

void Initializer::initializeHighImpactScanDescriptor(std::ofstream &of) {
    boost::optional<boost::property_tree::ptree &> ctree = xml_.get_child_optional("master.highImpact");
    if (ctree.is_initialized()) {

        of << "High impact block found!\n";
        unsigned int interval = ctree->get("interval",60);
        unsigned int repeat   = ctree->get("repeat",300);
        of << "    high impact check interval: " << interval << "\n";
        of << "    high impact repeat        : " << repeat << "\n";

        boost::property_tree::ptree PARA_station = xml_.get_child("master.station");
        unordered_map<std::string, std::vector<std::string> > groups = readGroups(PARA_station, GroupType::station);

        HighImpactScanDescriptor himp = HighImpactScanDescriptor(interval, repeat);
        for(const auto &any: *ctree){
            if(any.first == "targetAzEl"){
                auto member = any.second.get<string>("member");
                vector<unsigned long> staids;

                if (groups.find(member) != groups.end()) {
                    const vector<string> &group = groups.at(member);
                    for(auto &station : network_.getStations()){
                        if(find(group.begin(),group.end(),station.getName()) != group.end()){
                            staids.push_back(station.getId());
                        }
                    }
                } else {
                    for(auto &station : network_.getStations()){
                        if(station.hasName(member)){
                            staids.push_back(station.getId());
                        }
                    }
                }

                auto az = any.second.get<double>("az");
                auto el = any.second.get<double>("el");
                auto margin = any.second.get<double>("margin");

                of << "    target az: " << az << " el: " << el << " margin: " << margin << " station: ";
                for(unsigned long i:staids){
                    of << network_.getStation(i).getName() << " ";
                }
                of << "\n";

                himp.addAzElDescriptor(az*deg2rad,el*deg2rad,margin*deg2rad,staids);
            }
        }
        himp_ = himp;
    }
}

std::vector<unsigned long> Initializer::getMembers(const std::string &name, const std::vector<Station> &stations) {
    vector<unsigned long> ids;

    // add all ids if name == "__all__"
    if(name == "__all__"){
        ids.resize(stations.size());
        iota(ids.begin(), ids.end(), 0);

    // add all ids from group if name is equal to a group name
    }else if(staGroups_.find(name) != staGroups_.end()){
        const auto &members = staGroups_.at(name);
        for(const auto &thisTarget : members){
            for(const auto &thisObject : stations){
                if(thisObject.hasName(name)){
                    ids.push_back(thisObject.getId());
                    break;
                }
            }
        }

    // add single id instead
    }else{
        for(const auto &any : stations){
            if(any.hasName(name)){
                ids.push_back(any.getId());
            }
        }
    }

    return ids;
}

std::vector<unsigned long> Initializer::getMembers(const std::string &name, const std::vector<Baseline> &baselines) {
    vector<unsigned long> ids;

    // add all ids if name == "__all__"
    if(name == "__all__"){
        ids.resize(baselines.size());
        iota(ids.begin(), ids.end(), 0);

        // add all ids from group if name is equal to a group name
    }else if(blGroups_.find(name) != blGroups_.end()){
        const auto &members = blGroups_.at(name);
        for(const auto &thisTarget : members){
            for(const auto &thisObject : baselines){
                if(thisObject.hasName(name)){
                    ids.push_back(thisObject.getId());
                    break;
                }
            }
        }

        // add single id instead
    }else{
        for(const auto &any : baselines){
            if(any.hasName(name)){
                ids.push_back(any.getId());
            }
        }
    }

    return ids;
}

std::vector<unsigned long> Initializer::getMembers(const std::string &name, const std::vector<Source> &sources) {
    vector<unsigned long> ids;

    // add all ids if name == "__all__"
    if(name == "__all__"){
        ids.resize(sources.size());
        iota(ids.begin(), ids.end(), 0);

        // add all ids from group if name is equal to a group name
    }else if(srcGroups_.find(name) != srcGroups_.end()){
        const auto &members = srcGroups_.at(name);
        for(const auto &thisTarget : members){
            for(const auto &thisObject : sources){
                if(thisObject.hasName(name)){
                    ids.push_back(thisObject.getId());
                    break;
                }
            }
        }

        // add single id instead
    }else{
        for(const auto &any : sources){
            if(any.hasName(name)){
                ids.push_back(any.getId());
            }
        }
    }

    return ids;
}
