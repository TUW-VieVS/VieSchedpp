/*
 *  VieSched++ Very Long Baseline Interferometry (VLBI) Scheduling Software
 *  Copyright (C) 2018  Matthias Schartner
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Vex.h"


using namespace std;
using namespace VieVS;
unsigned long Vex::nextId = 0;


Vex::Vex( const string &file ) : VieVS_Object( nextId++ ) {
    of = ofstream( file );
    of << "VEX_rev = 1.5;\n";
}


void Vex::writeVex( const Network &network, const std::vector<Source> &sources, const std::vector<Scan> &scans,
                    const std::shared_ptr<const ObservingMode> &obsModes, const boost::property_tree::ptree &xml ) {
    global_block( xml.get( "VieSchedpp.general.experimentName", "schedule" ) );

    exper_block( xml );

    station_block( network.getStations() );
    mode_block( obsModes );
    sched_block( scans, network, sources, obsModes );

    sites_block( network.getStations() );
    antenna_block( network.getStations() );
    das_block( network.getStations() );

    source_block( sources );

    bbc_block( obsModes );
    if_block( obsModes );
    tracks_block( obsModes );
    freq_block( obsModes );

    pass_order_block();
    roll_block();
    phase_cal_detect_block();
}


void Vex::global_block( const std::string &expName ) {
    of << "*========================================================================================================="
          "\n";
    of << "$GLOBAL;\n";
    of << "*========================================================================================================="
          "\n";
    of << "    ref $EXPER = " << expName << eol;
}


void Vex::exper_block( const boost::property_tree::ptree &xml ) {
    const string &expName = boost::trim_copy( xml.get( "VieSchedpp.general.experimentName", "dummy" ) );
    const string &expDescription =
        boost::trim_copy( xml.get( "VieSchedpp.output.experimentDescription", "no further description" ) );
    const string &piName = boost::trim_copy( xml.get( "VieSchedpp.output.piName", "" ) );
    const string &piEmail = boost::trim_copy( xml.get( "VieSchedpp.output.piEmail", "" ) );
    const string &schedulerName = boost::trim_copy( xml.get( "VieSchedpp.created.name", "" ) );
    const string &schedulerEmail = boost::trim_copy( xml.get( "VieSchedpp.created.email", "" ) );
    const string &notes = boost::trim_copy( xml.get( "VieSchedpp.output.notes", "" ) );
    const string &targetCorrelator = boost::trim_copy( xml.get( "VieSchedpp.output.correlator", "unknown" ) );
    const string &gui_version = boost::trim_copy( xml.get( "VieSchedpp.software.GUI_version", "unknown" ) );

    of << "*========================================================================================================="
          "\n";
    of << "$EXPER;\n";
    of << "*========================================================================================================="
          "\n";
    of << "    def " << expName << eol;
    of << "        exper_name = " << boost::replace_all_copy( expName, " ", "_" ) << eol;
    of << "        exper_description = " << boost::replace_all_copy( expDescription, " ", "_" ) << eol;
    auto st = TimeSystem::startTime;
    of << "        exper_nominal_start = " << TimeSystem::time2string_doy_units( st ) << eol;
    auto et = TimeSystem::endTime;
    of << "        exper_nominal_stop = " << TimeSystem::time2string_doy_units( et ) << eol;

    if ( !piName.empty() ) {
        of << "        PI_name = " << boost::replace_all_copy( piName, " ", "_" ) << eol;
    }
    if ( !piEmail.empty() ) {
        of << "        PI_email = " << piEmail << eol;
    }

    const auto &pt = xml.get_child_optional( "VieSchedpp.output.contacts" );
    if ( pt.is_initialized() ) {
        for ( const auto &any : *pt ) {
            const string &name = any.second.get( "name", "" );
            const string &email = any.second.get( "email", "" );
            const string &phone = any.second.get( "phone", "" );
            const string &affiliation = any.second.get( "affiliation", "" );
            if ( !name.empty() ) {
                string l = name;
                if ( !email.empty() ) {
                    l += ", " + email;
                }
                if ( !affiliation.empty() ) {
                    l += " (" + affiliation + ")";
                }
                if ( !phone.empty() ) {
                    l += " tel.: " + phone;
                }
                of << "*       contact = " << l << eol;
            }
        }
    }

    if ( !schedulerName.empty() ) {
        of << "        scheduler_name = " << boost::replace_all_copy( schedulerName, " ", "_" ) << eol;
    }
    if ( !schedulerEmail.empty() ) {
        of << "        scheduler_email = " << schedulerEmail << eol;
    }
    if ( !notes.empty() ) {
        of << "*       notes = \n";
        of << "*               " << boost::replace_all_copy( notes, "\\n", "\n*               " ) << eol;
    }

    of << "        target_correlator = " << targetCorrelator << eol;
    of << "*       software = VieSched++" << eol;
    string versionNr = util::version();
    of << "*       software_version = " << versionNr << eol;
    of << "*       software_gui_version = " << gui_version << eol;
    of << "    enddef;\n";
}


void Vex::station_block( const std::vector<Station> &stations ) {
    of << "*========================================================================================================="
          "\n";
    of << "$STATION;\n";
    of << "*========================================================================================================="
          "\n";

    for ( const auto &any : stations ) {
        any.toVexStationBlock( of );

        //        const string &name = any.getName();
        //
        //        vector<string> tmp = acat.at(name);
        //        string id_EQ = boost::algorithm::to_upper_copy(tmp.at(14)) + "|" + name;
        //
        //        const vector<string> &eq = cat.at(id_EQ);
        //        const string & recorder = eq.at(eq.size()-1);
        //        const string & rack = eq.at(eq.size()-2);
        //        const string & id = tmp.at(14);
        //        const string & id_name = any.getAlternativeName() + "_" + tmp.at(14);
        //
        //
        //        of << "    def " << any.getAlternativeName() << eol;
        //        of << "        ref $SITE = " << name << eol;
        //        of << "        ref $ANTENNA = " << name << eol;
        //        of << "        ref $DAS = " << recorder << "_recorder" << eol;
        //        if(rack == "DBBC"){
        //            of << "        ref $DAS = " << rack << "_DDC_rack" << eol;
        //        }else{
        //            of << "        ref $DAS = " << rack << "_rack" << eol;
        //        }
        //        of << "        ref $DAS = " << id_name << eol;
        //        of << "*        ref $PHASE_CAL_DETECT = " << "Standard" << eol;
        //        of << "    enddef;\n";
    }
}


void Vex::sites_block( const std::vector<Station> &stations ) {
    of << "*========================================================================================================="
          "\n";
    of << "$SITE;\n";
    of << "*========================================================================================================="
          "\n";
    for ( const auto &any : stations ) {
        any.toVexSiteBlock( of );

        //        const string &name = any.getName();
        //        of << "    def " << name << eol;
        //        of << "        site_type = fixed;\n";
        //        of << "        site_name = " << name << eol;
        //        of << "        site_ID = " << any.getAlternativeName() << eol;
        //        of << boost::format("        site_position = %12.3f m : %12.3f m : %12.3f m;\n") %
        //        any.getPosition().getX() % any.getPosition().getY() % any.getPosition().getZ(); of << "
        //        site_position_ref = sked_position.cat;\n"; of << "        occupation_code_ = " <<
        //        skdCatalogReader.getPositionCatalog().at(skdCatalogReader.positionKey(name)).at(5) << eol;
        //        if(any.hasHorizonMask()){
        //            any.getMask().vexOutput();
        //        }
        //        of << "    enddef;\n";
    }
}


void Vex::antenna_block( const std::vector<Station> &stations ) {
    of << "*========================================================================================================="
          "\n";
    of << "$ANTENNA;\n";
    of << "*========================================================================================================="
          "\n";
    for ( const auto &any : stations ) {
        any.toVexAntennaBlock( of );

        //        of << "    def " << any.getName() << eol;
        //        of << "*       antenna_name = " << any.getName() << eol;
        //        of << "        antenna_diam = " << any.getAntenna().getDiam() << " m" << eol;
        //
        //        auto motions = any.getCableWrap().getMotions();
        //        const string &motion1 = motions.first;
        //        const string &motion2 = motions.second;
        //
        //        of << "        axis_type = "<< motion1 << " : " << motion2 <<";\n";
        //
        //
        //        of << "        axis_offset = " << any.getAntenna().getOffset() << " m" << eol;
        //        of << boost::format("        antenna_motion = %3s: %3.0f deg/min: %3d sec;\n") % motion1 %
        //        (any.getAntenna().getRate1()*rad2deg*60) % (any.getAntenna().getCon1()); of << boost::format("
        //        antenna_motion = %3s: %3.0f deg/min: %3d sec;\n") % motion2 % (any.getAntenna().getRate2()*rad2deg*60)
        //        % (any.getAntenna().getCon2());
        //
        //        of << any.getCableWrap().vexPointingSectors();
        //
        //        of << "    enddef;\n";
    }
}


void Vex::das_block( const std::vector<Station> &stations ) {
    of << "*========================================================================================================="
          "\n";
    of << "$DAS" << eol;
    of << "*========================================================================================================="
          "\n";

    set<string> racks;
    set<string> recorders;
    vector<string> ids;
    vector<string> idNames;

    for ( const auto &any : stations ) {
        const string &recorder = any.getRecord_transport_type();
        string rack = any.getElectronics_rack_type_();
        if ( rack == "DBBC" ) {
            rack.append( "_DDC" );
        }
        const string &id = any.getRecording_system_id();
        const string &id_name = any.getAlternativeName() + "_" + id;

        racks.insert( rack );
        recorders.insert( recorder );

        ids.push_back( id );
        idNames.push_back( id_name );
    }

    for ( const auto &rack : racks ) {
        of << "    def " << rack << "_rack" << eol;
        of << "        electronics_rack_type = " << rack << eol;
        of << "    enddef;\n";
    }

    for ( const auto &recorder : recorders ) {
        of << "    def " << recorder << "_recorder" << eol;
        of << "        record_transport_type = " << recorder << eol;
        of << "    enddef;\n";
    }
    for ( unsigned long i = 0; i < ids.size(); ++i ) {
        of << "    def " << idNames.at( i ) << eol;
        of << "        recording_system_ID = " << ids.at( i ) << eol;
        of << "    enddef;\n";
    }
}


void Vex::source_block( const std::vector<Source> &sources ) {
    of << "*========================================================================================================="
          "\n";
    of << "$SOURCE;\n";
    of << "*========================================================================================================="
          "\n";
    for ( const auto &any : sources ) {
        if ( any.getNTotalScans() == 0 ) {
            continue;
        }
        of << "    def " << any.getName() << eol;
        of << "        source_type = star" << eol;
        of << "        source_name = " << any.getName() << eol;
        if ( any.hasAlternativeName() ) {
            of << "        IAU_name = " << any.getAlternativeName() << eol;
        } else {
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
    of << "*========================================================================================================="
          "\n";
    of << "$PHASE_CAL_DETECT;\n";
    of << "*========================================================================================================="
          "\n";
    of << "* WARNING: This block is hard coded!\n";
    of << "    def Standard;\n";
    of << "        phase_cal_detect = &U_cal : 1;\n";
    of << "    enddef;\n";
}


void Vex::sched_block( const std::vector<Scan> &scans, const Network &network, const std::vector<Source> &sources,
                       const std::shared_ptr<const ObservingMode> &obsModes ) {
    of << "*========================================================================================================="
          "\n";
    of << "$SCHED;\n";
    of << "*========================================================================================================="
          "\n";

    for ( int i = 0; i < scans.size(); ++i ) {
        const Scan &scan = scans[i];
        string scanId = scan.getName( i, scans );

        unsigned long nsta = scan.getNSta();
        unsigned long srcid = scan.getSourceId();
        of << "    scan " << scanId << eol;
        of << "        start = "
           << TimeSystem::time2string_doy_units( scan.getTimes().getObservingTime( Timestamp::start ) ) << eol;
        of << "        mode = " << obsModes->getMode( 0 )->getName() << eol;
        of << "        source = " << sources.at( srcid ).getName() << eol;

        const auto &times = scan.getTimes();
        unsigned int start = times.getObservingTime( Timestamp::start );

        for ( int j = 0; j < nsta; ++j ) {
            const PointingVector &pv = scan.getPointingVector( j );
            unsigned long staid = pv.getStaid();
            const Station &thisStation = network.getStation( staid );
            const string &thisTlc = thisStation.getAlternativeName();
            string cwvex;

            switch ( thisStation.getCableWrap().cableWrapFlag( pv ) ) {
                case AbstractCableWrap::CableWrapFlag::ccw:
                    cwvex = "&ccw";
                    break;
                case AbstractCableWrap::CableWrapFlag::n:
                    cwvex = "&n";
                    break;
                case AbstractCableWrap::CableWrapFlag::cw:
                    cwvex = "&cw";
                    break;
            }

            int dataGood = times.getObservingTime( j, Timestamp::start );
            int dataObs = times.getObservingTime( j, Timestamp::end );
            of << boost::format( "        station = %2s : %4d sec : %4d sec : 0 ft : 1A : %4s : 1;\n" ) % thisTlc %
                      ( dataGood - start ) % ( dataObs - start ) % cwvex;
        }

        vector<string> ignoreBaseline;
        for ( int staidx1 = 0; staidx1 < scan.getNSta(); ++staidx1 ) {
            for ( int staidx2 = staidx1 + 1; staidx2 < scan.getNSta(); ++staidx2 ) {
                unsigned long staid1 = scan.getPointingVector( staidx1 ).getStaid();
                unsigned long staid2 = scan.getPointingVector( staidx2 ).getStaid();
                bool found = false;
                for ( const auto &any : scan.getObservations() ) {
                    if ( any.containsStation( staid1 ) && any.containsStation( staid2 ) ) {
                        found = true;
                        break;
                    }
                }
                if ( !found ) {
                    ignoreBaseline.push_back( network.getBaseline( staid1, staid2 ).getName() );
                }
            }
        }

        if ( !ignoreBaseline.empty() ) {
            of << "*       ignore observation = ";
            for ( int i_ignore = 0; i_ignore < ignoreBaseline.size(); ++i_ignore ) {
                if ( i_ignore % 6 == 0 && i_ignore > 0 ) {
                    of << "\n*                            ";
                }
                of << ignoreBaseline[i_ignore];
                if ( i_ignore + 1 != ignoreBaseline.size() ) {
                    of << " : ";
                }
            }
            of << ";\n";
        }

        of << "    endscan;\n";
    }
}


void Vex::mode_block( const std::shared_ptr<const ObservingMode> &obsModes ) {
    of << "*========================================================================================================="
          "\n";
    of << "$MODE;\n";
    of << "*========================================================================================================="
          "\n";

    obsModes->toVexModeBlock( of );
}


void Vex::freq_block( const std::shared_ptr<const ObservingMode> &obsModes ) {
    of << "*========================================================================================================="
          "\n";
    of << "$FREQ;\n";
    of << "*========================================================================================================="
          "\n";
    if ( ObservingMode::type != ObservingMode::Type::simple ) {
        obsModes->toVexFreqBlock( of );
    } else {
        of << "simple observation mode used!";
    }
}


void Vex::bbc_block( const std::shared_ptr<const ObservingMode> &obsModes ) {
    of << "*========================================================================================================="
          "\n";
    of << "$BBC;\n";
    of << "*========================================================================================================="
          "\n";
    if ( ObservingMode::type != ObservingMode::Type::simple ) {
        obsModes->toVexBbcBlock( of );
    } else {
        of << "simple observation mode used!";
    }
}


void Vex::if_block( const std::shared_ptr<const ObservingMode> &obsModes ) {
    of << "*========================================================================================================="
          "\n";
    of << "$IF;\n";
    of << "*========================================================================================================="
          "\n";
    //    of << "* WARNING: Polarization, Phase-cal frequency interval and Phase-cal frequency is hard coded!\n";
    if ( ObservingMode::type != ObservingMode::Type::simple ) {
        obsModes->toVexIfBlock( of );

    } else {
        of << "simple observation mode used!";
    }
}


void Vex::tracks_block( const std::shared_ptr<const ObservingMode> &obsModes ) {
    of << "*========================================================================================================="
          "\n";
    of << "$TRACKS;\n";
    of << "*========================================================================================================="
          "\n";
    if ( ObservingMode::type != ObservingMode::Type::simple ) {
        obsModes->toVexTracksBlock( of );
    } else {
        of << "simple observation mode used!";
    }
}


void Vex::head_pos_block() {}


void Vex::pass_order_block() {
    of << "*========================================================================================================="
          "\n";
    of << "$PASS_ORDER;\n";
    of << "*========================================================================================================="
          "\n";
    of << "* WARNING: This block is hard coded!\n";
    of << "    def passOrder;\n";
    of << "        pass_order =   1A :   2A :   3A :   4A :   5A :   6A :   7A :   8A :   9A :  10A :  11A :  12A :  "
          "13A :  14A;\n";
    of << "    enddef;\n";
}


void Vex::roll_block() {
    of << "*========================================================================================================="
          "\n";
    of << "$ROLL;\n";
    of << "*========================================================================================================="
          "\n";
    of << "* WARNING: This block is hard coded!\n";
    of << "    def NO_ROLL;\n";
    of << "        roll = off;\n";
    of << "    enddef;\n";
}
