//
// Created by mschartn on 07.12.17.
//

#include "Vex.h"

using namespace std;
using namespace VieVS;

VieVS::Vex::Vex() = default;

Vex::Vex(const string &file){
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

    exper_block(xml.get("master.output.experimentName","schedule"),
                xml.get("master.output.experimentDescription","schedule"),
                xml.get("master.created.name","UNKNOWN"),
                xml.get("master.created.email","UNKNOWN"),
                xml.get("master.output.correlator","UNKNOWN"));

    station_block(stations, skdCatalogReader);
    sites_block(stations, skdCatalogReader);
    antenna_block(stations);
    das_block(stations, skdCatalogReader);

    source_block(sources);

    sched_block(scans, stations, sources, skdCatalogReader);

}

void Vex::global_block(const std::string &expName) {
    of << "*=========================================================================================================\n";
    of << "$GLOBAL;\n";
    of << "*=========================================================================================================\n";
    of << "    ref $EXPER = " << expName << eol;
    of << "    ref $SCHEDULING_PARAMS = PARAMETERS_XML;\n";
}

void Vex::exper_block(const std::string &expName, const std::string &expDescription, const std::string &schedulerName,
                      const std::string &schedulerEmail, const std::string &targetCorrelator) {
    of << "*=========================================================================================================\n";
    of << "$EXPER;\n";
    of << "*=========================================================================================================\n";
    of << "    def " << expName << eol;
    of << "        exper_name = " << expName << eol;
    of << "        exper_description = " << expDescription << eol;
    of << "        scheduler_name = " << schedulerName << eol;
    of << "        scheduler_email = " << schedulerEmail << eol;
    of << "        target_correlator = " << targetCorrelator << eol;
    auto st = TimeSystem::startTime;
    of << "        exper_nominal_start = " << TimeSystem::ptime2string_doy(st) << eol;
    auto et = TimeSystem::endTime;
    of << "        exper_nominal_stop = " << TimeSystem::ptime2string_doy(et) << eol;
    of << "    enddef;\n";
}

void Vex::station_block(const std::vector<Station> &stations, const SkdCatalogReader &skdCatalogReader) {
    of << "*=========================================================================================================\n";
    of << "$Station;\n";
    of << "*=========================================================================================================\n";
    const auto & tlc = skdCatalogReader.getTwoLetterCode();
    const map<string, vector<string> > & cat = skdCatalogReader.getEquipCatalog();
    const map<string, vector<string> > & acat = skdCatalogReader.getAntennaCatalog();

    for(const auto &any: stations){
        const string &name = any.getName();

        vector<string> tmp = acat.at(name);
        string id_EQ = boost::algorithm::to_upper_copy(tmp.at(14)) + "|" + name;

        const vector<string> &eq = cat.at(id_EQ);
        const string & rack = eq.at(eq.size()-1);
        const string & recorder = eq.at(eq.size()-2);
        const string & id = tmp.at(14);
        const string & id_name = skdCatalogReader.getTwoLetterCode().at(name) + "_" + tmp.at(14);


        of << "    def " << tlc.at(name) << eol;
        of << "        ref $SITE = " << name << eol;
        of << "        ref $ANTENNA = " << name << eol;
        of << "        ref $DAS = " << rack << "_rack" << eol;
        of << "        ref $DAS = " << recorder << "_recorder" << eol;
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
        of << boost::format("    site_position = %12.3f m : %12.3f m : %12.3f m;\n") % any.getPosition().getX() % any.getPosition().getY() % any.getPosition().getZ();
        of << "        site_position_ref = sked_position.cat;\n";
        of << "        occupation_code = " << skdCatalogReader.getPositionCatalog().at(boost::algorithm::to_upper_copy(tlc.at(name))).at(5) << eol;
        switch (any.getMask().getType()){
            case HorizonMask::Category::step:{
                const auto &az = any.getMask().getAzimuth();
                of << "        horizon_map_az = ";
                for(int i=0; i<az.size(); ++i){
                    of << az.at(static_cast<unsigned int>(i));
                    if(i==0){
                        of << " deg";
                    }
                    of << " : ";
                }
                of << eol;

                of << "        horizon_map_el = ";
                const auto &el = any.getMask().getElevation();
                for(int i=0; i<el.size(); ++i){
                    of << el.at(static_cast<unsigned int>(i));
                    if(i==0){
                        of << " deg";
                    }
                    of << " : ";
                }
                of << eol;

                break;
            }
            case HorizonMask::Category::line:{break;}
            case HorizonMask::Category::none:{break;}
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
        of << "        antenna_name = " << any.getName() << eol;
        of << "        antenna_diam = " << any.getAntenna().getDiam() << "m" << eol;
        string motion1, motion2;
        switch(any.getAntenna().getAxisType()){
            case Antenna::AxisType::AZEL:{
                of << "        axis_type = az : el;\n";
                motion1 = "az";
                motion2 = "el";
                break;
            }
            case Antenna::AxisType::HADC:{
                of << "        axis_type = ha : dec;\n";
                motion1 = "ha";
                motion2 = "dec";
                break;
            }
            case Antenna::AxisType::XYEW:{
                of << "        axis_type = x : yew;\n";
                motion1 = "s";
                motion2 = "yew";
                break;
            }
            case Antenna::AxisType::XYNS:{
                of << "        axis_type = x : yns;\n";
                motion1 = "x";
                motion2 = "yns";
                break;
            }
            case Antenna::AxisType::RICH:{break;}
            case Antenna::AxisType::SEST:{break;}
            case Antenna::AxisType::ALGO:{break;}
            case Antenna::AxisType::undefined:{break;}
        }
        of << "        axis_offset = " << any.getAntenna().getOffset() << eol;
        of << boost::format("        antenna_motion = %3s: %3.0f deg/min: %3d sec;\n") % motion1 % (any.getAntenna().getRate1()*rad2deg*60) % (any.getAntenna().getCon1());
        of << boost::format("        antenna_motion = %3s: %3.0f deg/min: %3d sec;\n") % motion2 % (any.getAntenna().getRate2()*rad2deg*60) % (any.getAntenna().getCon2());
        if(any.getAntenna().getAxisType() == Antenna::AxisType::AZEL){
            double azwl = any.getCableWrap().getWLow()*rad2deg;
            double azwh = any.getCableWrap().getWUp()*rad2deg;
            double aznl = any.getCableWrap().getNLow()*rad2deg;
            double aznh = any.getCableWrap().getWUp()*rad2deg;
            double azcl = any.getCableWrap().getCLow()*rad2deg;
            double azch = any.getCableWrap().getWUp()*rad2deg;
            double ell  = any.getCableWrap().getCUp()*rad2deg;
            double elh  = any.getCableWrap().getAxis2Up()*rad2deg;
            of << boost::format("        pointing_sector = &ccw   : %3s : %4.0f deg : %4.0f deg : %3s : %4.0f deg : %4.0f deg ;\n") % motion1 % azwl % azwh % motion2 % ell % elh;
            of << boost::format("        pointing_sector = &n     : %3s : %4.0f deg : %4.0f deg : %3s : %4.0f deg : %4.0f deg ;\n") % motion1 % aznl % aznh % motion2 % ell % elh;
            of << boost::format("        pointing_sector = &cw    : %3s : %4.0f deg : %4.0f deg : %3s : %4.0f deg : %4.0f deg ;\n") % motion1 % azcl % azch % motion2 % ell % elh;
        }else{
            double hal  = any.getCableWrap().getNLow()*rad2deg;
            double hah  = any.getCableWrap().getNUp()*rad2deg;
            double dcl  = any.getCableWrap().getAxis2Low()*rad2deg;
            double dch  = any.getCableWrap().getAxis2Up()*rad2deg;
            of << boost::format("        pointing_sector = &ccw   : %3s : %4.0f deg : %4.0f deg : %3s : %4.0f deg : %4.0f deg ;\n") % motion1 % hal % hah % motion2 % dcl % dch;
        }
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
        const string &rack = eq.at(eq.size()-1);
        const string & recorder = eq.at(eq.size()-2);
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
        of << "    enddef" << eol;
    }

    for (const auto &recorder : recorders) {
        of << "    def " << recorder << "_recorder" << eol;
        of << "        record_transport_type = " << recorder <<  eol;
        of << "    enddef" << eol;
    }
    for(unsigned long i=0; i<ids.size(); ++i){
        of << "    def " << idNames.at(i) << eol;
        of << "        recording_system_ID = " << ids.at(i) << eol;
        of << "    enddef" << eol;
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
        if(!any.getAlternativeName().empty()){
            of << "        IAU_name = " << any.getAlternativeName() << eol;
        }else{
            of << "        IAU_name = " << any.getName() << eol;
        }
        of << "        ra = " << any.getRaString() << eol;
        of << "        dec = " << any.getDeString() << eol;
        of << "        ref_coord_frame = J2000" << eol;
        of << "        ra_rate = 0" << eol;
        of << "        dec_rate = 0" << eol;
        of << "    enddef;\n";
    }
}

void Vex::phase_cal_detect_block() {
    of << "$PHASE_CAL_DETECT;\n";
    of << "* WARNING: This block is hard coded!\n";
    of << "    def Standard;\n";
//    of << "    phase_cal_detect = &U_cal : 1\n";
    of << "    enddef;\n";
}

void Vex::sched_block(const std::vector<Scan> &scans, const std::vector<Station> &stations,
                      const std::vector<Source> &sources, const SkdCatalogReader &skdCatalogReader) {
    of << "*=========================================================================================================\n";
    of << "$SCHED;\n";
    of << "*=========================================================================================================\n";
    boost::posix_time::ptime sessionStart = TimeSystem::startTime;
    auto tlc = skdCatalogReader.getTwoLetterCode();
    vector<string>scanIds;
    for(const auto &scan:scans) {
        unsigned long nsta = scan.getNSta();
        int srcid = scan.getSourceId();
        boost::posix_time::ptime scanStart =
                sessionStart + boost::posix_time::seconds(static_cast<long>(scan.getPointingVector(0).getTime()));
        int doy = scanStart.date().day_of_year();
        int hour = scanStart.time_of_day().hours();
        int min = scanStart.time_of_day().minutes();
        string scanId = (boost::format("%03d-%02d%02d") % doy % hour % min).str();
        scanIds.push_back(scanId);
    }

    unordered_map<string,char> suffix;
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
        boost::posix_time::ptime scanStart =
                sessionStart + boost::posix_time::seconds(static_cast<long>(scan.getPointingVector(0).getTime()));
        of << "    scan " << scanId << eol;
        of << "        start = " << TimeSystem::ptime2string(scanStart) << eol;
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
            int duration = scan.getTimes().getEndOfScanTime(j)-scan.getTimes().getEndOfCalibrationTime(j);
            of << boost::format("        station = %2s : %4d sec : %4d sec : 0ft : 1A : %4s : 1;\n") % thisTlc % 0 % duration % cwvex;
        }
        of << "    endscan" << eol;
    }
}

void Vex::mode_block() {

}

void Vex::freq_block() {

}

void Vex::bbc_block() {

}

void Vex::if_block() {

}

void Vex::tracks_block() {

}

void Vex::head_pos_block() {

}

void Vex::pass_order_block() {

}

void Vex::roll_block() {

}


