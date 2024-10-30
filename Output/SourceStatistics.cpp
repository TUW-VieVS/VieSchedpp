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

#include "SourceStatistics.h"


using namespace VieVS;
using namespace std;

unsigned long SourceStatistics::nextId = 0;


SourceStatistics::SourceStatistics( const std::string &file ) : VieVS_Object( nextId++ ) { of = ofstream( file ); }


void SourceStatistics::writeFile( Network &network, SourceList &sourceList, const std::vector<Scan> &scans,
                                  const boost::property_tree::ptree &xml ) {
    const auto &tmp0 = xml.get_child_optional( "VieSchedpp.source" );

    if ( tmp0.is_initialized() ) {
        boost::property_tree::ptree PARA_source = *tmp0;
        unordered_map<std::string, std::vector<std::string>> group_source = readGroups( PARA_source, sourceList );


        vector<string> interestedSrcGroups;
        const auto &tmp = xml.get_child_optional( "VieSchedpp.output.sourceGroupsForStatistic" );
        if ( tmp.is_initialized() ) {
            for ( const auto &any : *tmp ) {
                if ( any.first == "name" ) {
                    interestedSrcGroups.push_back( any.second.get_value<string>() );
                }
            }
            if ( interestedSrcGroups.empty() ) {
                interestedSrcGroups.emplace_back( "__all__" );
            }
        } else {
            interestedSrcGroups.emplace_back( "__all__" );
        }

        auto nsrc = sourceList.getNSrc();
        vector<double> weight( nsrc );
        vector<unsigned int> n_target( nsrc );
        vector<char> sourceIncludedInOutput( nsrc );
        vector<double> minRepeat( nsrc );
        vector<vector<pair<unsigned int, unsigned int>>> visibleTimes( nsrc );
        vector<double> minVisible( nsrc, 0.0 );

        for ( const auto &src : sourceList.refSources() ) {
            auto srcid = src->getId();

            sourceIncludedInOutput[srcid] = false;
            for ( const auto &group : group_source ) {
                if ( find( interestedSrcGroups.begin(), interestedSrcGroups.end(), group.first ) ==
                     interestedSrcGroups.end() ) {
                    continue;
                }
                if ( find( group.second.begin(), group.second.end(), src->getName() ) != group.second.end() ||
                     ( src->hasAlternativeName() && find( group.second.begin(), group.second.end(),
                                                          src->getAlternativeName() ) != group.second.end() ) ) {
                    sourceIncludedInOutput[srcid] = true;
                }
            }
            if ( !sourceIncludedInOutput[src->getId()] ) {
                continue;
            }

            src->setNextEvent( 0 );
            bool hardBreak = false;
            src->checkForNewEvent( 0, hardBreak );
            weight[srcid] = src->getPARA().weight;
            if ( src->getPARA().tryToObserveXTimesEvenlyDistributed.is_initialized() ) {
                n_target[srcid] = *src->getPARA().tryToObserveXTimesEvenlyDistributed;
            } else {
                n_target[srcid] = 0;
            }
            minRepeat[srcid] = static_cast<double>( src->getPARA().minRepeat ) / 3600.0;
            visibleTimes[srcid] = minutesVisible( network, src );
            for ( const auto &any : visibleTimes[srcid] ) {
                minVisible[srcid] += static_cast<double>( any.second - any.first ) / 3600.0;
            }
        }

        vector<vector<unsigned int>> scanStartTime( nsrc );
        vector<vector<unsigned long>> nsta( nsrc );
        vector<vector<unsigned long>> nobs( nsrc );
        vector<vector<unsigned long>> dur( nsrc );
        vector<vector<Scan::ScanType>> flag( nsrc );
        vector<vector<unsigned int>> scanTimePerStation( nsrc, vector<unsigned int>( network.getNSta(), 0 ) );
        vector<vector<string>> tlcs( nsrc );
        map<string, vector<unsigned int>> totalScans;
        for ( const auto &group : group_source ) {
            totalScans[group.first] = vector<unsigned int>( 96, 0 );
        }

        for ( const auto &scan : scans ) {
            unsigned long srcid = scan.getSourceId();
            if ( !sourceIncludedInOutput[srcid] ) {
                continue;
            }
            unsigned int startTime = scan.getTimes().getObservingTime( Timestamp::start );
            for ( const auto &group : group_source ) {
                const auto &src = sourceList.getSource( srcid );
                if ( find( group.second.begin(), group.second.end(), src->getName() ) != group.second.end() ||
                     ( src->hasAlternativeName() && find( group.second.begin(), group.second.end(),
                                                          src->getAlternativeName() ) != group.second.end() ) ) {
                    ++totalScans[group.first][startTime / 900];
                }
            }

            scanStartTime[srcid].push_back( startTime );
            nsta[srcid].push_back( scan.getNSta() );
            nobs[srcid].push_back( scan.getNObs() );
            dur[srcid].push_back( scan.getTimes().getObservingDuration() );
            flag[srcid].push_back( scan.getType() );
            string stations;
            for ( int i = 0; i < scan.getNSta(); ++i ) {
                unsigned long staid = scan.getPointingVector( i ).getStaid();
                unsigned int duration = scan.getTimes().getObservingDuration( i );
                scanTimePerStation[srcid][staid] += duration;
                stations.append( network.getStation( staid ).getAlternativeName() );
            }
            tlcs[srcid].push_back( stations );
        }
        unsigned int total_nobs = 0;
        for ( const auto &scan : scans ) {
            total_nobs += scan.getNObs();
        }

        unsigned int total_sources = 0;
        for ( const auto &src : sourceList.getSources() ) {
            if ( src->getNTotalScans() > 0 ) {
                total_sources += 1;
            }
        }

        of << "\n";
        of << " ============================= GROUP BASED STATISTICS =============================\n\n";
        of << ".-----------------------------------------------.\n";
        of << boost::format( "| %-15s | %7s | %7s | %7s |\n" ) % "Group" % "sources" % "scans" % "obs";
        of << boost::format( "| %-15s | %7d | %7d | %7d |\n" ) % "Total" % total_sources % scans.size() % total_nobs;
        of << "|-----------------|---------|---------|---------|\n";
        for ( const auto &group : group_source ) {
            if ( find( interestedSrcGroups.begin(), interestedSrcGroups.end(), group.first ) ==
                 interestedSrcGroups.end() ) {
                continue;
            }
            // summary statistics:
            int sumTotalScans = 0;
            int sumSources = 0;
            int sumObs = 0;

            for ( const auto &src : sourceList.getSources() ) {
                unsigned long srcid = src->getId();
                if ( find( group.second.begin(), group.second.end(), src->getName() ) != group.second.end() ||
                     ( src->hasAlternativeName() && find( group.second.begin(), group.second.end(),
                                                          src->getAlternativeName() ) != group.second.end() ) ) {
                    auto nscans = scanStartTime[srcid].size();
                    sumTotalScans += nscans;
                    for ( int i = 0; i < nscans; ++i ) {
                        sumObs += nobs[srcid][i];
                    }
                    if ( nscans > 0 ) {
                        sumSources += 1;
                    }
                }
            }
            of << boost::format( "| %-15s | %7d | %7d | %7d |\n" ) % group.first % sumSources % sumTotalScans % sumObs;
            of << boost::format( "| %-15s | %6.2f%% | %6.2f%% | %6.2f%% |\n" ) % group.first %
                      ( static_cast<double>( sumSources ) / total_sources * 100 ) %
                      ( static_cast<double>( sumTotalScans ) / scans.size() * 100 ) %
                      ( static_cast<double>( sumObs ) / total_nobs * 100 );
        }
        of << "'-----------------------------------------------'\n";


        for ( const auto &group : group_source ) {
            if ( find( interestedSrcGroups.begin(), interestedSrcGroups.end(), group.first ) ==
                 interestedSrcGroups.end() ) {
                continue;
            }

            of << "\n";
            of << " ----------------------------- GROUP: " << group.first << " -----------------------------\n";
            of << "\n";

            // summary statistics:
            vector<unsigned int> groupScanTimePerStation( network.getNSta(), 0 );
            int sumTotalScans = 0;
            int sumFillinScans = 0;
            int sumCalibratorScans = 0;
            int sumHighImpactScans = 0;
            int sumObs = 0;
            vector<unsigned long> nscansPerSource;

            for ( const auto &src : sourceList.getSources() ) {
                unsigned long srcid = src->getId();
                if ( find( group.second.begin(), group.second.end(), src->getName() ) != group.second.end() ||
                     ( src->hasAlternativeName() && find( group.second.begin(), group.second.end(),
                                                          src->getAlternativeName() ) != group.second.end() ) ) {
                    auto nscans = scanStartTime[srcid].size();
                    sumTotalScans += nscans;
                    for ( int i = 0; i < nscans; ++i ) {
                        if ( flag[srcid][i] == Scan::ScanType::fillin ) {
                            ++sumFillinScans;
                        } else if ( flag[srcid][i] == Scan::ScanType::astroCalibrator ) {
                            ++sumCalibratorScans;
                        } else if ( flag[srcid][i] == Scan::ScanType::highImpact ) {
                            ++sumHighImpactScans;
                        }
                        sumObs += nobs[srcid][i];
                    }
                    nscansPerSource.push_back( nscans );

                    for ( int i = 0; i < network.getNSta(); ++i ) {
                        groupScanTimePerStation[i] += scanTimePerStation[srcid][i];
                    }
                }
            }

            of << boost::format( "number of scans: %4d / %4d (total) %6.2f%%\n" ) % sumTotalScans % scans.size() %
                      ( static_cast<double>( sumTotalScans ) / scans.size() * 100 );
            if ( sumFillinScans > 0 ) {
                of << boost::format( "  fillin mode:   %4d\n" ) % sumFillinScans;
            }
            if ( sumCalibratorScans > 0 ) {
                of << boost::format( "  calibrator:    %4d\n" ) % sumCalibratorScans;
            }
            if ( sumHighImpactScans > 0 ) {
                of << boost::format( "  high impact:   %4d\n" ) % sumHighImpactScans;
            }
            of << boost::format( "  sources:       %4d / %4d (in group) \n" ) %
                      std::count_if( nscansPerSource.begin(), nscansPerSource.end(), []( int e ) { return e > 0; } ) %
                      nscansPerSource.size();
            of << boost::format( "  observations: %5d / %4d (total) %6.2f%%\n" ) % sumObs % total_nobs %
                      ( static_cast<double>( sumObs ) / total_nobs * 100 );

            of << "\n";
            unsigned int total = accumulate( groupScanTimePerStation.begin(), groupScanTimePerStation.end(), 0u );
            double total_hours = static_cast<double>( total ) / 3600.;
            of << boost::format( "total obsering time:   %7d [s] %6.2f [h]\n" ) % total % total_hours;
            double average_percent =
                ( static_cast<double>( total ) / network.getNSta() / static_cast<double>( TimeSystem::duration ) ) *
                100;
            of << boost::format( "average obsering time: %7d [s] %6.2f [h] (%6.2f%%)\n" ) %
                      ( static_cast<double>( total ) / network.getNSta() ) % ( total_hours / network.getNSta() ) %
                      average_percent;

            for ( unsigned long i = 0; i < network.getNSta(); ++i ) {
                unsigned int this_total = groupScanTimePerStation[i];
                double this_total_hours = static_cast<double>( this_total ) / 3600.;
                double percent =
                    ( static_cast<double>( this_total ) / static_cast<double>( TimeSystem::duration ) ) * 100;

                of << boost::format( "  %8s:            %7d s %6.2f h (%6.2f%%)\n" ) %
                          network.getStation( i ).getName() % this_total % this_total_hours % percent;
            }


            of << "\n";
            if ( !nscansPerSource.empty() ) {
                unsigned long max = *max_element( nscansPerSource.begin(), nscansPerSource.end() );
                for ( unsigned long i = max; i >= 2; --i ) {
                    long x = count( nscansPerSource.begin(), nscansPerSource.end(), i );
                    if ( x == 1 ) {
                        of << boost::format( "%3d source  is  observed in %3d scans\n" ) % x % i;
                    } else if ( x > 1 ) {
                        of << boost::format( "%3d sources are observed in %3d scans\n" ) % x % i;
                    }
                }
                of << "\n";
            }

            // output per source:
            for ( const auto &src : sourceList.getSources() ) {
                unsigned long srcid = src->getId();
                if ( find( group.second.begin(), group.second.end(), src->getName() ) != group.second.end() ||
                     ( src->hasAlternativeName() && find( group.second.begin(), group.second.end(),
                                                          src->getAlternativeName() ) != group.second.end() ) ) {
                    of << boost::format(
                              "%-8s  #scans: %4d  observations: %4d  weight: %6.2f min repeat time: %5.2f [h]  "
                              "visible: "
                              "%5.2f [h]; " ) %
                              src->getName() % scanStartTime[srcid].size() %
                              accumulate( nobs[srcid].begin(), nobs[srcid].end(), 0ul ) % weight[srcid] %
                              minRepeat[srcid] % minVisible[srcid];

                    if ( n_target[srcid] > 0 ) {
                        of << boost::format( "#target scans: %4d; " ) % n_target[srcid];
                    }
                    of << "\n";

                    for ( int i = 0; i < scanStartTime[srcid].size(); ++i ) {
                        unsigned int startTime = scanStartTime[srcid][i];

                        of << boost::format( "          start: %s  duration: %3d s  type: %-12s  stations: %s \n" ) %
                                  TimeSystem::time2timeOfDay( startTime ) % dur[srcid][i] %
                                  Scan::toString( flag[srcid][i] ).append( ";" ) % tlcs[srcid][i];
                    }
                }
            }

            // time plot
            of << "\n";
            of << "Group:  " << group.first << "\n";
            of << "Source: 'X'=scheduled; '-'=visible; ' '=not visible;\n";
            of << ".-------------------------------------------------------------"
                  "---------------------------------------------.\n";
            of << "|          time since session start (1 char equals 15 minutes)"
                  "                                             |       visible times\n";
            of << "|  SOURCE |0   1   2   3   4   5   6   7   8   9   10  11  12 "
                  " 13  14  15  16  17  18  19  20  21  22  23  |  [ start    -      end ]\n";
            of << "|---------|+---+---+---+---+---+---+---+---+---+---+---+---+--"
                  "-+---+---+---+---+---+---+---+---+---+---+---|\n";

            of << boost::format( "| %8s|" ) % "Total";
            for ( int j = 0; j < 96; ++j ) {
                int n = totalScans[group.first][j];
                char flagChar = util::numberOfScans2char( n );
                of << flagChar;
            }
            of << "|\n";
            of << "|-------------------------------------------------------------"
                  "---------------------------------------------|\n";


            for ( const auto &src : sourceList.getSources() ) {
                if ( !( find( group.second.begin(), group.second.end(), src->getName() ) != group.second.end() ||
                        ( src->hasAlternativeName() && find( group.second.begin(), group.second.end(),
                                                             src->getAlternativeName() ) != group.second.end() ) ) ) {
                    continue;
                }

                unsigned long srcid = src->getId();

                of << boost::format( "| %8s|" ) % src->getName();

                unsigned int timeStart = 0;
                unsigned int timeEnd = 900;
                for ( int j = 0; j < 96; ++j ) {
                    char flagChar = ' ';
                    for ( auto i : scanStartTime[srcid] ) {
                        if ( i >= timeStart && i <= timeEnd ) {
                            flagChar = 'X';
                            break;
                        }
                    }
                    if ( flagChar != 'X' ) {
                        for ( auto i : visibleTimes[srcid] ) {
                            if ( i.first <= timeStart && i.second >= timeEnd ) {
                                flagChar = '-';
                                break;
                            } else if ( i.first >= timeStart && i.first <= timeEnd ) {
                                flagChar = '-';
                                break;
                            } else if ( i.second >= timeStart && i.second <= timeEnd ) {
                                flagChar = '-';
                                break;
                            }
                        }
                    }
                    of << flagChar;
                    timeEnd += 900;
                    timeStart += 900;
                }
                of << "| ";
                for ( auto i : visibleTimes[srcid] ) {
                    of << " [ " << TimeSystem::time2timeOfDay( i.first ) << " - "
                       << TimeSystem::time2timeOfDay( i.second ) << " ] ";
                }
                of << "\n";
            }
            of << "'-------------------------------------------------------------"
                  "---------------------------------------------'\n\n";
        }
    }
}


unordered_map<string, vector<string>> SourceStatistics::readGroups( boost::property_tree::ptree root,
                                                                    const SourceList &sourceList ) noexcept {
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

    std::vector<std::string> members;
    for ( const auto &any : sourceList.getSources() ) {
        members.push_back( any->getName() );
    }
    groups["__all__"] = members;

    return groups;
}

vector<pair<unsigned int, unsigned int>> SourceStatistics::minutesVisible(
    Network &network, const std::shared_ptr<AbstractSource> &source ) {
    vector<pair<unsigned int, unsigned int>> visibleTimes;
    const auto &parameters = source->getPARA();
    unsigned int minVisible = parameters.minNumberOfStations;

    vector<unsigned long> reqSta = parameters.requiredStations;
    vector<unsigned long> ignSta = parameters.ignoreStations;

    for ( auto &any : network.refStations() ) {
        any.setNextEvent( 0 );
        bool dummy = false;
        any.checkForNewEvent( 0, dummy );
    }

    bool flagVisible = false;
    unsigned int visibleStart;

    bool dummy = false;
    for ( unsigned int t = 0; t <= TimeSystem::duration; t += 60 ) {
        unsigned int visible = 0;
        source->checkForNewEvent( t, dummy );

        bool requiredStationNotVisible = false;
        for ( unsigned long staid = 0; staid < network.getNSta(); ++staid ) {
            Station &thisSta = network.refStation( staid );
            thisSta.checkForNewEvent( t, dummy );

            if ( find( ignSta.begin(), ignSta.end(), staid ) != ignSta.end() ) {
                continue;
            }
            if ( !thisSta.getPARA().available || thisSta.getPARA().tagalong ) {
                continue;
            }

            PointingVector p( staid, source->getId() );
            p.setTime( t );

            thisSta.calcAzEl_simple( source, p );

            // check if source is up from station
            bool flag = thisSta.isVisible( p, source->getPARA().minElevation );
            if ( flag ) {
                ++visible;
            } else {
                if ( find( reqSta.begin(), reqSta.end(), staid ) != reqSta.end() ) {
                    requiredStationNotVisible = true;
                    break;
                }
            }
        }
        if ( visible >= minVisible && !flagVisible && !requiredStationNotVisible ) {
            visibleStart = t;
            flagVisible = true;
        } else if ( ( visible < minVisible && flagVisible ) || ( requiredStationNotVisible && flagVisible ) ) {
            flagVisible = false;
            visibleTimes.emplace_back( visibleStart, t );
        }
    }
    if ( flagVisible ) {
        visibleTimes.emplace_back( visibleStart, TimeSystem::duration );
    }

    for ( auto &any : network.refStations() ) {
        any.setNextEvent( 0 );
        bool dummy = false;
        any.checkForNewEvent( 0, dummy );
    }

    return visibleTimes;
}
