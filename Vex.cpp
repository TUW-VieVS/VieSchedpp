//
// Created by mschartn on 07.12.17.
//

#include "Vex.h"

#include <utility>

using namespace std;
using namespace VieVS;

VieVS::Vex::Vex() = default;

Vex::Vex(SkdCatalogReader skd, const string &file): skd_{std::move(skd)} {

    ofstream of(file);

    of << "VEX_rev = 1.5;\n";

}

void Vex::global_block(const std::string &expName) {
    of << "$GLOBAL;\n";
    of << "    ref $EXPER = " << expName << eol;
    of << "    ref $SCHEDULING_PARAMS = PARAMETERS_XML;\n";
}

void Vex::exper_block(const std::string &expName, const std::string &expDescription, const std::string &schedulerName,
                      const std::string &schedulerEmail, const std::string &targetCorrelator) {
    of << "$EXPER;\n";
    of << "  def " << expName << eol;
    of << "    exper_name = " << expName << eol;
    of << "    exper_description = " << expDescription << eol;
    of << "    scheduler_name = " << schedulerName << eol;
    of << "    scheduler_email = " << schedulerEmail << eol;
    of << "    target_correlator = " << targetCorrelator << eol;
    auto st = TimeSystem::startTime;
    of << "    exper_nominal_start = " << boost::format("%04d%03d%02d%02d%02d") % st.date().year() % st.date().day_of_year() %
                                          st.time_of_day().hours() % st.time_of_day().minutes() % st.time_of_day().seconds() << eol;
    auto et = TimeSystem::endTime;
    of << "    exper_nominal_stop = " << boost::format("%04d%03d%02d%02d%02d") % et.date().year() % et.date().day_of_year() %
                                          et.time_of_day().hours() % et.time_of_day().minutes() % et.time_of_day().seconds() << eol;
    of << "  enddef;\n";
}

void Vex::station_block(const std::vector<Station> &stations) {
    of << "$Station;\n";
    const auto & tlc = skd_.getTwoLetterCode();
    const map<string, vector<string> > & cat = skd_.getEquipCatalog();
    const map<string, vector<string> > & acat = skd_.getAntennaCatalog();

    for(const auto &any: stations){
        const string &name = any.getName();

        vector<string> tmp = acat.at(name);
        string id_EQ = boost::algorithm::to_upper_copy(tmp.at(14)) + "|" + name;

        const vector<string> &eq = cat.at(id_EQ);
        const string & rack = eq.at(eq.size()-1);
        const string & recorder = eq.at(eq.size()-2);
        const string & id = tmp.at(14);
        const string & id_name = skd_.getTwoLetterCode().at(name) + "_" + tmp.at(14);


        of << "  def " << tlc.at(name) << eol;
        of << "    ref $SITE = " << name << eol;
        of << "    ref $ANTENNA = " << name << eol;
        of << "    ref $DAS = " << rack << "_rack" << eol;
        of << "    ref $DAS = " << recorder << "_recorder" << eol;
        of << "    ref $DAS = " << id_name << eol;
        of << "    ref $PHASE_CAL_DETECT = " << "STANDARD" << eol;
        of << "  enddef;\n";
    }
}

void Vex::sites_block(const std::vector<Station> &stations){
    of << "$SITE;\n";
    const auto & tlc = skd_.getTwoLetterCode();
    for(const auto &any: stations){
        const string &name = any.getName();
        of << "  def " << name << eol;
        of << "    site_type = fixed;\n";
        of << "    site_name = " << name << eol;
        of << "    site_ID = " << tlc.at(name) << eol;
        of << boost::format("    site_position = %12.3f m : %12.3f m : %12.3f m;\n") % any.getPosition().getX() % any.getPosition().getY() % any.getPosition().getZ();
        of << "    site_position_ref = sked_position.cat;\n";
        of << "    occupation_code = " << skd_.getPositionCatalog().at(boost::algorithm::to_upper_copy(tlc.at(name))).at(5) << eol;
        switch (any.getMask().getType()){
            case HorizonMask::Category::step:{
                const auto &az = any.getMask().getAzimuth();
                of << "    horizon_map_az = ";
                for(int i=0; i<az.size(); ++i){
                    of << az.at(i);
                    if(i==0){
                        of << " deg";
                    }
                    of << " : ";
                }
                of << eol;

                of << "    horizon_map_el = ";
                const auto &el = any.getMask().getElevation();
                for(int i=0; i<el.size(); ++i){
                    of << el.at(i);
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
        of << "  enddef;\n";
    }
}

void Vex::antenna_block(const std::vector<Station> &stations) {
    of << "$ANTENNA;\n";
    for(const auto &any: stations){
        of << "  def " << any.getName() << eol;
        of << "    antenna_name = " << any.getName() << eol;
        of << "    antenna_diam = " << any.getAntenna().getDiam() << "m" << eol;
        string motion1, motion2;
        switch(any.getAntenna().getAxisType()){
            case Antenna::AxisType::AZEL:{
                of << "    axis_type = az : el;\n";
                motion1 = "az";
                motion2 = "el";
                break;
            }
            case Antenna::AxisType::HADC:{
                of << "    axis_type = ha : dec;\n";
                motion1 = "ha";
                motion2 = "dec";
                break;
            }
            case Antenna::AxisType::XYEW:{
                of << "    axis_type = x : yew;\n";
                motion1 = "s";
                motion2 = "yew";
                break;
            }
            case Antenna::AxisType::XYNS:{
                of << "    axis_type = x : yns;\n";
                motion1 = "x";
                motion2 = "yns";
                break;
            }
            case Antenna::AxisType::RICH:{break;}
            case Antenna::AxisType::SEST:{break;}
            case Antenna::AxisType::ALGO:{break;}
            case Antenna::AxisType::undefined:{break;}
        }
        of << "    axis_offset = " << any.getAntenna().getOffset() << eol;
        of << boost::format("    antenna_motion = %3s: %3.0f deg/min: %3d sec;\n") % motion1 % (any.getAntenna().getRate1()*rad2deg*60) % (any.getAntenna().getCon1());
        of << boost::format("    antenna_motion = %3s: %3.0f deg/min: %3d sec;\n") % motion2 % (any.getAntenna().getRate2()*rad2deg*60) % (any.getAntenna().getCon2());
        if(any.getAntenna().getAxisType() == Antenna::AxisType::AZEL){
            of << boost::format("    pointing_sector = &ccw   : %3s : %4.0f deg : 4.0f deg : %3s : %4.0f deg : %4.0f deg ;\n") %
                    motion1 % (any.getCableWrap().getWLow()*rad2deg) % (any.getCableWrap().getWUp()*rad2deg) %
                    motion2 % (any.getCableWrap().getAxis2Low()*rad2deg) % (any.getCableWrap().getAxis2Up()*rad2deg);
            of << boost::format("    pointing_sector = &n     : %3s : %4.0f deg : 4.0f deg : %3s : %4.0f deg : %4.0f deg ;\n") %
                  motion1 % (any.getCableWrap().getNLow()*rad2deg) % (any.getCableWrap().getNUp()*rad2deg) %
                  motion2 % (any.getCableWrap().getAxis2Low()*rad2deg) % (any.getCableWrap().getAxis2Up()*rad2deg);
            of << boost::format("    pointing_sector = &cw    : %3s : %4.0f deg : 4.0f deg : %3s : %4.0f deg : %4.0f deg ;\n") %
                  motion1 % (any.getCableWrap().getCLow()*rad2deg) % (any.getCableWrap().getCUp()*rad2deg) %
                  motion2 % (any.getCableWrap().getAxis2Low()*rad2deg) % (any.getCableWrap().getAxis2Up()*rad2deg);
        }else{
            of << boost::format("    pointing_sector = &ccw   : %3s : %4.0f deg : 4.0f deg : %3s : %4.0f deg : %4.0f deg ;\n") %
                  motion1 % (any.getCableWrap().getNLow()*rad2deg) % (any.getCableWrap().getNUp()*rad2deg) %
                  motion2 % (any.getCableWrap().getAxis2Low()*rad2deg) % (any.getCableWrap().getAxis2Up()*rad2deg);
        }
        of << "  enddef;\n";
    }
}

void Vex::das_block(const std::vector<Station> &stations) {
    of << "$DAS" << eol;
    const map<string, vector<string> > & cat = skd_.getEquipCatalog();
    const map<string, vector<string> > & acat = skd_.getAntennaCatalog();

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
        const string & id_name = skd_.getTwoLetterCode().at(any.getName()) + "_" + tmp.at(14);

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
        of << "  def " << rack << "_rack" << eol;
        of << "    electronics_rack_type = " << rack << eol;
        of << "  enddef" << eol;
    }

    for (const auto &recorder : recorders) {
        of << "  def " << recorder << "_recorder" << eol;
        of << "    record_transport_type = " << recorder <<  eol;
        of << "  enddef" << eol;
    }
    for(unsigned long i=0; i<ids.size(); ++i){
        of << "  def " << idNames.at(i) << eol;
        of << "    recording_system_ID = " << ids.at(i) << eol;
        of << "  enddef" << eol;
    }
}

void Vex::source_block(const std::vector<Source> &sources) {
    of << "$SOURCE;\n";
    for(const auto&any:sources){
        of << "  def " << any.getName() << eol;
        of << "    source_type = star" << eol;
        of << "    source_name = " << any.getName() << eol;
        if(!any.getAlternativeName().empty()){
            of << "    IAU_name = " << any.getAlternativeName() << eol;
        }else{
            of << "    IAU_name = " << any.getName() << eol;
        }
        of << "    ra = " << any.getRaString() << eol;
        of << "    dec = " << any.getDeString() << eol;
        of << "    ref_coord_frame = J2000" << eol;
        of << "    ra_rate = 0" << eol;
        of << "    dec_rate = 0" << eol;
        of << "  enddef;\n";
    }
}

void Vex::phase_cal_detect_block() {
    of << "$PHASE_CAL_DETECT;\n";
    of << "  def Standard;\n";
//    of << "    phase_cal_detect = &U_cal : 1\n";
    of << "  enddef;\n";
}

void Vex::sched_block(const std::vector<Scan> &scans, const std::vector<Station> &stations,
                      const std::vector<Source> &sources) {

    of << "$SCHED;\n";
    boost::posix_time::ptime sessionStart = TimeSystem::startTime;
    auto tlc = skd_.getTwoLetterCode();
    vector<string>scanIds;
    for(const auto &scan:scans) {
        unsigned long nsta = scan.getNSta();
        int srcid = scan.getSourceId();
        boost::posix_time::ptime scanStart =
                sessionStart + boost::posix_time::seconds(scan.getPointingVector(0).getTime());
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
                sessionStart + boost::posix_time::seconds(scan.getPointingVector(0).getTime());
        of << "  scan " << scanId << eol;
        of << "    start = " << TimeSystem::ptime2string(scanStart) << eol;
        of << "    mode = GEOSX.SX" << eol;
        of << "    source = " << sources.at(static_cast<unsigned long>(srcid)).getName() << eol;
        for(int i = 0; i<nsta; ++i){
            const PointingVector &pv = scan.getPointingVector(i);
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
            int duration = scan.getTimes().getEndOfScanTime(i)-scan.getTimes().getEndOfCalibrationTime(i);
            of << boost::format("    station = %2s : %4d sec : %4d sec : 0ft : 1A : %4s : 1;\n") % thisTlc % 0 % duration % cwvex;
        }
        of << "  endscan" << eol;
    }


}


