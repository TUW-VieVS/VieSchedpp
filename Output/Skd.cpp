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

#include "Skd.h"
#include "../ObservingMode/ObservingMode.h"


using namespace std;
using namespace VieVS;

unsigned long Skd::nextId = 0;


Skd::Skd( const string &file ) : VieVS_Object( nextId++ ) { of = ofstream( file ); }


void Skd::writeSkd( const Network &network, const std::vector<Source> &sources, const std::vector<Scan> &scans,
                    const SkdCatalogReader &skdCatalogReader, const boost::property_tree::ptree &xml ) {
    of << "$EXPER " << xml.get<string>( "VieSchedpp.general.experimentName" ) << endl;
    //    if(xml.get_optional<std::string>("VieSchedpp.output.piName").is_initialized()){
    //        of << "* PI name:       " << *xml.get_optional<std::string>("VieSchedpp.output.piName") << "\n";
    //    }
    //    if(xml.get_optional<std::string>("VieSchedpp.output.piEmail").is_initialized()){
    //        of << "* PI email:      " << *xml.get_optional<std::string>("VieSchedpp.output.piEmail") << "\n";
    //    }
    //    if(xml.get_optional<std::string>("VieSchedpp.output.contactName").is_initialized()){
    //        of << "* contact name:  " << *xml.get_optional<std::string>("VieSchedpp.output.contactName") << "\n";
    //    }
    //    if(xml.get_optional<std::string>("VieSchedpp.output.contactEmail").is_initialized()){
    //        of << "* contact email: " << *xml.get_optional<std::string>("VieSchedpp.output.contactEmail") << "\n";
    //    }
    //    if(xml.get_optional<std::string>("VieSchedpp.output.notes").is_initialized()){
    //        string notes = "*";
    //        notes.append(*xml.get_optional<std::string>("VieSchedpp.output.notes")).append("\n");
    //        notes = boost::replace_all_copy(notes,"\\n","\n*");
    //        of << "* notes: \n";
    //        of << notes << "*\n*";
    //    }

    skd_PARAM( network, xml, skdCatalogReader );
    skd_OP();
    skd_DOWNTIME( network );
    skd_MAJOR( network.getStations(), sources, xml, skdCatalogReader );
    skd_MINOR();
    skd_ASTROMETRIC();
    skd_BROADBAND();
    skd_CATALOG_USED( xml, skdCatalogReader );
    skd_CODES( network.getStations(), skdCatalogReader );
    skd_STATIONS( network.getStations(), skdCatalogReader );
    skd_STATWT( network.getStations() );
    skd_SOURCES( sources, skdCatalogReader );
    skd_SRCWT( sources );
    skd_SKED( network.getStations(), sources, scans, skdCatalogReader );
    skd_FLUX( sources, skdCatalogReader );
}


void Skd::skd_PARAM( const Network &network, const boost::property_tree::ptree &xml,
                     const SkdCatalogReader &skdCatalogReader ) {
    //    of << "*\n";
    //    of <<
    //    "*=========================================================================================================\n";
    of << "$PARAM\n";
    //    of <<
    //    "*=========================================================================================================\n";
    //    of << "* WARNING: some of the following parameters are not compatible with VieSched++!\n";
    //    of << "*\n";
    of << "DESCRIPTION " << xml.get<string>( "VieSchedpp.output.experimentDescription", "no description" ) << endl;
    of << "SCHEDULING_SOFTWARE VieSched++\n";

    string versionNr = util::version();
    of << "SOFTWARE_VERSION " << versionNr << "\n";
    //    string GUI_versionNr = xml.get("VieSchedpp.software.GUI_version","unknown");
    //    of << "* GUI_VERSION " << GUI_versionNr << "\n";
    auto ctstr = xml.get<string>( "VieSchedpp.created.time", "unknown" );
    boost::posix_time::ptime ct = TimeSystem::string2ptime( ctstr );
    of << boost::format( "SCHEDULE_CREATE_DATE %s \n" ) % TimeSystem::time2string_doy( ct );
    of << "SCHEDULER " << xml.get( "VieSchedpp.output.scheduler", "----" ) << " ";
    of << "CORRELATOR " << xml.get( "VieSchedpp.output.correlator", "----" ) << " ";
    auto st = TimeSystem::startTime;
    of << boost::format( "START %s " ) % TimeSystem::time2string_doy( st );
    auto et = TimeSystem::endTime;
    of << boost::format( "END %s \n" ) % TimeSystem::time2string_doy( et );

    of << boost::format( "%-12s %4d " ) % "CALIBRATION" % network.getStation( 0 ).getWaittimes().preob;
    of << boost::format( "%-12s %4d " ) % "CORSYNCH" % network.getStation( 0 ).getWaittimes().midob;
    of << boost::format( "%-12s %4d\n" ) % "DURATION" % 196;

    of << boost::format( "%-12s %4d " ) % "EARLY" % 0;
    of << boost::format( "%-12s %4d " ) % "IDLE" % 0;
    of << boost::format( "%-12s %4d\n" ) % "LOOKAHEAD" % 0;

    of << boost::format( "%-12s %4d " ) % "MAXSCAN" % network.getStation( 0 ).getPARA().maxScan;
    of << boost::format( "%-12s %4d " ) % "MINSCAN" % network.getStation( 0 ).getPARA().minScan;
    of << boost::format( "%-12s %4d\n" ) % "MINIMUM" % 0;

    of << boost::format( "%-12s %4d " ) % "MIDTP" % 0;
    of << boost::format( "%-12s %4d " ) % "MODULAR" % 1;
    of << boost::format( "%-12s %4d " ) % "MODSCAN" % 1;
    of << boost::format( "%-12s %4d\n" ) % "PARITY" % 0;

    of << boost::format( "%-12s %4d " ) % "SETUP" % 0;
    of << boost::format( "%-12s %4d " ) % "SOURCE" % network.getStation( 0 ).getWaittimes().fieldSystem;
    of << boost::format( "%-12s %4d " ) % "TAPETM" % 0;
    of << boost::format( "%-12s %4d\n" ) % "WIDTH" % 0;

    of << boost::format( "%-12s %4s " ) % "CONFIRM" % "Y";
    of << boost::format( "%-12s %4s\n" ) % "VSCAN" % "Y";

    of << boost::format( "%-12s %4s " ) % "DEBUG" % "N";
    of << boost::format( "%-12s %4s " ) % "KEEP_LOG" % "N";
    of << boost::format( "%-12s %4s\n" ) % "VERBOSE" % "N";

    of << boost::format( "%-12s %4s " ) % "PRFLAG" % "YNNN";
    of << boost::format( "%-12s %4s\n" ) % "SNR" % "AUTO";

    of << "FREQUENCY   SX PREOB      PREOB  MIDOB     MIDOB  POSTOB     POSTOB\n";

    of << boost::format( "%-12s %4.1d\n" ) % "ELEVATION _" %
              ( network.getStation( 0 ).getPARA().minElevation * rad2deg );

    of << "TAPE_MOTION _ START&STOP\n";

    const map<string, vector<string>> &ant = skdCatalogReader.getAntennaCatalog();
    const map<string, vector<string>> &equ = skdCatalogReader.getEquipCatalog();

    int counter = 0;
    for ( const auto &any : network.getStations() ) {
        const string &staName = any.getName();
        vector<string> tmp = ant.at( staName );
        string id_EQ = boost::algorithm::to_upper_copy( tmp.at( 14 ) ) + "|" + staName;
        vector<string> tmp2 = equ.at( id_EQ );

        const string &equip = tmp2[tmp2.size() - 1];
        if ( counter == 0 ) {
            of << "TAPE_TYPE ";
        }
        of << any.getAlternativeName() << " " << equip << " ";
        ++counter;
        if ( counter == 4 ) {
            of << "\n";
            counter = 0;
        }
    }
    if ( counter != 0 ) {
        of << "\n";
    }

    counter = 0;
    for ( const auto &any : network.getStations() ) {
        const string &staName = any.getName();

        if ( counter == 0 ) {
            of << "TAPE_ALLOCATION ";
        }
        of << any.getAlternativeName() << " AUTO ";
        ++counter;
        if ( counter == 4 ) {
            of << "\n";
            counter = 0;
        }
    }
    if ( counter != 0 ) {
        of << "\n";
    }

    counter = 0;
    for ( unsigned long staid1 = 0; staid1 < network.getNSta(); ++staid1 ) {
        const Station &sta1 = network.getStation( staid1 );
        for ( unsigned long staid2 = staid1 + 1; staid2 < network.getNSta(); ++staid2 ) {
            const Station &sta2 = network.getStation( staid2 );

            if ( counter == 0 ) {
                of << "SNR ";
            }
            double minSNR_X = network.getBaseline( staid1, staid2 ).getParameters().minSNR.at( "X" );
            if ( sta1.getPARA().minSNR.at( "X" ) > minSNR_X ) {
                minSNR_X = sta1.getPARA().minSNR.at( "X" );
            }
            if ( sta2.getPARA().minSNR.at( "X" ) > minSNR_X ) {
                minSNR_X = sta2.getPARA().minSNR.at( "X" );
            }
            double minSNR_S = network.getBaseline( staid1, staid2 ).getParameters().minSNR.at( "S" );
            if ( sta1.getPARA().minSNR.at( "S" ) > minSNR_S ) {
                minSNR_S = sta1.getPARA().minSNR.at( "S" );
            }
            if ( sta2.getPARA().minSNR.at( "S" ) > minSNR_S ) {
                minSNR_S = sta2.getPARA().minSNR.at( "S" );
            }
            of << boost::format( " %2s-%2s X %-4d %2s-%2s S %-4d " ) % sta1.getAlternativeName() %
                      sta2.getAlternativeName() % minSNR_X % sta1.getAlternativeName() % sta2.getAlternativeName() %
                      minSNR_S;
            ++counter;
            if ( counter == 3 ) {
                of << "\n";
                counter = 0;
            }
        }
    }
    if ( counter != 0 ) {
        of << "\n";
    }
}


void Skd::skd_OP() {
    //    of << "*\n";
    //    of <<
    //    "*=========================================================================================================\n";
    of << "$OP\n";
    //    of <<
    //    "*=========================================================================================================\n";
    //    of << "* OP is not supported in VieSched++ \n";
    //    of << "*\n";
}


void Skd::skd_DOWNTIME( const Network &network ) {
    //    of << "*\n";
    //    of <<
    //    "*=========================================================================================================\n";
    of << "$DOWNTIME\n";
    //    of <<
    //    "*=========================================================================================================\n";
    for ( const auto &sta : network.getStations() ) {
        bool tmp = sta.listDownTimes( of, true );
    }
    for ( const auto &sta : network.getStations() ) {
        bool tmp = sta.listTagalongTimes( of, true );
    }
}


void Skd::skd_MAJOR( const vector<Station> &stations, const vector<Source> &sources,
                     const boost::property_tree::ptree &xml, const SkdCatalogReader &skdCatalogReader ) {
    //    of << "*\n";
    //    of <<
    //    "*=========================================================================================================\n";
    of << "$MAJOR\n";
    //    of <<
    //    "*=========================================================================================================\n";
    //    of << "* Sked MAJOR parameters can not be translated directly to VieSched++ parameters\n";
    //    of << "*\n";
    of << "Subnet ";
    for ( const auto &any : stations ) {
        of << any.getAlternativeName();
    }
    of << "\n";
    of << boost::format( "%-14s %6s\n" ) % "SkyCov" % "No";
    of << boost::format( "%-14s %6s\n" ) % "AllBlGood" % "Yes";
    of << boost::format( "%-14s %6.2f\n" ) % "MaxAngle" % 180;
    of << boost::format( "%-14s %6.2f\n" ) % "MinAngle" % 2;
    of << boost::format( "%-14s %6d\n" ) % "MinBetween" % ( sources[0].getPARA().minRepeat / 60 );
    of << boost::format( "%-14s %6d\n" ) % "MinSunDist" % ( sources[0].getPARA().minSunDistance * rad2deg );
    //    of << boost::format( "%-14s %6d\n" ) % "MinSlewTime" % stations[0].getPARA().minSlewtime;
    of << boost::format( "%-14s %6d\n" ) % "MaxSlewTime" % stations[0].getPARA().maxSlewtime;
    of << boost::format( "%-14s %6.2f\n" ) % "TimeWindow" % ( SkyCoverage::maxInfluenceTime / 3600 );
    of << boost::format( "%-14s %6.2f\n" ) % "MinSubNetSize" % sources[0].getPARA().minNumberOfStations;
    if ( xml.get<bool>( "VieSchedpp.general.subnetting", false ) ) {
        of << boost::format( "%-14s %6d\n" ) % "NumSubNet" % 1;
    } else {
        of << boost::format( "%-14s %6d\n" ) % "NumSubNet" % 2;
    }
    of << boost::format( "%-14s %6d\n" ) % "Best" % 100;
    bool fillin = xml.get<bool>( "VieSchedpp.general.fillinmodeDuringScanSelection", false ) ||
                  xml.get<bool>( "VieSchedpp.general.fillinmodeAPosteriori", false );
    if ( fillin ) {
        of << boost::format( "%-14s %6s\n" ) % "FillIn" % "Yes";
    } else {
        of << boost::format( "%-14s %6s\n" ) % "FillIn" % "No";
    }
    of << boost::format( "%-14s %6d\n" ) % "FillMinSub" % sources[0].getPARA().minNumberOfStations;
    of << boost::format( "%-14s %6d\n" ) % "FillMinTime" % 20;
    of << boost::format( "%-14s %6d\n" ) % "FillBest" % 100;
    of << boost::format( "%-14s %6.2f\n" ) % "Add_ps" % 0.00;
    of << boost::format( "%-14s %6s\n" ) % "SNRWts" % "No";
}


void Skd::skd_MINOR() {
    //    of << "*\n";
    //    of <<
    //    "*=========================================================================================================\n";
    of << "$MINOR\n";
    //    of <<
    //    "*=========================================================================================================\n";
    //    of << "* Sked MINOR parameters can not be translated directly to VieSched++ parameters\n";
    //    of << "*\n";
    of << boost::format( "%-14s %-3s %-3s %8.2f\n" ) % "Astro" % "No" % "Abs" % 0.00;
    of << boost::format( "%-14s %-3s %-3s %8.2f\n" ) % "BegScan" % "No" % "Abs" % 0.00;
    of << boost::format( "%-14s %-3s %-3s %8.2f\n" ) % "Covar" % "No" % "Abs" % 0.00;
    of << boost::format( "%-14s %-3s %-3s %8.2f\n" ) % "EndScan" % "Yes" % "Abs" % WeightFactors::weightDuration;
    of << boost::format( "%-14s %-3s %-3s %8.2f\n" ) % "LowDec" % "No" % "Abs" % 0.00;
    of << boost::format( "%-14s %-3s %-3s %8.2f 0.00\n" ) % "NumLoEl" % "No" % "Abs" % 0.00;
    of << boost::format( "%-14s %-3s %-3s %8.2f\n" ) % "NumRiseSet" % "No" % "Abs" % 0.00;
    of << boost::format( "%-14s %-3s %-3s %8.2f\n" ) % "NumObs" % "Yes" % "Abs" %
              WeightFactors::weightNumberOfObservations;
    of << boost::format( "%-14s %-3s %-3s %8.2f\n" ) % "SkyCov" % "Yes" % "Abs" % WeightFactors::weightSkyCoverage;
    of << boost::format( "%-14s %-3s %-3s %8.2f NONE\n" ) % "SrcEvn" % "Yes" % "Abs" %
              WeightFactors::weightAverageSources;
    of << boost::format( "%-14s %-3s %-3s %8.2f\n" ) % "SrcWt" % "Yes" % "Abs" % 1.00;
    of << boost::format( "%-14s %-3s %-3s %8.2f NONE\n" ) % "StatEvn" % "Yes" % "Abs" %
              WeightFactors::weightAverageStations;
    of << boost::format( "%-14s %-3s %-3s %8.2f\n" ) % "StatIdle" % "No" % "Abs" % 0.00;
    of << boost::format( "%-14s %-3s %-3s %8.2f\n" ) % "StatWt" % "Yes" % "Abs" % 1.00;
    of << boost::format( "%-14s %-3s %-3s %8.2f\n" ) % "TimeVar" % "No" % "Abs" % 0.00;
}


void Skd::skd_ASTROMETRIC() {
    //    of << "*\n";
    //    of <<
    //    "*=========================================================================================================\n";
    of << "$ASTROMETRIC\n";
    //    of <<
    //    "*=========================================================================================================\n";
    //    of << "* ASTROMETRIC is not supported in VieSched++ \n";
    //    of << "*\n";
}


void Skd::skd_STATWT( const std::vector<Station> &stations ) {
    //    of << "*\n";
    //    of <<
    //    "*=========================================================================================================\n";
    of << "$STATWT\n";
    //    of <<
    //    "*=========================================================================================================\n";
    //    of << "* all weights except weight=1 are listed here\n";
    //    of << "*\n";
    for ( const auto &any : stations ) {
        if ( any.getPARA().weight != 1 ) {
            of << boost::format( "%-10s %6.2f\n" ) % any.getName() % any.getPARA().weight;
        }
    }
}


void Skd::skd_SRCWT( const std::vector<Source> &sources ) {
    //    of << "*\n";
    //    of <<
    //    "*=========================================================================================================\n";
    of << "$SRCWT\n";
    //    of <<
    //    "*=========================================================================================================\n";
    //    of << "* all weights except weight=1 are listed here\n";
    //    of << "*\n";
    for ( const auto &any : sources ) {
        if ( any.getNTotalScans() > 0 ) {
            if ( any.getPARA().weight != 1 ) {
                of << boost::format( "%-10s %6.2f\n" ) % any.getName() % any.getPARA().weight;
            }
        }
    }
}


void Skd::skd_CATALOG_USED( const boost::property_tree::ptree &xml, const SkdCatalogReader &skdCatalogReader ) {
    //    of << "*\n";
    //    of <<
    //    "*=========================================================================================================\n";
    of << "$CATALOG_USED\n";
    //    of <<
    //    "*=========================================================================================================\n";
    //    of << "*\n";
    string source = xml.get<string>( "VieSchedpp.catalogs.source", "UNKNOWN" );
    string flux = xml.get<string>( "VieSchedpp.catalogs.flux", "UNKNOWN" );

    string antenna = xml.get<string>( "VieSchedpp.catalogs.antenna", "UNKNOWN" );
    string position = xml.get<string>( "VieSchedpp.catalogs.position", "UNKNOWN" );
    string equip = xml.get<string>( "VieSchedpp.catalogs.equip", "UNKNOWN" );
    string mask = xml.get<string>( "VieSchedpp.catalogs.mask", "UNKNOWN" );

    string modes = xml.get<string>( "VieSchedpp.catalogs.modes", "UNKNOWN" );
    string freq = xml.get<string>( "VieSchedpp.catalogs.freq", "UNKNOWN" );
    string rec = xml.get<string>( "VieSchedpp.catalogs.rec", "UNKNOWN" );
    string rx = xml.get<string>( "VieSchedpp.catalogs.rx", "UNKNOWN" );
    string loif = xml.get<string>( "VieSchedpp.catalogs.loif", "UNKNOWN" );
    string tracks = xml.get<string>( "VieSchedpp.catalogs.tracks", "UNKNOWN" );
    string hdpos = xml.get<string>( "VieSchedpp.catalogs.hdpos", "UNKNOWN" );

    of << boost::format( "%-10s %-20s %s\n" ) % "SOURCE" % skdCatalogReader.getVersion( "source" ) % source;
    of << boost::format( "%-10s %-20s %s\n" ) % "FLUX" % skdCatalogReader.getVersion( "flux" ) % flux;

    of << boost::format( "%-10s %-20s %s\n" ) % "ANTENNA" % skdCatalogReader.getVersion( "antenna" ) % antenna;
    of << boost::format( "%-10s %-20s %s\n" ) % "POSITION" % skdCatalogReader.getVersion( "position" ) % position;
    of << boost::format( "%-10s %-20s %s\n" ) % "EQUIP" % skdCatalogReader.getVersion( "equip" ) % equip;
    of << boost::format( "%-10s %-20s %s\n" ) % "MASK" % skdCatalogReader.getVersion( "mask" ) % mask;

    of << boost::format( "%-10s %-20s %s\n" ) % "MODES" % skdCatalogReader.getVersion( "modes" ) % modes;
    of << boost::format( "%-10s %-20s %s\n" ) % "FREQ" % skdCatalogReader.getVersion( "freq" ) % freq;
    of << boost::format( "%-10s %-20s %s\n" ) % "REC" % skdCatalogReader.getVersion( "rec" ) % rec;
    of << boost::format( "%-10s %-20s %s\n" ) % "RX" % skdCatalogReader.getVersion( "rx" ) % rx;
    of << boost::format( "%-10s %-20s %s\n" ) % "LOIF" % skdCatalogReader.getVersion( "loif" ) % loif;
    of << boost::format( "%-10s %-20s %s\n" ) % "TRACKS" % skdCatalogReader.getVersion( "tracks" ) % tracks;
    of << boost::format( "%-10s %-20s %s\n" ) % "HDPOS" % skdCatalogReader.getVersion( "hdpos" ) % hdpos;
}


void Skd::skd_BROADBAND() {
    //    of << "*\n";
    //    of <<
    //    "*=========================================================================================================\n";
    of << "$BROADBAND\n";
    //    of <<
    //    "*=========================================================================================================\n";
    //    of << "* $BROADBAND is not supported in VieSched++ \n";
    //    of << "*\n";
}


void Skd::skd_SOURCES( const std::vector<Source> &sources, const SkdCatalogReader &skdCatalogReader ) {
    //    of << "*\n";
    //    of <<
    //    "*=========================================================================================================\n";
    of << "$SOURCES\n";
    //    of <<
    //    "*=========================================================================================================\n";
    //    of << "*\n";
    const map<string, vector<string>> &src = skdCatalogReader.getSourceCatalog();

    for ( const auto &any : sources ) {
        if ( any.getNTotalScans() > 0 ) {
            vector<string> tmp;
            if ( src.find( any.getName() ) != src.end() ) {
                tmp = src.at( any.getName() );
            } else {
                tmp = src.at( any.getAlternativeName() );
            }

            of << boost::format( " %-8s %-8s   %2s %2s %9s    %3s %2s %9s %6s %3s " ) % tmp[0] % tmp[1] % tmp[2] %
                      tmp[3] % tmp[4] % tmp[5] % tmp[6] % tmp[7] % tmp[8] % tmp[9];
            for ( int i = 10; i < tmp.size(); ++i ) {
                of << tmp[i] << " ";
            }
            of << "\n";
        }
    }
}


void Skd::skd_STATIONS( const std::vector<Station> &stations, const SkdCatalogReader &skdCatalogReader ) {
    //    of << "*\n";
    //    of <<
    //    "*=========================================================================================================\n";
    of << "$STATIONS\n";
    //    of <<
    //    "*=========================================================================================================\n";
    //    of << "*\n";
    const map<string, vector<string>> &ant = skdCatalogReader.getAntennaCatalog();
    const map<string, vector<string>> &pos = skdCatalogReader.getPositionCatalog();
    const map<string, vector<string>> &equ = skdCatalogReader.getEquipCatalog();
    const map<string, vector<string>> &mas = skdCatalogReader.getMaskCatalog();

    map<string, string> posMap;
    map<string, string> equMap;
    map<string, string> masMap;

    for ( const auto &any : stations ) {
        const string &staName = any.getName();

        vector<string> tmp = ant.at( staName );

        string id_PO = tmp.at( 13 );
        posMap[staName] = id_PO;
        string id_EQ = boost::algorithm::to_upper_copy( tmp.at( 14 ) ) + "|" + staName;
        equMap[staName] = id_EQ;
        string id_MS = tmp.at( 15 );
        masMap[staName] = id_MS;

        const auto &olc = skdCatalogReader.getOneLetterCode();

        of << boost::format( "A %2s %-8s %4s %8s  %6s %3s %6s %6s %6s %3s %6s %6s %5s  %3s %3s %3s \n" ) %
                  olc.at( tmp[1] ) % tmp[1] % tmp[2] % tmp[3] % tmp[4] % tmp[5] % tmp[6] % tmp[7] % tmp[8] % tmp[9] %
                  tmp[10] % tmp[11] % tmp[12] % tmp[13] % tmp[14] % tmp[15];
    }

    for ( const auto &any : stations ) {
        vector<string> tmp = pos.at( posMap[any.getName()] );
        of << boost::format( "P %2s %-8s %14s  %14s  %14s  %8s  %7s %6s " ) % tmp[0] % tmp[1] % tmp[2] % tmp[3] %
                  tmp[4] % tmp[5] % tmp[6] % tmp[7];
        for ( int i = 8; i < tmp.size(); ++i ) {
            of << tmp[i] << " ";
        }
        of << "\n";
    }

    for ( const auto &any : stations ) {
        const string &staname = any.getName();
        vector<string> atmp = ant.at( staname );
        const string &id_EQ = boost::algorithm::to_upper_copy( atmp.at( 14 ) ) + "|" + staname;
        const vector<string> &tmp = equ.at( id_EQ );
        of << boost::format( "T %3s %8s  %7s %8s   %1s %5s  %1s %5s " ) % tmp[1] % tmp[2] % tmp[3] % tmp[4] % tmp[5] %
                  tmp[6] % tmp[7] % tmp[8];
        for ( int i = 9; i < tmp.size(); ++i ) {
            of << tmp[i] << " ";
        }
        of << "\n";
    }

    for ( const auto &any : stations ) {
        string id = masMap[any.getName()];
        if ( id != "--" && mas.find( id ) != mas.end() ) {
            vector<string> tmp = mas.at( id );
            tmp.erase( next( tmp.begin(), 1 ) );
            if ( !tmp.empty() ) {
                for ( const auto &any2 : tmp ) {
                    if ( any2 != "-" ) {
                        of << any2 << " ";
                    }
                }
                of << "\n";
            }
        }
    }
}


void Skd::skd_FLUX( const vector<Source> &sources, const SkdCatalogReader &skdCatalogReader ) {
    //    of << "*\n";
    //    of <<
    //    "*=========================================================================================================\n";
    of << "$FLUX\n";
    //    of <<
    //    "*=========================================================================================================\n";
    //    of << "*\n";

    const map<string, vector<string>> &flu = skdCatalogReader.getFluxCatalog();
    for ( const auto &any : sources ) {
        if ( any.getNTotalScans() > 0 ) {
            const string &name = any.getName();

            if ( flu.find( name ) != flu.end() ) {
                vector<string> tmp = flu.at( name );
                for ( const auto &any2 : tmp ) {
                    of << any2 << "\n";
                }
            }
        }
    }
}


void Skd::skd_SKED( const std::vector<Station> &stations, const std::vector<Source> &sources,
                    const std::vector<Scan> &scans, const SkdCatalogReader &skdCatalogReader ) {
    //    of << "*\n";
    //    of <<
    //    "*=========================================================================================================\n";
    of << "$SKED\n";
    //    of <<
    //    "*=========================================================================================================\n";
    //    of << "*\n";
    int preob = stations[0].getWaittimes().preob;

    const map<string, char> &olc = skdCatalogReader.getOneLetterCode();

    for ( const auto &scan : scans ) {
        const string &srcName = sources[scan.getSourceId()].getName();

        unsigned int scanTime = scan.getTimes().getObservingDuration();

        string ftlc;
        if ( skdCatalogReader.getFreqTwoLetterCode().empty() ) {
            ftlc = "SX";
        } else {
            ftlc = skdCatalogReader.getFreqTwoLetterCode();
        }
        of << boost::format( "%-8s %3d %s PREOB  %s  %8d MIDOB         0 POSTOB " ) % srcName % preob % ftlc %
                  TimeSystem::time2string_doy( scan.getTimes().getObservingTime( Timestamp::start ) ) % scanTime;

        for ( int i = 0; i < scan.getNSta(); ++i ) {
            const PointingVector &pv = scan.getPointingVector( i );
            const Station &thisSta = stations[pv.getStaid()];
            const string &staName = thisSta.getName();
            of << olc.at( staName );
            switch ( thisSta.getCableWrap().cableWrapFlag( pv ) ) {
                case AbstractCableWrap::CableWrapFlag::ccw:
                    of << "W";
                    break;
                case AbstractCableWrap::CableWrapFlag::n:
                    of << "-";
                    break;
                case AbstractCableWrap::CableWrapFlag::cw:
                    of << "C";
                    break;
            }
        }
        of << " ";
        for ( int i = 0; i < scan.getNSta(); ++i ) {
            of << "1F000000 ";
        }
        of << "YYNN ";
        for ( int i = 0; i < scan.getNSta(); ++i ) {
            unsigned int thisScanTime = scan.getTimes().getObservingDuration( i );
            of << boost::format( "%5d " ) % thisScanTime;
        }
        of << "\n";
    }
}


void Skd::skd_CODES( const std::vector<Station> &stations, const SkdCatalogReader &skd ) {
    //    of << "*\n";
    //    of <<
    //    "*=========================================================================================================\n";
    of << "$CODES\n";
    //    of <<
    //    "*=========================================================================================================\n";
    //    of << "*\n";
    if ( ObservingMode::type == ObservingMode::Type::sked ) {
        unsigned long nchannels = skd.getChannelNumber2band().size();
        const std::map<std::string, char> &olc = skd.getOneLetterCode();

        for ( const auto &trackId : skd.getTracksIds() ) {
            // output first line!
            of << "F " << skd.getFreqName() << " " << skd.getFreqTwoLetterCode();
            for ( const auto &any : skd.getStaName2tracksMap() ) {
                if ( any.second == trackId ) {
                    of << " " << any.first;
                }
            }
            of << "\n";

            // output C block
            for ( int i = 1; i < nchannels + 1; ++i ) {
                of << "C " << skd.getFreqTwoLetterCode() << " " << skd.getChannelNumber2band().at( i ) << " "
                   << skd.getChannelNumber2skyFreq().at( i ) << " " << skd.getChannelNumber2phaseCalFrequency().at( i )
                   << " " << boost::format( "%2d" ) % skd.getChannelNumber2BBC().at( i )
                   << " Mk341:" << skd.getTracksId2fanoutMap().at( trackId )
                   << boost::format( "%6.2f" ) % skd.getBandWidth() << " "
                   << skd.getTracksId2channelNumber2tracksMap().at( trackId ).at( i ) << "\n";
            }
        }
        for ( const auto &sta : stations ) {
            if ( skd.getStaName2tracksMap().find( sta.getName() ) == skd.getStaName2tracksMap().end() ) {
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( warning )
                    << "skd output: F" << skd.getFreqName() << " " << skd.getFreqTwoLetterCode() << " " << sta.getName()
                    << " MISSING in this mode";
#else
                cout << "[warning] skd output: F" << skd.getFreqName() << " " << skd.getFreqTwoLetterCode() << " "
                     << sta.getName() << " MISSING in this mode";
#endif
                of << "* F" << skd.getFreqName() << " " << skd.getFreqTwoLetterCode() << " " << sta.getName()
                   << " MISSING in this mode!\n";
            }
        }

        of << "R " << skd.getFreqTwoLetterCode() << " " << skd.getSampleRate() << "\n";
        of << "B " << skd.getFreqTwoLetterCode() << "\n";

        for ( const auto &staName : skd.getStaNames() ) {
            const auto &loifId = skd.getStaName2loifId().at( staName );
            const vector<string> loif = skd.getLoifId2loifInfo().at( loifId );
            for ( auto any : loif ) {
                any = boost::algorithm::trim_copy( any );
                vector<string> splitVector;
                boost::split( splitVector, any, boost::is_space(), boost::token_compress_on );
                string nr = splitVector[1];
                string IF = splitVector[2];
                string band = splitVector[3];
                string freq = splitVector[4];
                string sideBand = splitVector[5];
                of << boost::format( "L %c %2s %2s %2s %8s %3s %s\n" ) % olc.at( staName ) %
                          skd.getFreqTwoLetterCode() % band % IF % freq % nr % sideBand;
            }
        }

    } else {
        of << "* no sked observind mode used! \n";
        //        of << "    bits:     " << ObservationMode::bits << "\n";
        //        of << "    channels: " << ObservationMode::sampleRate << "\n";
        //        for (const auto &any: ObservationMode::bands){
        //            of << "    band: " << any << " nChannels: " << ObservationMode::nChannels[any] << " wavelength: "
        //            << ObservationMode::wavelength[any] <<"\n";
        //        }
    }
}
