//
// Created by matth on 28.01.2018.
//

#include "VieSchedpp.h"


using namespace std;
using namespace VieVS;


VieSchedpp::VieSchedpp(const std::string &inputFile): inputFile_{inputFile}{

    std::size_t found = inputFile.find_last_of("/\\");
    std::string path;
    if(found == std::string::npos){
        path_ = "";
    }else{
        path_ = inputFile.substr(0,found+1);
    }

    ifstream is(inputFile_);
    boost::property_tree::read_xml(is, xml_, boost::property_tree::xml_parser::trim_whitespace);

    try {
        fileName_ = boost::to_lower_copy(xml_.get<std::string>("master.output.experimentName"));
    } catch(const boost::property_tree::ptree_error &e){
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(error) << "unable to open " << inputFile_;
        #endif
        terminate();
    }

}


void VieSchedpp::run() {

    #ifdef VIESCHEDPP_LOG
    init_log();
    #endif

    // get path of input file
    VieVS::Initializer init(xml_);

    // open headerlog and statistics file
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(info) << "start initializing scheduler";
    BOOST_LOG_TRIVIAL(info) << "writing initializer output to: initializer.txt";
    #endif

    ofstream of(path_+"initializer.txt");
    ofstream statisticsOf(path_+"statistics.csv");
    init.statisticsLogHeader(statisticsOf);

    // initialize skd catalogs and lookup table
    readSkdCatalogs();
    LookupTable::initialize();

    // initialize all Parameters
    init.initializeGeneral( of );
    init.initializeAstronomicalParameteres();
    init.initializeObservingMode(skdCatalogs_, of);

    init.createStations(skdCatalogs_, of);
    init.initializeStations();
    init.initializeBaselines();

    init.createSources(skdCatalogs_, of);
    init.precalcSubnettingSrcIds();
    init.initializeSources();
    init.initializeSourceSequence();
    init.initializeCalibrationBlocks( of );
    init.initializeOptimization(of);

    init.initializeHighImpactScanDescriptor(of);
    init.initializeWeightFactors();
    init.initializeSkyCoverages();

    // check if multi scheduling is selected
    bool flag_multiSched = false;
    unsigned long nsched = 1;
    multiSchedParameters_ = init.readMultiSched(of);
    if (!multiSchedParameters_.empty()) {
        flag_multiSched = true;
        nsched = multiSchedParameters_.size();
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(info) << "multi scheduling found ... creating " << nsched << " schedules!";
        #endif
    }
    of.close();

    // check if openmp is available
    #ifdef _OPENMP
    // use openmp to parallelize upcoming for loop

    if(flag_multiSched){
        multiCoreSetup();
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(info) << "Using OpenMP to parallize multi scheduling!";
        #endif
        int nThreads = omp_get_num_threads();

        omp_sched_t omp_sched;
        int chunkSize;
        omp_get_schedule(&omp_sched, &chunkSize);

        string jobScheduling;
        switch (omp_sched){
            case omp_sched_static:  jobScheduling = "static";   break;
            case omp_sched_dynamic: jobScheduling = "dynamic";  break;
            case omp_sched_guided:  jobScheduling = "guided";   break;
            case omp_sched_auto:    jobScheduling = "auto";     break;
        }

        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(info) << boost::format("OpenMP: starting %d threads job scheduling %s chunk size %d") %nThreads %jobScheduling %chunkSize;
        #endif
    }

    #pragma omp parallel for schedule(runtime)
    #else
    if(nsched > 1){
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(warning) << "VLBI Scheduler was not compiled with OpenMP! Recompile it with OpenMP for multi core support";
        #endif
    }
    #endif
    // create all required schedules
    for (int i = 0; i < nsched; ++i) {

        // create initializer and set static parameters for each thread
        Initializer newInit(init);
        newInit.initializeWeightFactors();
        newInit.initializeSkyCoverages();

        // get version number
        int version = 0;
        if (flag_multiSched) {
            version = i+1;
        }

        // get file name
        string fname = fileName_;

        // if you have multi schedule append version number to file name
        if (flag_multiSched) {
            fname.append((boost::format("_v%03d") % (i+1)).str());
            #ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL(info) << boost::format("creating multi scheduling version %d of %d") % (i+1) % nsched;
            #endif
        }

        // add multi scheduling parameters
        if (flag_multiSched){
            newInit.applyMultiSchedParameters(multiSchedParameters_[i]);
        }

        // create scheduler and start scheduling
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(info) << "start scheduling";
        #endif
        VieVS::Scheduler scheduler = VieVS::Scheduler(newInit, path_, fname);
        scheduler.start();

        // create output
        #ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL(info) << "start writing output";
        #endif

        VieVS::Output output(scheduler, path_, fname, version);
        output.createAllOutputFiles(statisticsOf, skdCatalogs_);

        if(flag_multiSched){
        #ifdef VIESCHEDPP_LOG
         BOOST_LOG_TRIVIAL(info) << boost::format("version %d finished") % (i + 1);
        #endif
        }
    }
    statisticsOf.close();

    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(info) << "VieSchedpp is closing";
    BOOST_LOG_TRIVIAL(info) << "created scans: " << Scan::numberOfCreatedObjects();
    BOOST_LOG_TRIVIAL(info) << "created observations: " << Observation::numberOfCreatedObjects();
    BOOST_LOG_TRIVIAL(info) << "created antenna pointings: " << PointingVector::numberOfCreatedObjects();
    #endif
}

void VieSchedpp::readSkdCatalogs() {

    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "read skd catalogs";
    #endif
    vector<string> staNames;
    boost::property_tree::ptree ptree_stations = xml_.get_child("master.general.stations");
    auto it = ptree_stations.begin();
    while (it != ptree_stations.end()) {
        auto item = it->second.data();
        staNames.push_back(item);
        ++it;
    }
    skdCatalogs_.setStationNames(staNames);
    skdCatalogs_.setCatalogFilePathes(xml_.get_child("master.catalogs"));
    skdCatalogs_.initializeStationCatalogs();
    skdCatalogs_.initializeSourceCatalogs();
    auto modeName = xml_.get_optional<std::string>("master.mode.skdMode");
    if(modeName.is_initialized()){
        skdCatalogs_.initializeModesCatalogs(*modeName);
    }
}

#ifdef  _OPENMP
void VieSchedpp::multiCoreSetup() {

    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(debug) << "read parallel processing setup";
    #endif
    std::string threads = xml_.get<std::string>("master.multiCore.threads","auto");

    int chunkSize = xml_.get<int>("master.multiCore.chunkSize",-1);

    int nThreads = 1;
    if(threads == "manual"){
        nThreads = xml_.get<int>("master.multiCore.nThreads",1);
    } else if (threads == "single"){
        nThreads = 1;
    } else if (threads == "auto"){
        nThreads = thread::hardware_concurrency();
    }

    omp_set_num_threads(nThreads);

    string jobSchedulingString = xml_.get<std::string>("master.multiCore.jobScheduling","auto");
    if(jobSchedulingString == "auto"){
        omp_set_schedule(omp_sched_auto,    chunkSize);
    }else if(jobSchedulingString == "static"){
        omp_set_schedule(omp_sched_static,  chunkSize);
    }else if(jobSchedulingString == "dynamic"){
        omp_set_schedule(omp_sched_dynamic, chunkSize);
    }else if(jobSchedulingString == "guided"){
        omp_set_schedule(omp_sched_guided,  chunkSize);
    }
}
#endif

#ifdef VIESCHEDPP_LOG
void VieSchedpp::init_log() {

    boost::log::add_common_attributes();

    string logSeverity = xml_.get<string>("master.general.logSeverity","info");
    if(logSeverity == "trace"){
        boost::log::core::get()->set_filter(
                boost::log::trivial::severity >= boost::log::trivial::trace
        );
    } else if(logSeverity == "debug"){
        boost::log::core::get()->set_filter(
                boost::log::trivial::severity >= boost::log::trivial::debug
        );
    } else if(logSeverity == "info"){
        boost::log::core::get()->set_filter(
                boost::log::trivial::severity >= boost::log::trivial::info
        );
    } else if(logSeverity == "warning"){
        boost::log::core::get()->set_filter(
                boost::log::trivial::severity >= boost::log::trivial::warning
        );
    } else if(logSeverity == "error"){
        boost::log::core::get()->set_filter(
                boost::log::trivial::severity >= boost::log::trivial::error
        );
    } else if(logSeverity == "fatal"){
        boost::log::core::get()->set_filter(
                boost::log::trivial::severity >= boost::log::trivial::fatal
        );
    }

    auto fmtTimeStamp = boost::log::expressions::
    format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f");
    auto fmtThreadId = boost::log::expressions::
    attr<boost::log::attributes::current_thread_id::value_type>("ThreadID");
    auto fmtSeverity = boost::log::expressions::
    attr<boost::log::trivial::severity_level>("Severity");
    boost::log::formatter logFmt =
            boost::log::expressions::format("[%1%] (%2%) [%3%] %4%")
            % fmtTimeStamp % fmtThreadId % fmtSeverity
            % boost::log::expressions::smessage;

    /* console sink */
    auto consoleSink = boost::log::add_console_log(std::clog);
    consoleSink->set_formatter(logFmt);
    consoleSink->set_filter(
            boost::log::trivial::severity >= boost::log::trivial::info
    );


    /* fs sink */
    auto fsSink = boost::log::add_file_log(
            boost::log::keywords::file_name = path_+"VieSchedpp_%Y-%m-%d_%H-%M-%S.%3N.log",
//            boost::log::keywords::file_name = path_+"VieSchedpp_%3N.log",
            boost::log::keywords::rotation_size = 10 * 1024 * 1024,
            boost::log::keywords::min_free_space = 30 * 1024 * 1024,
            boost::log::keywords::open_mode = std::ios_base::app);
    fsSink->set_formatter(logFmt);
    fsSink->locked_backend()->auto_flush(true);
}
#endif
