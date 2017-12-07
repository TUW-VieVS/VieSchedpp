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
    of << "  enddef;\n";
}

void Vex::station_block(const std::vector<Station> &stations) {
    of << "$Station;\n";
    const auto & tlc = skd_.getTwoLetterCode();
    for(const auto &any: stations){
        const string &name = any.getName();
        of << "  def " << tlc.at(name) << eol;
        of << "    ref $SITE = " << name << eol;
        of << "    ref $ANTENNA = " << name << eol;
        of << "    ref $DAS = " << "" << eol;
        of << "    ref $DAS = " << "" << eol;
        of << "    ref $DAS = " << "" << eol;
        of << "    ref $DAS = " << "" << eol;
        of << "    ref $DAS = " << "" << eol;
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

}

void Vex::source_block(const std::vector<Source> &stations) {

}

void Vex::phase_cal_detect_block() {
    of << "$PHASE_CAL_DETECT;\n";
    of << "  def Standard;\n";
//    of << "    phase_cal_detect = &U_cal : 1\n";
    of << "  enddef;\n";
}


