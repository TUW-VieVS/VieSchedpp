//
// Created by matth on 28.01.2018.
//

#include "VieVS_Scheduler.h"
#include "HighImpactScanDescriptor.h"

using namespace std;
using namespace VieVS;


VieVS_Scheduler::VieVS_Scheduler(const std::string &inputFile):inputFile_{inputFile} {

}


void VieVS_Scheduler::run() {
    cout << "writing header log file to: log_initializer.txt;\n";

    // get path of input file
    VieVS::Initializer init(inputFile_);
    std::size_t found = inputFile_.find_last_of("/\\");
    std::string path;
    if(found == std::string::npos){
        path = "";
    }else{
        path = inputFile_.substr(0,found+1);
    }

    // open headerlog and statistics file
    ofstream headerLog(path+"log_initializer.txt");
    ofstream statisticsLog(path+"statistics.csv");

    // start initialization
    VieVS::SkdCatalogReader skdCatalogReader = init.createSkdCatalogReader();

    init.initializeObservingMode(skdCatalogReader, headerLog);
    LookupTable::initialize();
    init.initializeSkyCoverages();

    init.createStations(skdCatalogReader, headerLog);
    init.statisticsLogHeader(statisticsLog);

    init.createSources(skdCatalogReader, headerLog);

    init.initializeSourceSequence();
    init.initializeCalibrationBlocks( headerLog );
    init.initializeWeightFactors();

    init.initializeOptimization(headerLog);

    boost::optional<HighImpactScanDescriptor> himp = init.initializeHighImpactScanDescriptor(headerLog);


    // check if multi scheduling is selected
    bool flag_multiSched = false;
    unsigned long nsched = 1;
    vector<VieVS::MultiScheduling::Parameters> all_multiSched_PARA = init.readMultiSched(headerLog);
    if (!all_multiSched_PARA.empty()) {
        flag_multiSched = true;
        nsched = all_multiSched_PARA.size();
        cout << "multi scheduling found ... creating " << nsched << " schedules!;\n";
    }


    headerLog.close();
    unsigned long numberOfCreatedScans = 0;

// check if openmp is available
#ifdef _OPENMP
    int nThreads = 1;
    if(flag_multiSched){
        std::string jobScheduling;
        int chunkSize;
        std::string threadPlace;

        init.initializeMultiCore(nThreads,jobScheduling,chunkSize,threadPlace);

        cout << "Using OpenMP to parallize multi scheduling;\n";
        cout << (boost::format("OpenMP: starting %d threads;\n") % nThreads).str();
    }
    // use openmp to parallelize upcoming for loop
    omp_set_num_threads(nThreads);
#pragma omp parallel for reduction(+:numberOfCreatedScans)
#else
    if(nsched > 1){
            cout << "VLBI Scheduler was not compiled with OpenMP! Recompile it with OpenMP for multi core support!";
        }
#endif
    // create all required schedules
    for (int i = 0; i < nsched; ++i) {

        int version = 0;
        if (flag_multiSched) {
            version = i+1;
        }

        ofstream bodyLog;
        string threadNumberPrefix;

        // get file name
        string fname = boost::to_lower_copy(init.getXml().get("master.output.experimentName","schedule"));

        // copy initializer (necessary for upcoming steps for multi scheduling)
        VieVS::Initializer newInit = Initializer(init);

        // if you have multi schedule append version number to file name
        if (flag_multiSched) {
            fname.append((boost::format("v%03d") % (i+1)).str());
        }

        // open body log file
        bodyLog.open(path+fname+"_iteration_0"+".log");

        // if you have multicore support add thread number as prefix to output
        if (flag_multiSched){
            // get thread number if you use openmp
            #ifdef  _OPENMP
            threadNumberPrefix = (boost::format("thread %d: ") % omp_get_thread_num()).str();
            #endif

            cout << boost::format("%screating multi scheduling version %d of %d;\n") % threadNumberPrefix % (i + 1) % nsched;
            cout << boost::format("version %d: writing log file to: %s.log;\n") % (i+1) % fname;
        }

        // initialize all parameters
        newInit.initializeGeneral(bodyLog);
        newInit.precalcSubnettingSrcIds();
        newInit.initializeStations();
        newInit.initializeSources();
        newInit.initializeBaselines();
        newInit.initializeWeightFactors();

        // re initialize some more parameters if you use multi scheduling
        if (flag_multiSched){
            newInit.initializeSourceSequence();
            newInit.initializeCalibrationBlocks( bodyLog );
            newInit.applyMultiSchedParameters(all_multiSched_PARA[i], bodyLog);
        }

        // initialize time dependend parameters
        newInit.initializeAstronomicalParameteres();

        // create scheduler and start scheduling
        VieVS::Scheduler scheduler = VieVS::Scheduler(newInit,path,fname,version);
        if(himp.is_initialized()){
            scheduler.highImpactScans(himp.get(), bodyLog);
        }
        scheduler.start(bodyLog);

        numberOfCreatedScans += scheduler.numberOfCreatedScans();

        // create output
        VieVS::Output output(scheduler,path,version);
        output.createAllOutputFiles(statisticsLog, skdCatalogReader);

        if(flag_multiSched){
            string txt3 = threadNumberPrefix+(boost::format("version %d finished;\n") % (i + 1)).str();
            cout << txt3;
        }

    }
    statisticsLog.close();
    cout << "everything finally finished!;\n";
    cout << "created total number of scans: " << numberOfCreatedScans << ";\n";

}

