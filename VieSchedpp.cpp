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

#include "VieSchedpp.h"


using namespace std;
using namespace VieVS;


VieSchedpp::VieSchedpp( const std::string &inputFile ) : inputFile_{ inputFile } {
    std::size_t found = inputFile.find_last_of( "/\\" );
    std::string path;
    if ( found == std::string::npos ) {
        path_ = "";
    } else {
        path_ = inputFile.substr( 0, found + 1 );
    }

    ifstream is( inputFile_ );
    boost::property_tree::read_xml( is, xml_, boost::property_tree::xml_parser::trim_whitespace );

    try {
        sessionName_ = boost::to_lower_copy( xml_.get<std::string>( "VieSchedpp.general.experimentName", "dummy" ) );
    } catch ( const boost::property_tree::ptree_error &e ) {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( error ) << "unable to open " << inputFile_;
#else
        cout << "[error] unable to open " << inputFile_;
#endif
    }
}


void VieSchedpp::run() {
    init_log();

    string versionNr = util::version();
    string GUI_versionNr = xml_.get( "VieSchedpp.software.GUI_version", "unknown" );
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "VieSched++ version: " << versionNr;
    BOOST_LOG_TRIVIAL( info ) << "VieSched++ GUI version: " << GUI_versionNr;
#else
    cout << "[info] VieSched++ version: " << versionNr;
    cout << "[info] VieSched++ GUI version: " << GUI_versionNr;
#endif

    // get path of input file
    VieVS::Initializer init( xml_ );

// open headerlog and statistics file
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "start initializing scheduler";
    BOOST_LOG_TRIVIAL( info ) << "writing initializer output to: initializer.txt";
#else
    cout << "[info] start initializing scheduler";
    cout << "[info] writing initializer output to: initializer.txt";
#endif

    ofstream of;
    if ( xml_.get( "VieSchedpp.output.initializer_log", true ) ) {
        of.open( path_ + sessionName_ + "_initializer.txt" );
    }

    ofstream statisticsOf( path_ + "statistics.csv" );

    // initialize skd catalogs and lookup table
    readSkdCatalogs();
    LookupTable::initialize();

    // initialize all Parameters
    init.initializeGeneral( of );
    Initializer::initializeAstronomicalParameteres();
    init.initializeFocusCornersAlgorithm();
    init.initializeObservingMode( skdCatalogs_, of );

    init.createSources( skdCatalogs_, of );
    init.createStations( skdCatalogs_, of );
    init.connectObservingMode( of );

    init.initializeStations();
    init.precalcAzElStations();
    init.initializeBaselines();

    init.precalcSubnettingSrcIds();
    init.initializeSources();
    init.initializeSourceSequence();
    init.initializeAstrometricCalibrationBlocks( of );
    init.initializeOptimization( of );

    init.initializeCalibrationBlocks();
    init.initializeHighImpactScanDescriptor( of );
    init.initializeWeightFactors();
    init.initializeSkyCoverages();

    // check if multi scheduling is selected
    bool flag_multiSched = false;
    unsigned long nsched = 1;
    multiSchedParameters_ = init.readMultiSched( of );
    if ( !multiSchedParameters_.empty() ) {
        flag_multiSched = true;
        nsched = multiSchedParameters_.size();
    }
    of.close();

    init.statisticsLogHeader( statisticsOf, multiSchedParameters_ );
    int counter = 0;

// check if openmp is available
#ifdef _OPENMP
    // use openmp to parallelize upcoming for loop

    if ( flag_multiSched ) {
        multiCoreSetup();
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( info ) << "using OpenMP to parallelize multi scheduling!";
#else
        cout << "[info] using OpenMP to parallize multi scheduling!\n";
#endif

        omp_sched_t omp_sched;
        int chunkSize;
        omp_get_schedule( &omp_sched, &chunkSize );

        string jobScheduling;
        switch ( omp_sched ) {
            case omp_sched_static:
                jobScheduling = "static";
                break;
            case omp_sched_dynamic:
                jobScheduling = "dynamic";
                break;
            case omp_sched_guided:
                jobScheduling = "guided";
                break;
            case omp_sched_auto:
                jobScheduling = "auto";
                break;
        }

#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( info ) << boost::format( "OpenMP: job scheduling %s chunk size %d" ) % jobScheduling %
                                         chunkSize;
        BOOST_LOG_TRIVIAL( info ) << boost::format( "number of threads %d" ) % omp_get_num_threads();
#else
        cout << boost::format( "[info] OpenMP: job scheduling %s chunk size %d\n" ) % jobScheduling % chunkSize;
        cout << boost::format( "number of threads %d\n" ) % omp_get_num_threads();
#endif
    }
#else
    if ( nsched > 1 ) {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( warning )
            << "VieSchedpp was not compiled with OpenMP! Recompile it with OpenMP for multi core support";
#else
        cout << "[warning] VieSchedpp was not compiled with OpenMP! Recompile it with OpenMP for multi core support\n";
#endif
    }
#endif

    int versionOffset = xml_.get( "VieSchedpp.general.versionOffset", 0 );
    int startCounter = 0;

    int nsched_total = nsched;
    int maxGeneration = 1;
    if ( xml_.get_child_optional( "VieSchedpp.multisched.genetic" ).is_initialized() ) {
        maxGeneration = xml_.get( "VieSchedpp.multisched.genetic.evolutions", 2 );
        int n_it = xml_.get( "VieSchedpp.multisched.genetic.evolutions", 2 );
        int n = xml_.get( "VieSchedpp.multisched.genetic.population_size", 32 );
        nsched_total += ( n_it - 1 ) * n;
    }

    unsigned int population_size = 32;
    double keep_best = 0.1;
    double keep_random = 0.025;
    double mutation_factor = 0.5;

    for ( int i_generation = 0; i_generation < maxGeneration; ++i_generation ) {
        // main scheduling code start
#ifdef _OPENMP
#pragma omp parallel for schedule( runtime )
#endif
        // create all required schedules
        for ( int i = 0; i < nsched; ++i ) {
            // create initializer and set static parameters for each thread
            Initializer newInit( init );
            newInit.initializeWeightFactors();
            newInit.initializeSkyCoverages();

            // get version number
            int version = startCounter + versionOffset;
            if ( flag_multiSched ) {
                version += i + 1;
                // change version number in case you only process one solution
                auto o_version = xml_.get_optional<int>( "VieSchedpp.multisched.version" );
                if ( o_version.is_initialized() ) {
                    version = *o_version;
                }
            }

            // get file name
            string fname = sessionName_;

            // increment counter of multi scheduling version
#ifdef _OPENMP
#pragma omp atomic
#endif
            ++counter;
            string prefix = "";
            if ( version > 0 ) {
                prefix = ( boost::format( "version %d: " ) % version ).str();
                fname.append( ( boost::format( "_v%03d" ) % ( version ) ).str() );
            }
            // if you have multi schedule append version number to file name
            if ( flag_multiSched ) {
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( info ) << boost::format( "creating multi scheduling version %d (%d of %d)" ) %
                                                 version % counter % nsched_total;
#else
                cout << boost::format( "[info] creating multi scheduling version %d (%d of %d)\n" ) % version %
                            counter % nsched;
#endif
            }

            // add multi scheduling parameters
            if ( flag_multiSched ) {
                newInit.applyMultiSchedParameters( multiSchedParameters_[startCounter + i] );
            }
            VieVS::Scheduler scheduler = VieVS::Scheduler( newInit, path_, fname );
            scheduler.start();

// create output
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( info ) << prefix << "start writing output";
#else
            cout << "[info] " + prefix + "start writing output";
#endif

            VieVS::Output output( scheduler, path_, fname, version );
            output.createAllOutputFiles( statisticsOf, skdCatalogs_ );

            if ( auto ctree = xml_.get_child_optional( "VieSchedpp.simulator" ).is_initialized() ) {
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( info ) << prefix << "start simulation";
#else
                cout << "[info] " + prefix + "start simulation";
#endif
                VieVS::Simulator simulator( output, path_, fname, version );
                simulator.start();

#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( info ) << prefix << "start analysis";
#else
                cout << "[info] " + prefix + "start analysis";
#endif
                VieVS::Solver solver( simulator, fname );
                solver.start();
                solver.writeStatistics( statisticsOf );
            }

#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( info ) << prefix << "finished";
#else
            cout << boost::format( "[info] " + prefix + "finished" ) % ( i + 1 );
#endif
        }

        if ( auto ctree = xml_.get_child_optional( "VieSchedpp.simulator" ).is_initialized() ) {
            map<int, double> scores = summarizeSimulationResult();

            // generate new population of multi-scheduling parameters
            if ( nsched > 0 && i_generation + 1 < maxGeneration ) {
                startCounter += nsched;
                vector<MultiScheduling::Parameters> newPara =
                    MultiScheduling::evolution_step( i_generation, multiSchedParameters_, scores, xml_ );
                nsched = newPara.size();
                multiSchedParameters_.insert( multiSchedParameters_.end(), newPara.begin(), newPara.end() );
            }
        }
    }

    statisticsOf.close();

    // TODO: temporary output of evolution (maybe remove this in future)
    if ( maxGeneration > 1 ) {
        ofstream fid_genOutput( path_ + "evolution.csv" );
        int n_it = xml_.get( "VieSchedpp.multisched.genetic.evolutions", 2 );
        int n = xml_.get( "VieSchedpp.multisched.genetic.population_size", 32 );
        double best_f = xml_.get( "VieSchedpp.multisched.genetic.select_best_percent", 20.0 ) / 100;
        double random_f = xml_.get( "VieSchedpp.multisched.genetic.select_random_percent", 5.0 ) / 100;
        double mutation = xml_.get( "VieSchedpp.multisched.genetic.mutation_acceleration", 0.5 );
        double minMutation = xml_.get( "VieSchedpp.multisched.genetic.min_mutation_percent", 10.0 ) / 100;
        int n_parents = xml_.get( "VieSchedpp.multisched.genetic.parents_for_crossover", 2 );
        fid_genOutput << boost::format( "# %d %d %f %f %f %f %d\n" ) % n_it % n % best_f % random_f % mutation %
                             minMutation % n_parents;
        fid_genOutput << "score,";
        multiSchedParameters_[0].statisticsHeaderOutput( fid_genOutput );
        fid_genOutput << endl;
        map<int, double> scores = summarizeSimulationResult( false );
        vector<double> scores_vec;
        for ( const auto &any : scores ) {
            scores_vec.push_back( any.second );
        }
        for ( int i = 0; i < multiSchedParameters_.size(); ++i ) {
            fid_genOutput << scores_vec[i] << "," << multiSchedParameters_[i].statisticsOutput() << endl;
        }
    }


#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "VieSched++ is closing";
    BOOST_LOG_TRIVIAL( info ) << "created scans: " << Scan::numberOfCreatedObjects();
    BOOST_LOG_TRIVIAL( info ) << "created observations: " << Observation::numberOfCreatedObjects();
    BOOST_LOG_TRIVIAL( info ) << "created antenna pointings: " << PointingVector::numberOfCreatedObjects();
#else
    cout << "[info] VieSched++ is closing";
    cout << "[info] created scans: " << Scan::numberOfCreatedObjects();
    cout << "[info] created observations: " << Observation::numberOfCreatedObjects();
    cout << "[info] created antenna pointings: " << PointingVector::numberOfCreatedObjects();
#endif
}


void VieSchedpp::readSkdCatalogs() {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "read skd catalogs";
#endif
    const auto &ptree_stations = xml_.get_child_optional( "VieSchedpp.general.stations" );
    if ( ptree_stations.is_initialized() ) {
        vector<string> staNames = util::getStationNames( xml_ );
        skdCatalogs_.setStationNames( staNames );
        skdCatalogs_.setCatalogFilePathes( xml_.get_child( "VieSchedpp.catalogs" ) );
        skdCatalogs_.initializeStationCatalogs();
        skdCatalogs_.initializeSourceCatalogs();
        auto modeName = xml_.get_optional<std::string>( "VieSchedpp.mode.skdMode" );
        if ( modeName.is_initialized() ) {
            skdCatalogs_.initializeModesCatalogs( *modeName );
        }
    } else {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( fatal ) << "no stations selected";
#else
        cout << "[fatal] no stations selected";
        terminate();
#endif
    }
}


void VieSchedpp::multiCoreSetup() {
#ifdef _OPENMP

#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "read parallel processing setup";
#endif
    std::string threads = xml_.get<std::string>( "VieSchedpp.multiCore.threads", "auto" );

    int chunkSize = xml_.get<int>( "VieSchedpp.multiCore.chunkSize", -1 );

    int nThreads = 1;
    if ( threads == "manual" ) {
        nThreads = xml_.get<int>( "VieSchedpp.multiCore.nThreads", 1 );
    } else if ( threads == "single" ) {
        nThreads = 1;
    } else if ( threads == "auto" ) {
        nThreads = std::thread::hardware_concurrency();
        //        nThreads = 4;
    }

    omp_set_num_threads( nThreads );

    string jobSchedulingString = xml_.get<std::string>( "VieSchedpp.multiCore.jobScheduling", "auto" );
    if ( jobSchedulingString == "auto" ) {
        omp_set_schedule( omp_sched_auto, chunkSize );
    } else if ( jobSchedulingString == "static" ) {
        omp_set_schedule( omp_sched_static, chunkSize );
    } else if ( jobSchedulingString == "dynamic" ) {
        omp_set_schedule( omp_sched_dynamic, chunkSize );
    } else if ( jobSchedulingString == "guided" ) {
        omp_set_schedule( omp_sched_guided, chunkSize );
    }
#endif
}


void VieSchedpp::init_log() {
#ifdef VIESCHEDPP_LOG

    boost::log::add_common_attributes();

    string logSeverityFile = xml_.get<string>( "VieSchedpp.general.logSeverityFile", "info" );
    string logSeverityConsole = xml_.get<string>( "VieSchedpp.general.logSeverityConsole", "info" );
    if ( logSeverityFile == "trace" ) {
        boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::trace );
        Flags::logTrace = true;
    } else if ( logSeverityFile == "debug" ) {
        boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::debug );
        Flags::logDebug = true;
        Flags::logTrace = true;
    } else if ( logSeverityFile == "info" ) {
        boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::info );
    } else if ( logSeverityFile == "warning" ) {
        boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::warning );
    } else if ( logSeverityFile == "error" ) {
        boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::error );
    } else if ( logSeverityFile == "fatal" ) {
        boost::log::core::get()->set_filter( boost::log::trivial::severity >= boost::log::trivial::fatal );
    }

    auto fmtTimeStamp =
        boost::log::expressions::format_date_time<boost::posix_time::ptime>( "TimeStamp", "%Y-%m-%d %H:%M:%S.%f" );
    auto fmtSeverity = boost::log::expressions::attr<boost::log::trivial::severity_level>( "Severity" );

    auto tmp = xml_.get_child_optional( "VieSchedpp.multisched" );
    bool withThreadId;
    if ( tmp.is_initialized() ) {
        std::string threads = xml_.get<std::string>( "VieSchedpp.multiCore.threads", "auto" );
        int nThreads = xml_.get<int>( "VieSchedpp.multiCore.nThreads", 1 );
        withThreadId = !( threads == "single" || ( threads == "manual" && nThreads == 1 ) );
    } else {
        withThreadId = false;
    }

    boost::log::formatter logFmt;
    if ( withThreadId ) {
        auto fmtThreadId =
            boost::log::expressions::attr<boost::log::attributes::current_thread_id::value_type>( "ThreadID" );
        logFmt = boost::log::expressions::format( "[%1%] (%2%) [%3%] %4%" ) % fmtTimeStamp % fmtThreadId % fmtSeverity %
                 boost::log::expressions::smessage;

    } else {
        logFmt = boost::log::expressions::format( "[%1%] [%2%] %3%" ) % fmtTimeStamp % fmtSeverity %
                 boost::log::expressions::smessage;
    }

    /* console sink */
    auto consoleSink = boost::log::add_console_log( std::cout );
    consoleSink->set_formatter( logFmt );

    if ( logSeverityConsole == "trace" ) {
        consoleSink->set_filter( boost::log::trivial::severity >= boost::log::trivial::trace );
        Flags::logTrace = true;
    } else if ( logSeverityConsole == "debug" ) {
        consoleSink->set_filter( boost::log::trivial::severity >= boost::log::trivial::debug );
        Flags::logDebug = true;
        Flags::logTrace = true;
    } else if ( logSeverityConsole == "info" ) {
        consoleSink->set_filter( boost::log::trivial::severity >= boost::log::trivial::info );
    } else if ( logSeverityConsole == "warning" ) {
        consoleSink->set_filter( boost::log::trivial::severity >= boost::log::trivial::warning );
    } else if ( logSeverityConsole == "error" ) {
        consoleSink->set_filter( boost::log::trivial::severity >= boost::log::trivial::error );
    } else if ( logSeverityConsole == "fatal" ) {
        consoleSink->set_filter( boost::log::trivial::severity >= boost::log::trivial::fatal );
    }

    /* fs sink */
    auto fsSink = boost::log::add_file_log(
        boost::log::keywords::file_name = path_ + "VieSchedpp_%Y-%m-%d_%H-%M-%S.%3N.log",
        //            boost::log::keywords::file_name = path_+"VieSchedpp_%3N.log",
        boost::log::keywords::rotation_size = 10 * 1024 * 1024, boost::log::keywords::min_free_space = 30 * 1024 * 1024,
        boost::log::keywords::open_mode = std::ios_base::app );
    fsSink->set_formatter( logFmt );
    fsSink->locked_backend()->auto_flush( true );
    if ( logSeverityConsole == "warning" ) {
        cout << "Note: only [WARNING], [ERROR] and [FATAL] messages are displayed!\n";
    }
    if ( logSeverityConsole == "error" ) {
        cout << "Note: only [ERROR] and [FATAL] messages are displayed!\n";
    }
    if ( logSeverityConsole == "fatal" ) {
        cout << "Note: only [FATAL] messages are displayed!\n";
    }
#endif
}

std::map<int, double> VieSchedpp::summarizeSimulationResult( bool output ) {
    ofstream of( path_ + "simulation_summary.txt" );
    map<int, double> scores;

    vector<string> types{ "mean formal errors", "repeatability" };
    map<int, double> mfe_costs;
    map<int, double> rep_costs;
    map<int, vector<double>> storage;

    for ( const auto &type : types ) {
        of << type << ":\n";

        ifstream in( path_ + "statistics.csv" );
        if ( in.is_open() ) {
            string header;
            getline( in, header );
            vector<string> splitHeader;
            boost::split( splitHeader, header, boost::is_any_of( "," ), boost::token_compress_on );
            vector<tuple<string, int, double>> priorityLookup = getPriorityCoefficients( type, splitHeader );

            storage = map<int, vector<double>>();
            string line;
            while ( getline( in, line ) ) {
                vector<double> vals;
                vector<string> splitLine;
                boost::split( splitLine, line, boost::is_any_of( "," ), boost::token_compress_on );

                int idx;
                try {
                    idx = boost::lexical_cast<int>( splitLine[0] );
                } catch ( const boost::bad_lexical_cast & ) {
                    idx = -1;
                }

                int c = 0;
                for ( const auto &any : priorityLookup ) {
                    double val;
                    try {
                        val = boost::lexical_cast<double>( splitLine[get<1>( any )] );
                    } catch ( const boost::bad_lexical_cast & ) {
                        val = numeric_limits<double>::quiet_NaN();
                    }
                    if ( val == 9999 ) {
                        val = numeric_limits<double>::quiet_NaN();
                    }
                    ++c;
                    vals.push_back( val );
                }
                storage[idx] = vals;
            }
            map<int, double> costs = listBest( of, type, storage, priorityLookup );
            if ( type == "mean formal errors" ) {
                mfe_costs = move( costs );
            } else if ( type == "repeatability" ) {
                rep_costs = move( costs );
            }

            in.close();
            of << endl;
        }
    }
    scores = printRecommendation( mfe_costs, rep_costs, storage, output );
    return scores;
}

vector<tuple<string, int, double>> VieSchedpp::getPriorityCoefficients( const string &type,
                                                                        const std::vector<std::string> &header ) {
    vector<tuple<string, int, double>> v;

    const auto &tree = xml_.get_child( "VieSchedpp.priorities" );

    string prefix;
    if ( type == "mean formal errors" ) {
        prefix = "sim_mean_formal_error_";
    } else {
        prefix = "sim_repeatability_";
    }

    for ( const auto &any : tree ) {
        if ( any.first == "variable" ) {
            string name = any.second.get( "<xmlattr>.name", "" );
            if ( name.empty() ) {
                continue;
            }
            auto val = any.second.get_value<double>();

            if ( name == "#obs" ) {
                auto it = find( header.begin(), header.end(), "n_observations" );
                if ( it != header.end() ) {
                    int idx = distance( header.begin(), it );
                    v.emplace_back( name, idx, val );
                }
            } else if ( name == "EOP" ) {
                vector<string> EOPs{ prefix + "x_pol_[muas]", prefix + "y_pol_[muas]", prefix + "dUT1_[mus]",
                                     prefix + "x_nut_[muas]", prefix + "y_nut_[muas]" };
                vector<string> EOP_name{ "XPO", "YPO", "dUT1", "NUTX", "NUTY" };
                val /= 5;
                int c = 0;
                for ( const auto &n : EOPs ) {
                    auto it = find( header.begin(), header.end(), n );
                    if ( it != header.end() ) {
                        int idx = distance( header.begin(), it );
                        v.emplace_back( EOP_name[c], idx, val );
                    }
                    ++c;
                }

            } else if ( name == "stations" ) {
                const auto &stas = xml_.get_child( "VieSchedpp.general.stations" );
                int nsta = 0;
                for ( const auto &station : stas ) {
                    ++nsta;
                }
                val /= nsta;
                for ( const auto &station : stas ) {
                    const string &sta = station.second.get_value<string>();
                    auto it = find( header.begin(), header.end(), prefix + sta );
                    if ( it != header.end() ) {
                        int idx = distance( header.begin(), it );
                        v.emplace_back( sta, idx, val );
                    }
                }
            } else if ( name == "XPO" ) {
                auto it = find( header.begin(), header.end(), prefix + "x_pol_[muas]" );
                if ( it != header.end() ) {
                    int idx = distance( header.begin(), it );
                    v.emplace_back( name, idx, val );
                }
            } else if ( name == "YPO" ) {
                auto it = find( header.begin(), header.end(), prefix + "y_pol_[muas]" );
                if ( it != header.end() ) {
                    int idx = distance( header.begin(), it );
                    v.emplace_back( name, idx, val );
                }
            } else if ( name == "dUT1" ) {
                auto it = find( header.begin(), header.end(), prefix + "dUT1_[mus]" );
                if ( it != header.end() ) {
                    int idx = distance( header.begin(), it );
                    v.emplace_back( name, idx, val );
                }
            } else if ( name == "NUTX" ) {
                auto it = find( header.begin(), header.end(), prefix + "x_nut_[muas]" );
                if ( it != header.end() ) {
                    int idx = distance( header.begin(), it );
                    v.emplace_back( name, idx, val );
                }
            } else if ( name == "NUTY" ) {
                auto it = find( header.begin(), header.end(), prefix + "y_nut_[muas]" );
                if ( it != header.end() ) {
                    int idx = distance( header.begin(), it );
                    v.emplace_back( name, idx, val );
                }
            } else {
                auto it = find( header.begin(), header.end(), prefix + name );
                if ( it != header.end() ) {
                    int idx = distance( header.begin(), it );
                    v.emplace_back( name, idx, val );
                }
            }
        }
    }
    return v;
}


map<int, double> VieSchedpp::listBest( ofstream &of, const string &type,
                                       const std::map<int, std::vector<double>> &storage,
                                       const std::vector<std::tuple<std::string, int, double>> &priorityLookup ) {
    unsigned long n = storage.size();
    double percentile = xml_.get( "VieSchedpp.priorities.percentile", 0.75 );
    long nq = lround( n * percentile - 1 );
    if ( nq >= n ) {
        nq = n - 1;
    }
    if ( nq < 0 ) {
        nq = 0;
    }

    map<int, double> costs;
    vector<int> versions;
    for ( const auto &a : storage ) {
        versions.push_back( a.first );
        costs[a.first] = 0;
    }

    for ( int j = 0; j < priorityLookup.size(); ++j ) {
        const string &name = get<0>( priorityLookup[j] );
        double scale = get<2>( priorityLookup[j] );
        if ( scale == 0 ) {
            continue;
        }

        vector<double> vals;
        for ( int version : versions ) {
            double d = storage.at( version )[j];
            if ( !isnan( d ) ) {
                vals.push_back( d );
            }
        }

        if ( vals.empty() ) {
            continue;
        }

        if ( name[0] == '#' ) {
            sort( vals.begin(), vals.end(), greater<double>() );
        } else {
            sort( vals.begin(), vals.end() );
        }

        double bestVal = vals[0];
        double pVal = vals[nq];

        for ( int version : versions ) {
            double v;
            v = storage.at( version )[j];

            if ( isnan( v ) ) {
                costs[version] = numeric_limits<double>::quiet_NaN();
            }
            double cost;
            if ( bestVal == pVal ) {
                cost = 0;
            } else {
                cost = ( abs( v - bestVal ) ) / ( abs( pVal - bestVal ) ) * scale;
            }
            if ( cost > 1 ) {
                cost = 1;
            }

            costs[version] += cost;
        }
    }

    if ( n > 1 ) {
        double minScore = numeric_limits<double>::max();
        double maxScore = numeric_limits<double>::min();
        for ( const auto &any : costs ) {
            double cost = any.second;
            if ( isnan( cost ) ) {
                continue;
            }
            if ( cost < minScore ) {
                minScore = cost;
            }
            if ( cost > maxScore ) {
                maxScore = cost;
            }
        }

        for ( auto &any : costs ) {
            double cost = any.second;
            if ( isnan( cost ) || maxScore == minScore ) {
                continue;
            }

            cost = ( cost - minScore ) / ( maxScore - minScore );
            costs[any.first] = cost;
        }
    } else {
        costs[versions[0]] = 1;
    }

    of << ".------------------";
    for ( int i = 0; i < priorityLookup.size() - 1; ++i ) {
        of << "-------------";
    }
    of << "-----------.\n";

    of << boost::format( "| %=4s | %=7s | " ) % "v" % "score";
    for ( const auto &v : priorityLookup ) {
        const string &name = get<0>( v );
        of << boost::format( "%=10s | " ) % name;
    }
    of << "\n";

    of << "|------|---------|-";
    for ( int i = 0; i < priorityLookup.size() - 1; ++i ) {
        of << "-----------|-";
    }
    of << "-----------|\n";

    // write to statistics file
    for ( const auto &any : costs ) {
        double cost = any.second;
        string costStr;
        if ( isnan( cost ) ) {
            costStr = "--";
        } else {
            costStr = ( boost::format( "%7.4f" ) % ( 1 - cost ) ).str();
        }

        int version = any.first;
        const vector<double> &vals = storage.at( version );
        of << boost::format( "| %4d | %=7s | %10d | " ) % version % costStr % vals[0];
        bool first = true;
        for ( const auto &v : vals ) {
            if ( first ) {
                first = false;
                continue;
            }
            if ( isnan( v ) || v < 1e-10 ) {
                of << boost::format( "%=10s | " ) % "--";
            } else {
                of << boost::format( "%10.4f | " ) % v;
            }
        }
        of << "\n";
    }
    of << "'------------------";
    for ( int i = 0; i < priorityLookup.size() - 1; ++i ) {
        of << "-------------";
    }
    of << "-----------'\n";

    return costs;
}

map<int, double> VieSchedpp::printRecommendation( const std::map<int, double> &mfe_costs,
                                                  const std::map<int, double> &rep_costs,
                                                  const std::map<int, std::vector<double>> &storage, bool output ) {
    int n = mfe_costs.size();
    map<int, double> costs;
    map<int, double> scores;
    const auto &type = xml_.get_child_optional( "VieSchedpp.priorities.type" );
    if ( type.is_initialized() ) {
        double fraction = xml_.get( "VieSchedpp.priorities.type.fraction", 70.0 );
        double f_mfe = 1 - fraction / 100.;
        double f_rep = fraction / 100.;

        for ( const auto &any : mfe_costs ) {
            int version = any.first;
            double c = mfe_costs.at( version ) * f_mfe + rep_costs.at( version ) * f_rep;
            costs[version] = c;
            scores[version] = 1 - c;
        }
    } else {
        return scores;
    }


    // write recommended to log file
    bool first = true;
    int counter = 0;
    map<double, int> best = util::flip_map( costs );
    for ( const auto &any : best ) {
        double cost = any.first;
        int version = any.second;

        const vector<double> &vals = storage.at( version );

        string prefix;
        if ( first ) {
            prefix = "recommended";
            first = false;
        } else {
            prefix = "alternative";
        }
        if ( output && ( ( cost < 0.1 && counter < 10 ) || counter < 3 ) ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( info ) << boost::format( "%s schedule: version %d (score: %.4f # obs: %d)" ) % prefix %
                                             version % ( 1 - cost ) % vals[0];
#else
            cout << boost::format( "%s schedule: version %d (score: %.4f # obs: %d)" ) % prefix % version %
                        ( 1 - cost ) % vals[0];
#endif
            ++counter;
        }
    }
    return scores;
}
