/*
 * File:   Initializer.cpp
 * Author: mschartn
 * 
 * Created on June 28, 2017, 12:38 PM
 */

#include "Initializer.h"

using namespace std;
using namespace VieVS;

Initializer::Initializer(const std::string &path) {
    ifstream is(path);
    boost::property_tree::read_xml(is, xml_);
}

Initializer::~Initializer() = default;

void Initializer::precalcSubnettingSrcIds() noexcept {
    unsigned long nsrc = sources_.size();
    vector<vector<int> > subnettingSrcIds(nsrc);
    for (int i = 0; i < nsrc; ++i) {
        for (int j = i + 1; j < nsrc; ++j) {
            double dist = sources_[i].angleDistance(sources_[j]);
            if (dist > parameters_.minAngleBetweenSubnettingSources) {
                subnettingSrcIds[i].push_back(j);
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

    vector<string> sel_stations;
    boost::property_tree::ptree ptree_stations = xml_.get_child("master.general.stations");
    auto it = ptree_stations.begin();
    while (it != ptree_stations.end()) {
        auto item = it->second.data();
        sel_stations.push_back(item);
        ++it;
    }
    vector<string> selectedStations = sel_stations;


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
        string id_PO = boost::algorithm::to_upper_copy(any.second.at(13));
        string id_EQ = boost::algorithm::to_upper_copy(any.second.at(14));
        string id_MS = boost::algorithm::to_upper_copy(any.second.at(15));

        // check if corresponding position and equip CATALOG exists.
        if (positionCatalog.find(id_PO) == positionCatalog.end()){
            headerLog << "*** ERROR: position CATALOG not found ***\n";
            continue;
        }
        if (equipCatalog.find(id_EQ+"|"+name) == equipCatalog.end()){
            headerLog << "*** ERROR: equip CATALOG not found ***\n";
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
            headerLog << "*** ERROR: " << e.what() << " ***\n";
            continue;
        }

        // check if position.cat is long enough. Otherwise not all information is available.
        vector<string> po_cat = positionCatalog.at(id_PO);
        if (po_cat.size()<5){
            headerLog << "*** ERROR: " << any.first << ": positon.cat to small ***\n";
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
            headerLog << "*** ERROR: " << e.what() << " ***\n";
            continue;
        }

        // check if equip.cat is long enough. Otherwise not all information is available.
        vector<string> eq_cat = equipCatalog.at(id_EQ + "|" + name);
        if (eq_cat.size()<9){
            headerLog << "*** ERROR: " << any.first << ": equip.cat to small ***\n";
            continue;
        }
        // check if SEFD_ information is in X and S band
        if (eq_cat[5] != "X" || eq_cat[7] != "S") {
            headerLog << "*** ERROR: " << any.first << ": we only support SX equipment ***\n";
            continue;
        }

        unordered_map<std::string,double> SEFDs;
        // convert all items from equip.cat
        unordered_map<std::string,double> SEFD_found;
        try{
            SEFD_found[eq_cat.at(5)] = boost::lexical_cast<double>(eq_cat.at(6));
            SEFD_found[eq_cat.at(7)] = boost::lexical_cast<double>(eq_cat.at(8));
        }
        catch(const std::exception& e){
            headerLog << "*** ERROR: " << e.what() << "\n";
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
            cerr << "*** ERROR: station " << name << " required SEFD_ information missing!\n";
            continue;
        }

        if(SEFDs.size() != ObservationMode::bands.size()){
            if(SEFDs.empty()){
                cerr << "*** ERROR: station " << name << " no SEFD_ information found to calculate backup value!\n";
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
                    headerLog << "*** ERROR: " << e.what() << " ***\n";
                    headerLog << mask_cat.at(i) << "\n";
                }
            }
        } else {
            if (!id_MS.compare("--")==0){
                headerLog << "*** ERROR: mask CATALOG not found ***\n";
            }
        }
        stations_.emplace_back(name,
                               created,
                               Antenna(offset,diam,rate1,con1,rate2,con2),
                               CableWrap(axis1_low,axis1_up,axis2_low,axis2_up),
                               Position(x,y,z),
                               Equipment(SEFDs),
                               HorizonMask(hmask),
                               type);
        created++;
        headerLog << boost::format("  %-8s added\n") % name;

    }
    headerLog << "Finished! " << created << " of " << nant << " stations created\n\n";
}

void Initializer::createSources(const SkdCatalogReader &reader, std::ofstream &headerLog) noexcept {
    const map<string, vector<string> > &sourceCatalog = reader.getSourceCatalog();
    const map<string, vector<string> > &fluxCatalog = reader.getFluxCatalog();

    int counter = 0;
    unsigned long nsrc = sourceCatalog.size();
    int created = 0;
    headerLog << "Create Sources:\n";

    for (auto any: sourceCatalog){
        counter ++;
        string name = any.first;

        if (any.second.size() < 8){
            headerLog << "*** ERROR: " << any.first << ": source.cat to small ***\n";
            continue;
        }
//        if (any.second.at(1) != "$"){
//            name = any.second.at(1);
//        }

        if (fluxCatalog.find(name) == fluxCatalog.end()){
            headerLog << "*** WARNING: source " << name << ": flux information not found ***\n";
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
            headerLog << "*** ERROR: " << e.what() << " ***\n";
            continue;
        }
        double ra = 15*(ra_h + ra_m/60 + ra_s/3600);
        double de =    (abs(de_deg) + de_m/60 + de_s/3600);
        if (sign == '-'){
            de = -1*de;
        }

        vector<string> flux_cat = fluxCatalog.at(name);
//            if (flux_cat.size() < 6){
//                headerLog <<"*** ERROR: "<< name << ": flux.cat to small ***\n";
//                continue;
//            }

        unordered_map<string,Flux> flux;

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
                    }else {
                        cerr << "*** ERROR: Source:" << name << "Flux: " << thisBand << "\n";
                        cerr << "    You can not mix B and M flux information for one band!";
                    }
                }
            }

            Flux srcFlux(thisType);
            bool flagFlux = srcFlux.addFluxParameters(parameters);

            srcFlux.setWavelength(ObservationMode::wavelength[thisBand]);


            if (flagFlux){
                flux[thisBand] = srcFlux;
            }else{
                cerr << "error reading flux info of: "<< name << "\n";
            }
            ++cflux;
        }

        bool fluxBandInfoOk = true;
        for(const auto &bandName: ObservationMode::bands){
            if(flux.find(bandName) == flux.end()){
                if(ObservationMode::sourceProperty[bandName] == ObservationMode::Property::required){
                    fluxBandInfoOk = false;
                    break;
                }
                if(ObservationMode::sourceBackup[bandName] == ObservationMode::Backup::value){
                    Flux tmp("B");
                    tmp.setWavelength(ObservationMode::wavelength[bandName]);
                    tmp.addFluxParameters(vector<string>{"0",boost::lexical_cast<std::string>(ObservationMode::stationBackupValue[bandName]),"13000.0"});
                    flux[bandName] = tmp;
                }
            }
        }
        if(!fluxBandInfoOk){
            cerr << "WARNING: source " << name << " required SEFD_ information missing!\n";
        }

        if(flux.size() != ObservationMode::bands.size()){
            if(flux.empty()){
                cerr << "*** ERROR: station " << name << " no SEFD_ information found to calculate backup value!\n";
                continue;
            }
            double max = 0;
            double min = std::numeric_limits<double>::max();
            for(const auto &any2:flux){
                //TODO use something like .getMinimumFlux instead of .getMaximumFlux
                if(any2.second.getMaximumFlux()<min){
                    min = any2.second.getMaximumFlux();
                }
                if(any2.second.getMaximumFlux()>max){
                    max = any2.second.getMaximumFlux();
                }
            }
            for(const auto &bandName:ObservationMode::bands){
                if(flux.find(bandName) == flux.end()){
                    if(ObservationMode::stationBackup[bandName] == ObservationMode::Backup::minValueTimes){
                        Flux tmp("B");
                        tmp.setWavelength(ObservationMode::wavelength[bandName]);
                        tmp.addFluxParameters(vector<string>{"0",boost::lexical_cast<std::string>(min * ObservationMode::stationBackupValue[bandName]),"13000.0"});
                        flux[bandName] = tmp;
                    }
                    if(ObservationMode::stationBackup[bandName] == ObservationMode::Backup::maxValueTimes){
                        Flux tmp("B");
                        tmp.setWavelength(ObservationMode::wavelength[bandName]);
                        tmp.addFluxParameters(vector<string>{"0",boost::lexical_cast<std::string>(max * ObservationMode::stationBackupValue[bandName]),"13000.0"});

                        flux[bandName] = tmp;
                    }
                }
            }
        }



        if (!flux.empty()){
            sources_.emplace_back(name, ra, de, flux, created);
            created++;
            headerLog << boost::format("  %-8s added\n") % name;
        }
    }
    headerLog << "Finished! " << created << " of " << nsrc << " sources created\n\n";
}

void Initializer::createSkyCoverages(ofstream &headerLog) noexcept {
    unsigned long nsta = stations_.size();
    std::vector<char> alreadyConsidered(nsta, false);
    int skyCoverageId = 0;
    vector<vector<int> > stationsPerId(nsta);

    headerLog << "Create Sky Coverage Objects:\n";
    for(int i=0; i<nsta; ++i){
        if(!alreadyConsidered[i]){
            stations_[i].setSkyCoverageId(skyCoverageId);
            headerLog << boost::format("  station: %-8s belongs to sky coverage object %2d\n") %stations_[i].getName() %skyCoverageId;
            stationsPerId[skyCoverageId].push_back(i);
            alreadyConsidered[i] = true;
            for(int j=i+1; j<nsta; ++j){
                double dist = stations_[i].distance(stations_[j]);

                if(!alreadyConsidered[j] && dist<SkyCoverage::maxTwinTelecopeDistance){
                    stations_[j].setSkyCoverageId(skyCoverageId);
                    headerLog << boost::format("  station: %8s belongs to sky coverage object %2d\n") %stations_[j].getName() %skyCoverageId;
                    stationsPerId[skyCoverageId].push_back(i);
                    alreadyConsidered[j] = true;
                }
            }
            skyCoverageId++;
        }
    }

    for (int i=0; i<skyCoverageId; ++i){
        skyCoverages_.emplace_back(stationsPerId[i],i);
    }

    vector<int> sta2sky_(nsta);

    for (int i = 0; i < skyCoverages_.size(); ++i) {
        vector<int> sky2sta = skyCoverages_[i].getStaids();
        for (int j : sky2sta) {
            sta2sky_[j] = i;
        }
    }
    headerLog << "Finished! "<< skyCoverages_.size() << " sky coverage objects were created\n\n";

}

void Initializer::initializeGeneral(ofstream &headerLog) noexcept {
    try {
        boost::posix_time::ptime startTime = xml_.get<boost::posix_time::ptime>("master.general.startTime");
        headerLog << "start time:" << startTime << "\n";
        int sec_ = startTime.time_of_day().total_seconds();
        double mjdStart = startTime.date().modjulian_day() + sec_ / 86400.0;


        boost::posix_time::ptime endTime = xml_.get<boost::posix_time::ptime>("master.general.endTime");
        headerLog << "end time:" << endTime << "\n";


        boost::posix_time::time_duration a = endTime - startTime;
        int sec = a.total_seconds();
        if (sec < 0) {
            cerr << "ERROR: duration is less than zero seconds!\n";
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
        parameters_.fillinmode = xml_.get<bool>("master.general.fillinmode");

        HorizonMask::minElevation = xml_.get<double>("master.general.minElevation") * deg2rad;

    } catch (const boost::property_tree::ptree_error &e) {
        headerLog << "ERROR: reading parameters.xml file!" << endl;
    }

    headerLog << "\n";
}


void Initializer::initializeStations() noexcept {
    boost::property_tree::ptree PARA_station;
    try{
        PARA_station = xml_.get_child("master.station");
    }catch(const boost::property_tree::ptree_error &e){
        cout << "ERROR: reading parameters.xml file!" <<
             "    probably missing <station> block?" << endl;
    }

    unordered_map<std::string, std::vector<std::string> > groups = readGroups(PARA_station, GroupType::station);

    unordered_map<std::string, Station::PARAMETERS> parameters;
    for (auto &it: PARA_station) {
        string name = it.first;
        if (name == "parameters") {
            string parameterName = it.second.get_child("<xmlattr>.name").data();

            Station::PARAMETERS PARA;

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
                } else if (paraName == "minSNR") {
                    string bandName = it2.second.get_child("<xmlattr>.band").data();
                    double value = it2.second.get_value<double>();
                    PARA.minSNR[bandName] = value;
                } else if (paraName == "ignoreSources") {
                    for (auto &it3: it2.second) {
                        string srcName = it3.second.data();
                        for (int i = 0; i < sources_.size(); ++i) {
                            if (srcName == sources_[i].getName()) {
                                PARA.ignoreSources.push_back(i);
                                break;
                            }
                        }
                    }
                } else {
                    cerr << "<station> <parameter>: " << parameterName << ": parameter <" << name
                         << "> not understood! (Ignored)\n";
                }
            }
            parameters[parameterName] = PARA;
        }
    }

    vector<vector<Station::EVENT> > events(stations_.size());

    Station::PARAMETERS parentPARA;
    parentPARA.firstScan = false;
    parentPARA.available = true;
    parentPARA.maxSlewtime = 9999;
    parentPARA.maxWait = 9999;
    parentPARA.maxScan = 600;
    parentPARA.minScan = 30;
    for (const auto &any:ObservationMode::bands) {
        const string &name = any;
        parentPARA.minSNR[name] = 0;
    }

    parentPARA.weight = 1;


    for (int i = 0; i < stations_.size(); ++i) {
        Station::EVENT newEvent_start;
        newEvent_start.time = 0;
        newEvent_start.softTransition = false;
        newEvent_start.PARA = parentPARA;

        events[i].push_back(newEvent_start);

        Station::EVENT newEvent_end;
        newEvent_end.time = TimeSystem::duration;
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

    unsigned long nsta = stations_.size();
    for (int i = 0; i < nsta; ++i) {
        Station &thisStation = stations_[i];

        PointingVector pV(i, 0);
        pV.setAz(thisStation.getCableWrap().neutralPoint(1));
        pV.setEl(thisStation.getCableWrap().neutralPoint(2));
        pV.setTime(0);

        thisStation.setCurrentPointingVector(pV);
        thisStation.setEVENTS(events[i]);
        bool hardBreak = false;
        ofstream dummy;
        thisStation.checkForNewEvent(0, hardBreak, false, dummy);

        vector<double> distance(nsta);
        vector<double> dx(nsta);
        vector<double> dy(nsta);
        vector<double> dz(nsta);
        for (int j = i+1; j<nsta; ++j) {
            Station &otherStation = stations_[j];

            distance[j] = thisStation.distance(otherStation);
            dx[j] = otherStation.getPosition().getX() - thisStation.getPosition().getX();
            dy[j] = otherStation.getPosition().getY() - thisStation.getPosition().getY();
            dz[j] = otherStation.getPosition().getZ() - thisStation.getPosition().getZ();
        }

        thisStation.preCalc(distance, dx, dy, dz);
        thisStation.referencePARA().setFirstScan(true);
    }

    vector<string> waitTimesInitialized;
    for (auto &it: PARA_station) {
        string name = it.first;
        if (name == "waitTimes") {
            vector<string> waitTimesNow;
            string memberName = it.second.get_child("<xmlattr>.member").data();
            if (groups.find(memberName) != groups.end()) {
                waitTimesNow.insert(waitTimesNow.end(), groups[memberName].begin(), groups[memberName].end());
            } else if (memberName == "__all__") {
                for (const auto &sta:stations_) {
                    waitTimesNow.push_back(sta.getName());
                }
            } else {
                waitTimesNow.push_back(memberName);
            }

            bool errorFlagWaitTime = false;
            for (const auto &any:waitTimesNow) {
                if (find(waitTimesInitialized.begin(), waitTimesInitialized.end(), any) != waitTimesInitialized.end()) {
                    cerr << "ERROR: double use of station/group " << name
                         << " in wait times block! This whole block is ignored!";
                    errorFlagWaitTime = true;
                }
            }
            if (errorFlagWaitTime) {
                continue;
            }


            for (const auto &any: waitTimesNow) {
                for (auto &sta:stations_) {
                    if (any == sta.getName()) {
                        Station::WAITTIMES wtimes;
                        wtimes.setup = it.second.get<double>("setup");
                        wtimes.source = it.second.get<double>("source");
                        wtimes.tape = it.second.get<double>("tape");
                        wtimes.calibration = it.second.get<double>("calibration");
                        wtimes.corsynch = it.second.get<double>("corsynch");
                        sta.setWaitTimes(wtimes);
                        break;
                    }
                }
            }
            waitTimesInitialized.insert(waitTimesInitialized.end(), waitTimesNow.begin(), waitTimesNow.end());
        }
    }

    vector<string> cableInitialized;
    for (auto &it: PARA_station) {
        string name = it.first;
        if (name == "cableWrapBuffer") {
            vector<string> cableNow;
            string memberName = it.second.get_child("<xmlattr>.member").data();
            if (groups.find(memberName) != groups.end()) {
                cableNow.insert(cableNow.end(), groups[memberName].begin(), groups[memberName].end());
            } else if (memberName == "__all__") {
                for (const auto &sta:stations_) {
                    cableNow.push_back(sta.getName());
                }
            } else {
                cableNow.push_back(memberName);
            }

            bool errorFlagWaitTime = false;
            for (const auto &any:cableNow) {
                if (find(cableInitialized.begin(), cableInitialized.end(), any) != cableInitialized.end()) {
                    cerr << "ERROR: double use of station/group " << name
                         << " in wait times block! This whole block is ignored!";
                    errorFlagWaitTime = true;
                }
            }
            if (errorFlagWaitTime) {
                continue;
            }


            for (const auto &any: cableNow) {
                for (auto &sta:stations_) {
                    if (any == sta.getName()) {
                        double axis1Low = it.second.get<double>("axis1LowOffset");
                        double axis1Up = it.second.get<double>("axis1UpOffset");
                        double axis2Low = it.second.get<double>("axis2LowOffset");
                        double axis2Up = it.second.get<double>("axis2UpOffset");
                        sta.referenceCableWrap().setMinimumOffsets(axis1Low, axis1Up, axis2Low, axis2Up);
                        break;
                    }
                }
            }
            cableInitialized.insert(cableInitialized.end(), cableNow.begin(), cableNow.end());
        }
    }


}

void Initializer::stationSetup(vector<vector<Station::EVENT> > &events,
                                    const boost::property_tree::ptree &tree,
                                    const unordered_map<std::string, Station::PARAMETERS> &parameters,
                                    const unordered_map<std::string, std::vector<std::string> > &groups,
                                    const Station::PARAMETERS &parentPARA) noexcept {

    vector<string> members;
    Station::PARAMETERS combinedPARA = parentPARA;
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
                for (const auto &any:stations_) {
                    members.push_back(any.getName());
                }
            } else {
                members.push_back(tmp);
            }
        } else if (paraName == "parameter") {
            string tmp = it.second.data();
            Station::PARAMETERS newPARA = parameters.at(tmp);
            if (newPARA.available.is_initialized()) {
                combinedPARA.available = *newPARA.available;
            }
            if (newPARA.firstScan.is_initialized()) {
                combinedPARA.firstScan = *newPARA.firstScan;
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
            if (newPARA.maxSlewtime.is_initialized()) {
                combinedPARA.maxSlewtime = *newPARA.maxSlewtime;
            }
            if (newPARA.maxWait.is_initialized()) {
                combinedPARA.maxWait = *newPARA.maxWait;
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
            boost::posix_time::ptime thisStartTime = it.second.get_value<boost::posix_time::ptime>();
            boost::posix_time::time_duration a = thisStartTime - TimeSystem::startTime;
            int sec = a.total_seconds();
            start = static_cast<unsigned int>(sec);
        } else if (paraName == "end") {
            boost::posix_time::ptime thisEndTime = it.second.get_value<boost::posix_time::ptime>();
            boost::posix_time::time_duration a = thisEndTime - TimeSystem::startTime;
            int sec = a.total_seconds();
            end = static_cast<unsigned int>(sec);
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
    for (const auto &any:stations_) {
        staNames.push_back(any.getName());
    }

    for (const auto &any:members) {

        auto it = find(staNames.begin(), staNames.end(), any);
        int id = it - staNames.begin();
        auto &thisEvents = events[id];


        Station::EVENT newEvent_start;
        newEvent_start.time = start;
        newEvent_start.softTransition = softTransition;
        newEvent_start.PARA = combinedPARA;

        for (auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit) {
            if (iit->time > newEvent_start.time) {
                thisEvents.insert(iit, newEvent_start);
                break;
            }
        }

        Station::EVENT newEvent_end;
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

void Initializer::initializeSources() noexcept {

    boost::property_tree::ptree PARA_source;
    try{
        PARA_source = xml_.get_child("master.source");
    }catch(const boost::property_tree::ptree_error &e){
        cout << "ERROR: reading parameters.xml file!" <<
             "    probably missing <source> block?" << endl;
    }

    unordered_map<std::string, std::vector<std::string> > groups = readGroups(PARA_source, GroupType::source);

    unordered_map<std::string, Source::PARAMETERS> parameters;
    for (auto &it: PARA_source) {
        string name = it.first;
        if (name == "parameters") {
            string parameterName = it.second.get_child("<xmlattr>.name").data();

            Source::PARAMETERS PARA;

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
                } else if (paraName == "maxNumberOfScans") {
                    PARA.maxNumberOfScans = it2.second.get_value < unsigned
                    int > ();
                } else if (paraName == "tryToFocusIfObservedOnce") {
                    PARA.tryToFocusIfObservedOnce = it2.second.get_value<bool>();
                } else if (paraName == "minSNR") {
                    string bandName = it2.second.get_child("<xmlattr>.band").data();
                    double value = it2.second.get_value<double>();
                    PARA.minSNR[bandName] = value;
                } else if (paraName == "ignoreStations") {
                    for (auto &it3: it2.second) {
                        string staName = it3.second.data();
                        for (int i = 0; i < stations_.size(); ++i) {
                            if (staName == stations_[i].getName()) {
                                PARA.ignoreStations.push_back(i);
                                break;
                            }
                        }
                    }
                } else if (paraName == "requiredStations") {
                    for (auto &it3: it2.second) {
                        string staName = it3.second.data();
                        for (int i = 0; i < stations_.size(); ++i) {
                            if (staName == stations_[i].getName()) {
                                PARA.requiredStations.push_back(i);
                                break;
                            }
                        }
                    }
                } else if (paraName == "ignoreBaselines") {
                    for (auto &it3: it2.second) {
                        string baselineName = it3.second.data();
                        vector<string> splitVec;
                        boost::split(splitVec, baselineName, boost::is_any_of("-"));
                        string station1 = splitVec[0];
                        int staid1;
                        for (int i = 0; i < stations_.size(); ++i) {
                            if (station1 == stations_[i].getName()) {
                                staid1 = i;
                                break;
                            }
                        }
                        string station2 = splitVec[1];
                        int staid2;
                        for (int i = 0; i < stations_.size(); ++i) {
                            if (station2 == stations_[i].getName()) {
                                staid2 = i;
                                break;
                            }
                        }
                        if (staid1 > staid2) {
                            swap(staid1, staid2);
                        }
                        PARA.ignoreBaselines.push_back({staid1, staid2});
                    }
                } else {
                    cerr << "<source> <parameter>: " << parameterName << ": parameter <" << name
                            << "> not understood! (Ignored)\n";
                }
            }
            parameters[parameterName] = PARA;
        }
    }

    Source::PARAMETERS parentPARA;
    parentPARA.available = true;

    parentPARA.weight = 1; ///< multiplicative factor of score for scans to this source

    parentPARA.minNumberOfStations = 2; ///< minimum number of stations for a scan
    parentPARA.minFlux = .01; ///< minimum flux density required for this source in jansky
    parentPARA.minRepeat = 1800; ///< minimum time between two observations of this source in seconds
    parentPARA.maxScan = 600; ///< maximum allowed scan time in seconds
    parentPARA.minScan = 30; ///< minimum required scan time in seconds

    for (const auto &any:ObservationMode::bands) {
        const string &name = any;
        parentPARA.minSNR[name] = 0;
    }

    vector<vector<Source::EVENT> > events(sources_.size());

    for (int i = 0; i < sources_.size(); ++i) {
        Source::EVENT newEvent_start;
        newEvent_start.time = 0;
        newEvent_start.softTransition = false;
        newEvent_start.PARA = parentPARA;

        events[i].push_back(newEvent_start);

        Source::EVENT newEvent_end;
        newEvent_end.time = TimeSystem::duration;
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
    for (int i = 0; i < sources_.size(); ++i) {
        sources_[i].setEVENTS(events[i]);
    }

    for (auto &any:sources_) {
        bool hardBreak = false;
        ofstream dummy;
        any.checkForNewEvent(0, hardBreak, false, dummy);
    }


}


void Initializer::sourceSetup(vector<vector<Source::EVENT> > &events,
                              const boost::property_tree::ptree &tree,
                              const unordered_map<std::string, Source::PARAMETERS> &parameters,
                              const unordered_map<std::string, std::vector<std::string> > &groups,
                              const Source::PARAMETERS &parentPARA) noexcept {

    vector<string> members;
    Source::PARAMETERS combinedPARA = parentPARA;
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
            string tmp = it.second.data();
            Source::PARAMETERS newPARA = parameters.at(tmp);
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
            if (newPARA.maxNumberOfScans.is_initialized()) {
                combinedPARA.maxNumberOfScans = *newPARA.maxNumberOfScans;
            }
            if (newPARA.tryToFocusIfObservedOnce.is_initialized()) {
                combinedPARA.tryToFocusIfObservedOnce = *newPARA.tryToFocusIfObservedOnce;
            }

            if (newPARA.fixedScanDuration.is_initialized()) {
                combinedPARA.fixedScanDuration = *newPARA.fixedScanDuration;
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
            boost::posix_time::ptime thisStartTime = it.second.get_value<boost::posix_time::ptime>();
            boost::posix_time::time_duration a = thisStartTime - TimeSystem::startTime;
            int sec = a.total_seconds();
            start = static_cast<unsigned int>(sec);
        } else if (paraName == "end") {
            boost::posix_time::ptime thisEndTime = it.second.get_value<boost::posix_time::ptime>();
            boost::posix_time::time_duration a = thisEndTime - TimeSystem::startTime;
            int sec = a.total_seconds();
            end = static_cast<unsigned int>(sec);
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
    for (const auto &any:sources_) {
        staNames.push_back(any.getName());
    }

    for (const auto &any:members) {

        auto it = find(staNames.begin(), staNames.end(), any);
        if (it == staNames.end()) {
            continue;
        }
        int id = it - staNames.begin();
        auto &thisEvents = events[id];


        Source::EVENT newEvent_start;
        newEvent_start.time = start;
        newEvent_start.softTransition = softTransition;
        newEvent_start.PARA = combinedPARA;

        for (auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit) {
            if (iit->time > newEvent_start.time) {
                thisEvents.insert(iit, newEvent_start);
                break;
            }
        }

        Source::EVENT newEvent_end;
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


void Initializer::displaySummary(ofstream &headerLog) noexcept {

    headerLog << "List of all sources:\n";
    headerLog << "------------------------------------\n";
    for(auto& any:stations_){
        headerLog << any;
    }
    headerLog << "List of all sources:\n";
    headerLog << "------------------------------------\n";
    for(auto& any:sources_){
        headerLog << any;
    }
}

void Initializer::initializeNutation() noexcept {
    vector<unsigned int> nut_t;
    vector<double> nut_x;
    vector<double> nut_y;
    vector<double> nut_s;

    double date1 = 2400000.5;
    double date2 = TimeSystem::mjdStart;

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

    Nutation::nutX = nut_x;
    Nutation::nutY = nut_y;
    Nutation::nutS = nut_s;
    Nutation::nutTime = nut_t;
}

void Initializer::initializeEarth() noexcept {
    double date1 = 2400000.5;
    double date2 = TimeSystem::mjdStart;
    double pvh[2][3];
    double pvb[2][3];
    iauEpv00(date1, date2, pvh, pvb);
    double aud2ms = DAU / DAYSEC;
    double vearth[3] = {aud2ms * pvb[1][0],
                        aud2ms * pvb[1][1],
                        aud2ms * pvb[1][2]};
    Earth::velocity = {vearth[0], vearth[1], vearth[2]};
}

void Initializer::initializeLookup() noexcept {


    unordered_map<int, double> sinLookup;
    double x = 0;
    int counter = 0;
    while (x < pi + 0.001) {
        double val = sin(x);
        sinLookup.insert(make_pair(counter, val));
        x += .001;
        ++counter;
    }
    LookupTable::sinLookup = sinLookup;


    unordered_map<int, double> cosLookup;
    x = 0;
    counter = 0;
    while (x < pi + 0.001) {
        double val = cos(x);
        cosLookup.insert(make_pair(counter, val));
        x += .001;
        ++counter;
    }
    LookupTable::cosLookup = cosLookup;

    unordered_map<int, double> acosLookup;
    x = 0;
    counter = 0;
    while (x < 1) {
        double val = acos(x);
        acosLookup.insert(make_pair(counter, val));
        x += .001;
        ++counter;
    }
    LookupTable::acosLookup = acosLookup;

}

void Initializer::initializeWeightFactors() noexcept {
    WeightFactors::weightSkyCoverage = xml_.get<double>("master.weightFactor.skyCoverage", 0);
    WeightFactors::weightNumberOfObservations = xml_.get<double>("master.weightFactor.numberOfObservations", 0);
    WeightFactors::weightDuration = xml_.get<double>("master.weightFactor.duration", 0);
    WeightFactors::weightAverageSources = xml_.get<double>("master.weightFactor.averageSources", 0);
    WeightFactors::weightAverageStations = xml_.get<double>("master.weightFactor.averageStations", 0);

    WeightFactors::weightDeclination = xml_.get<double>("master.weightFactor.weightDeclination", 0);
    WeightFactors::declinationSlopeStart = xml_.get<double>("master.weightFactor.declinationSlopeStart", 0) * deg2rad;
    WeightFactors::declinationSlopeEnd = xml_.get<double>("master.weightFactor.declinationSlopeEnd", 0) * deg2rad;

    WeightFactors::weightLowElevation = xml_.get<double>("master.weightFactor.weightLowElevation", 0);
    WeightFactors::lowElevationSlopeStart = xml_.get<double>("master.weightFactor.lowElevationSlopeStart", 0) * deg2rad;
    WeightFactors::lowElevationSlopeEnd = xml_.get<double>("master.weightFactor.lowElevationSlopeEnd", 0) * deg2rad;
}

void Initializer::initializeSkyCoverages() noexcept {
    unsigned int maxEl = 91;
    unsigned int sizeAz = 181;
    vector<vector<vector<float> > > storage;

    for (unsigned int thisEl = 0; thisEl < maxEl; ++thisEl) {
        unsigned int sizeEl = maxEl-thisEl;

        vector<vector<float> > thisStorage(sizeAz,vector<float>(sizeEl,0));


        for (int deltaAz = 0; deltaAz < sizeAz; ++deltaAz) {
            for (int deltaEl = 0; deltaEl < sizeEl; ++deltaEl) {
                auto tmp = static_cast<float>(sin(thisEl * deg2rad) * sin(thisEl * deg2rad + deltaEl * deg2rad) +
                                              cos(thisEl * deg2rad) * cos(thisEl * deg2rad + deltaEl * deg2rad) *
                                              cos(deltaAz * deg2rad));
                float angle = acos(tmp);
                thisStorage[deltaAz][deltaEl] = angle;
            }
        }
        storage.push_back(thisStorage);
    }
    SkyCoverage::angularDistanceLookup = storage;

    SkyCoverage::maxInfluenceDistance = xml_.get<double>("master.skyCoverage.skyCoverageDistance", 30) * deg2rad;
    SkyCoverage::maxInfluenceTime = xml_.get<double>("master.skyCoverage.skyCoverageInterval", 3600);
    SkyCoverage::maxTwinTelecopeDistance = xml_.get<double>("master.skyCoverage.maxTwinTelecopeDistance", 0);

}


void Initializer::initializeBaselines() noexcept {

    boost::property_tree::ptree PARA_baseline;
    try {
        PARA_baseline = xml_.get_child("master.baseline");
    } catch (const boost::property_tree::ptree_error &e) {
        cout << "ERROR: reading parameters.xml file!" <<
             "    probably missing <baseline> block?" << endl;
    }

    unordered_map<std::string, std::vector<std::string> > groups = readGroups(PARA_baseline, GroupType::baseline);

    unordered_map<std::string, Baseline::PARAMETERS> parameters;
    for (auto &it: PARA_baseline) {
        string name = it.first;
        if (name == "parameters") {
            string parameterName = it.second.get_child("<xmlattr>.name").data();

            Baseline::PARAMETERS PARA;

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

    unsigned long nsta = stations_.size();
    vector<vector <char> > ignore(nsta, vector< char>(nsta,false));
    vector< vector<unsigned int> > minScan(nsta, vector<unsigned int>(nsta,0));
    vector< vector<unsigned int> > maxScan(nsta, vector<unsigned int>(nsta,numeric_limits<unsigned int>::max()));
    vector< vector<double> > weight(nsta, vector<double>(nsta,1));
    unordered_map<string,vector< vector<double> >> minSNR;

    for (const auto &any:ObservationMode::bands) {
        const string &name = any;
        vector<vector<double> > tmp(nsta, vector<double>(nsta, 0));
        minSNR[name] = tmp;
    }

    Baseline::PARA.ignore = ignore;
    Baseline::PARA.minScan = minScan;
    Baseline::PARA.maxScan = maxScan;
    Baseline::PARA.weight = weight;
    Baseline::PARA.minSNR = minSNR;

    Baseline::PARAMETERS parentPARA;
    parentPARA.ignore = false;
    parentPARA.weight = 1; ///< multiplicative factor of score for scans to this source
    parentPARA.maxScan = 9999; ///< maximum allowed scan time in seconds
    parentPARA.minScan = 0; ///< minimum required scan time in seconds
    for (const auto &any:ObservationMode::bands) {
        const string &name = any;
        parentPARA.minSNR[name] = 0;
    }

    vector<vector<vector<Baseline::EVENT> > > events(nsta, vector<vector<Baseline::EVENT> >(nsta));
    vector<vector<unsigned int> > nextEvent(nsta, vector<unsigned int>(nsta, 0));

    for (int i = 0; i < nsta; ++i) {
        for (int j = i + 1; j < nsta; ++j) {
            Baseline::EVENT newEvent_start;
            newEvent_start.time = 0;
            newEvent_start.softTransition = false;
            newEvent_start.PARA = parentPARA;

            events[i][j].push_back(newEvent_start);

            Baseline::EVENT newEvent_end;
            newEvent_end.time = TimeSystem::duration;
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

    Baseline::EVENTS = events;
    Baseline::nextEvent = nextEvent;
    bool hardBreak = false;
    ofstream dummy;
    Baseline::checkForNewEvent(0, hardBreak, false, dummy);

}

void Initializer::baselineSetup(vector<vector<vector<Baseline::EVENT> > > &events,
                                     const boost::property_tree::ptree &tree,
                                     const unordered_map<std::string, Baseline::PARAMETERS> &parameters,
                                     const unordered_map<std::string, std::vector<std::string> > &groups,
                                     const Baseline::PARAMETERS &parentPARA) noexcept {

    vector<string> members;
    Baseline::PARAMETERS combinedPARA = parentPARA;
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
            string tmp = it.second.data();
            Baseline::PARAMETERS newPARA = parameters.at(tmp);
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
            boost::posix_time::ptime thisStartTime = it.second.get_value<boost::posix_time::ptime>();
            boost::posix_time::time_duration a = thisStartTime - TimeSystem::startTime;
            int sec = a.total_seconds();
            start = static_cast<unsigned int>(sec);
        } else if (paraName == "end") {
            boost::posix_time::ptime thisEndTime = it.second.get_value<boost::posix_time::ptime>();
            boost::posix_time::time_duration a = thisEndTime - TimeSystem::startTime;
            int sec = a.total_seconds();
            end = static_cast<unsigned int>(sec);
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
    for (const auto &any:stations_) {
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

        Baseline::EVENT newEvent_start;
        newEvent_start.time = start;
        newEvent_start.softTransition = softTransition;
        newEvent_start.PARA = combinedPARA;

        for (auto iit = events[id1][id2].begin(); iit < events[id1][id2].end(); ++iit) {
            if (iit->time > newEvent_start.time) {
                events[id1][id2].insert(iit, newEvent_start);
                break;
            }
        }

        Baseline::EVENT newEvent_end;
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


void Initializer::initializeObservingMode(ofstream &headerLog) noexcept {
    auto PARA_mode = xml_.get_child("master.mode");
    for (const auto &it: PARA_mode) {
        if (it.first == "sampleRate") {
            ObservationMode::sampleRate = it.second.get_value<unsigned int>();
        }else if(it.first == "bits"){
            ObservationMode::bits = it.second.get_value<unsigned int>();
        }else if(it.first == "band"){
            double wavelength;
            unsigned int channels;
            ObservationMode::Property station_property;
            ObservationMode::Backup station_backup = ObservationMode::Backup::none;
            double station_backupValue;
            ObservationMode::Property source_property;
            ObservationMode::Backup source_backup = ObservationMode::Backup::none;
            double source_backupValue;
            string name;

            for (const auto &it_band:it.second){

                if(it_band.first == "<xmlattr>"){
                    name = it_band.second.get_child("name").data();
                }else if(it_band.first == "wavelength"){
                    wavelength = it_band.second.get_value<double>();
                }else if(it_band.first == "chanels"){
                    channels = it_band.second.get_value<unsigned int>();
                }else if(it_band.first == "station"){
                    for (const auto &it_band_station:it_band.second){
                        string thisName = it_band_station.first.data();
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
                }else if(it_band.first == "source"){
                    for (const auto &it_band_source:it_band.second){
                        string thisName = it_band_source.first.data();
                        if (thisName == "tag"){
                            if(it_band_source.second.get_value<std::string>() == "required") {
                                source_property = ObservationMode::Property::required;
                            }else if(it_band_source.second.get_value<std::string>() == "optional"){
                                source_property = ObservationMode::Property::optional;
                            }
                        } else if (thisName == "backup_maxValueTimes"){
                            source_backup = ObservationMode::Backup::maxValueTimes;
                            source_backupValue = it_band_source.second.get_value<double>();

                        } else if (thisName == "backup_minValueTimes"){
                            source_backup = ObservationMode::Backup::minValueTimes;
                            source_backupValue = it_band_source.second.get_value<double>();

                        } else if (thisName == "backup_value"){
                            source_backup = ObservationMode::Backup::value;
                            source_backupValue = it_band_source.second.get_value<double>();
                        }
                    }
                }
            }
            ObservationMode::bands.push_back(name);

            ObservationMode::nChannels[name] = channels;
            ObservationMode::wavelength[name] = wavelength;

            ObservationMode::stationProperty[name] = station_property;
            ObservationMode::stationBackup[name] = station_backup;
            ObservationMode::stationBackupValue[name] = station_backupValue;

            ObservationMode::sourceProperty[name] = source_property;
            ObservationMode::sourceBackup[name] = source_backup;
            ObservationMode::sourceBackupValue[name] = source_backupValue;
        }
    }

}

unordered_map<string, vector<string> > Initializer::readGroups(boost::property_tree::ptree root, GroupType type) noexcept {
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
            for(const auto&any:stations_){
                members.push_back(any.getName());
            }
            groups["__all__"] = members;
            break;
        }
        case GroupType::baseline:{
            std::vector<std::string> members;
            for(int i = 0; i<stations_.size(); ++i){
                for (int j = i+1; j<stations_.size(); ++j){
                    members.push_back(stations_[i].getName() + "-" + stations_[j].getName());
                }
            }
            groups["__all__"] = members;
            break;
        }
    }

    return groups;
}

void Initializer::applyMultiSchedParameters(const VieVS::MultiScheduling::Parameters &parameters,
                                                 ofstream &bodyLog) {
    parameters.output(bodyLog);

    boost::property_tree::ptree PARA_station = xml_.get_child("master.station");
    unordered_map<std::string, std::vector<std::string> > group_station = readGroups(PARA_station, GroupType::station);
    boost::property_tree::ptree PARA_source = xml_.get_child("master.source");
    unordered_map<std::string, std::vector<std::string> > group_source = readGroups(PARA_source, GroupType::source);
    boost::property_tree::ptree PARA_baseline = xml_.get_child("master.baseline");
    unordered_map<std::string, std::vector<std::string> > group_baseline = readGroups(PARA_baseline, GroupType::baseline);



    unsigned long nsta = stations_.size();

    if (parameters.start.is_initialized()) {
        boost::posix_time::ptime startTime = *parameters.start;
        int sec_ = startTime.time_of_day().total_seconds();
        double mjdStart = startTime.date().modjulian_day() + sec_ / 86400;

        boost::posix_time::ptime endTime = startTime + boost::posix_time::seconds(
                static_cast<long>(TimeSystem::duration));

        TimeSystem::mjdStart = mjdStart;
        TimeSystem::startTime = startTime;
        TimeSystem::endTime = endTime;
    }
    if (parameters.subnetting.is_initialized()) {
        parameters_.subnetting = *parameters.subnetting;
    }
    if (parameters.fillinmode.is_initialized()) {
        parameters_.fillinmode = *parameters.fillinmode;
    }

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

    if (!parameters.stationMaxSlewtime.empty()) {
        for (const auto &any:parameters.stationMaxSlewtime) {
            string name = any.first;
            if (group_station.find(name) != group_station.end()) {
                vector<string> members = group_station[name];
                for (const auto &thisName:members) {
                    for (auto &thisStation:stations_) {
                        if (thisStation.getName() == thisName) {
                            thisStation.referencePARA().maxSlewtime = any.second;
                        }
                    }
                }
            } else {
                for (auto &thisStation:stations_) {
                    if (thisStation.getName() == name) {
                        thisStation.referencePARA().maxSlewtime = any.second;
                    }
                }
            }
        }
    }
    if (!parameters.stationWeight.empty()) {
        for (const auto &any:parameters.stationWeight) {
            string name = any.first;
            if (group_station.find(name) != group_station.end()) {
                vector<string> members = group_station[name];
                for (const auto &thisName:members) {
                    for (auto &thisStation:stations_) {
                        if (thisStation.getName() == thisName) {
                            thisStation.referencePARA().weight = any.second;
                        }
                    }
                }
            } else {
                for (auto &thisStation:stations_) {
                    if (thisStation.getName() == name) {
                        thisStation.referencePARA().weight = any.second;
                    }
                }
            }
        }
    }
    if (!parameters.stationMaxWait.empty()) {
        for (const auto &any:parameters.stationMaxWait) {
            string name = any.first;
            if (group_station.find(name) != group_station.end()) {
                vector<string> members = group_station[name];
                for (const auto &thisName:members) {
                    for (auto &thisStation:stations_) {
                        if (thisStation.getName() == thisName) {
                            thisStation.referencePARA().maxWait = any.second;
                        }
                    }
                }
            } else {
                for (auto &thisStation:stations_) {
                    if (thisStation.getName() == name) {
                        thisStation.referencePARA().maxWait = any.second;
                    }
                }
            }
        }
    }
    if (!parameters.stationMinScan.empty()) {
        for (const auto &any:parameters.stationMinScan) {
            string name = any.first;
            if (group_station.find(name) != group_station.end()) {
                vector<string> members = group_station[name];
                for (const auto &thisName:members) {
                    for (auto &thisStation:stations_) {
                        if (thisStation.getName() == thisName) {
                            thisStation.referencePARA().minScan = any.second;
                        }
                    }
                }
            } else {
                for (auto &thisStation:stations_) {
                    if (thisStation.getName() == name) {
                        thisStation.referencePARA().minScan = any.second;
                    }
                }
            }
        }
    }
    if (!parameters.stationMaxScan.empty()) {
        for (const auto &any:parameters.stationMaxScan) {
            string name = any.first;
            if (group_station.find(name) != group_station.end()) {
                vector<string> members = group_station[name];
                for (const auto &thisName:members) {
                    for (auto &thisStation:stations_) {
                        if (thisStation.getName() == thisName) {
                            thisStation.referencePARA().maxScan = any.second;
                        }
                    }
                }
            } else {
                for (auto &thisStation:stations_) {
                    if (thisStation.getName() == name) {
                        thisStation.referencePARA().maxScan = any.second;
                    }
                }
            }
        }
    }

    if (!parameters.sourceMinNumberOfStations.empty()) {
        for (const auto &any:parameters.sourceMinNumberOfStations) {
            string name = any.first;
            if (group_source.find(name) != group_source.end()) {
                vector<string> members = group_source[name];
                for (const auto &thisName:members) {
                    for (auto &thisSource:sources_) {
                        if (thisSource.getName() == thisName) {
                            thisSource.referencePARA().minNumberOfStations = any.second;
                        }
                    }
                }
            } else {
                for (auto &thisSource:sources_) {
                    if (thisSource.getName() == name) {
                        thisSource.referencePARA().minNumberOfStations = any.second;
                    }
                }
            }
        }
    }
    if (!parameters.sourceMinFlux.empty()) {
        for (const auto &any:parameters.sourceMinFlux) {
            string name = any.first;
            if (group_source.find(name) != group_source.end()) {
                vector<string> members = group_source[name];
                for (const auto &thisName:members) {
                    for (auto &thisSource:sources_) {
                        if (thisSource.getName() == thisName) {
                            thisSource.referencePARA().minFlux = any.second;
                        }
                    }
                }
            } else {
                for (auto &thisSource:sources_) {
                    if (thisSource.getName() == name) {
                        thisSource.referencePARA().minFlux = any.second;
                    }
                }
            }
        }
    }
    if (!parameters.sourceMinRepeat.empty()) {
        for (const auto &any:parameters.sourceMinRepeat) {
            string name = any.first;
            if (group_source.find(name) != group_source.end()) {
                vector<string> members = group_source[name];
                for (const auto &thisName:members) {
                    for (auto &thisSource:sources_) {
                        if (thisSource.getName() == thisName) {
                            thisSource.referencePARA().minRepeat = any.second;
                        }
                    }
                }
            } else {
                for (auto &thisSource:sources_) {
                    if (thisSource.getName() == name) {
                        thisSource.referencePARA().minRepeat = any.second;
                    }
                }
            }
        }
    }
    if (!parameters.sourceWeight.empty()) {
        for (const auto &any:parameters.sourceWeight) {
            string name = any.first;
            if (group_source.find(name) != group_source.end()) {
                vector<string> members = group_source[name];
                for (const auto &thisName:members) {
                    for (auto &thisSource:sources_) {
                        if (thisSource.getName() == thisName) {
                            thisSource.referencePARA().weight = any.second;
                        }
                    }
                }
            } else {
                for (auto &thisSource:sources_) {
                    if (thisSource.getName() == name) {
                        thisSource.referencePARA().weight = any.second;
                    }
                }
            }
        }
    }
    if (!parameters.sourceMinScan.empty()) {
        for (const auto &any:parameters.sourceMinScan) {
            string name = any.first;
            if (group_source.find(name) != group_source.end()) {
                vector<string> members = group_source[name];
                for (const auto &thisName:members) {
                    for (auto &thisSource:sources_) {
                        if (thisSource.getName() == thisName) {
                            thisSource.referencePARA().minScan = any.second;
                        }
                    }
                }
            } else {
                for (auto &thisSource:sources_) {
                    if (thisSource.getName() == name) {
                        thisSource.referencePARA().minScan = any.second;
                    }
                }
            }
        }
    }
    if (!parameters.sourceMaxScan.empty()) {
        for (const auto &any:parameters.sourceMaxScan) {
            string name = any.first;
            if (group_source.find(name) != group_source.end()) {
                vector<string> members = group_source[name];
                for (const auto &thisName:members) {
                    for (auto &thisSource:sources_) {
                        if (thisSource.getName() == thisName) {
                            thisSource.referencePARA().maxScan = any.second;
                        }
                    }
                }
            } else {
                for (auto &thisSource:sources_) {
                    if (thisSource.getName() == name) {
                        thisSource.referencePARA().maxScan = any.second;
                    }
                }
            }
        }
    }

    if (!parameters.baselineWeight.empty()) {
        for (const auto &any:parameters.baselineWeight) {
            string name = any.first;
            if (group_baseline.find(name) != group_baseline.end()) {
                vector<string> members = group_baseline[name];
                for (const auto &thisName:members) {
                    vector<string> baseline_stations;
                    boost::split(baseline_stations, thisName, boost::is_any_of("-"));
                    string sta0 = baseline_stations[0];
                    string sta1 = baseline_stations[1];
                    int staid0;
                    int staid1;
                    for (int i = 0; i < nsta; ++i) {
                        if (stations_[i].getName() == sta0) {
                            staid0 = i;
                        } else if (stations_[i].getName() == sta1) {
                            staid1 = i;
                        }
                    }
                    if (staid0 > staid1) {
                        std::swap(staid0, staid1);
                    }

                    Baseline::PARA.weight[staid0][staid1] = any.second;
                }
            } else {
                vector<string> baseline_stations;
                boost::split(baseline_stations, name, boost::is_any_of("-"));
                string sta0 = baseline_stations[0];
                string sta1 = baseline_stations[1];
                int staid0;
                int staid1;
                for (int i = 0; i < nsta; ++i) {
                    if (stations_[i].getName() == sta0) {
                        staid0 = i;
                    } else if (stations_[i].getName() == sta1) {
                        staid1 = i;
                    }
                }
                if (staid0 > staid1) {
                    std::swap(staid0, staid1);
                }

                Baseline::PARA.weight[staid0][staid1] = any.second;
            }
        }
    }
    if (!parameters.baselineMinScan.empty()) {
        for (const auto &any:parameters.baselineMinScan) {
            string name = any.first;
            if (group_baseline.find(name) != group_baseline.end()) {
                vector<string> members = group_baseline[name];
                for (const auto &thisName:members) {
                    vector<string> baseline_stations;
                    boost::split(baseline_stations, thisName, boost::is_any_of("-"));
                    string sta0 = baseline_stations[0];
                    string sta1 = baseline_stations[1];
                    int staid0;
                    int staid1;
                    for (int i = 0; i < nsta; ++i) {
                        if (stations_[i].getName() == sta0) {
                            staid0 = i;
                        } else if (stations_[i].getName() == sta1) {
                            staid1 = i;
                        }
                    }
                    if (staid0 > staid1) {
                        std::swap(staid0, staid1);
                    }

                    Baseline::PARA.minScan[staid0][staid1] = any.second;
                }
            } else {
                vector<string> baseline_stations;
                boost::split(baseline_stations, name, boost::is_any_of("-"));
                string sta0 = baseline_stations[0];
                string sta1 = baseline_stations[1];
                int staid0;
                int staid1;
                for (int i = 0; i < nsta; ++i) {
                    if (stations_[i].getName() == sta0) {
                        staid0 = i;
                    } else if (stations_[i].getName() == sta1) {
                        staid1 = i;
                    }
                }
                if (staid0 > staid1) {
                    std::swap(staid0, staid1);
                }

                Baseline::PARA.minScan[staid0][staid1] = any.second;
            }
        }
    }
    if (!parameters.baselineMaxScan.empty()) {
        for (const auto &any:parameters.baselineMaxScan) {
            string name = any.first;
            if (group_baseline.find(name) != group_baseline.end()) {
                vector<string> members = group_baseline[name];
                for (const auto &thisName:members) {
                    vector<string> baseline_stations;
                    boost::split(baseline_stations, thisName, boost::is_any_of("-"));
                    string sta0 = baseline_stations[0];
                    string sta1 = baseline_stations[1];
                    int staid0;
                    int staid1;
                    for (int i = 0; i < nsta; ++i) {
                        if (stations_[i].getName() == sta0) {
                            staid0 = i;
                        } else if (stations_[i].getName() == sta1) {
                            staid1 = i;
                        }
                    }
                    if (staid0 > staid1) {
                        std::swap(staid0, staid1);
                    }

                    Baseline::PARA.maxScan[staid0][staid1] = any.second;
                }
            } else {
                vector<string> baseline_stations;
                boost::split(baseline_stations, name, boost::is_any_of("-"));
                string sta0 = baseline_stations[0];
                string sta1 = baseline_stations[1];
                int staid0;
                int staid1;
                for (int i = 0; i < nsta; ++i) {
                    if (stations_[i].getName() == sta0) {
                        staid0 = i;
                    } else if (stations_[i].getName() == sta1) {
                        staid1 = i;
                    }
                }
                if (staid0 > staid1) {
                    std::swap(staid0, staid1);
                }

                Baseline::PARA.maxScan[staid0][staid1] = any.second;
            }
        }
    }
}

vector<MultiScheduling::Parameters> Initializer::readMultiSched() {
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


        for (const auto &any:mstree) {
            std::string name = any.first;
            if (name == "start") {
                vector<boost::posix_time::ptime> data;
                for (const auto &any2:any.second) {
                    data.push_back(any2.second.get_value<boost::posix_time::ptime>());
                }
                ms.setStart(data);
            } else if (name == "multisched_subnetting") {
                ms.setMultiSched_subnetting(true);
            } else if (name == "multisched_fillinmode") {
                ms.setMultiSched_fillinmode(true);
            } else if (name == "weight_skyCoverage") {
                vector<double> data;
                for (const auto &any2:any.second) {
                    data.push_back(any2.second.get_value<double>());
                }
                ms.setWeight_skyCoverage(data);
            } else if (name == "weight_numberOfObservations") {
                vector<double> data;
                for (const auto &any2:any.second) {
                    data.push_back(any2.second.get_value<double>());
                }
                ms.setWeight_numberOfObservations(data);
            } else if (name == "weight_duration") {
                vector<double> data;
                for (const auto &any2:any.second) {
                    data.push_back(any2.second.get_value<double>());
                }
                ms.setWeight_duration(data);
            } else if (name == "weight_averageSources") {
                vector<double> data;
                for (const auto &any2:any.second) {
                    data.push_back(any2.second.get_value<double>());
                }
                ms.setWeight_averageSources(data);
            } else if (name == "weight_averageStations") {
                vector<double> data;
                for (const auto &any2:any.second) {
                    data.push_back(any2.second.get_value<double>());
                }
                ms.setWeight_averageStations(data);
            } else if (name == "station_maxSlewtime") {
                vector<unsigned int> data;
                string name;
                for (const auto &any2:any.second) {
                    if (any2.first == "<xmlattr>") {
                        for (const auto &any3:any2.second) {
                            if (any3.first == "member") {
                                name = any3.second.get_value<std::string>();
                            }
                        }
                    }
                    if (any2.first == "value") {
                        data.push_back(any2.second.get_value < unsigned
                        int > ());
                    }
                }
                if (name.empty()) {
                    cerr << "missing member attribute in parameter file!\n";
                    terminate();
                }
                if (group_station.find(name) != group_station.end()) {
                    ms.setStation_maxSlewtime(ParameterGroup(name, group_station[name]), data);

                } else {
                    ms.setStation_maxSlewtime(name, data);
                }
            } else if (name == "stationMaxWait_") {
                vector<unsigned int> data;
                string name;
                for (const auto &any2:any.second) {
                    if (any2.first == "<xmlattr>") {
                        for (const auto &any3:any2.second) {
                            if (any3.first == "member") {
                                name = any3.second.get_value<std::string>();
                            }
                        }
                    }
                    if (any2.first == "value") {
                        data.push_back(any2.second.get_value < unsigned
                        int > ());
                    }
                }
                if (name.empty()) {
                    cerr << "missing member attribute in parameter file!\n";
                    terminate();
                }
                if (group_station.find(name) != group_station.end()) {
                    ms.setStation_maxWait(ParameterGroup(name, group_station[name]), data);

                } else {
                    ms.setStation_maxWait(name, data);
                }
            } else if (name == "station_maxScan") {
                vector<unsigned int> data;
                string name;
                for (const auto &any2:any.second) {
                    if (any2.first == "<xmlattr>") {
                        for (const auto &any3:any2.second) {
                            if (any3.first == "member") {
                                name = any3.second.get_value<std::string>();
                            }
                        }
                    }
                    if (any2.first == "value") {
                        data.push_back(any2.second.get_value < unsigned
                        int > ());
                    }
                }
                if (name.empty()) {
                    cerr << "missing member attribute in parameter file!\n";
                    terminate();
                }
                if (group_station.find(name) != group_station.end()) {
                    ms.setStation_maxScan(ParameterGroup(name, group_station[name]), data);

                } else {
                    ms.setStation_maxScan(name, data);
                }
            } else if (name == "station_minScan") {
                vector<unsigned int> data;
                string name;
                for (const auto &any2:any.second) {
                    if (any2.first == "<xmlattr>") {
                        for (const auto &any3:any2.second) {
                            if (any3.first == "member") {
                                name = any3.second.get_value<std::string>();
                            }
                        }
                    }
                    if (any2.first == "value") {
                        data.push_back(any2.second.get_value < unsigned
                        int > ());
                    }
                }
                if (name.empty()) {
                    cerr << "missing member attribute in parameter file!\n";
                    terminate();
                }
                if (group_station.find(name) != group_station.end()) {
                    ms.setStation_minScan(ParameterGroup(name, group_station[name]), data);

                } else {
                    ms.setStation_minScan(name, data);
                }
            } else if (name == "station_weight") {
                vector<double> data;
                string name;
                for (const auto &any2:any.second) {
                    if (any2.first == "<xmlattr>") {
                        for (const auto &any3:any2.second) {
                            if (any3.first == "member") {
                                name = any3.second.get_value<std::string>();
                            }
                        }
                    }
                    if (any2.first == "value") {
                        data.push_back(any2.second.get_value<double>());
                    }
                }
                if (name.empty()) {
                    cerr << "missing member attribute in parameter file!\n";
                    terminate();
                }
                if (group_station.find(name) != group_station.end()) {
                    ms.setStation_weight(ParameterGroup(name, group_station[name]), data);

                } else {
                    ms.setStation_weight(name, data);
                }
            } else if (name == "source_minNumberOfStations") {
                vector<unsigned int> data;
                string name;
                for (const auto &any2:any.second) {
                    if (any2.first == "<xmlattr>") {
                        for (const auto &any3:any2.second) {
                            if (any3.first == "member") {
                                name = any3.second.get_value<std::string>();
                            }
                        }
                    }
                    if (any2.first == "value") {
                        data.push_back(any2.second.get_value < unsigned
                        int > ());
                    }
                }
                if (name.empty()) {
                    cerr << "missing member attribute in parameter file!\n";
                    terminate();
                }
                if (group_source.find(name) != group_source.end()) {
                    ms.setSource_minNumberOfStations(ParameterGroup(name, group_source[name]), data);

                } else {
                    ms.setSource_minNumberOfStations(name, data);
                }
            } else if (name == "source_minFlux") {
                vector<double> data;
                string name;
                for (const auto &any2:any.second) {
                    if (any2.first == "<xmlattr>") {
                        for (const auto &any3:any2.second) {
                            if (any3.first == "member") {
                                name = any3.second.get_value<std::string>();
                            }
                        }
                    }
                    if (any2.first == "value") {
                        data.push_back(any2.second.get_value<double>());
                    }
                }
                if (name.empty()) {
                    cerr << "missing member attribute in parameter file!\n";
                    terminate();
                }
                if (group_source.find(name) != group_source.end()) {
                    ms.setSource_minFlux(ParameterGroup(name, group_source[name]), data);

                } else {
                    ms.setSource_minFlux(name, data);
                }
            } else if (name == "source_minRepeat") {
                vector<unsigned int> data;
                string name;
                for (const auto &any2:any.second) {
                    if (any2.first == "<xmlattr>") {
                        for (const auto &any3:any2.second) {
                            if (any3.first == "member") {
                                name = any3.second.get_value<std::string>();
                            }
                        }
                    }
                    if (any2.first == "value") {
                        data.push_back(any2.second.get_value < unsigned
                        int > ());
                    }
                }
                if (name.empty()) {
                    cerr << "missing member attribute in parameter file!\n";
                    terminate();
                }
                if (group_source.find(name) != group_source.end()) {
                    ms.setSource_minRepeat(ParameterGroup(name, group_source[name]), data);

                } else {
                    ms.setSource_minRepeat(name, data);
                }
            } else if (name == "source_maxScan") {
                vector<unsigned int> data;
                string name;
                for (const auto &any2:any.second) {
                    if (any2.first == "<xmlattr>") {
                        for (const auto &any3:any2.second) {
                            if (any3.first == "member") {
                                name = any3.second.get_value<std::string>();
                            }
                        }
                    }
                    if (any2.first == "value") {
                        data.push_back(any2.second.get_value < unsigned
                        int > ());
                    }
                }
                if (name.empty()) {
                    cerr << "missing member attribute in parameter file!\n";
                    terminate();
                }
                if (group_source.find(name) != group_source.end()) {
                    ms.setSource_maxScan(ParameterGroup(name, group_source[name]), data);

                } else {
                    ms.setSource_maxScan(name, data);
                }
            } else if (name == "source_minScan") {
                vector<unsigned int> data;
                string name;
                for (const auto &any2:any.second) {
                    if (any2.first == "<xmlattr>") {
                        for (const auto &any3:any2.second) {
                            if (any3.first == "member") {
                                name = any3.second.get_value<std::string>();
                            }
                        }
                    }
                    if (any2.first == "value") {
                        data.push_back(any2.second.get_value < unsigned
                        int > ());
                    }
                }
                if (name.empty()) {
                    cerr << "missing member attribute in parameter file!\n";
                    terminate();
                }
                if (group_source.find(name) != group_source.end()) {
                    ms.setSource_minScan(ParameterGroup(name, group_source[name]), data);

                } else {
                    ms.setSource_minScan(name, data);
                }
            } else if (name == "source_weight") {
                vector<double> data;
                string name;
                for (const auto &any2:any.second) {
                    if (any2.first == "<xmlattr>") {
                        for (const auto &any3:any2.second) {
                            if (any3.first == "member") {
                                name = any3.second.get_value<std::string>();
                            }
                        }
                    }
                    if (any2.first == "value") {
                        data.push_back(any2.second.get_value<double>());
                    }
                }
                if (name.empty()) {
                    cerr << "missing member attribute in parameter file!\n";
                    terminate();
                }
                if (group_source.find(name) != group_source.end()) {
                    ms.setSource_weight(ParameterGroup(name, group_source[name]), data);

                } else {
                    ms.setSource_weight(name, data);
                }
            } else if (name == "baseline_maxScan") {
                vector<unsigned int> data;
                string name;
                for (const auto &any2:any.second) {
                    if (any2.first == "<xmlattr>") {
                        for (const auto &any3:any2.second) {
                            if (any3.first == "member") {
                                name = any3.second.get_value<std::string>();
                            }
                        }
                    }
                    if (any2.first == "value") {
                        data.push_back(any2.second.get_value < unsigned
                        int > ());
                    }
                }
                if (name.empty()) {
                    cerr << "missing member attribute in parameter file!\n";
                    terminate();
                }
                if (group_baseline.find(name) != group_baseline.end()) {
                    ms.setBaseline_maxScan(ParameterGroup(name, group_baseline[name]), data);

                } else {
                    ms.setBaseline_maxScan(name, data);
                }
            } else if (name == "baseline_minScan") {
                vector<unsigned int> data;
                string name;
                for (const auto &any2:any.second) {
                    if (any2.first == "<xmlattr>") {
                        for (const auto &any3:any2.second) {
                            if (any3.first == "member") {
                                name = any3.second.get_value<std::string>();
                            }
                        }
                    }
                    if (any2.first == "value") {
                        data.push_back(any2.second.get_value < unsigned
                        int > ());
                    }
                }
                if (name.empty()) {
                    cerr << "missing member attribute in parameter file!\n";
                    terminate();
                }
                if (group_baseline.find(name) != group_baseline.end()) {
                    ms.setBaseline_minScan(ParameterGroup(name, group_baseline[name]), data);

                } else {
                    ms.setBaseline_minScan(name, data);
                }
            } else if (name == "baseline_weight") {
                vector<double> data;
                string name;
                for (const auto &any2:any.second) {
                    if (any2.first == "<xmlattr>") {
                        for (const auto &any3:any2.second) {
                            if (any3.first == "member") {
                                name = any3.second.get_value<std::string>();
                            }
                        }
                    }
                    if (any2.first == "value") {
                        data.push_back(any2.second.get_value<double>());
                    }
                }
                if (name.empty()) {
                    cerr << "missing member attribute in parameter file!\n";
                    terminate();
                }
                if (group_baseline.find(name) != group_baseline.end()) {
                    ms.setBaseline_weight(ParameterGroup(name, group_baseline[name]), data);

                } else {
                    ms.setBaseline_weight(name, data);
                }
            }


        }

        return ms.createMultiScheduleParameters();
    }
    return std::vector<MultiScheduling::Parameters>{};
}

SkdCatalogReader Initializer::createSkdCatalogReader() const noexcept {
    SkdCatalogReader reader;

    vector<string> staNames;
    boost::property_tree::ptree ptree_stations = xml_.get_child("master.general.stations");
    auto it = ptree_stations.begin();
    while (it != ptree_stations.end()) {
        auto item = it->second.data();
        staNames.push_back(item);
        ++it;
    }
    reader.setStationNames(staNames);
    reader.setCatalogFilePathes(xml_.get_child("master.catalogs"));

    return reader;
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
                vector<int> targetIds;

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
                vector<int> targetIds;
                int c = 0;
                for (const auto &source:sources_) {
                    if(c==9){
                        headerLog << "\n    ";
                        c = 0;
                    }
                    const string &name = source.getName();
                    headerLog << boost::format("%-8s ") % name;
                    if (find(targetSources.begin(), targetSources.end(), name) != targetSources.end()) {
                        targetIds.push_back(source.getId());
                    }
                    ++c;
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

            }

        }
    }
}

