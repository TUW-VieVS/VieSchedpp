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

VieSchedpp::VieSchedpp( const std::string &inputFile ) : inputFile_{inputFile} {
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
    init.initializeAstronomicalParameteres();
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
    init.initializeCalibrationBlocks( of );
    init.initializeOptimization( of );

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

// check if openmp is available
#ifdef _OPENMP
    // use openmp to parallelize upcoming for loop

    if ( flag_multiSched ) {
        multiCoreSetup();
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( info ) << "using OpenMP to parallize multi scheduling!";
#else
        cout << "[info] using OpenMP to parallize multi scheduling!";
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
#else
        cout << boost::format( "[info] OpenMP: job scheduling %s chunk size %d" ) % jobScheduling % chunkSize;
#endif
    }

#pragma omp parallel for schedule( runtime )
#else
    if ( nsched > 1 ) {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( warning )
            << "VieSchedpp was not compiled with OpenMP! Recompile it with OpenMP for multi core support";
#else
        cout << "[warning] VieSchedpp was not compiled with OpenMP! Recompile it with OpenMP for multi core support";
#endif
    }
#endif

    // create all required schedules
    for ( int i = 0; i < nsched; ++i ) {
        if ( i == 0 && nsched > 1 ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( info ) << boost::format( "number of threads %d" ) % omp_get_num_threads();
#else
            cout << boost::format( "number of threads %d" ) % omp_get_num_threads();
#endif
        }

        // create initializer and set static parameters for each thread
        Initializer newInit( init );
        newInit.initializeWeightFactors();
        newInit.initializeSkyCoverages();

        // get version number
        int version = 0;
        if ( flag_multiSched ) {
            version = i + 1;
            // change version number in case you only process one solution
            auto o_version = xml_.get_optional<int>( "VieSchedpp.multisched.version" );
            if ( o_version.is_initialized() ) {
                version = *o_version;
            }
        }

        // get file name
        string fname = sessionName_;

        // if you have multi schedule append version number to file name
        if ( flag_multiSched ) {
            fname.append( ( boost::format( "_v%03d" ) % ( version ) ).str() );
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( info ) << boost::format( "creating multi scheduling version %d of %d" ) % version %
                                             nsched;
#else
            cout << boost::format( "[info] creating multi scheduling version %d of %d" ) % ( i + 1 ) % nsched;
#endif
        }

        // add multi scheduling parameters
        if ( flag_multiSched ) {
            newInit.applyMultiSchedParameters( multiSchedParameters_[i] );
        }

// create scheduler and start scheduling
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( info ) << "start scheduling";
#else
        cout << "[info] start scheduling";
#endif
        VieVS::Scheduler scheduler = VieVS::Scheduler( newInit, path_, fname );
        scheduler.start();

// create output
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( info ) << "start writing output";
#else
        cout << "[info] start writing output";
#endif

        VieVS::Output output( scheduler, path_, fname, version );
        output.createAllOutputFiles( statisticsOf, skdCatalogs_ );

        if ( flag_multiSched ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( info ) << boost::format( "version %d finished" ) % version;
#else
            cout << boost::format( "[info] version %d finished" ) % ( i + 1 );
#endif
        }
    }
    statisticsOf.close();

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
        if ( threads == "single" || ( threads == "manual" && nThreads == 1 ) ) {
            withThreadId = false;
        } else {
#ifdef _OPENMP
            withThreadId = true;
#else
            withThreadId = false;
#endif
        }
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
#endif
}
