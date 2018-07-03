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
        cerr << "cannot open parameter file: " << inputFile_ << endl;
        cerr << e.what() << endl;
        terminate();
    }

}


void VieSchedpp::run() {
    cout << "writing header log file to: log_initializer.txt;\n";

    // get path of input file
    VieVS::Initializer init(xml_);

    // open headerlog and statistics file
    ofstream headerLog(path_+"log_initializer.txt");
    ofstream statisticsLog(path_+"statistics.csv");
    init.statisticsLogHeader(statisticsLog);

    // initialize skd catalogs and lookup table
    readSkdCatalogs();
    LookupTable::initialize();

    // initialize all Parameters
    init.initializeGeneral( headerLog );
    init.initializeAstronomicalParameteres();
    init.initializeObservingMode(skdCatalogs_, headerLog);

    init.createStations(skdCatalogs_, headerLog);
    init.initializeStations();
    init.initializeBaselines();

    init.createSources(skdCatalogs_, headerLog);
    init.precalcSubnettingSrcIds();
    init.initializeSources();
    init.initializeSourceSequence();
    init.initializeCalibrationBlocks( headerLog );
    init.initializeOptimization(headerLog);

    init.initializeHighImpactScanDescriptor(headerLog);
    init.initializeWeightFactors();
    init.initializeSkyCoverages();

    // check if multi scheduling is selected
    bool flag_multiSched = false;
    unsigned long nsched = 1;
    multiSchedParameters_ = init.readMultiSched(headerLog);
    if (!multiSchedParameters_.empty()) {
        flag_multiSched = true;
        nsched = multiSchedParameters_.size();
        cout << "multi scheduling found ... creating " << nsched << " schedules!;\n";
    }
    headerLog.close();

    // check if openmp is available
#ifdef _OPENMP
    if(flag_multiSched){
        readMultiCoreSetup();
        cout << "Using OpenMP to parallize multi scheduling;\n";
        cout << (boost::format("OpenMP: starting %d threads;\n") % nThreads_).str();
    }
    // use openmp to parallelize upcoming for loop
    omp_set_num_threads(nThreads_);
#pragma omp parallel for
#else
    if(nsched > 1){
        cout << "VLBI Scheduler was not compiled with OpenMP! Recompile it with OpenMP for multi core support!";
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
        }

        // if you have multicore support add thread number as prefix to output
        string threadNumberPrefix;
        if (flag_multiSched){
            // get thread number if you use openmp
            #ifdef  _OPENMP
            threadNumberPrefix = (boost::format("thread %d: ") % omp_get_thread_num()).str();
            #endif

            cout << boost::format("%screating multi scheduling version %d of %d;\n") % threadNumberPrefix % (i+1) % nsched;
            cout << boost::format("version %d: writing log file to: %s.log;\n") % (i+1) % fname;
        }

        // add multi scheduling parameters
        if (flag_multiSched){
            newInit.applyMultiSchedParameters(multiSchedParameters_[i]);
        }

        // create scheduler and start scheduling
        VieVS::Scheduler scheduler = VieVS::Scheduler(newInit, path_, fname);
        scheduler.start();

        // create output
        VieVS::Output output(scheduler, path_, fname, version);
        output.createAllOutputFiles(statisticsLog, skdCatalogs_);

        if(flag_multiSched){
            cout << threadNumberPrefix+(boost::format("version %d finished;\n") % (i + 1)).str();
        }
    }
    statisticsLog.close();
    cout << "everything finally finished!;\n";
    cout << "created total number of scans: " << Scan::numberOfCreatedObject() << ";\n";
}

void VieSchedpp::readSkdCatalogs() {

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

void VieSchedpp::readMultiCoreSetup() {
    std::string threads = xml_.get<std::string>("master.multiCore.threads","auto");

    if(threads == "manual"){
        nThreads_ = xml_.get<int>("master.multiCore.nThreads",1);
    } else if (threads == "single"){
        nThreads_ = 1;
    } else if (threads == "auto"){
        //TODO: proper implementation of multi scheduling

        nThreads_ = 4;

    }

    jobScheduling_ = xml_.get<std::string>("master.multiCore.jobScheduling","auto");
    chunkSize_ = xml_.get<int>("master.multiCore.chunkSize",1);
    threadPlace_ = xml_.get<std::string>("master.multiCore.threadPlace","auto");
}


