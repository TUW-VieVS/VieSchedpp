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

#include "OperationNotes.h"


using namespace VieVS;
using namespace std;

unsigned long OperationNotes::nextId = 0;


OperationNotes::OperationNotes( const std::string &file ) : VieVS_Object( nextId++ ) { of = ofstream( file ); }


void OperationNotes::writeOperationNotes( const Network &network, const SourceList &sourceList,
                                          const std::vector<Scan> &scans,
                                          const std::shared_ptr<const ObservingMode> &obsModes,
                                          const boost::property_tree::ptree &xml, int version,
                                          boost::optional<MultiScheduling::Parameters> multiSchedulingParameters ) {
    string expName = xml.get( "VieSchedpp.general.experimentName", "schedule" );
    string description = xml.get( "VieSchedpp.output.experimentDescription", "" );

    string currentTimeString = xml.get<string>( "VieSchedpp.created.time", "" );

    boost::posix_time::ptime currentTime = TimeSystem::startTime;

    int year = currentTime.date().year();
    int month = currentTime.date().month();
    int day = currentTime.date().day();
    int doy = currentTime.date().day_of_year();

    int maxDoy;
    boost::gregorian::gregorian_calendar::is_leap_year( year ) ? maxDoy = 366 : maxDoy = 365;
    short weekday = currentTime.date().day_of_week();

    string wd = util::weekDay2string( weekday );
    string monthStr = util::month2string( month );

    double yearDecimal = year + static_cast<double>( doy ) / static_cast<double>( maxDoy );

    vector<string> functions;
    vector<string> names;
    vector<string> emails;
    vector<string> phones;
    vector<string> affiliations;
    map<int, int> nameId2affiliationId;
    contactInformations( functions, names, emails, phones, affiliations, nameId2affiliationId, xml );


    string experimentName_header = "<<<<< " + expName + " >>>>>";
    of << boost::format( "%=80s\n" ) % experimentName_header;
    if ( !description.empty() && description != "no further description" ) {
        string description_header = "<<< " + description + " >>>";
        of << boost::format( "%=80s\n" ) % description_header;
    }
    of << "\n";

    for ( int i = 0; i < names.size(); ++i ) {
        of << names[i];
        if ( nameId2affiliationId[i] != -1 ) {
            int id = nameId2affiliationId[i];
            of << "(" << id << "), ";
        } else {
            of << ", ";
        }
    }
    of << "\n";

    for ( int i = 0; i < affiliations.size(); ++i ) {
        of << "(" << i + 1 << ") " << affiliations[i] << "\n";
    }
    of << "\n";


    of << boost::format( "Date of experiment: %4d,%3s,%02d\n" ) % ( TimeSystem::startTime.date().year() ) %
              ( TimeSystem::startTime.date().month() ) % ( TimeSystem::startTime.date().day() );
    of << boost::format( "Nominal Start Time: %02dh%02d UT\n" ) % ( TimeSystem::startTime.time_of_day().hours() ) %
              ( TimeSystem::startTime.time_of_day().minutes() );
    of << boost::format( "Nominal End Time:   %02dh%02d UT\n" ) % ( TimeSystem::endTime.time_of_day().hours() ) %
              ( TimeSystem::endTime.time_of_day().minutes() );
    of << boost::format( "Duration:           %.1f hr\n" ) % ( TimeSystem::duration / 3600. );
    of << "Correlator:         " << xml.get( "VieSchedpp.output.correlator", "unknown" ) << "\n\n";

    of << "Participating stations: (" << network.getNSta() << ")\n";
    for ( const auto &any : network.getStations() ) {
        of << boost::format( "%-8s    %2s \n" ) % any.getName() % any.getAlternativeName();
    }
    of << "\n";

    std::string schedulerName = xml.get( "VieSchedpp.created.name", "" );
    if ( !schedulerName.empty() && schedulerName != "unknown" ) {
        of << "scheduler: " << schedulerName << "\n";
    }
    of << "\n";

    of << "Contact\n";
    of << "=======\n";

    unsigned long maxFunction = 0;
    if ( !functions.empty() ) {
        maxFunction = std::max_element( functions.begin(), functions.end(),
                                        []( const std::string &str1, const std::string &str2 ) {
                                            return str1.length() < str2.length();
                                        } )
                          ->size();
    }

    unsigned long maxName = 0;
    if ( !names.empty() ) {
        maxName = std::max_element( names.begin(), names.end(), []( const std::string &str1, const std::string &str2 ) {
                      return str1.length() < str2.length();
                  } )->size();
    }

    unsigned long maxEmail = 0;
    if ( !emails.empty() ) {
        maxEmail =
            std::max_element( emails.begin(), emails.end(), []( const std::string &str1, const std::string &str2 ) {
                return str1.length() < str2.length();
            } )->size();
    }

    string format_noAffil =
        ( boost::format( "%%-%ds %%-%ds      %%-%ds  %%s\n" ) % maxFunction % maxName % maxEmail ).str();
    string format_affil =
        ( boost::format( "%%-%ds %%-%ds (%%d)  %%-%ds  %%s\n" ) % maxFunction % maxName % maxEmail ).str();

    for ( int i = 0; i < names.size(); ++i ) {
        if ( nameId2affiliationId[i] != -1 ) {
            of << boost::format( format_affil ) % functions[i] % names[i] % nameId2affiliationId[i] % emails[i] %
                      phones[i];
        } else {
            of << boost::format( format_noAffil ) % functions[i] % names[i] % emails[i] % phones[i];
        }
    }

    of << "\n";
    of << "Notes: \n";
    of << "===========================================================\n";

    std::string notes = xml.get( "VieSchedpp.output.notes", "" );
    if ( !notes.empty() ) {
        of << boost::replace_all_copy( notes, "\\n", "\n" ) << "\n";
    }

    bool down = false;
    of << "Station down times:\n";
    for ( const auto &sta : network.getStations() ) {
        bool flag = sta.listDownTimes( of );
        down = down || flag;
    }
    if ( !down ) {
        of << "    none\n";
    }

    of << "Tagalong mode used:\n";
    bool tag = false;
    for ( const auto &sta : network.getStations() ) {
        bool flag = sta.listTagalongTimes( of );
        tag = tag || flag;
    }
    if ( !tag ) {
        of << "    none\n";
    }
    of << "\n";
    of << "===========================================================\n";

    of << "Session Notes for session: " << expName << "\n";
    of << "===========================================================\n";
    of << boost::format( " Experiment: %-17s            Description: %-s\n" ) % expName %
              ( xml.get( "VieSchedpp.output.experimentDescription", "no_description" ) );
    of << boost::format( " Scheduler:  %-17s            Correlator:  %-s\n" ) %
              ( xml.get( "VieSchedpp.output.scheduler", "unknown" ) ) %
              ( xml.get( "VieSchedpp.output.correlator", "unknown" ) );
    of << boost::format( " Start:      %-17s            End:         %-s\n" ) %
              ( TimeSystem::time2string_doySkdDowntime( 0 ) ) %
              ( TimeSystem::time2string_doySkdDowntime( TimeSystem::duration ) );
    of << boost::format( " Current yyyyddd:    %4d%03d (%7.2f)  ( %5d MJD, %s. %2d%s.)\n" ) % year % doy % yearDecimal %
              ( currentTime.date().modjulian_day() ) % wd % day % monthStr;
    of << "===========================================================\n";
    of << boost::format( " Software:   %-17s            Version:     %-s\n" ) % "VieSched++" %
              ( util::version().substr( 0, 7 ) );
    of << boost::format( " GUI:        %-17s            Version:     %-s\n" ) % "VieSched++" %
              ( xml.get( "VieSchedpp.software.GUI_version", "unknown" ).substr( 0, 7 ) );
    if ( !schedulerName.empty() && schedulerName != "unknown" ) {
        of << boost::format( " scheduler:  %-27s  mail:        %-s\n" ) % schedulerName %
                  xml.get( "VieSchedpp.created.email", "" );
    }
    of << "===========================================================\n";
    firstLastObservations_skdStyle( expName, network, sourceList, scans );
    of << "===========================================================\n";
    calibrators_skdStyle( expName, network, sourceList, scans );

    if ( multiSchedulingParameters.is_initialized() ) {
        of << " Schedule was created using multi scheduling tool\n";
        of << "    version " << version << "\n";
        multiSchedulingParameters->output( of );
        of << "===========================================================\n";
    }
    listKeys( network );
    displayTimeStatistics( network, obsModes );
    displayBaselineStatistics( network );
    displayNstaStatistics( network, scans );
    of << "===========================================================\n";

    obsModes->operationNotesSummary( of );

    of << "============================================================================================================"
          "====================================\n";
    of << "                                                         ADDITONAL NOTES FOR SCHEDULER                      "
          "                                   \n";
    of << "============================================================================================================"
          "====================================\n\n";

    displayGeneralStatistics( scans );
    WeightFactors::summary( of );

    displaySkyCoverageScore( network );
    displayStationStatistics( network );
    displaySourceStatistics( sourceList );
    displayBaselineTimeStatistics( network );
    displayMostSubnets( scans, network );
    displaySkyCoverage( network );
    obsModes->summary( of );
    of << "\n";
    displaySNRSummary( network, sourceList, scans, obsModes );
    displayScanDurationStatistics( network, scans );
    if ( scans.size() >= 2 ) {
        of << "First Scans:\n";
        of << ".-------------------------------------------------------------------------------------------------------"
              "---------------------------------------.\n";
        for ( unsigned long i = 0; i < 3; ++i ) {
            const auto &thisScan = scans[i];
            thisScan.output( i, network, sourceList.getSource( thisScan.getSourceId() ), of );
        }
        of << "\n";
        of << "Last Scans:\n";
        of << ".-------------------------------------------------------------------------------------------------------"
              "---------------------------------------.\n";
        for ( unsigned long i = scans.size() - 3; i < scans.size(); ++i ) {
            const auto &thisScan = scans[i];
            thisScan.output( i, network, sourceList.getSource( thisScan.getSourceId() ), of );
        }
        of << "\n";
    }
    displayAstronomicalParameters();

    of << "============================================================================================================"
          "====================================\n";
    of << "                                                              FULL SCHEDULING SETUP                         "
          "                                    \n";
    of << "============================================================================================================"
          "====================================\n";
    of << "Can be used to recreate schedule:\n\n";
    boost::property_tree::xml_parser::write_xml(
        of, xml, boost::property_tree::xml_writer_make_settings<std::string>( '\t', 1 ) );
}


void OperationNotes::writeSkdsum( const Network &network, const SourceList &sourceList,
                                  const std::vector<Scan> &scans ) {
    {
        displayGeneralStatistics( scans );
        displayBaselineStatistics( network );
        displayStationStatistics( network );
        displaySourceStatistics( sourceList );
        displayNstaStatistics( network, scans );
        displayScanDurationStatistics( network, scans );
        displayAstronomicalParameters();
    }
}


void OperationNotes::displayGeneralStatistics( const std::vector<Scan> &scans ) {
    auto n_scans = static_cast<int>( scans.size() );
    unsigned long n_standard = 0;
    unsigned long n_highImpact = 0;
    unsigned long n_fillin = 0;
    unsigned long n_calibrator = 0;
    unsigned long n_single = 0;
    unsigned long n_subnetting = 0;

    unsigned long obs_max = 0;
    unsigned long obs = 0;
    unsigned long obs_standard = 0;
    unsigned long obs_highImpact = 0;
    unsigned long obs_fillin = 0;
    unsigned long obs_calibrator = 0;
    unsigned long obs_single = 0;
    unsigned long obs_subnetting = 0;

    for ( const auto &any : scans ) {
        switch ( any.getType() ) {
            case Scan::ScanType::fillin: {
                ++n_fillin;
                obs_fillin += any.getNObs();
                break;
            }
            case Scan::ScanType::astroCalibrator: {
                ++n_calibrator;
                obs_calibrator += any.getNObs();
                break;
            }
            case Scan::ScanType::calibrator: {
                ++n_calibrator;
                obs_calibrator += any.getNObs();
                break;
            }
            case Scan::ScanType::standard: {
                ++n_standard;
                obs_standard += any.getNObs();
                break;
            }
            case Scan::ScanType::highImpact: {
                ++n_highImpact;
                obs_highImpact += any.getNObs();
                break;
            }
            default: {
                break;
            }
        }
        switch ( any.getScanConstellation() ) {
            case Scan::ScanConstellation::single: {
                ++n_single;
                obs_single += any.getNObs();
                break;
            }
            case Scan::ScanConstellation::subnetting: {
                ++n_subnetting;
                obs_subnetting += any.getNObs();
                break;
            }
            default: {
                break;
            }
        }
        obs += any.getNObs();
        obs_max += ( any.getNSta() * ( any.getNSta() - 1 ) ) / 2;
    }

    if ( obs_max - obs > 0 ) {
        of << "number of scheduled observations: " << obs << " of " << obs_max;
        int diff = obs_max - obs;
        of << boost::format( " -> %d (%.2f [%%]) observations not optimized for SNR\n" ) % diff %
                  ( static_cast<double>( diff ) / static_cast<double>( obs_max ) * 100 );
    }

    of << boost::format( "                 #scans     #obs   \n" );
    of << "--------------------------------- \n";
    of << boost::format( " total           %6d   %6d  \n" ) % n_scans % obs;
    of << "--------------------------------- \n";
    of << boost::format( " single source   %6d   %6d  \n" ) % n_single % obs_single;
    of << boost::format( " subnetting      %6d   %6d  \n" ) % n_subnetting % obs_subnetting;
    of << "--------------------------------- \n";
    of << boost::format( " standard        %6d   %6d  \n" ) % n_standard % obs_standard;
    of << boost::format( " fillin mode     %6d   %6d  \n" ) % n_fillin % obs_fillin;
    if ( n_calibrator > 0 ) {
        of << boost::format( " calibrator      %6d   %6d  \n" ) % n_calibrator % obs_calibrator;
    }
    if ( n_highImpact > 0 ) {
        of << boost::format( " high impact     %6d   %6d  \n" ) % n_highImpact % obs_highImpact;
    }
    of << "\n";
}


void OperationNotes::displayBaselineStatistics( const Network &network ) {
    unsigned long nsta = network.getNSta();
    of << "\n      # OF OBSERVATIONS BY BASELINE \n";

    of << "  |";
    for ( const auto &any : network.getStations() ) {
        of << "  " << any.getAlternativeName() << " ";
    }
    of << " Total\n";
    of << "---";
    for ( int i = 0; i < network.getNSta(); ++i ) {
        of << "-----";
    }
    of << "--------\n";

    for ( unsigned long staid1 = 0; staid1 < nsta; ++staid1 ) {
        of << network.getStation( staid1 ).getAlternativeName() << "|";
        for ( unsigned long staid2 = 0; staid2 < nsta; ++staid2 ) {
            if ( staid2 < staid1 + 1 ) {
                of << "     ";
            } else {
                unsigned long nBl = network.getBaseline( staid1, staid2 ).getStatistics().scanStartTimes.size();
                of << boost::format( "%4d " ) % nBl;
            }
        }
        of << boost::format( "%7d\n" ) % network.getStation( staid1 ).getNObs();
    }
}


void OperationNotes::firstLastObservations_skdStyle( const string &expName, const Network &network,
                                                     const SourceList &sourceList, const std::vector<Scan> &scans ) {
    of << " First observations\n";
    of << " Observation listing from file " << boost::algorithm::to_lower_copy( expName ) << ".skd for experiment "
       << expName << "\n";
    of << " Source      Start      DURATIONS           \n";
    of << " name     yyddd-hhmmss   ";
    for ( const auto &any : network.getStations() ) {
        of << any.getAlternativeName() << "  ";
    }
    of << "\n";
    vector<char> found( network.getNSta(), false );
    int counter = 0;
    for ( const auto &scan : scans ) {
        of << scan.toSkedOutputTimes( sourceList.getSource( scan.getSourceId() ), network.getNSta() );
        scan.includesStations( found );
        if ( counter > 5 || all_of( found.begin(), found.end(), []( bool v ) { return v; } ) ) {
            break;
        }
        ++counter;
    }

    found = vector<char>( network.getNSta(), false );
    of << " Last observations\n";
    unsigned long i = scans.size() - 1;
    counter = 0;
    for ( ; i >= 0; --i ) {
        scans[i].includesStations( found );
        if ( counter > 5 || all_of( found.begin(), found.end(), []( bool v ) { return v; } ) ) {
            break;
        }
        ++counter;
    }
    for ( ; i < scans.size(); ++i ) {
        const auto &scan = scans[i];
        of << scan.toSkedOutputTimes( sourceList.getSource( scan.getSourceId() ), network.getNSta() );
    }
}

void OperationNotes::calibrators_skdStyle( const string &expName, const Network &network, const SourceList &sourceList,
                                           const std::vector<Scan> &scans ) {
    bool first = true;
    for ( const auto &scan : scans ) {
        if ( scan.getType() == Scan::ScanType::calibrator ) {
            if ( first ) {
                of << " Calibrator scans\n";
                of << " Observation listing from file " << boost::algorithm::to_lower_copy( expName )
                   << ".skd for experiment " << expName << "\n";
                of << " Source      Start      DURATIONS           \n";
                of << " name     yyddd-hhmmss   ";
                for ( const auto &any : network.getStations() ) {
                    of << any.getAlternativeName() << "  ";
                }
                of << "\n";
            }
            first = false;
            of << scan.toSkedOutputTimes( sourceList.getSource( scan.getSourceId() ), network.getNSta() );
        }
    }
    if ( !first ) {
        of << "===========================================================\n";
    }
}


void OperationNotes::displayStationStatistics( const Network &network ) {
    of << "number of scans per 15 minutes:\n";
    of << util::numberOfScans2char_header() << "\n";
    of << ".-------------------------------------------------------------"
          "----------------------------------------------------------------------------.\n";
    of << "|          time since session start (1 char equals 15 minutes)"
          "                                             | #SCANS #OBS |   OBS Time [s] |\n";
    of << "| STATION |0   1   2   3   4   5   6   7   8   9   10  11  12 "
          " 13  14  15  16  17  18  19  20  21  22  23  |             |   sum  average |\n";
    of << "|---------|+---+---+---+---+---+---+---+---+---+---+---+---+--"
          "-+---+---+---+---+---+---+---+---+---+---+---|-------------|----------------|\n";
    for ( const auto &thisStation : network.getStations() ) {
        of << boost::format( "| %8s|" ) % thisStation.getName();
        const Station::Statistics &stat = thisStation.getStatistics();
        const auto &time_sta = stat.scanStartTimes;
        unsigned int timeStart = 0;
        unsigned int timeEnd = 900;
        for ( int j = 0; j < 96; ++j ) {
            long c = count_if( time_sta.begin(), time_sta.end(),
                               [timeEnd, timeStart]( unsigned int k ) { return k >= timeStart && k < timeEnd; } );
            char flag = util::numberOfScans2char( c );
            of << flag;

            timeEnd += 900;
            timeStart += 900;
        }
        of << boost::format( "| %6d %4d " ) % thisStation.getNTotalScans() % thisStation.getNObs();
        of << boost::format( "| %5d %8.1f |\n" ) % thisStation.getStatistics().totalObservingTime %
                  ( static_cast<double>( thisStation.getStatistics().totalObservingTime ) /
                    static_cast<double>( thisStation.getNTotalScans() ) );
    }
    of << "'--------------------------------------------------------------"
          "---------------------------------------------------------------------------'\n\n";
}


void OperationNotes::displaySourceStatistics( const SourceList &sourceList ) {
    of << "number of available sources:   " << sourceList.getNSrc() << "\n";

    int n_name = 8;

    long number = 0;
    for ( const auto &any : sourceList.getSources() ) {
        if ( any->getNTotalScans() > 0 ) {
            ++number;
        }
        if ( any->getName().size() > n_name ) {
            n_name = any->getName().size();
        }
    }
    long nQuasars = 0;
    for ( const auto &any : sourceList.getQuasars() ) {
        if ( any->getNTotalScans() > 0 ) {
            ++nQuasars;
        }
    }
    long nSatellites = 0;
    for ( const auto &any : sourceList.getSatellites() ) {
        if ( any->getNTotalScans() > 0 ) {
            ++nSatellites;
        }
    }
    //    long number = count_if( sourceList.getSources().begin(), sourceList.getSources().end(),
    //                  []( const auto &any ) { return any->getNTotalScans() > 0; } );
    //    long nQuasars = count_if( sourceList.getQuasars().begin(), sourceList.getQuasars().end(),
    //                  []( const auto &any ) { return any->getNTotalScans() > 0; } );
    //    long nSatellites = count_if( sourceList.getSatellites().begin(), sourceList.getSatellites().end(),
    //                  []( const auto &any ) { return any->getNTotalScans() > 0; } );

    string name_format = ( boost::format( "| %%-%ds |" ) % n_name ).str();
    n_name += 2;

    if ( nSatellites > 0 ) {
        of << "number of scheduled quasars:    " << nQuasars << "\n";
        of << "number of scheduled satellites: " << nSatellites << "\n";
    } else {
        of << "number of scheduled sources:    " << number << "\n";
    }
    of << "number of scans per 15 minutes:\n";
    of << util::numberOfScans2char_header() << "\n";
    of << "." << string( n_name, '-' )
       << "-----------------------------------------------------"
          "--------------------------------------------------------------------------------------.\n";
    of << "|" << string( n_name, ' ' )
       << " time since session start (1 char equals 15 minutes)"
          "                                             | #SCANS #OBS |   OBS Time [s] | independ |\n";
    of << "" << boost::format( name_format ) % "SOURCE"
       << "0   1   2   3   4   5   6   7   8   9   10  11  12 "
          " 13  14  15  16  17  18  19  20  21  22  23  |             |   sum  average | closures |\n";
    of << "|" << string( n_name, '-' )
       << "|+---+---+---+---+---+---+---+---+---+---+---+---+--"
          "-+---+---+---+---+---+---+---+---+---+---+---|-------------|----------------|----------|\n";
    for ( const auto &thisSource : sourceList.getQuasars() ) {
        const AbstractSource::Statistics &stat = thisSource->getStatistics();
        const auto &time_sta = stat.scanStartTimes;

        if ( thisSource->getNObs() == 0 ) {
            continue;
        }
        of << boost::format( name_format ) % thisSource->getName();

        unsigned int timeStart = 0;
        unsigned int timeEnd = 900;
        for ( int j = 0; j < 96; ++j ) {
            long c = count_if( time_sta.begin(), time_sta.end(),
                               [timeEnd, timeStart]( unsigned int k ) { return k >= timeStart && k < timeEnd; } );
            char flag = util::numberOfScans2char( c );
            of << flag;

            timeEnd += 900;
            timeStart += 900;
        }
        of << boost::format( "| %6d %4d " ) % thisSource->getNTotalScans() % thisSource->getNObs();
        of << boost::format( "| %5d %8.1f " ) % thisSource->getStatistics().totalObservingTime %
                  ( static_cast<double>( thisSource->getStatistics().totalObservingTime ) /
                    static_cast<double>( thisSource->getNTotalScans() ) );
        of << boost::format( "| %8d |\n") % thisSource->getNClosures();
    }
    bool first = true;
    for ( const auto &thisSource : sourceList.getSatellites() ) {
        const AbstractSource::Statistics &stat = thisSource->getStatistics();
        const auto &time_sta = stat.scanStartTimes;

        if ( thisSource->getNObs() == 0 ) {
            continue;
        }
        if ( first ) {
            of << "|" << string( n_name, '-' )
               << "|+---+---+---+---+---+---+---+---+---+---+---+---+--"
                  "-+---+---+---+---+---+---+---+---+---+---+---|-------------|----------------|----------|\n";
            first = false;
        }
        of << boost::format( name_format ) % thisSource->getName();

        unsigned int timeStart = 0;
        unsigned int timeEnd = 900;
        for ( int j = 0; j < 96; ++j ) {
            long c = count_if( time_sta.begin(), time_sta.end(),
                               [timeEnd, timeStart]( unsigned int k ) { return k >= timeStart && k < timeEnd; } );
            char flag = util::numberOfScans2char( c );
            of << flag;

            timeEnd += 900;
            timeStart += 900;
        }
        of << boost::format( "| %6d %4d " ) % thisSource->getNTotalScans() % thisSource->getNObs();
        of << boost::format( "| %5d %8.1f |\n" ) % thisSource->getStatistics().totalObservingTime %
                  ( static_cast<double>( thisSource->getStatistics().totalObservingTime ) /
                    static_cast<double>( thisSource->getNTotalScans() ) );
    }
    of << "'" << string( n_name, '-' )
       << "-----------------------------------------------------"
          "--------------------------------------------------------------------------------------'\n\n";
}

void OperationNotes::displayBaselineTimeStatistics( const Network &network ) {
    of << "number of scans per 15 minutes:\n";
    of << util::numberOfScans2char_header() << "\n";
    of << ".-------------------------------------------------------------"
          "----------------------------------------------------------------------.\n";
    of << "|           time since session start (1 char equals 15 minutes)"
          "                                             | #OBS |   OBS Time [s] |\n";
    of << "| BASELINE |0   1   2   3   4   5   6   7   8   9   10  11  12 "
          " 13  14  15  16  17  18  19  20  21  22  23  |      |   sum  average |\n";
    of << "|----------|+---+---+---+---+---+---+---+---+---+---+---+---+--"
          "-+---+---+---+---+---+---+---+---+---+---+---|------|----------------|\n";
    for ( const auto &thisBaseline : network.getBaselines() ) {
        of << boost::format( "| %=9s|" ) % thisBaseline.getName();
        const Baseline::Statistics &bl = thisBaseline.getStatistics();
        const auto &time_sta = bl.scanStartTimes;
        unsigned int timeStart = 0;
        unsigned int timeEnd = 900;
        for ( int j = 0; j < 96; ++j ) {
            long c = count_if( time_sta.begin(), time_sta.end(),
                               [timeEnd, timeStart]( unsigned int k ) { return k >= timeStart && k < timeEnd; } );
            char flag = util::numberOfScans2char( c );
            of << flag;

            timeEnd += 900;
            timeStart += 900;
        }
        of << boost::format( "|%5d " ) % thisBaseline.getNObs() ;
        of << boost::format( "| %5d %8.1f |\n" ) % thisBaseline.getStatistics().totalObservingTime %
              ( static_cast<double>( thisBaseline.getStatistics().totalObservingTime ) /
                static_cast<double>( thisBaseline.getNObs() ) );
    }
    of << "'--------------------------------------------------------"
          "---------------------------------------------------------------------------'\n\n";
}

void OperationNotes::displayMostSubnets( const vector<Scan> &scans, const Network &network ) {
    map<vector<unsigned long>, int> storage;
    for ( const auto &scan : scans){
        vector<unsigned long> staids =  scan.getStationIds();
        sort(staids.begin(), staids.end());
        if ( storage.find(staids) == storage.end()){
            storage[staids] = 1;
        } else {
            storage[staids] += 1;
        }
    }

    // copy to vector
    vector<pair<string, int>> storage_vec;
    for (const auto &any : storage){
        const auto &staids = any.first;
        string statlcs;
        for (unsigned long staid : staids){
            statlcs.append(network.getStation(staid).getAlternativeName());
        }
        int val = any.second;
        storage_vec.emplace_back(statlcs, val);
    }
    sort(storage_vec.begin(), storage_vec.end(), [](const auto &a, const auto &b) {
        return a.second > b.second;
    });

    if ( storage_vec.size() == 1){
        return;
    }

    int c_max = 50;
    int length = 0;
    int c = 0;
    for(const auto &any : storage_vec){
        if(any.first.size() > length){
            length = any.first.size();
        }
        ++c;
        if(c > c_max){
            break;
        }
    }

    c = 0;
    int columns = 5;
    if (length > 14) {
        columns = 4;
        c_max = 48;
    }
    if (length > 20 ) {
        columns = 3;
        c_max = 48;
    }
    if ( storage_vec.size() < 5 ){
        columns = storage_vec.size();
    }

    string fmt = (boost::format(" %%-%ds %%4d (%%5.2f%%%%) |") % length).str();
    of << "list of most observed subnetworks:\n";

    of << ".";
    for (int i =0; i<columns; ++i) {
        for(int j = 0; j<length+4+3+9; ++j){
            of << "-";
        }
        if(i<columns-1){
            of << "-";
        }
    }
    of << ".\n";

    of << "|";
    for (int i =0; i<columns; ++i) {
        if (length>7){
            of << boost::format((boost::format(" %%-%ds               |") % length).str()) % "network";
        }else{
            of << boost::format((boost::format(" %%-%ds               |") % length).str()) % "net.";
        }
    }
    of << "\n";

    of << "|";
    for (int i =0; i<columns; ++i) {
        for(int j = 0; j<length+4+3+9; ++j){
            of << "-";
        }
        if(i<columns-1){
            of << "-";
        }
    }
    of << "|\n";

    of << "|";
    for(const auto &any : storage_vec){
        if(c>0 & c % columns == 0){
            of << "\n|";
        }
        of << boost::format(fmt) % any.first % any.second % (static_cast<double>(any.second)/scans.size()*100.);
        ++c;
        if(c >= c_max){
            break;
        }
    }
    of << "\n";

    of << "'";
    for (int i =0; i<columns; ++i) {
        for(int j = 0; j<length+4+3+9; ++j){
            of << "-";
        }
        if(i<columns-1){
            of << "-";
        }
    }
    of << "'\n";


    of << "\n\n";
}


void OperationNotes::displayNstaStatistics( const Network &network, const std::vector<Scan> &scans ) {
    unsigned long nsta = network.getNSta();
    vector<int> nstas( nsta + 1, 0 );
    int obs = 0;
    int intObs = 0;
    for ( const auto &scan : scans ) {
        ++nstas[scan.getNSta()];
        obs += scan.getNObs();
        for ( int i = 0; i < scan.getNObs(); ++i ) {
            const auto &tobs = scan.getObservation( i );
            unsigned long idx1 = *scan.findIdxOfStationId( tobs.getStaid1() );
            unsigned long idx2 = *scan.findIdxOfStationId( tobs.getStaid2() );
            intObs += scan.getTimes().getObservingDuration( idx1, idx2 );
        }
    }

    unsigned long sum = scans.size();

    for ( int i = 2; i <= nsta; ++i ) {
        of << boost::format( " Number of %2d-station scans:   %4d (%6.2f %%)\n" ) % i % nstas[i] %
                  ( static_cast<double>( nstas[i] ) / static_cast<double>( sum ) * 100 );
    }
    of << boost::format( "Total number of scans:    %9d\n" ) % ( scans.size() );
    of << boost::format( "Total number of obs:      %9d\n" ) % obs;
    of << boost::format( "Total integrated obs-time:%9d\n" ) % intObs;
    of << boost::format( "Average obs-time:         %9.1f\n" ) % ( static_cast<double>( intObs ) / obs );
}


void OperationNotes::displayScanDurationStatistics( const Network &network, const std::vector<Scan> &scans ) {
    unsigned long nsta = network.getNSta();
    of << "scan observing durations:\n";
    vector<vector<vector<unsigned int>>> bl_durations( nsta, vector<vector<unsigned int>>( nsta ) );
    vector<unsigned int> maxScanDurations;

    for ( const auto &any : scans ) {
        unsigned long nbl = any.getNObs();
        maxScanDurations.push_back( any.getTimes().getObservingDuration() );

        for ( int i = 0; i < nbl; ++i ) {
            const Observation &obs = any.getObservation( i );
            unsigned long staid1 = obs.getStaid1();
            unsigned long staid2 = obs.getStaid2();
            unsigned int obs_duration = obs.getObservingTime();

            if ( staid1 < staid2 ) {
                swap( staid1, staid2 );
            }
            bl_durations[staid1][staid2].push_back( obs_duration );
        }
    }
    if ( maxScanDurations.empty() ) {
        return;
    }
    unsigned int maxMax = *max_element( maxScanDurations.begin(), maxScanDurations.end() );
    maxMax = maxMax / 10 * 10 + 10;
    unsigned int cache_size = ( 1 + maxMax / 100 ) * 10;
    vector<unsigned int> bins;

    unsigned int upper_bound = 0;
    while ( upper_bound < maxMax + cache_size ) {
        bins.push_back( upper_bound );
        upper_bound += cache_size;
    }

    vector<unsigned int> hist( bins.size() - 1, 0 );
    for ( unsigned int any : maxScanDurations ) {
        int i = 1;
        while ( any > bins[i] ) {
            ++i;
        }
        ++hist[i - 1];
    }

    of << "scan duration:\n";
    for ( int i = 0; i < hist.size(); ++i ) {
        of << boost::format( "%3d-%3d | " ) % bins[i] % ( bins[i + 1] - 1 );
        double percent = 100 * static_cast<double>( hist[i] ) / static_cast<double>( maxScanDurations.size() );
        percent = round( percent );
        for ( int j = 0; j < percent; ++j ) {
            of << "+";
        }
        of << "\n";
    }
    of << "\n";

    of << "scheduled scan length:\n";
    of << ".-----------------------------------------------------------------------------------.\n";
    of << "| S1-S2 |  min    10%    50%    90%    95%  97.5%    99%    max   |    sum  average |\n";
    of << "|-------|---------------------------------------------------------|-----------------|\n";

    {
        auto n = static_cast<int>( maxScanDurations.size() - 1 );
        sort( maxScanDurations.begin(), maxScanDurations.end() );
        of << boost::format( "|  ALL  | " );
        of << boost::format( "%4d   " ) % maxScanDurations[0];
        of << boost::format( "%4d   " ) % maxScanDurations[n * 0.1];
        of << boost::format( "%4d   " ) % maxScanDurations[n / 2];
        of << boost::format( "%4d   " ) % maxScanDurations[n * 0.9];
        of << boost::format( "%4d   " ) % maxScanDurations[n * 0.95];
        of << boost::format( "%4d   " ) % maxScanDurations[n * 0.975];
        of << boost::format( "%4d   " ) % maxScanDurations[n * 0.99];
        of << boost::format( "%4d   " ) % maxScanDurations[n];
        unsigned int sum = accumulate( maxScanDurations.begin(), maxScanDurations.end(), 0u );
        double average = static_cast<double>( sum ) / ( n + 1 );
        of << boost::format( "| %6d %8.1f |" ) % sum % average;
        of << "\n";
    }

    of << "|-------|---------------------------------------------------------|-----------------|\n";

    for ( unsigned long i = 1; i < nsta; ++i ) {
        for ( unsigned long j = 0; j < i; ++j ) {
            vector<unsigned int> &this_duration = bl_durations[i][j];
            if ( this_duration.empty() ) {
                continue;
            }
            int n = (int)this_duration.size() - 1;
            sort( this_duration.begin(), this_duration.end() );
            of << boost::format( "| %5s | " ) % network.getBaseline( i, j ).getName();
            of << boost::format( "%4d   " ) % this_duration[0];
            of << boost::format( "%4d   " ) % this_duration[n * 0.1];
            of << boost::format( "%4d   " ) % this_duration[n / 2];
            of << boost::format( "%4d   " ) % this_duration[n * 0.9];
            of << boost::format( "%4d   " ) % this_duration[n * 0.95];
            of << boost::format( "%4d   " ) % this_duration[n * 0.975];
            of << boost::format( "%4d   " ) % this_duration[n * 0.99];
            of << boost::format( "%4d   " ) % this_duration[n];
            unsigned int sum = accumulate( this_duration.begin(), this_duration.end(), 0u );
            double average = static_cast<double>( sum ) / ( n + 1 );
            of << boost::format( "| %6d %8.1f |" ) % sum % average;
            of << "\n";
        }
    }
    of << "'-----------------------------------------------------------------------------------'\n\n";
}


void OperationNotes::displayTimeStatistics( const Network &network,
                                            const std::shared_ptr<const ObservingMode> &obsModes ) {
    unsigned long nstaTotal = network.getNSta();

    of << "                 ";
    for ( const auto &station : network.getStations() ) {
        of << boost::format( "    %s " ) % station.getAlternativeName();
    }
    of << "   Avg\n";

    of << " % obs. time:    ";
    vector<double> obsPer;
    for ( const auto &station : network.getStations() ) {
        int t = station.getStatistics().totalObservingTime;
        obsPer.push_back( static_cast<double>( t ) / static_cast<double>( TimeSystem::duration ) * 100 );
    }
    for ( auto p : obsPer ) {
        of << boost::format( "%6.2f " ) % p;
    }
    of << boost::format( "%6.2f " ) % ( accumulate( obsPer.begin(), obsPer.end(), 0.0 ) / ( network.getNSta() ) );
    of << "\n";

    of << " % cal. time:    ";
    vector<double> preobPer;
    for ( const auto &station : network.getStations() ) {
        int t = station.getStatistics().totalPreobTime;
        preobPer.push_back( static_cast<double>( t ) / static_cast<double>( TimeSystem::duration ) * 100 );
    }
    for ( auto p : preobPer ) {
        of << boost::format( "%6.2f " ) % p;
    }
    of << boost::format( "%6.2f " ) % ( accumulate( preobPer.begin(), preobPer.end(), 0.0 ) / ( network.getNSta() ) );
    of << "\n";

    of << " % slew time:    ";
    vector<double> slewPer;
    for ( const auto &station : network.getStations() ) {
        int t = station.getStatistics().totalSlewTime;
        slewPer.push_back( static_cast<double>( t ) / static_cast<double>( TimeSystem::duration ) * 100 );
    }
    for ( auto p : slewPer ) {
        of << boost::format( "%6.2f " ) % p;
    }
    of << boost::format( "%6.2f " ) % ( accumulate( slewPer.begin(), slewPer.end(), 0.0 ) / ( network.getNSta() ) );
    of << "\n";

    of << " % idle time:    ";
    vector<double> idlePer;
    for ( const auto &station : network.getStations() ) {
        int t = station.getStatistics().totalIdleTime;
        idlePer.push_back( static_cast<double>( t ) / static_cast<double>( TimeSystem::duration ) * 100 );
    }
    for ( auto p : idlePer ) {
        of << boost::format( "%6.2f " ) % p;
    }
    of << boost::format( "%6.2f " ) % ( accumulate( idlePer.begin(), idlePer.end(), 0.0 ) / ( network.getNSta() ) );
    of << "\n";

    of << " % field system: ";
    vector<double> fieldPer;
    for ( const auto &station : network.getStations() ) {
        int t = station.getStatistics().totalFieldSystemTime;
        fieldPer.push_back( static_cast<double>( t ) / static_cast<double>( TimeSystem::duration ) * 100 );
    }
    for ( auto p : fieldPer ) {
        of << boost::format( "%6.2f " ) % p;
    }
    of << boost::format( "%6.2f " ) % ( accumulate( fieldPer.begin(), fieldPer.end(), 0.0 ) / ( network.getNSta() ) );
    of << "\n";

    of << " total # scans:  ";
    vector<int> scans;
    for ( const auto &station : network.getStations() ) {
        scans.push_back( station.getNTotalScans() );
    }
    for ( auto p : scans ) {
        of << boost::format( "%6d " ) % static_cast<double>( p );
    }
    of << boost::format( "%6d " ) % roundl( accumulate( scans.begin(), scans.end(), 0.0 ) / ( network.getNSta() ) );
    of << "\n";

    of << " # scans/hour:   ";
    vector<double> scansPerH;
    for ( const auto &station : network.getStations() ) {
        scansPerH.push_back( static_cast<double>( station.getNTotalScans() ) / ( TimeSystem::duration / 3600. ) );
    }
    for ( auto p : scansPerH ) {
        of << boost::format( "%6.2f " ) % p;
    }
    of << boost::format( "%6.2f " ) % ( accumulate( scansPerH.begin(), scansPerH.end(), 0.0 ) / ( network.getNSta() ) );
    of << "\n";

    of << " total # obs:    ";
    vector<int> obs;
    for ( const auto &station : network.getStations() ) {
        obs.push_back( station.getNObs() );
    }
    for ( auto p : obs ) {
        of << boost::format( "%6d " ) % static_cast<double>( p );
    }
    of << boost::format( "%6d " ) % roundl( accumulate( obs.begin(), obs.end(), 0.0 ) / ( network.getNSta() ) );
    of << "\n";

    of << " # obs/hour:     ";
    vector<double> obsPerH;
    for ( const auto &station : network.getStations() ) {
        obsPerH.push_back( static_cast<double>( station.getNObs() ) / ( TimeSystem::duration / 3600. ) );
    }
    for ( auto p : obsPerH ) {
        of << boost::format( "%6.2f " ) % p;
    }
    of << boost::format( "%6.2f " ) % ( accumulate( obsPerH.begin(), obsPerH.end(), 0.0 ) / ( network.getNSta() ) );
    of << "\n";

    of << " Avg scan (sec): ";
    vector<double> scanSec;
    for ( const auto &station : network.getStations() ) {
        scanSec.push_back( static_cast<double>( station.getStatistics().totalObservingTime ) /
                           station.getNTotalScans() );
    }
    for ( auto p : scanSec ) {
        of << boost::format( "%6.2f " ) % p;
    }
    of << boost::format( "%6.2f " ) % ( accumulate( scanSec.begin(), scanSec.end(), 0.0 ) / ( network.getNSta() ) );
    of << "\n";

    if ( ObservingMode::type != ObservingMode::Type::simple ) {
        of << " # Mk5 tracks:   ";
        for ( const auto &station : network.getStations() ) {
            const auto &tracksBlock = obsModes->getMode( 0 )->getTracks( station.getId() );
            if ( tracksBlock.is_initialized() ) {
                int tracks = tracksBlock.get()->numberOfTracks();
                of << boost::format( "%6d " ) % tracks;
            } else {
                of << boost::format( "%6s " ) % "-";
            }
        }
        of << "\n";
    }

    of << " Total TB:       ";
    vector<double> total_tb;
    for ( const auto &station : network.getStations() ) {
        double obsFreq = obsModes->getMode( 0 )->recordingRate( station.getId() ) / 1e6;
        int t = station.getStatistics().totalObservingTime;

        total_tb.push_back( static_cast<double>( t ) * obsFreq / ( 1000 * 1000 * 8 ) );
    }
    for ( auto p : total_tb ) {
        of << boost::format( "%6.2f " ) % p;
    }
    of << boost::format( "%6.2f " ) %
              ( accumulate( total_tb.begin(), total_tb.end(), 0.0 ) / ( network.getNSta() ) );
    of << "\n";
}


void OperationNotes::displayAstronomicalParameters() {
    of << ".------------------------------------------.\n";
    of << "| sun position:        | earth velocity:   |\n";
    of << "|----------------------|-------------------|\n";
    of << "| RA:   " << util::ra2dms( AstronomicalParameters::sun_ra[1] ) << " "
       << boost::format( "| x: %8.0f [m/s] |\n" ) % AstronomicalParameters::earth_velocity[0];
    of << "| DEC: " << util::dc2hms( AstronomicalParameters::sun_dec[1] ) << " "
       << boost::format( "| y: %8.0f [m/s] |\n" ) % AstronomicalParameters::earth_velocity[1];
    of << "|                      "
       << boost::format( "| z: %8.0f [m/s] |\n" ) % AstronomicalParameters::earth_velocity[2];
    of << "'------------------------------------------'\n\n";

    of << ".--------------------------------------------------------------------.\n";
    of << "| earth nutation:                                                    |\n";
    of << boost::format( "| %=19s | %=14s %=14s %=14s |\n" ) % "time" % "X" % "Y" % "S";
    of << "|---------------------|----------------------------------------------|\n";
    for ( int i = 0; i < AstronomicalParameters::earth_nutTime.size(); ++i ) {
        of << boost::format( "| %19s | %+14.6e %+14.6e %+14.6e |\n" ) %
                  TimeSystem::time2string( AstronomicalParameters::earth_nutTime[i] ) %
                  AstronomicalParameters::earth_nutX[i] % AstronomicalParameters::earth_nutY[i] %
                  AstronomicalParameters::earth_nutS[i];
    }
    of << "'--------------------------------------------------------------------'\n\n";
}


void OperationNotes::displaySNRSummary( const Network &network, const SourceList &sourceList,
                                        const std::vector<Scan> &scans,
                                        const std::shared_ptr<const ObservingMode> &obsModes ) {
    map<string, vector<vector<double>>> SNRs;
    const auto &bands = ObservingMode::bands;

    for ( const auto &band : bands ) {
        SNRs[band] = vector<vector<double>>( network.getNBls() );
    }

    for ( const auto &scan : scans ) {
        unsigned long srcid = scan.getSourceId();
        const auto &source = sourceList.getSource( srcid );

        for ( int i = 0; i < scan.getNObs(); ++i ) {
            const auto &obs = scan.getObservation( i );
            unsigned long blid = obs.getBlid();

            unsigned long staid1 = obs.getStaid1();
            unsigned long staid2 = obs.getStaid2();

            const auto &sta1 = network.getStation( staid1 );
            const auto &sta2 = network.getStation( staid2 );

            double el1 = scan.getPointingVector( static_cast<int>( *scan.findIdxOfStationId( staid1 ) ) ).getEl();
            double el2 = scan.getPointingVector( static_cast<int>( *scan.findIdxOfStationId( staid2 ) ) ).getEl();

            double date1 = 2400000.5;
            unsigned int startTime = obs.getStartTime();
            double date2 = TimeSystem::mjdStart + static_cast<double>( startTime ) / 86400.0;
            double gmst = iauGmst82( date1, date2 );

            unsigned int duration = obs.getObservingTime();

            for ( const auto &band : bands ) {
                double observedFlux;
                if ( source->hasFluxInformation( band ) ) {
                    // calculate observed flux density for each band
                    observedFlux = source->observedFlux( band, startTime, gmst, network.getDxyz( staid1, staid2 ) );
                } else if ( ObservingMode::sourceBackup[band] == ObservingMode::Backup::internalModel ) {
                    // calculate observed flux density based on model
                    double wavelength = ObservingMode::wavelengths[band];
                    observedFlux =
                        source->observedFlux_model( wavelength, startTime, gmst, network.getDxyz( staid1, staid2 ) );
                } else {
                    observedFlux = 1e-3;
                }

                double SEFD_sta1 = sta1.getEquip().getSEFD( band, el1 );
                double SEFD_sta2 = sta2.getEquip().getSEFD( band, el2 );

                double recordingRate = obsModes->getMode( 0 )->recordingRate( staid1, staid2, band );
                double efficiency = obsModes->getMode( 0 )->efficiency( sta1.getId(), sta2.getId() );
                double snr =
                    efficiency * observedFlux / ( sqrt( SEFD_sta1 * SEFD_sta2 ) ) * sqrt( recordingRate * duration );

                SNRs[band][blid].push_back( snr );
            }
        }
    }

    unsigned long nsta = network.getNSta();
    for ( const auto &snr : SNRs ) {
        of << "average theoretical SNR for " << snr.first << "-band per baseline:\n";
        of << ".-----------";
        for ( int i = 0; i < nsta - 1; ++i ) {
            of << "----------";
        }
        of << "-----------";
        of << "----------.\n";

        of << boost::format( "| %8s |" ) % "STATIONS";
        for ( const auto &any : network.getStations() ) {
            of << boost::format( " %8s " ) % any.getName();
        }
        of << "|";
        of << boost::format( " %8s " ) % "AVERAGE";
        of << "|\n";

        of << "|----------|";
        for ( int i = 0; i < nsta - 1; ++i ) {
            of << "----------";
        }
        of << "----------|";
        of << "----------|\n";

        vector<double> sumSNR( nsta, 0.0 );
        vector<int> counterSNR( nsta, 0 );
        for ( unsigned long staid1 = 0; staid1 < nsta; ++staid1 ) {
            of << boost::format( "| %8s |" ) % network.getStation( staid1 ).getName();
            for ( unsigned long staid2 = 0; staid2 < nsta; ++staid2 ) {
                if ( staid2 < staid1 + 1 ) {
                    of << "          ";
                } else {
                    unsigned long blid = network.getBaseline( staid1, staid2 ).getId();
                    if ( snr.second[blid].empty() ) {
                        of << "        - ";
                    } else {
                        double SNR = accumulate( snr.second[blid].begin(), snr.second[blid].end(), 0.0 );
                        int n = static_cast<int>( snr.second[blid].size() );
                        sumSNR[staid1] += SNR;
                        counterSNR[staid1] += n;
                        sumSNR[staid2] += SNR;
                        counterSNR[staid2] += n;

                        of << boost::format( " %8.2f " ) % ( SNR / n );
                    }
                }
            }
            of << "|";
            of << boost::format( " %8.2f " ) % ( sumSNR[staid1] / counterSNR[staid1] );
            of << "|\n";
        }

        of << "'-----------";
        for ( int i = 0; i < nsta - 1; ++i ) {
            of << "----------";
        }
        of << "-----------";
        of << "----------'\n\n";
    }
}


void OperationNotes::listKeys( const Network &network ) {
    of << " Key:     ";
    int i = 0;
    for ( const auto &station : network.getStations() ) {
        of << boost::format( "%2s=%-8s   " ) % station.getAlternativeName() % station.getName();
        ++i;
        if ( i % 5 == 0 && i < network.getNSta() ) {
            of << "\n          ";
        }
    }
    of << "\n\n";
}


void OperationNotes::displaySkyCoverageScore( const Network &network ) {
    vector<double> a13m8;
    vector<double> a25m8;
    vector<double> a37m8;
    vector<double> a13m15;
    vector<double> a25m15;
    vector<double> a37m15;
    vector<double> a13m30;
    vector<double> a25m30;
    vector<double> a37m30;
    vector<double> a13m60;
    vector<double> a25m60;
    vector<double> a37m60;
    for ( const auto &station : network.getStations() ) {
        unsigned long id = station.getId();
        const auto &map = network.getStaid2skyCoverageId();
        unsigned long skyCovId = map.at( id );
        const auto &skyCov = network.getSkyCoverage( skyCovId );
        a13m8.push_back( skyCov.getSkyCoverageScore_a13m8() );
        a25m8.push_back( skyCov.getSkyCoverageScore_a25m8() );
        a37m8.push_back( skyCov.getSkyCoverageScore_a37m8() );
        a13m15.push_back( skyCov.getSkyCoverageScore_a13m15() );
        a25m15.push_back( skyCov.getSkyCoverageScore_a25m15() );
        a37m15.push_back( skyCov.getSkyCoverageScore_a37m15() );
        a13m30.push_back( skyCov.getSkyCoverageScore_a13m30() );
        a25m30.push_back( skyCov.getSkyCoverageScore_a25m30() );
        a37m30.push_back( skyCov.getSkyCoverageScore_a37m30() );
        a13m60.push_back( skyCov.getSkyCoverageScore_a13m60() );
        a25m60.push_back( skyCov.getSkyCoverageScore_a25m60() );
        a37m60.push_back( skyCov.getSkyCoverageScore_a37m60() );
    }

    of << "sky coverage score (1 means perfect distribution)\n";
    of << ".--------------------";
    for ( int i = 0; i < network.getNSta(); ++i ) {
        of << boost::format( "----------" );
    }
    of << "-----------.\n";

    of << "|                   |";
    for ( const auto &station : network.getStations() ) {
        of << boost::format( " %8s " ) % station.getName();
    }
    of << "| average  |\n";

    of << "|-------------------|";
    for ( int i = 0; i < network.getNSta(); ++i ) {
        of << boost::format( "----------" );
    }
    of << "|----------|\n";

    of << "| 13 areas @  8 min |";
    for ( double v : a13m8 ) {
        of << boost::format( " %8.2f " ) % v;
    }
    of << boost::format( "| %8.2f |\n" ) % ( accumulate( a13m8.begin(), a13m8.end(), 0.0 ) / network.getNSta() );

    of << "| 25 areas @  8 min |";
    for ( double v : a25m8 ) {
        of << boost::format( " %8.2f " ) % v;
    }
    of << boost::format( "| %8.2f |\n" ) % ( accumulate( a25m8.begin(), a25m8.end(), 0.0 ) / network.getNSta() );

    of << "| 37 areas @  8 min |";
    for ( double v : a37m8 ) {
        of << boost::format( " %8.2f " ) % v;
    }
    of << boost::format( "| %8.2f |\n" ) % ( accumulate( a37m8.begin(), a37m8.end(), 0.0 ) / network.getNSta() );

    of << "|--------------------";
    for ( const auto &station : network.getStations() ) {
        of << boost::format( "----------" );
    }
    of << "-----------|\n";

    of << "| 13 areas @ 15 min |";
    for ( double v : a13m15 ) {
        of << boost::format( " %8.2f " ) % v;
    }
    of << boost::format( "| %8.2f |\n" ) % ( accumulate( a13m15.begin(), a13m15.end(), 0.0 ) / network.getNSta() );

    of << "| 25 areas @ 15 min |";
    for ( double v : a25m15 ) {
        of << boost::format( " %8.2f " ) % v;
    }
    of << boost::format( "| %8.2f |\n" ) % ( accumulate( a25m15.begin(), a25m15.end(), 0.0 ) / network.getNSta() );

    of << "| 37 areas @ 15 min |";
    for ( double v : a37m15 ) {
        of << boost::format( " %8.2f " ) % v;
    }
    of << boost::format( "| %8.2f |\n" ) % ( accumulate( a37m15.begin(), a37m15.end(), 0.0 ) / network.getNSta() );

    of << "|--------------------";
    for ( const auto &station : network.getStations() ) {
        of << boost::format( "----------" );
    }
    of << "-----------|\n";

    of << "| 13 areas @ 30 min |";
    for ( double v : a13m30 ) {
        of << boost::format( " %8.2f " ) % v;
    }
    of << boost::format( "| %8.2f |\n" ) % ( accumulate( a13m30.begin(), a13m30.end(), 0.0 ) / network.getNSta() );

    of << "| 25 areas @ 30 min |";
    for ( double v : a25m30 ) {
        of << boost::format( " %8.2f " ) % v;
    }
    of << boost::format( "| %8.2f |\n" ) % ( accumulate( a25m30.begin(), a25m30.end(), 0.0 ) / network.getNSta() );

    of << "| 37 areas @ 30 min |";
    for ( double v : a37m30 ) {
        of << boost::format( " %8.2f " ) % v;
    }
    of << boost::format( "| %8.2f |\n" ) % ( accumulate( a37m30.begin(), a37m30.end(), 0.0 ) / network.getNSta() );

    of << "|--------------------";
    for ( const auto &station : network.getStations() ) {
        of << boost::format( "----------" );
    }
    of << "-----------|\n";

    of << "| 13 areas @ 60 min |";
    for ( double v : a13m60 ) {
        of << boost::format( " %8.2f " ) % v;
    }
    of << boost::format( "| %8.2f |\n" ) % ( accumulate( a13m60.begin(), a13m60.end(), 0.0 ) / network.getNSta() );

    of << "| 25 areas @ 60 min |";
    for ( double v : a25m60 ) {
        of << boost::format( " %8.2f " ) % v;
    }
    of << boost::format( "| %8.2f |\n" ) % ( accumulate( a25m60.begin(), a25m60.end(), 0.0 ) / network.getNSta() );

    of << "| 37 areas @ 60 min |";
    for ( double v : a37m60 ) {
        of << boost::format( " %8.2f " ) % v;
    }
    of << boost::format( "| %8.2f |\n" ) % ( accumulate( a37m60.begin(), a37m60.end(), 0.0 ) / network.getNSta() );

    of << "'--------------------";
    for ( int i = 0; i < network.getNSta(); ++i ) {
        of << boost::format( "----------" );
    }
    of << "-----------'\n\n";
}


void OperationNotes::contactInformations( std::vector<std::string> &functions, std::vector<std::string> &names,
                                          std::vector<std::string> &emails, std::vector<std::string> &phoneNumbers,
                                          std::vector<std::string> &affiliations,
                                          std::map<int, int> &nameId2affiliationId,
                                          const boost::property_tree::ptree &xml ) {
    const auto &pt = xml.get_child_optional( "VieSchedpp.output.contacts" );
    if ( pt.is_initialized() ) {
        for ( const auto &any : *pt ) {
            string function = any.second.get( "function", "" );
            string name = any.second.get( "name", "" );
            string email = any.second.get( "email", "" );
            string phone = any.second.get( "phone", "" );
            string affiliation = any.second.get( "affiliation", "" );

            functions.push_back( function );
            names.push_back( name );

            emails.push_back( email );
            phoneNumbers.push_back( phone );


            if ( !affiliation.empty() &&
                 find( affiliations.begin(), affiliations.end(), affiliation ) == affiliations.end() ) {
                affiliations.push_back( affiliation );
                nameId2affiliationId.insert( { names.size() - 1, affiliations.size() } );
            } else if ( !affiliation.empty() ) {
                auto it = find( affiliations.begin(), affiliations.end(), affiliation );
                int id = std::distance( affiliations.begin(), it );

                nameId2affiliationId.insert( { names.size() - 1, id + 1 } );
            } else {
                nameId2affiliationId.insert( { names.size() - 1, -1 } );
            }
        }
    }
}
void OperationNotes::mean_el_hist( const vector<Scan> &scans ) {
    vector<double> x = vector<double>(10,0.0);
    for(const auto &scan : scans){
        double el = 0;
        for(int i = 0; i < scan.getNSta(); ++i){
            el += scan.getPointingVector(i).getEl();
        }
        el /= static_cast<double>(scan.getNSta());
        el *= rad2deg;
        int mod = static_cast<int>(floor(el / 10));
        ++x[mod];
    }

    int sum = accumulate(x.begin(), x.end(), 0.0);
    for(double &v : x){
        v /= sum;
    }

    of << "mean elevation histogram:\n";
    int tmp = 0;
    for ( int i = 0; i < 9; ++i ) {
        double val = x[i];
        of << boost::format( "%2d-%2d: %.2f\n" ) % tmp % ( tmp + 10 ) % val;
        tmp += 10;
    }
    of << "\n";
}
void OperationNotes::displaySkyCoverage( const Network &network ) {
    const auto &t = network.getStaid2skyCoverageId();

    of << ".------------------------------------------.\n";
    of << "| ID |   dist | time |   dist   |   time   |\n";
    of << "|    |  [deg] |  [s] |   type   |   type   |\n";
    of << "|------------------------------------------|\n";
    for ( const auto &any : network.getSkyCoverages() ) {
        of << any.paramters2string() << " <-- ";
        for ( const auto &tmp : t ) {
            unsigned long staid = tmp.first;
            unsigned long id = tmp.second;
            if ( id == any.getId() ) {
                of << network.getStation( staid ).getName() << " ";
            }
        }
        of << "\n";
    }
    of << "'------------------------------------------'\n";
    of << "\n";
}
