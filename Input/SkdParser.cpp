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

#include "SkdParser.h"


using namespace VieVS;
using namespace std;

unsigned long SkdParser::nextId = 0;


SkdParser::SkdParser(const std::string &filename) : VieVS_Object(nextId++) {
    std::size_t found = filename.find_last_of("/\\");
    if (found == std::string::npos) {
        fpath_ = "";
        fname_ = filename;
    } else {
        fpath_ = filename.substr(0, found + 1);
        fname_ = filename.substr(found + 1);
    }

    std::size_t dot = fname_.find_last_of('.');
    fname_ = fname_.substr(0, dot);

}


void SkdParser::read() {

    string filename = fpath_ + fname_ + ".skd";
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(info) << "read " << filename;
#else
    std::cout << "read " << filename << std::endl;
#endif

    Initializer init;
    vector<string> staNames;
    ifstream fid(filename);

    int bits = 0;
    double samRate = 0;

    if ( !fid.is_open() ) {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(fatal) << "unable to open " << filename;
#else
        cout << "[fatal] unable to open " << filename;
#endif
        terminate();
    } else {
        vector<string> splitVector;
        string line;
        bool found = false;
        bool sourceFound = false;
        bool tapeFound = false;
        bool corSynchFound = false;
        bool calibrationFound = false;
        bool freqFound = false;
        bool bandsFound = false;

        // loop through file
        int counter = 0;
        while ( getline( fid, line ) ) {
            if ( line.empty() || line.at( 0 ) == '*' ) {
                continue;
            }
            string trimmed = boost::trim_copy( line );

            if ( counter == 6 ) {
                boost::split( splitVector, trimmed, boost::is_space(), boost::token_compress_on );
                if ( splitVector[4] == "START" ) {
                    TimeSystem::startTime = TimeSystem::string_doy2ptime( splitVector[5] );
                } else {
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( fatal ) << "session start time not found";
#else
                    cout << "[fatal] session start time not found";
#endif
                    terminate();
                    return;
                }
                if ( splitVector[6] == "END" ) {
                    TimeSystem::endTime = TimeSystem::string_doy2ptime( splitVector[7] );
                } else {
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( fatal ) << "session end time not found";
#else
                    cout << "[fatal] session end time not found";
#endif
                    terminate();
                    return;
                }
            }
            if ( !sourceFound && trimmed.find( "SOURCE" ) != trimmed.npos ) {
                boost::split( splitVector, trimmed, boost::is_space(), boost::token_compress_on );
                for ( int i = 0; i < splitVector.size(); ++i ) {
                    if ( splitVector[i] == "SOURCE" ) {
                        systemDelay_ += boost::lexical_cast<unsigned int>( splitVector[i + 1] );
                    }
                }
                sourceFound = true;
            }
            if ( !tapeFound && trimmed.find( "TAPETM" ) != trimmed.npos ) {
                boost::split( splitVector, trimmed, boost::is_space(), boost::token_compress_on );
                for ( int i = 0; i < splitVector.size(); ++i ) {
                    if ( splitVector[i] == "TAPETM" ) {
                        systemDelay_ += boost::lexical_cast<unsigned int>( splitVector[i + 1] );
                    }
                }
                tapeFound = true;
            }
            if ( !corSynchFound && trimmed.find( "CORSYNCH" ) != trimmed.npos ) {
                boost::split( splitVector, trimmed, boost::is_space(), boost::token_compress_on );
                for ( int i = 0; i < splitVector.size(); ++i ) {
                    if ( splitVector[i] == "CORSYNCH" ) {
                        midob_ = boost::lexical_cast<unsigned int>( splitVector[i + 1] );
                    }
                }
                corSynchFound = true;
            }
            if ( !calibrationFound && trimmed.find( "CALIBRATION" ) != trimmed.npos ) {
                boost::split( splitVector, trimmed, boost::is_space(), boost::token_compress_on );
                for ( int i = 0; i < splitVector.size(); ++i ) {
                    if ( splitVector[i] == "CALIBRATION" ) {
                        preob_ = boost::lexical_cast<unsigned int>( splitVector[i + 1] );
                    }
                }
                calibrationFound = true;
            }

            if ( !found && trimmed == "$STATIONS" ) {
                while ( getline( fid, line ) ) {
                    if ( line.empty() || line.at( 0 ) == '*' ) {
                        continue;
                    }
                    string trimmed2 = boost::trim_copy( line );

                    boost::split( splitVector, trimmed2, boost::is_space(), boost::token_compress_on );
                    if ( splitVector[0] == "A" ) {
                        staNames.push_back( splitVector[2] );
                    } else {
                        found = true;
                        break;
                    }
                    splitVector.clear();
                }
            }
            if ( !bandsFound && trimmed.substr( 0, 3 ) == "SNR" ) {
                set<string> bands;
                bandsFound = true;
                do {
                    boost::split( splitVector, trimmed, boost::is_space(), boost::token_compress_on );
                    for ( int i = 2; i < splitVector.size(); i += 3 ) {
                        string band = splitVector[i];
                        bands.insert( band );
                    }


                    getline( fid, line );
                    if ( line.empty() || line.at( 0 ) == '*' ) {
                        continue;
                    }
                    trimmed = boost::trim_copy( line );
                } while ( trimmed.substr( 0, 3 ) == "SNR" );
                ObservingMode::bands = bands;
            }
            if ( !freqFound && trimmed == "$CODES" ) {
                freqFound = true;
                bool first = true;
                while ( getline( fid, line ) ) {
                    if ( line == "* no sked observind mode used! " ) {
                        break;
                    }
                    if ( line.empty() || line.at( 0 ) == '*' ) {
                        continue;
                    }
                    string trimmed2 = boost::trim_copy( line );

                    boost::split( splitVector, trimmed2, boost::is_space(), boost::token_compress_on );
                    if ( splitVector[0] == "C" ) {
                        if ( splitVector.size() >= 4 ) {
                            string band = splitVector[2];
                            try {
                                auto freq = boost::lexical_cast<double>( splitVector.at( 3 ) );
                                if ( freqs_.find( band ) != freqs_.end() ) {
                                    std::vector<double> &f = freqs_.at( band );
                                    if ( find( f.begin(), f.end(), freq ) == f.end() ) {
                                        f.push_back( freq );
                                    }
                                } else {
                                    std::vector<double> f{ freq };
                                    freqs_[band] = f;
                                }
                            } catch ( const std::exception &e ) {
#ifdef VIESCHEDPP_LOG
                                BOOST_LOG_TRIVIAL( warning ) << "band " << band << " cannot read frequency";
#else
                                cout << "[error] band " << band << " cannot read frequency\n";
#endif
                            }

                            if ( first ) {
                                string txt = splitVector[8];
                                int x = std::count( txt.begin(), txt.end(), ',' );
                                if ( x == 3 ) {
                                    bits = 2;
                                } else {
                                    bits = 1;
                                }
                            }
                            first = false;

                        } else {
#ifdef VIESCHEDPP_LOG
                            BOOST_LOG_TRIVIAL( warning ) << "cannot read frequency setup";
#else
                            cout << "cannot read frequency setup\n";
#endif
                        }
                    }

                    if ( splitVector[0] == "R" ) {
                        samRate = boost::lexical_cast<double>( splitVector.at( 2 ) );
                    }

                    if ( splitVector[0] == "L" ) {
                        break;
                    }
                }
            }
            ++counter;
        }
    }
    fid.close();

    int sec_ = TimeSystem::startTime.time_of_day().total_seconds();
    TimeSystem::mjdStart = TimeSystem::startTime.date().modjulian_day() + sec_ / 86400.0;

    int sec = util::duration( TimeSystem::startTime, TimeSystem::endTime );
    if ( sec < 0 ) {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( error ) << "duration is less than zero seconds";
#else
        cout << "[error] duration is less than zero seconds";
#endif
    }
    auto duration = static_cast<unsigned int>( sec );
    TimeSystem::duration = duration;

    skd_.setStationNames( staNames );
    skd_.setCatalogFilePathes(fpath_ + fname_ + ".skd");
    skd_.initializeStationCatalogs();
    skd_.initializeSourceCatalogs();

    string path = fpath_;
    path.append( "/skdParser.log" );
    ofstream of( path );

    std::unordered_map<std::string, double> band2wavelength{ { "L", 0.3 },      { "S", 0.131 },
//                                                             { "C", 0.06 },
                                                             { "X", 0.0349 },   { "Ku", 0.0231 }, { "K", 0.0134 },
                                                             { "Ka", 0.01000 }, { "E", 0.005 },   { "W", 0.00375 },
                                                             { "A", 0.0921 },   { "B", 0.0545 }, { "C", 0.0453 },
                                                             { "D", 0.0287 }, };
    for ( const auto &any : freqs_ ) {
        double mfreq = accumulate( any.second.begin(), any.second.end(), 0.0 );
        if ( any.first == "X" ) {
            mfreq += any.second.front();
            mfreq += any.second.back();
            mfreq /= ( any.second.size() + 2 );
        } else {
            mfreq /= any.second.size();
        }
        band2wavelength[any.first] = util::freqency2wavelenth( mfreq * 1e6 );
    }


    std::unordered_map<std::string, unsigned int> band2channel{ { "X", 10 }, { "S", 6 } };
    init.initializeObservingMode( staNames.size(), samRate, bits, band2channel, band2wavelength );

    std::unordered_map<string, ObservingMode::Property> optional{
        { "L", ObservingMode::Property::optional },  { "S", ObservingMode::Property::optional },
//        { "C", ObservingMode::Property::optional },
        { "X", ObservingMode::Property::optional },
        { "Ku", ObservingMode::Property::optional }, { "K", ObservingMode::Property::optional },
        { "Ka", ObservingMode::Property::optional }, { "E", ObservingMode::Property::optional },
        { "W", ObservingMode::Property::optional },  { "A", ObservingMode::Property::optional },
        { "B", ObservingMode::Property::optional },  { "C", ObservingMode::Property::optional },
        { "D", ObservingMode::Property::optional } };

    std::unordered_map<string, ObservingMode::Backup> backup{
        { "L", ObservingMode::Backup::value },  { "S", ObservingMode::Backup::value },
//        { "C", ObservingMode::Backup::value },
        { "X", ObservingMode::Backup::value },
        { "Ku", ObservingMode::Backup::value }, { "K", ObservingMode::Backup::value },
        { "Ka", ObservingMode::Backup::value }, { "E", ObservingMode::Backup::value },
        { "W", ObservingMode::Backup::value },
        { "A", ObservingMode::Backup::internalModel },
        { "B", ObservingMode::Backup::internalModel },
        { "C", ObservingMode::Backup::internalModel },
        { "D", ObservingMode::Backup::internalModel } };

    std::unordered_map<string, double> sourceBackupValue{ { "L", 1 }, { "S", 1 },  { "C", 1 }, { "X", 1 }, { "Ku", 1 },
                                                          { "K", 1 }, { "Ka", 1 }, { "E", 1 }, { "W", 1 }, { "A", 1 },
                                                          { "B", 1 }, { "C", 1 },  { "D", 1 } };

    std::unordered_map<string, double> stationBackupValue{
        { "L", 1000 }, { "S", 1000 }, { "C", 1000 }, { "X", 1000 }, { "Ku", 1000 }, { "K", 1000 }, { "Ka", 1000 },
        { "E", 1000 }, { "W", 1000 }, { "A", 1000 }, { "B", 1000 }, { "C", 1000 },  { "D", 1000 } };

    ObservingMode::sourceProperty = optional;
    ObservingMode::stationProperty = optional;

    ObservingMode::sourceBackup = backup;
    ObservingMode::stationBackup = backup;

    ObservingMode::sourceBackupValue = sourceBackupValue;
    ObservingMode::stationBackupValue = stationBackupValue;

    init.createSources( skd_, of );

//    ==========>>>>>> dummy VGOS simple model <<<<<<==========
//    band2channel = { { "A", 1 }, { "B", 1 }, { "C", 1 }, { "D", 1 },  };
//    init.initializeObservingMode( staNames.size(), 1024, 2, band2channel, band2wavelength );

    init.createStations( skd_, of );
    Initializer::initializeAstronomicalParameteres();
    init.precalcAzElStations();

    network_ = move( init.network_ );
    for (auto &sta : network_.refStations()){
        sta.referenceCableWrap().setMinimumOffsets( 5., 5., 0., 0. );
    }
    sourceList_ = move( init.sourceList_ );
    obsModes_ = move( init.obsModes_ );

    createScans( of );
    copyScanMembersToObjects( of );


    for ( auto &sky : network_.refSkyCoverages() ) {
        sky.calculateSkyCoverageScores();
    }

    of.close();
}


void SkdParser::createScans( std::ofstream &of ) {
    string filename = fpath_ + fname_ + ".skd";
    ifstream fid(filename);
    if ( !fid.is_open() ) {
        of << "ERROR: Unable to open " << filename << " file!;\n";
        return;
    } else {
        string line;
        // read until you reach "$SKED"
        while ( getline( fid, line ) ) {
            if ( line.empty() || line.at( 0 ) == '*' ) {
                continue;
            }
            string trimmed = boost::trim_copy( line );
            if ( trimmed == "$SKED" ) {
                break;
            }
        }

        vector<unsigned int> eols( network_.getNSta(), 0 );  // end of last scan
        // Read SKED block
        int counter = 1;
        while ( getline( fid, line ) ) {
            if ( line.empty() || line.at( 0 ) == '*' ) {
                continue;
            }
            string trimmed = boost::trim_copy( line );
            if ( trimmed[0] == '$' ) {
                break;
            }

            vector<string> splitVector;
            boost::split( splitVector, trimmed, boost::is_space(), boost::token_compress_on );
            const string &srcName = splitVector[0];
            unsigned long srcid = numeric_limits<unsigned long>::max();
            for ( unsigned long isrc = 0; isrc < sourceList_.getNSrc(); ++isrc ) {
                if ( sourceList_.getSource( isrc )->hasName( srcName ) ) {
                    srcid = isrc;
                    break;
                }
            }
            auto preob = boost::lexical_cast<unsigned int>( splitVector[1] );
            auto scanStart = TimeSystem::string_doy2ptime( splitVector[4] );
            auto duration = boost::lexical_cast<unsigned int>( splitVector[5] );
            const string &flags = splitVector[9];
            unsigned long nsta = flags.length() / 2;
            vector<char> oneLetterCode;
            for ( int i = 0; i < flags.length(); i += 2 ) {
                oneLetterCode.push_back( flags[i] );
            }
            vector<char> cableWrapFlags;
            for ( int i = 1; i < flags.length(); i += 2 ) {
                cableWrapFlags.push_back( flags[i] );
            }
            vector<unsigned int> durations;
            for ( size_t i = 11 + nsta; i < splitVector.size(); ++i ) {
                durations.push_back( boost::lexical_cast<unsigned int>( splitVector[i] ) );
            }

            int sec = util::duration( TimeSystem::startTime, scanStart );
            if ( sec < 0 ) {
                of << "ERROR: duration is less than zero seconds!;\n";
            }
            auto time = static_cast<unsigned int>( sec );

            // calc pointingVectors
            vector<PointingVector> pv;
            vector<PointingVector> pv_end;
            vector<unsigned int> thisEols( nsta, 0 );
            vector<unsigned int> slewTimes( nsta, 0 );
            vector<unsigned int> preobTimes( nsta, 0 );
            vector<unsigned int> fieldSystemTimes( nsta, 0 );
            //            vector<unsigned int>idleTimes(nsta,0);
            for ( int i = 0; i < nsta; ++i ) {
                char olc = oneLetterCode[i];
                char cwflag = cableWrapFlags[i];

                string staName;
                for ( const auto &any : skd_.getOneLetterCode() ) {
                    char thisOlc = any.second;
                    if ( olc == thisOlc ) {
                        staName = any.first;
                        break;
                    }
                }
                unsigned long staid = network_.getStation( staName ).getId();

                PointingVector p( staid, srcid );

                p.setTime( time );
                const auto &thisSource = sourceList_.getSource( srcid );
                Station &thisSta = network_.refStation( staid );
                thisSta.calcAzEl_rigorous( thisSource, p );
                bool error = thisSta.getCableWrap().unwrapAzInSection( p, cwflag );
                if ( error ) {
                    pair<double, double> limits = thisSta.getCableWrap().getLimits( cwflag );
                    of << boost::format(
                              "Station %8s scan %4d source %8s time %s azimuth error! Flag: %c (from %7.2f to %7.2f) "
                              "calculated: %7.2f (or %7.2f)\n" ) %
                              thisSta.getName() % counter % thisSource->getName() %
                              TimeSystem::time2string_doy( scanStart ) % cwflag % ( limits.first * rad2deg ) %
                              ( limits.second * rad2deg ) % ( p.getAz() * rad2deg ) % ( p.getAz() * rad2deg - 360 );
                }
                pv.push_back( p );

                PointingVector p_end( staid, srcid );
                p_end.setTime( time + durations[i] );
                thisSta.calcAzEl_rigorous( thisSource, p_end );
                thisSta.getCableWrap().unwrapAzNearAz( p_end, p.getAz() );
                pv_end.push_back( p_end );

                thisEols[i] = eols[staid];
                eols[staid] = p_end.getTime();

                if ( thisSta.getPARA().firstScan ) {
                    thisSta.referencePARA().firstScan = false;
                    fieldSystemTimes[i] = 0;
                    preobTimes[i] = 0;
                    slewTimes[i] = 0;

                } else {
                    unsigned int thisSlewTime = thisSta.getAntenna().slewTime( thisSta.getCurrentPointingVector(), p );
                    if ( thisSlewTime < thisSta.getPARA().minSlewtime ) {
                        thisSlewTime = thisSta.getPARA().minSlewtime;
                    }
                    fieldSystemTimes[i] = systemDelay_;
                    preobTimes[i] = preob;
                    slewTimes[i] = thisSlewTime;
                }
                thisSta.setCurrentPointingVector( p_end );
            }

            Scan scan( pv, thisEols, Scan::ScanType::standard );
            bool valid;

            valid = scan.setScanTimes( thisEols, fieldSystemTimes, slewTimes, preobTimes, time, durations );

            const auto &thisSource = sourceList_.getSource( srcid );
            if ( !valid ) {
                const auto &tmp = scan.getTimes();
                for ( int i = 0; i < nsta; ++i ) {
                    if ( tmp.getObservingTime( i, Timestamp::start ) - tmp.getPreobDuration( i ) <
                         tmp.getSlewTime( i, Timestamp::end ) - 2 ) {
                        //                        unsigned int eost = tmp.getSlewTime(i, Timestamp::end);
                        //                        unsigned int eoit = tmp.getObservingTime(i, Timestamp::start)-
                        //                        tmp.getPreobDuration(i); boost::posix_time::ptime eostp =
                        //                        TimeSystem::internalTime2PosixTime(eost); boost::posix_time::ptime
                        //                        eoitp = TimeSystem::internalTime2PosixTime(eoit);

                        of << boost::format( "Station %8s scan %4d source %8s time %s idle time error!\n" ) %
                                  network_.getStation( scan.getPointingVector( i ).getStaid() ).getName() % counter %
                                  thisSource->getName() % TimeSystem::time2string_doy( scanStart );
                    }
                }
            }

            scan.setPointingVectorsEndtime( move( pv_end ) );

            scan.createDummyObservations( network_ );

            scan.output( counter, network_, thisSource, of );

            scans_.push_back( scan );
            ++counter;
        }
    }
}


void SkdParser::copyScanMembersToObjects( std::ofstream &of ) {
    for ( const auto &scan : scans_ ) {
        unsigned long srcid = scan.getSourceId();

        for ( int i = 0; i < scan.getNSta(); ++i ) {
            const PointingVector &pv = scan.getPointingVector( i );
            unsigned long staid = pv.getStaid();
            const PointingVector &pv_end = scan.getPointingVector( i, Timestamp::end );
            unsigned long nObs = scan.getNObs( staid );
            unsigned int obsDur = pv_end.getTime() - pv.getTime();

            network_.refStation( staid ).addObservingTime( obsDur );
            network_.update( nObs, pv_end );
        }
        for ( int i = 0; i < scan.getNObs(); ++i ) {
            const Observation &obs = scan.getObservation( i );
            network_.update( obs.getBlid() );
        }

        unsigned long nbl = ( scan.getNSta() * ( scan.getNSta() - 1 ) ) / 2;
        unsigned int latestTime = scan.getTimes().getObservingTime( Timestamp::start );
        const auto &thisSource = sourceList_.refSource( srcid );
        thisSource->update( scan.getNSta(), nbl, latestTime, true );
    }
}


std::vector<vector<unsigned int>> SkdParser::getScheduledTimes( const string &station ) {
    vector<vector<unsigned int>> times;

    unsigned long staid = network_.getStation( station ).getId();
    if ( staid == -1 ) {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( error ) << "station name " << station << "unknown";
#else
        cout << "[error] station name " << station << "unknown";
#endif
    } else {
        for ( const auto &scan : scans_ ) {
            int idx = -1;
            for ( int i = 0; i < scan.getNSta(); ++i ) {
                const auto &pv = scan.getPointingVector( i );
                if ( pv.getStaid() == staid ) {
                    idx = i;
                    break;
                }
            }
            if ( idx != -1 ) {
                times.emplace_back( vector<unsigned int>{
                    scan.getTimes().getSlewDuration( idx ), scan.getTimes().getIdleDuration( idx ),
                    scan.getTimes().getPreobDuration( idx ), scan.getTimes().getObservingDuration( idx ) } );
            }
        }
    }
    return times;
}


Scheduler SkdParser::createScheduler(boost::property_tree::ptree xml) {
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(info) << "recreate schedule";
#else
    std::cout << "[info] recreate schedule\n";
#endif

    for ( Station &station : network_.refStations() ) {
        station.referencePARA().systemDelay = systemDelay_;
        station.referencePARA().preob = preob_;
        station.referencePARA().midob = midob_;
    }

    xml.add( "general.startTime", TimeSystem::time2string( TimeSystem::startTime ) );
    xml.add( "general.endTime", TimeSystem::time2string( TimeSystem::endTime ) );

    boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
    xml.add( "created.time", now );


    xml.add("output.experimentName", fname_);
    string description = "created from skd file: " + fpath_ + fname_ + ".skd";

    xml.add( "output.experimentDescription", description );

    Scheduler sched(fname_, fpath_, network_, sourceList_, scans_, xml, obsModes_);
    ofstream dummy;
    sched.checkAndStatistics( dummy );
    return sched;
}


void SkdParser::setLogFiles() {
#ifdef VIESCHEDPP_LOG

    boost::log::add_common_attributes();

    boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::info );

    auto fmtTimeStamp =
        boost::log::expressions::format_date_time<boost::posix_time::ptime>( "TimeStamp", "%Y-%m-%d %H:%M:%S.%f" );
    auto fmtSeverity = boost::log::expressions::attr<boost::log::trivial::severity_level>( "Severity" );

    boost::log::formatter logFmt = boost::log::expressions::format( "[%1%] [%2%] %3%" ) % fmtTimeStamp % fmtSeverity %
                                   boost::log::expressions::smessage;

    auto consoleSink = boost::log::add_console_log( std::cout );
    consoleSink->set_formatter( logFmt );

    consoleSink->set_filter( boost::log::trivial::severity >= boost::log::trivial::info );

    auto fsSink = boost::log::add_file_log(
            boost::log::keywords::file_name = fpath_ + "VieSchedpp_sked_parser_%Y-%m-%d_%H-%M-%S.%3N.log",
        //            boost::log::keywords::file_name = path_+"VieSchedpp_%3N.log",
        boost::log::keywords::rotation_size = 10 * 1024 * 1024, boost::log::keywords::min_free_space = 30 * 1024 * 1024,
            boost::log::keywords::open_mode = std::ios_base::app );
    fsSink->set_formatter( logFmt );
    fsSink->locked_backend()->auto_flush( true );

#endif
}
