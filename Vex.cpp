//
// Created by mschartn on 07.12.17.
//

#include "Vex.h"

using namespace std;
using namespace VieVS;
int Vex::nextId=0;
Vex::Vex(const string &file): VieVS_Object(nextId++){
    of = ofstream(file);
    of << "VEX_rev = 1.5;\n";
    of << "* ########################################################################################################\n";
    of << "* ###  This Vex file was produced by the new VieVS Scheduling Software                                 ###\n";
    of << "* ###  it is still in experimental stage! Please double check that file is correct!                    ###\n";
    of << "* ###  if you found any bugs please contact matthias.schartner@geo.tuwien.ac.at                        ###\n";
    of << "* ########################################################################################################\n";
}


void Vex::writeVex(const std::vector<Station> &stations, const std::vector<Source> &sources, const std::vector<Scan> &scans,
              const SkdCatalogReader &skdCatalogReader, const boost::property_tree::ptree &xml) {

    global_block(xml.get("master.output.experimentName","schedule"));

    exper_block(boost::trim_copy(xml.get("master.output.experimentName","dummy")),
                boost::trim_copy(xml.get("master.output.experimentDescription","no further description")),
                boost::trim_copy(xml.get("master.output.piName","")),
                boost::trim_copy(xml.get("master.output.piEmail","")),
                boost::trim_copy(xml.get("master.output.contactName","")),
                boost::trim_copy(xml.get("master.output.contactEmail","")),
                boost::trim_copy(xml.get("master.created.name","")),
                boost::trim_copy(xml.get("master.created.email","")),
                boost::trim_copy(xml.get("master.output.notes","")),
                boost::trim_copy(xml.get("master.output.correlator","unknown")));

    station_block(stations, skdCatalogReader);
    mode_block(stations,skdCatalogReader);
    sched_block(scans, stations, sources, skdCatalogReader);

    sites_block(stations, skdCatalogReader);
    antenna_block(stations);
    das_block(stations, skdCatalogReader);

    source_block(sources);

    bbc_block(skdCatalogReader);
    if_block(skdCatalogReader);
    tracks_block(stations, skdCatalogReader);
    freq_block(skdCatalogReader);

    pass_order_block();
    roll_block();
    phase_cal_detect_block();
}

void Vex::global_block(const std::string &expName) {
    of << "*=========================================================================================================\n";
    of << "$GLOBAL;\n";
    of << "*=========================================================================================================\n";
    of << "    ref $EXPER = " << expName << eol;
}

void Vex::exper_block(const std::string &expName, const std::string &expDescription, const std::string &piName,
                      const std::string &piEmail, const std::string &contactName, const std::string &contactEmail,
                      const std::string &schedulerName, const std::string &schedulerEmail, const std::string &notes,
                      const std::string &targetCorrelator) {
    of << "*=========================================================================================================\n";
    of << "$EXPER;\n";
    of << "*=========================================================================================================\n";
    of << "    def " << expName << eol;
    of << "        exper_name = " << boost::replace_all_copy(expName," ","_") << eol;
    of << "        exper_description = " << boost::replace_all_copy(expDescription," ","_") << eol;
    auto st = TimeSystem::startTime;
    of << "        exper_nominal_start = " << TimeSystem::ptime2string_doy_units(st) << eol;
    auto et = TimeSystem::endTime;
    of << "        exper_nominal_stop = " << TimeSystem::ptime2string_doy_units(et) << eol;

    if(!piName.empty()){
        of << "        PI_name = " << boost::replace_all_copy(piName," ","_") << eol;
    }
    if(!piEmail.empty()){
        of << "        PI_email = " << piEmail << eol;
    }

    if(!contactName.empty()){
        of << "        contact_name = " << boost::replace_all_copy(contactName," ","_") << eol;
    }
    if(!contactEmail.empty()){
        of << "        contact_email = " << contactEmail << eol;
    }

    if(!schedulerName.empty()){
        of << "        scheduler_name = " << boost::replace_all_copy(schedulerName," ","_") << eol;
    }
    if(!schedulerEmail.empty()){
        of << "        scheduler_email = " << schedulerEmail << eol;
    }
    if(!notes.empty()){
        of << "*       notes = " << notes << eol;
    }

    of << "        target_correlator = " << targetCorrelator << eol;
    of << "    enddef;\n";
}

void Vex::station_block(const std::vector<Station> &stations, const SkdCatalogReader &skdCatalogReader) {
    of << "*=========================================================================================================\n";
    of << "$STATION;\n";
    of << "*=========================================================================================================\n";
    const auto & tlc = skdCatalogReader.getTwoLetterCode();
    const map<string, vector<string> > & cat = skdCatalogReader.getEquipCatalog();
    const map<string, vector<string> > & acat = skdCatalogReader.getAntennaCatalog();

    for(const auto &any: stations){
        const string &name = any.getName();

        vector<string> tmp = acat.at(name);
        string id_EQ = boost::algorithm::to_upper_copy(tmp.at(14)) + "|" + name;

        const vector<string> &eq = cat.at(id_EQ);
        const string & recorder = eq.at(eq.size()-1);
        const string & rack = eq.at(eq.size()-2);
        const string & id = tmp.at(14);
        const string & id_name = skdCatalogReader.getTwoLetterCode().at(name) + "_" + tmp.at(14);


        of << "    def " << tlc.at(name) << eol;
        of << "        ref $SITE = " << name << eol;
        of << "        ref $ANTENNA = " << name << eol;
        of << "        ref $DAS = " << recorder << "_recorder" << eol;
        of << "        ref $DAS = " << rack << "_DDC_rack" << eol;
        of << "        ref $DAS = " << id_name << eol;
        of << "        ref $PHASE_CAL_DETECT = " << "STANDARD" << eol;
        of << "    enddef;\n";
    }
}

void Vex::sites_block(const std::vector<Station> &stations, const SkdCatalogReader &skdCatalogReader){
    of << "*=========================================================================================================\n";
    of << "$SITE;\n";
    of << "*=========================================================================================================\n";
    const auto & tlc = skdCatalogReader.getTwoLetterCode();
    for(const auto &any: stations){
        const string &name = any.getName();
        of << "    def " << name << eol;
        of << "        site_type = fixed;\n";
        of << "        site_name = " << name << eol;
        of << "        site_ID = " << tlc.at(name) << eol;
        of << boost::format("        site_position = %12.3f m : %12.3f m : %12.3f m;\n") % any.getPosition().getX() % any.getPosition().getY() % any.getPosition().getZ();
        of << "        site_position_ref = sked_position.cat;\n";
        of << "        occupation_code = " << skdCatalogReader.getPositionCatalog().at(boost::algorithm::to_upper_copy(tlc.at(name))).at(5) << eol;
        if(any.hasHorizonMask()){
            any.getMask().vexOutput();
        }

        of << "    enddef;\n";
    }
}

void Vex::antenna_block(const std::vector<Station> &stations) {
    of << "*=========================================================================================================\n";
    of << "$ANTENNA;\n";
    of << "*=========================================================================================================\n";
    for(const auto &any: stations){
        of << "    def " << any.getName() << eol;
        of << "*       antenna_name = " << any.getName() << eol;
        of << "        antenna_diam = " << any.getAntenna().getDiam() << " m" << eol;

        auto motions = any.getCableWrap().getMotions();
        const string &motion1 = motions.first;
        const string &motion2 = motions.second;

        of << "        axis_type = "<< motion1 << " : " << motion2 <<";\n";


        of << "        axis_offset = " << any.getAntenna().getOffset() << " m" << eol;
        of << boost::format("        antenna_motion = %3s: %3.0f deg/min: %3d sec;\n") % motion1 % (any.getAntenna().getRate1()*rad2deg*60) % (any.getAntenna().getCon1());
        of << boost::format("        antenna_motion = %3s: %3.0f deg/min: %3d sec;\n") % motion2 % (any.getAntenna().getRate2()*rad2deg*60) % (any.getAntenna().getCon2());

        of << any.getCableWrap().vexPointingSectors();

        of << "    enddef;\n";
    }
}

void Vex::das_block(const std::vector<Station> &stations, const SkdCatalogReader &skdCatalogReader) {
    of << "*=========================================================================================================\n";
    of << "$DAS" << eol;
    of << "*=========================================================================================================\n";
    const map<string, vector<string> > & cat = skdCatalogReader.getEquipCatalog();
    const map<string, vector<string> > & acat = skdCatalogReader.getAntennaCatalog();

    vector<string> racks;
    vector<string> recorders;
    vector<string> ids;
    vector<string> idNames;

    for(const auto &any: stations){
        vector<string> tmp = acat.at(any.getName());
        string id_EQ = boost::algorithm::to_upper_copy(tmp.at(14)) + "|" + any.getName();

        const vector<string> &eq = cat.at(id_EQ);
        const string & recorder = eq.at(eq.size()-2);
        const string & rack = eq.at(eq.size()-1);
        const string & id = tmp.at(14);
        const string & id_name = skdCatalogReader.getTwoLetterCode().at(any.getName()) + "_" + tmp.at(14);

        if(find(racks.begin(),racks.end(),rack) == racks.end()){
            racks.push_back(rack);
        }

        if(find(recorders.begin(),recorders.end(),recorder) == recorders.end()){
            recorders.push_back(recorder);
        }

        ids.push_back(id);
        idNames.push_back(id_name);
    }

    for (const auto &rack : racks) {
        of << "    def " << rack << "_rack" << eol;
        of << "        electronics_rack_type = " << rack << eol;
        of << "    enddef;\n";
    }

    for (const auto &recorder : recorders) {
        of << "    def " << recorder << "_DDC_recorder" << eol;
        of << "        record_transport_type = " << recorder <<  eol;
        of << "    enddef;\n";
    }
    for(unsigned long i=0; i<ids.size(); ++i){
        of << "    def " << idNames.at(i) << eol;
        of << "        recording_system_ID = " << ids.at(i) << eol;
        of << "    enddef;\n";
    }
}

void Vex::source_block(const std::vector<Source> &sources) {
    of << "*=========================================================================================================\n";
    of << "$SOURCE;\n";
    of << "*=========================================================================================================\n";
    for(const auto&any:sources){
        if(any.getNTotalScans()==0){
            continue;
        }
        of << "    def " << any.getName() << eol;
        of << "        source_type = star" << eol;
        of << "        source_name = " << any.getName() << eol;
        if(!any.hasAlternativeName()){
            of << "        IAU_name = " << any.getAlternativeName() << eol;
        }else{
            of << "        IAU_name = " << any.getName() << eol;
        }
        of << "        ra = " << any.getRaString() << eol;
        of << "        dec = " << any.getDeString() << eol;
        of << "        ref_coord_frame = J2000" << eol;
        of << "        ra_rate = 0 asec/yr" << eol;
        of << "        dec_rate = 0 asec/yr" << eol;
        of << "    enddef;\n";
    }
}

void Vex::phase_cal_detect_block() {
    of << "*=========================================================================================================\n";
    of << "$PHASE_CAL_DETECT;\n";
    of << "*=========================================================================================================\n";
    of << "* WARNING: This block is hard coded!\n";
    of << "    def Standard;\n";
    of << "        phase_cal_detect = &U_cal : 1;\n";
    of << "    enddef;\n";
}

void Vex::sched_block(const std::vector<Scan> &scans, const std::vector<Station> &stations,
                      const std::vector<Source> &sources, const SkdCatalogReader &skdCatalogReader) {
    of << "*=========================================================================================================\n";
    of << "$SCHED;\n";
    of << "*=========================================================================================================\n";
    const auto & tlc = skdCatalogReader.getTwoLetterCode();
    vector<string>scanIds;
    for(const auto &scan:scans) {
        unsigned long nsta = scan.getNSta();
        int srcid = scan.getSourceId();
        boost::posix_time::ptime scanStart = TimeSystem::internalTime2PosixTime(scan.getPointingVector(0).getTime());
        int doy = scanStart.date().day_of_year();
        int hour = scanStart.time_of_day().hours();
        int min = scanStart.time_of_day().minutes();
        string scanId = (boost::format("%03d-%02d%02d") % doy % hour % min).str();
        scanIds.push_back(scanId);
    }

    unordered_map<std::string, char> suffix;
    for(int i=0; i<scans.size(); ++i){
        const Scan &scan = scans[i];
        string scanId = scanIds[i];
        long count = std::count(scanIds.begin(), scanIds.end(), scanId);
        if(count>1){
            char suf;
            if(suffix.find(scanId) == suffix.end()){
                suf = 'a';
                suffix[scanId] = 'a';
            }else{
                suf = ++suffix[scanId];
            }
            scanId += suf;
        }

        unsigned long nsta = scan.getNSta();
        int srcid = scan.getSourceId();
        boost::posix_time::ptime scanStart = TimeSystem::internalTime2PosixTime(scan.getPointingVector(0).getTime());
        of << "    scan " << scanId << eol;
        of << "        start = " << TimeSystem::ptime2string_doy_units(scanStart) << eol;
        of << "        mode = GEOSX.SX" << eol;
        of << "        source = " << sources.at(static_cast<unsigned long>(srcid)).getName() << eol;
        for(int j = 0; j<nsta; ++j){
            const PointingVector &pv = scan.getPointingVector(j);
            int staid = pv.getStaid();
            double az = pv.getAz();
            const Station &thisStation = stations.at(static_cast<unsigned long>(staid));
            const string &thisTlc = tlc.at(thisStation.getName());
            string cwvex;
            const string &cwskd = thisStation.getCableWrap().cableWrapFlag(pv);
            if(cwskd == "-"){
                cwvex = "&n";
            }else if(cwskd == "C"){
                cwvex = "&cw";
            }else if (cwskd == "W"){
                cwvex = "&ccw";
            }
            int duration = scan.getTimes().getScanTime(j);
            of << boost::format("        station = %2s : %4d sec : %4d sec : 0 ft : 1A : %4s : 1;\n") % thisTlc % 0 % duration % cwvex;
        }
        of << "    endscan;\n";
    }
}

void Vex::mode_block(const std::vector<Station>& stations, const SkdCatalogReader &skdCatalogReader) {
    of << "*=========================================================================================================\n";
    of << "$MODE;\n";
    of << "*=========================================================================================================\n";
    const auto & tlc = skdCatalogReader.getTwoLetterCode();

    of << "    def " << skdCatalogReader.getFreqName() << eol;

    of << "        ref $FREQ = " << skdCatalogReader.getFreqName();
    for(const auto &any:tlc){
        of << " : " << any.second;
    }
    of << eol;

    for(const auto &any:skdCatalogReader.getLoifId2loifInfo()) {
        string name = any.first;
        of << "        ref $BBC = " << name;
        for(const auto &any2: skdCatalogReader.getStaName2loifId()){
            if(any2.second == name){
                of << " : " << tlc.at(any2.first);
            }
        }
        of << eol;
    }

    for(const auto &any:skdCatalogReader.getLoifId2loifInfo()) {
        string name = any.first;
        of << "        ref $IF = " << name;
        for(const auto &any2: skdCatalogReader.getStaName2loifId()){
            if(any2.second == name){
                of << " : " << tlc.at(any2.first);
            }
        }
        of << eol;
    }

    for(const auto &any:skdCatalogReader.getTracksIds()) {
        const string &name = any;
        of << "        ref $TRACKS = " << name;
        for(const auto &any2: skdCatalogReader.getStaName2tracksMap()){
            if(any2.second == name){
                of << " : " << tlc.at(any2.first);
            }
        }
        of << eol;
    }

    of << "        ref $PASS_ORDER = passOrder";
    for(const auto &any:tlc){
        of << " : " << any.second;
    }
    of << eol;

    of << "        ref $ROLL = NO_ROLL";
    for(const auto &any:tlc){
        of << " : " << any.second;
    }
    of << eol;

    of << "        ref $PHASE_CAL_DETECT = Standard";
    for(const auto &any:tlc){
        of << " : " << any.second;
    }
    of << eol;


    const map<string, vector<string> > & cat = skdCatalogReader.getEquipCatalog();
    const map<string, vector<string> > & acat = skdCatalogReader.getAntennaCatalog();
    map<string,vector<string> > recorders;

    for(const auto &any: stations) {
        const string &name = any.getName();
        vector<string> tmp = acat.at(name);
        string id_EQ = boost::algorithm::to_upper_copy(tmp.at(14)) + "|" + name;
        const vector<string> &eq = cat.at(id_EQ);
        string recorder = eq.at(eq.size() - 2);
        if(recorder == "Mark5b" || recorder == "K5"){
            recorder = "Mark5b";
        }else{
            recorder = "Mark4";
        }
        recorders[recorder].push_back(tlc.at(name));
    }

    for(const auto &any:recorders){
        of << "        ref $TRACKS = " << any.first << "_format";
        for(const auto &any2:any.second){
             of << " : " << any2;
        }
    }
    of << eol;


    of << "    enddef;\n";
}

void Vex::freq_block(const SkdCatalogReader &skdCatalogReader) {
    of << "*=========================================================================================================\n";
    of << "$FREQ;\n";
    of << "*=========================================================================================================\n";
    if(!ObservationMode::manual) {
        of << "    def " << skdCatalogReader.getFreqName() << eol;
        const auto &channelNumber2band = skdCatalogReader.getChannelNumber2band();
        const auto &channelNumber2Bbc = skdCatalogReader.getChannelNumber2BBC();
        const auto &channelNumber2skyFreq = skdCatalogReader.getChannelNumber2skyFreq();

        int lastBbcNr = -1;
        for (const auto &any:channelNr2Bbc_) {
            int chNr = any.first;
            int bbcNr = any.second;
            string chStr = (boost::format("&CH%02d") % chNr).str();
            string bbcStr = (boost::format("&BBC%02d") % bbcNr).str();
            string ul = "U";
            if (bbcNr == lastBbcNr) {
                ul = "L";
            }
            of << boost::format("        chan_def = &%1s : %8s MHz : %1s : %6.3f MHz : %5s : %6s : &U_cal;\n") %
                  channelNumber2band.at(bbcNr)
                  % channelNumber2skyFreq.at(bbcNr) % ul % (skdCatalogReader.getBandWidth()) % chStr % bbcStr;
            lastBbcNr = bbcNr;
        }
        of << boost::format("        sample_rate = %.1f Ms/sec;\n") % (skdCatalogReader.getSampleRate());
        of << "    enddef;\n";
    }else{
        of << "manual observation mode used!";
    }
}

void Vex::bbc_block(const SkdCatalogReader &skdCatalogReader) {
    of << "*=========================================================================================================\n";
    of << "$BBC;\n";
    of << "*=========================================================================================================\n";
    if(!ObservationMode::manual) {
        const auto &loifId2loifInfo = skdCatalogReader.getLoifId2loifInfo();
        for(const auto &any:loifId2loifInfo){
            string name = any.first;
            vector<string> ifs = any.second;
            of << "    def " << name << eol;

            for(int i=0; i<ifs.size(); ++i) {
                const string & anyIf = ifs.at(i);
                vector<string> splitVector;
                boost::split(splitVector, anyIf, boost::is_space(), boost::token_compress_on);
                string if_id_link = "&IF_" + splitVector.at(2);
                string bbc_assign_id = (boost::format("&BBC%02d") %(i+1)).str();
                of << boost::format("        BBC_assign = %6s : %02d : %6s;\n") %bbc_assign_id %(i+1) %if_id_link;

            }
            of << "    enddef;\n";
        }
    }else{
        of << "manual observation mode used!";
    }

}

void Vex::if_block(const SkdCatalogReader &skdCatalogReader) {
    of << "*=========================================================================================================\n";
    of << "$IF;\n";
    of << "*=========================================================================================================\n";
    of << "* WARNING: Polarization, Phase-cal frequency interval and Phase-cal frequency is hard coded!\n";
    if(!ObservationMode::manual) {
        const auto &loifId2loifInfo = skdCatalogReader.getLoifId2loifInfo();

        for (const auto &any:loifId2loifInfo) {
            string name = any.first;
            vector<string> ifs = any.second;
            of << "    def " << name << eol;
            string lastId;
            for (const auto &anyIf : ifs) {
                vector<string> splitVector;
                boost::split(splitVector, anyIf, boost::is_space(), boost::token_compress_on);
                if (lastId == splitVector.at(2)) {
                    continue;
                }
                string if_id_link = "&IF_" + splitVector.at(2);
                of << boost::format("        if_def = %6s : %2s : R : %7s MHz : %1s : 1 MHz : 0 Hz;\n") %
                      if_id_link % splitVector.at(2) % splitVector.at(4) % splitVector.at(5);
                lastId = splitVector.at(2);
            }
            of << "    enddef;\n";
        }
    }else{
        of << "manual observation mode used!";
    }
}

void Vex::tracks_block(const std::vector<Station> &stations, const SkdCatalogReader &skdCatalogReader) {
    of << "*=========================================================================================================\n";
    of << "$TRACKS;\n";
    of << "*=========================================================================================================\n";
    if(!ObservationMode::manual) {
        const auto & tracksIds = skdCatalogReader.getTracksIds();
        const auto & tracksId2fanout = skdCatalogReader.getTracksId2fanoutMap();
        const auto & staName2tracks = skdCatalogReader.getStaName2tracksMap();
        const auto & channelNumber2tracksMap = skdCatalogReader.getTracksId2channelNumber2tracksMap();
        for(const auto &tracksId:tracksIds){
            int chn = 1;
            of << "    def " << tracksId << eol;
            int bbcNr = 1;
            if(tracksId2fanout.at(tracksId) == 1){
                // 1:1 fanout
                for(const auto &any:channelNumber2tracksMap.at(tracksId)){

                    const string &t = any.second;
                    string tracks = t.substr(2,t.size()-3);
                    vector<string> splitVector;
                    boost::split(splitVector, tracks, boost::is_any_of(","), boost::token_compress_off);
                    if(splitVector.size()<=2){
                        for(const auto &ch: splitVector){
                            // 1 bit
                            string chStr = (boost::format("&CH%02d") % chn).str();
                            auto nr = boost::lexical_cast<int>(ch);
                            of << boost::format("        fanout_def = A : %5s : sign : 1 : %02d;\n") % chStr %(nr+3);
                            channelNr2Bbc_[chn] = bbcNr;
                            ++ chn;
                        }
                    }else if(splitVector.size()>2){
                        for(int i=0; i<splitVector.size()/2; ++i){
                            // 2 bit
                            string chStr = (boost::format("&CH%02d") % chn).str();
                            auto nr = boost::lexical_cast<int>(splitVector.at(i));
                            of << boost::format("        fanout_def = A : %5s : sign : 1 : %02d;\n") % chStr %(nr+3);
                            nr = boost::lexical_cast<int>(splitVector.at(i+2));
                            of << boost::format("        fanout_def = A : %5s : mag  : 1 : %02d;\n") % chStr %(nr+3);
                            channelNr2Bbc_[chn] = bbcNr;
                            ++ chn;
                        }
                    }
                    bbcNr++;
                }
            }else if(tracksId2fanout.at(tracksId) == 2){
                // 1:2 fanout
                for(const auto &any:channelNumber2tracksMap.at(tracksId)){
                    const string &t = any.second;
                    unsigned long idx1 = t.find('(');
                    unsigned long idx2 = t.find(')');
                    string tracks = t.substr(idx1+1,idx2-idx1-1);
                    vector<string> splitVector;
                    boost::split(splitVector, tracks, boost::is_any_of(","), boost::token_compress_off);
                    if(splitVector.size()<=2){
                        // 1 bit
                        for(const auto &ch: splitVector){
                            // 1 bit
                            string chStr = (boost::format("&CH%02d") % chn).str();
                            auto nr = boost::lexical_cast<int>(ch);
                            of << boost::format("        fanout_def = A : %5s : sign : 1 : %02d : %02d;\n") % chStr %(nr+3) %(nr+5);
                            channelNr2Bbc_[chn] = bbcNr;
                            ++ chn;
                        }
                    }else if(splitVector.size()>2){
                        // 2 bits
                        for(int i=0; i<splitVector.size()/2; ++i){
                            // 2 bit
                            string chStr = (boost::format("&CH%02d") % chn).str();
                            auto nr = boost::lexical_cast<int>(splitVector.at(i));
                            of << boost::format("        fanout_def = A : %5s : sign : 1 : %02d : %02d;\n") % chStr %(nr+3) %(nr+5);
                            nr = boost::lexical_cast<int>(splitVector.at(i+2));
                            of << boost::format("        fanout_def = A : %5s : mag  : 1 : %02d : %02d;\n") % chStr %(nr+3) %(nr+5);
                            channelNr2Bbc_[chn] = bbcNr;
                            ++ chn;
                        }
                    }
                    bbcNr++;
                }
            }
            of << "    enddef;\n";
        }


        const map<string, vector<string> > & cat = skdCatalogReader.getEquipCatalog();
        const map<string, vector<string> > & acat = skdCatalogReader.getAntennaCatalog();
        vector<string> recorders;

        for(const auto &any: stations) {
            const string &name = any.getName();

            vector<string> tmp = acat.at(name);
            string id_EQ = boost::algorithm::to_upper_copy(tmp.at(14)) + "|" + name;

            const vector<string> &eq = cat.at(id_EQ);
            string recorder = eq.at(eq.size() - 2);
            if(recorder == "Mark5b" || recorder == "K5"){
                recorder = "Mark5b";
            }else{
                recorder = "Mark4";
            }

            if(find(recorders.begin(),recorders.end(),recorder) == recorders.end()){
                recorders.push_back(recorder);
                of << "    def " << recorder << "_format" << eol;
                of << "        track_frame_format = " << recorder << eol;
                of << "    enddef;\n";
            }
        }
    }else{
        of << "manual observation mode used!";
    }

}

void Vex::head_pos_block() {

}

void Vex::pass_order_block() {
    of << "*=========================================================================================================\n";
    of << "$PASS_ORDER;\n";
    of << "*=========================================================================================================\n";
    of << "* WARNING: This block is hard coded!\n";
    of << "    def passOrder;\n";
    of << "        pass_order =   1A :   2A :   3A :   4A :   5A :   6A :   7A :   8A :   9A :  10A :  11A :  12A :  13A :  14A;\n";
    of << "    enddef;\n";
}

void Vex::roll_block() {
    of << "*=========================================================================================================\n";
    of << "$ROLL;\n";
    of << "*=========================================================================================================\n";
    of << "* WARNING: This block is hard coded!\n";
    of << "    def NO_ROLL;\n";
    of << "        roll = off;\n";
    of << "    enddef;\n";
}


