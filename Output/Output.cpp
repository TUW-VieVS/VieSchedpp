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

#include "Output.h"


using namespace std;
using namespace VieVS;
unsigned long Output::nextId = 0;


Output::Output( Scheduler &sched, std::string path, string fname, int version )
    : VieVS_NamedObject( move( fname ), nextId++ ),
      xml_{sched.xml_},
      network_{std::move( sched.network_ )},
      sources_{std::move( sched.sources_ )},
      scans_{std::move( sched.scans_ )},
      obsModes_{sched.obsModes_},
      path_{std::move( path )},
      multiSchedulingParameters_{std::move( sched.multiSchedulingParameters_ )},
      version_{version} {}


void Output::createAllOutputFiles( std::ofstream &of, const SkdCatalogReader &skdCatalogReader ) {
    for ( auto &sky : network_.refSkyCoverages() ) {
        sky.calculateSkyCoverageScores();
    }

    if ( scans_.empty() ) {
        return;
    }

    writeStatistics( of );

    if ( xml_.get<bool>( "VieSchedpp.output.createSummary", false ) ) {
        writeSkdsum();
    }
    if ( xml_.get<bool>( "VieSchedpp.output.createNGS", false ) ) {
        writeNGS();
    }
    if ( xml_.get<bool>( "VieSchedpp.output.createSKD", false ) ) {
        writeSkd( skdCatalogReader );
    }
    if ( xml_.get<bool>( "VieSchedpp.output.createVEX", false ) ) {
        writeVex();
    }
    if ( xml_.get<bool>( "VieSchedpp.output.createOperationsNotes", false ) ) {
        writeOperationsNotes();
    }
    if ( xml_.get<bool>( "VieSchedpp.output.createSourceGroupStatistics", false ) ) {
        writeStatisticsPerSourceGroup();
    }
    if ( xml_.get<bool>( "VieSchedpp.output.createSnrTable", false ) ) {
        writeSnrTable();
    }
    if ( false ) {
        writeAstFile();
    }
}


void Output::writeVex() {
    string fileName = getName();
    fileName.append( ".vex" );
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing vex file to: " << fileName;
#else
    cout << "[info] writing vex file to: " << fileName;
#endif
    Vex vex( path_ + fileName );
    vex.writeVex( network_, sources_, scans_, obsModes_, xml_ );
}


void Output::writeSkd( const SkdCatalogReader &skdCatalogReader ) {
    string fileName = getName();
    fileName.append( ".skd" );
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing skd file to: " << fileName;
#else
    cout << "[info] writing skd file to: " << fileName;
#endif
    Skd skd( path_ + fileName );
    skd.writeSkd( network_, sources_, scans_, skdCatalogReader, xml_ );
}


void Output::writeOperationsNotes() {
    string fileName = getName();
    fileName.append( ".txt" );
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing operation notes file to: " << fileName;
#else
    cout << "[info] writing operation notes file to: " << fileName;
#endif
    OperationNotes notes( path_ + fileName );
    notes.writeOperationNotes( network_, sources_, scans_, obsModes_, xml_, version_, multiSchedulingParameters_ );
}


void Output::writeSkdsum() {
    string fileName = getName();
    fileName.append( "_skdsum.txt" );
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing skdsum to: " << fileName;
#else
    cout << "[info] writing skdsum to: " << fileName;
#endif
    OperationNotes notes( path_ + fileName );
    notes.writeSkdsum( network_, sources_, scans_ );
}


void Output::writeSnrTable() {
    string fileName = getName();
    fileName.append( ".snr" );
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing SNR table to: " << fileName;
#else
    cout << "[info] writing SNR table to: " << fileName;
#endif
    SNR_table snr( path_ + fileName );
    snr.writeTable( network_, sources_, scans_, obsModes_ );
}


void Output::writeAstFile() {
    string fileName = getName();
    fileName.append( ".ast" );
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing ast file to: " << fileName;
#else
    cout << "[info] writing ast file to: " << fileName;
#endif
    Ast ast( path_ + fileName );
    ast.writeAstFile( network_, sources_, scans_, xml_, obsModes_ );
}


void Output::writeNGS() {
    string fname;
    if ( version_ == 0 ) {
        fname = TimeSystem::time2date( TimeSystem::startTime ).erase( 0, 2 ).append( "VS_N000" );
    } else {
        fname = ( boost::format( "%sVS_v%03d" ) % TimeSystem::time2date( TimeSystem::startTime ).erase( 0, 2 ) %
                  ( version_ ) )
                    .str();
    }
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing empty NGS file to " << fname;
#else
    cout << "[info] writing empty NGS file to " << fname;
#endif

    string NGS_path = xml_.get( "VieSchedpp.output.NGS_directory", "" );
    unsigned long idx = path_.find_last_of( '/', path_.length() - 1 );
    if ( idx > 0 ) {
        NGS_path.append( path_.substr( idx + 1, path_.size() - 1 - idx ) );
    }

    ofstream of;
    if ( NGS_path.empty() ) {
        of.open( path_ + fname );
    } else {
        of.open( NGS_path + fname );
        if ( !of.good() ) {
            of.open( path_ + fname );
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( warning ) << "NGS output path changed to " << path_;
#else
            cout << "[warning] NGS output path changed to " << path_;
#endif
        }
    }

    boost::posix_time::ptime start = TimeSystem::startTime;
    unsigned long counter = 1;

    for ( const auto &any : scans_ ) {
        for ( int i = 0; i < any.getNObs(); ++i ) {
            const Observation &obs = any.getObservation( i );
            string sta1 = network_.getStation( obs.getStaid1() ).getName();
            string sta2 = network_.getStation( obs.getStaid2() ).getName();
            if ( sta1 > sta2 ) {
                swap( sta1, sta2 );
            }
            string src = sources_[obs.getSrcid()].getName();
            unsigned int time = obs.getStartTime();

            boost::posix_time::ptime tmp = TimeSystem::internalTime2PosixTime( time );
            int year = tmp.date().year();
            int month = tmp.date().month();
            int day = tmp.date().day();
            int hour = tmp.time_of_day().hours();
            int minute = tmp.time_of_day().minutes();
            double second = tmp.time_of_day().seconds();

            of << boost::format( "%-8s  %-8s  %-8s %4d %02d %02d %02d %02d  %13.10f            " ) % sta1 % sta2 % src %
                      year % month % day % hour % minute % second;
            of << boost::format( "%6d" ) % counter << "01\n";

            of << "    0000000.00000000    .00000  -000000.0000000000    .00000 0      I   ";
            of << boost::format( "%6d" ) % counter << "02\n";

            of << "    .00000    .00000    .00000    .00000   0.000000000000000        0.  ";
            of << boost::format( "%6d" ) % counter << "03\n";

            of << "       .00   .0       .00   .0       .00   .0       .00   .0            ";
            of << boost::format( "%6d" ) % counter << "04\n";

            of << "   -.00000   -.00000    .00000    .00000    .00000    .00000            ";
            of << boost::format( "%6d" ) % counter << "05\n";

            of << "     0.000    00.000   000.000   000.000    00.000    00.000 0 0        ";
            of << boost::format( "%6d" ) % counter << "06\n";

            of << "        0.0000000000    .00000        -.0000000000    .00000  0         ";
            of << boost::format( "%6d" ) % counter << "08\n";

            of << "          0.00000000    .00000        0.0000000000    .00000 0      I   ";
            of << boost::format( "%6d" ) % counter << "09\n";

            ++counter;
        }
    }

    of.close();
}


void Output::writeStatisticsPerSourceGroup() {
    const auto &tmp0 = xml_.get_child_optional( "VieSchedpp.source" );

    if ( tmp0.is_initialized() ) {
        boost::property_tree::ptree PARA_source = *tmp0;
        unordered_map<std::string, std::vector<std::string>> group_source =
            readGroups( PARA_source, GroupType::source );

        string expName = xml_.get( "VieSchedpp.general.experimentName", "schedule" );
        string fileName = getName();
        fileName.append( "_sourceStatistics.txt" );
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( info ) << "writing source statistics file to: " << fileName;
#else
        cout << "[info] writing source statistics file to: " << fileName;
#endif

        vector<string> interestedSrcGroups;
        const auto &tmp = xml_.get_child_optional( "VieSchedpp.output.sourceGroupsForStatistic" );
        if ( tmp.is_initialized() ) {
            for ( const auto &any : *tmp ) {
                if ( any.first == "name" ) {
                    interestedSrcGroups.push_back( any.second.get_value<string>() );
                }
            }
            if ( interestedSrcGroups.empty() ) {
                return;
            }
        } else {
            return;
        }

        ofstream of( path_ + fileName );

        auto nsrc = sources_.size();
        vector<double> sWeight( nsrc );
        vector<unsigned int> nscansTarget( nsrc );
        vector<unsigned int> targetScans( nsrc );
        vector<char> necessaryFlags( nsrc );
        vector<double> minRepeat( nsrc );
        vector<vector<pair<boost::posix_time::ptime, boost::posix_time::ptime>>> visibleTimes( nsrc );
        bool hardBreak = false;
        for ( auto &src : sources_ ) {
            auto srcid = src.getId();

            necessaryFlags[srcid] = false;
            for ( const auto &group : group_source ) {
                if ( find( interestedSrcGroups.begin(), interestedSrcGroups.end(), group.first ) ==
                     interestedSrcGroups.end() ) {
                    continue;
                }
                if ( find( group.second.begin(), group.second.end(), src.getName() ) != group.second.end() ) {
                    necessaryFlags[srcid] = true;
                }
            }
            if ( !necessaryFlags[src.getId()] ) {
                continue;
            }

            src.setNextEvent( 0 );
            src.checkForNewEvent( 0, hardBreak );
            sWeight.push_back( src.getPARA().weight );
            if ( src.getPARA().tryToObserveXTimesEvenlyDistributed.is_initialized() ) {
                nscansTarget[srcid] = *src.getPARA().tryToObserveXTimesEvenlyDistributed;
            } else {
                nscansTarget[srcid] = 0;
            }
            minRepeat[srcid] = static_cast<double>( src.getPARA().minRepeat ) / 3600.0;
            targetScans[srcid] = src.getPARA().maxNumberOfScans;
            auto visTimes = minutesVisible( src );

            unsigned int start = 0;
            unsigned int lastElement = 0;
            for ( auto const &t : visTimes ) {
                if ( start == 0 ) {
                    start = t;
                    lastElement = t;
                } else {
                    if ( t - lastElement != 60 ) {
                        boost::posix_time::ptime ptstart = TimeSystem::internalTime2PosixTime( start );
                        boost::posix_time::ptime ptend = TimeSystem::internalTime2PosixTime( lastElement );
                        visibleTimes[srcid].emplace_back( ptstart, ptend );
                        start = 0;
                    }
                    lastElement = t;
                }
            }
            if ( start != 0 ) {
                boost::posix_time::ptime ptstart = TimeSystem::internalTime2PosixTime( start );
                boost::posix_time::ptime ptend = TimeSystem::internalTime2PosixTime( lastElement );
                visibleTimes[srcid].emplace_back( ptstart, ptend );
            }
        }

        vector<vector<unsigned int>> scanTime( nsrc );
        vector<vector<unsigned long>> scanNsta( nsrc );
        vector<vector<unsigned long>> scanNbl( nsrc );
        vector<vector<char>> flag( nsrc );
        vector<vector<unsigned int>> scanTimePerStation( nsrc, vector<unsigned int>( network_.getNSta(), 0 ) );

        for ( const auto &scan : scans_ ) {
            unsigned long srcid = scan.getSourceId();
            if ( !necessaryFlags[srcid] ) {
                continue;
            }

            scanTime[srcid].push_back( scan.getPointingVector( 0 ).getTime() );
            scanNsta[srcid].push_back( scan.getNSta() );
            scanNbl[srcid].push_back( scan.getNObs() );
            switch ( scan.getType() ) {
                case Scan::ScanType::fillin: {
                    flag[srcid].push_back( '*' );
                    break;
                }
                case Scan::ScanType::calibrator: {
                    flag[srcid].push_back( '#' );
                    break;
                }
                case Scan::ScanType::standard: {
                    flag[srcid].push_back( ' ' );
                    break;
                }
                case Scan::ScanType::highImpact: {
                    flag[srcid].push_back( ' ' );
                    break;
                }
            }
            for ( int i = 0; i < scan.getNSta(); ++i ) {
                unsigned long staid = scan.getPointingVector( i ).getStaid();
                unsigned int duration = scan.getTimes().getObservingDuration( i );
                scanTimePerStation[srcid][staid] += duration;
            }
        }

        unsigned long nMaxScans = 0;
        for ( const auto &any : scanTime ) {
            unsigned long thisScans = any.size();
            if ( thisScans > nMaxScans ) {
                nMaxScans = thisScans;
            }
        }

        of << "* Columns:\n";
        of << "*     1  : Name\n";
        of << "*     2  : Id\n";
        of << "*     3  : total scans\n";
        of << "*     4  : standard scans\n";
        of << "*     5  : fillin mode scans\n";
        of << "*     6  : calibrator scans\n";
        of << "*     7  : weight at start of session\n";
        of << "*     8  : target scans\n";
        of << "*     9  : minimum source repeat time [h] (at session start)\n";
        of << "*     11+: list of scans containing:\n";
        of << "*             scan start time (UTC)\n";
        of << "*             scan flag:\n";
        of << "*                 ' ': single source or subnetting scan\n";
        of << "*                 '*': fillin mode scan\n";
        of << "*                 '#': calibrator scan\n";
        of << "*             number of stations\n";
        of << "*     end: source visibility time (estimated with parameters at session start)\n";

        map<string, vector<int>> srcgrpstat;
        map<string, vector<int>> srcgrpgeneralstat;
        map<string, vector<unsigned int>> srcGrpStationScanTime;

        of << "*\n";
        of << "* ============================= GROUP BASED STATISTICS =============================\n";

        for ( const auto &group : group_source ) {
            if ( find( interestedSrcGroups.begin(), interestedSrcGroups.end(), group.first ) ==
                 interestedSrcGroups.end() ) {
                continue;
            }
            vector<int> nscansPerGroup( nMaxScans + 1, 0 );
            vector<unsigned int> groupScanTimePerStation( network_.getNSta(), 0 );
            int sumTotalScans = 0;
            int sumScans = 0;
            int sumFillinModeScans = 0;
            int sumCalibratorScans = 0;

            int baselines = 0;

            of << "*\n";
            of << "* ----------------------------- GROUP: " << group.first << " -----------------------------\n";
            of << "*\n";
            for ( const auto &src : sources_ ) {
                unsigned long srcid = src.getId();
                if ( find( group.second.begin(), group.second.end(), src.getName() ) != group.second.end() ) {
                    unsigned long nscans = scanTime[srcid].size();
                    unsigned long nscansStd = 0;
                    unsigned long nscansFillin = 0;
                    unsigned long nscansCalibrator = 0;
                    ++nscansPerGroup[nscans];

                    for ( int i = 0; i < nscans; ++i ) {
                        if ( flag[srcid][i] == ' ' ) {
                            ++nscansStd;
                        } else if ( flag[srcid][i] == '*' ) {
                            ++nscansFillin;
                        } else if ( flag[srcid][i] == '#' ) {
                            ++nscansCalibrator;
                        }
                    }
                    sumTotalScans += nscans;
                    sumScans += nscansStd;
                    sumFillinModeScans += nscansFillin;
                    sumCalibratorScans += nscansCalibrator;

                    for ( int i = 0; i < network_.getNSta(); ++i ) {
                        groupScanTimePerStation[i] += scanTimePerStation[srcid][i];
                    }

                    of << boost::format( "%8s, %4d, %4d, %4d, %4d, %4d, %6.2f, %4d, %5.2f, ||, " ) % src.getName() %
                              src.getId() % nscans % nscansStd % nscansFillin % nscansCalibrator % sWeight[srcid] %
                              nscansTarget[srcid] % minRepeat[srcid];
                    for ( int i = 0; i < scanTime[srcid].size(); ++i ) {
                        unsigned int ttt = scanTime[srcid][i];

                        boost::posix_time::ptime pt = TimeSystem::internalTime2PosixTime( ttt );
                        of << TimeSystem::time2string( pt ).substr( 11, 5 ).append( "[" );
                        of << flag[srcid][i];
                        of << boost::format( "%02d], " ) % scanNsta[srcid][i];
                    }
                    for ( unsigned long i = scanTime[srcid].size(); i < nMaxScans; ++i ) {
                        of << "          , ";
                    }
                    of << "||, ";
                    for ( auto &i : visibleTimes[srcid] ) {
                        of << "[" << TimeSystem::time2string( i.first ).substr( 11, 5 ) << " - "
                           << TimeSystem::time2string( i.second ).substr( 11, 5 ) << "], ";
                    }
                    of << "\n";
                }
            }

            srcgrpstat[group.first] = nscansPerGroup;
            srcgrpgeneralstat[group.first] =
                vector<int>{sumTotalScans, sumScans, sumFillinModeScans, sumCalibratorScans};
            srcGrpStationScanTime[group.first] = groupScanTimePerStation;
        }

        of << "*\n";
        of << "* ============================= SESSION SUMMARY =============================\n";
        of << "*\n";
        of << " # scans: " << scans_.size() << "\n";
        unsigned int xxxstdScans = 0;
        unsigned int xxxfiScans = 0;
        unsigned int xxxcalScans = 0;
        for ( const auto &persource : flag ) {
            for ( const auto &perscan : persource ) {
                if ( perscan == ' ' ) {
                    ++xxxstdScans;
                } else if ( perscan == '*' ) {
                    ++xxxfiScans;
                } else if ( perscan == '#' ) {
                    ++xxxcalScans;
                }
            }
        }
        of << "   # standard scans:    " << xxxstdScans << "\n";
        of << "   # fillin mode scans: " << xxxfiScans << "\n";
        of << "   # calibrator scans:  " << xxxcalScans << "\n";

        vector<unsigned int> xxxstps( network_.getNSta(), 0 );
        for ( const auto &stps : scanTimePerStation ) {
            for ( int i = 0; i < network_.getNSta(); ++i ) {
                xxxstps[i] += stps[i];
            }
        }
        of << "* \n";
        of << "observing time per station:\n";
        for ( unsigned long ista = 0; ista < network_.getNSta(); ++ista ) {
            const string name = network_.getStation( ista ).getName();
            unsigned int seconds = xxxstps[ista];
            double percent = ( static_cast<double>( seconds ) / static_cast<double>( TimeSystem::duration ) ) * 100;
            of << boost::format( "    %8s: %8d [s]  (%5.1f [%%])\n" ) % name % seconds % percent;
        }

        of << "*\n";
        of << "* ============================= GROUP BASED SUMMARY =============================\n";

        for ( const auto &any : srcgrpstat ) {
            const string &grpName = any.first;
            const auto &scans = any.second;
            const auto &sum = srcgrpgeneralstat[grpName];

            of << "*\n";
            of << "* ----------------------------- SUMMARY GROUP: " << grpName << " -----------------------------\n";
            of << "*\n";
            of << boost::format( " # total scans:         %4d\n" ) % sum[0];
            of << boost::format( "   # standard scans:    %4d\n" ) % sum[1];
            of << boost::format( "   # fillin mode scans: %4d\n" ) % sum[2];
            of << boost::format( "   # calibrator scans:  %4d\n" ) % sum[3];
            of << "* \n";

            bool first = false;
            for ( unsigned long i = scans.size() - 1; i >= 0; --i ) {
                if ( first || scans[i] != 0 ) {
                    of << boost::format( " %3d sources are observed in %4d scans\n" ) % scans[i] % i;
                    first = true;
                }
                if ( i == 0 ) {
                    break;
                }
            }

            of << "* \n";
            of << "observing time per station:\n";
            for ( int i = 0; i < network_.getNSta(); ++i ) {
                const string name = network_.getStation( i ).getName();
                unsigned int seconds = srcGrpStationScanTime[grpName][i];
                double percent = ( static_cast<double>( seconds ) / static_cast<double>( TimeSystem::duration ) ) * 100;
                of << boost::format( "    %8s: %8d [s]  (%5.1f [%%])\n" ) % name % seconds % percent;
            }
        }
    }
}


unordered_map<string, vector<string>> Output::readGroups( boost::property_tree::ptree root, GroupType type ) noexcept {
    unordered_map<std::string, std::vector<std::string>> groups;
    auto groupTree = root.get_child_optional( "groups" );
    if ( groupTree.is_initialized() ) {
        for ( auto &it : *groupTree ) {
            string name = it.first;
            if ( name == "group" ) {
                string groupName = it.second.get_child( "<xmlattr>.name" ).data();
                std::vector<std::string> members;
                for ( auto &it2 : it.second ) {
                    if ( it2.first == "member" ) {
                        members.push_back( it2.second.data() );
                    }
                }
                groups[groupName] = members;
            }
        }
    }

    switch ( type ) {
        case GroupType::source: {
            std::vector<std::string> members;
            for ( const auto &any : sources_ ) {
                members.push_back( any.getName() );
            }
            groups["__all__"] = members;
            break;
        }
        case GroupType::station: {
            std::vector<std::string> members;
            for ( const auto &any : network_.getStations() ) {
                members.push_back( any.getName() );
            }
            groups["__all__"] = members;
            break;
        }
        case GroupType::baseline: {
            std::vector<std::string> members;
            for ( const auto &any : network_.getBaselines() ) {
                members.push_back( any.getName() );
            }
            groups["__all__"] = members;
            break;
        }
    }

    return groups;
}


vector<unsigned int> Output::minutesVisible( const Source &source ) {
    vector<unsigned int> visibleTimes;
    const auto &parameters = source.getPARA();
    unsigned int minVisible = parameters.minNumberOfStations;

    vector<unsigned long> reqSta = parameters.requiredStations;
    vector<unsigned long> ignSta = parameters.ignoreStations;

    for (auto &any : network_.refStations()) {
        any.setNextEvent(0);
        bool dummy = false;
        any.checkForNewEvent(0, dummy);
    }

    for ( unsigned int t = 0; t < TimeSystem::duration; t += 60 ) {
        unsigned int visible = 0;

        bool requiredStationNotVisible = false;
        for ( unsigned long staid = 0; staid < network_.getNSta(); ++staid ) {
            Station &thisSta = network_.refStation(staid);
            bool dummy = false;
            thisSta.checkForNewEvent(t, dummy);

            if ( find( ignSta.begin(), ignSta.end(), staid ) != ignSta.end() ) {
                continue;
            }
            if (!thisSta.getPARA().available || thisSta.getPARA().tagalong) {
                continue;
            }

            PointingVector p( staid, source.getId() );
            p.setTime( t );

            thisSta.calcAzEl_simple( source, p );
            thisSta.calcAzEl_rigorous( source, p );

            // check if source is up from station
            bool flag = thisSta.isVisible( p, source.getPARA().minElevation );
            if ( flag ) {
                ++visible;
            } else {
                if ( find( reqSta.begin(), reqSta.end(), staid ) != reqSta.end() ) {
                    requiredStationNotVisible = true;
                    break;
                }
            }
        }
        if ( requiredStationNotVisible ) {
            continue;
        }
        if ( visible >= minVisible ) {
            visibleTimes.push_back( t );
        }
    }

    for (auto &any : network_.refStations()) {
        any.setNextEvent(0);
        bool dummy = false;
        any.checkForNewEvent(0, dummy);
    }

    return visibleTimes;
}


void Output::writeStatistics( std::ofstream &of ) {
    string oString;

    auto n_scans = static_cast<int>( scans_.size() );
    int n_standard = 0;
    int n_fillin = 0;
    int n_calibrator = 0;
    int n_single = 0;
    int n_subnetting = 0;
    int n_obs_total = 0;
    vector<unsigned int> nscan_sta( network_.getNSta(), 0 );
    vector<unsigned int> nobs_sta( network_.getNSta(), 0 );
    vector<unsigned int> nobs_bl( network_.getNBls(), 0 );
    vector<unsigned int> nscan_src( sources_.size(), 0 );
    vector<unsigned int> nobs_src( sources_.size(), 0 );

    for ( const auto &any : scans_ ) {
        switch ( any.getType() ) {
            case Scan::ScanType::fillin: {
                ++n_fillin;
                break;
            }
            case Scan::ScanType::calibrator: {
                ++n_calibrator;
                break;
            }
            case Scan::ScanType::standard: {
                ++n_standard;
                break;
            }
            case Scan::ScanType::highImpact: {
                ++n_standard;
                break;
            }
        }
        switch ( any.getScanConstellation() ) {
            case Scan::ScanConstellation::single: {
                ++n_single;
                break;
            }
            case Scan::ScanConstellation::subnetting: {
                ++n_subnetting;
                break;
            }
        }
        auto n_obs = any.getNObs();
        n_obs_total += n_obs;
        for ( int ista = 0; ista < any.getNSta(); ++ista ) {
            const PointingVector &pv = any.getPointingVector( ista );
            unsigned long id = pv.getStaid();
            ++nscan_sta[id];
        }
        for ( int ibl = 0; ibl < any.getNObs(); ++ibl ) {
            const Observation &obs = any.getObservation( ibl );
            ++nobs_sta[obs.getStaid1()];
            ++nobs_sta[obs.getStaid2()];
            ++nobs_bl[network_.getBaseline( obs.getStaid1(), obs.getStaid2() ).getId()];
        }
        unsigned long id = any.getSourceId();
        ++nscan_src[id];
        nobs_src[id] += n_obs;
    }
    int n_src = static_cast<int>( count_if( nscan_src.begin(), nscan_src.end(), []( int i ) { return i > 0; } ) );

    auto totalTime = static_cast<double>( TimeSystem::duration );
    vector<double> obsPer;
    for ( const auto &station : network_.getStations() ) {
        int t = station.getStatistics().totalObservingTime;
        obsPer.push_back( static_cast<double>( t ) / totalTime * 100 );
    }
    double obsMean = accumulate( obsPer.begin(), obsPer.end(), 0.0 ) / ( network_.getNSta() );

    vector<double> preobPer;
    for ( const auto &station : network_.getStations() ) {
        int t = station.getStatistics().totalPreobTime;
        preobPer.push_back( static_cast<double>( t ) / totalTime * 100 );
    }
    double preobMean = accumulate( preobPer.begin(), preobPer.end(), 0.0 ) / ( network_.getNSta() );

    vector<double> slewPer;
    for ( const auto &station : network_.getStations() ) {
        int t = station.getStatistics().totalSlewTime;
        slewPer.push_back( static_cast<double>( t ) / totalTime * 100 );
    }
    double slewMean = accumulate( slewPer.begin(), slewPer.end(), 0.0 ) / ( network_.getNSta() );

    vector<double> idlePer;
    for ( const auto &station : network_.getStations() ) {
        int t = station.getStatistics().totalIdleTime;
        idlePer.push_back( static_cast<double>( t ) / totalTime * 100 );
    }
    double idleMean = accumulate( idlePer.begin(), idlePer.end(), 0.0 ) / ( network_.getNSta() );

    vector<double> fieldPer;
    for ( const auto &station : network_.getStations() ) {
        int t = station.getStatistics().totalFieldSystemTime;
        fieldPer.push_back( static_cast<double>( t ) / totalTime * 100 );
    }
    double fieldMean = accumulate( fieldPer.begin(), fieldPer.end(), 0.0 ) / ( network_.getNSta() );

    vector<double> a13m30;
    vector<double> a25m30;
    vector<double> a37m30;
    vector<double> a13m60;
    vector<double> a25m60;
    vector<double> a37m60;
    for ( const auto &station : network_.getStations() ) {
        unsigned long id = station.getId();
        const auto &map = network_.getStaid2skyCoverageId();
        unsigned long skyCovId = map.at( id );
        const auto &skyCov = network_.getSkyCoverage( skyCovId );
        a13m30.push_back( skyCov.getSkyCoverageScore_a13m30() );
        a25m30.push_back( skyCov.getSkyCoverageScore_a25m30() );
        a37m30.push_back( skyCov.getSkyCoverageScore_a37m30() );
        a13m60.push_back( skyCov.getSkyCoverageScore_a13m60() );
        a25m60.push_back( skyCov.getSkyCoverageScore_a25m60() );
        a37m60.push_back( skyCov.getSkyCoverageScore_a37m60() );
    }
    double a13m30Mean = accumulate( a13m30.begin(), a13m30.end(), 0.0 ) / ( network_.getNSta() );
    double a25m30Mean = accumulate( a25m30.begin(), a25m30.end(), 0.0 ) / ( network_.getNSta() );
    double a37m30Mean = accumulate( a37m30.begin(), a37m30.end(), 0.0 ) / ( network_.getNSta() );
    double a13m60Mean = accumulate( a13m60.begin(), a13m60.end(), 0.0 ) / ( network_.getNSta() );
    double a25m60Mean = accumulate( a25m60.begin(), a25m60.end(), 0.0 ) / ( network_.getNSta() );
    double a37m60Mean = accumulate( a37m60.begin(), a37m60.end(), 0.0 ) / ( network_.getNSta() );

    oString.append( std::to_string( version_ ) ).append( "," );
    oString.append( std::to_string( n_scans ) ).append( "," );
    oString.append( std::to_string( n_single ) ).append( "," );
    oString.append( std::to_string( n_subnetting ) ).append( "," );
    oString.append( std::to_string( n_fillin ) ).append( "," );
    oString.append( std::to_string( n_calibrator ) ).append( "," );
    oString.append( std::to_string( n_obs_total ) ).append( "," );
    oString.append( std::to_string( network_.getNSta() ) ).append( "," );
    oString.append( std::to_string( n_src ) ).append( "," );

    oString.append( std::to_string( obsMean ) ).append( "," );
    oString.append( std::to_string( preobMean ) ).append( "," );
    oString.append( std::to_string( slewMean ) ).append( "," );
    oString.append( std::to_string( idleMean ) ).append( "," );
    oString.append( std::to_string( fieldMean ) ).append( "," );

    oString.append( std::to_string( a13m30Mean ) ).append( "," );
    oString.append( std::to_string( a25m30Mean ) ).append( "," );
    oString.append( std::to_string( a37m30Mean ) ).append( "," );
    oString.append( std::to_string( a13m60Mean ) ).append( "," );
    oString.append( std::to_string( a25m60Mean ) ).append( "," );
    oString.append( std::to_string( a37m60Mean ) ).append( "," );

    for ( auto any : obsPer ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : preobPer ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : slewPer ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : idlePer ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : fieldPer ) {
        oString.append( std::to_string( any ) ).append( "," );
    }

    for ( auto any : a13m30 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : a25m30 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : a37m30 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : a13m60 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : a25m60 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : a37m60 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }

    for ( int i = 0; i < network_.getNSta(); ++i ) {
        oString.append( std::to_string( nscan_sta[i] ) ).append( "," );
    }
    for ( int i = 0; i < network_.getNSta(); ++i ) {
        oString.append( std::to_string( nobs_sta[i] ) ).append( "," );
    }
    for ( int i = 0; i < network_.getNBls(); ++i ) {
        oString.append( std::to_string( nobs_bl[i] ) ).append( "," );
    }
    for ( int i = 0; i < sources_.size(); ++i ) {
        oString.append( std::to_string( nscan_src[i] ) ).append( "," );
    }
    for ( int i = 0; i < sources_.size(); ++i ) {
        oString.append( std::to_string( nobs_src[i] ) ).append( "," );
    }

    if ( multiSchedulingParameters_.is_initialized() ) {
        oString.append( multiSchedulingParameters_->statisticsOutput() );
    }

#ifdef _OPENMP
#pragma omp critical
#endif
    { of << oString << endl; };
}
