//
// Created by matth on 28.01.2018.
//

#include "VieVS_Scheduler.h"

using namespace std;
using namespace VieVS;

VieVS_Scheduler::VieVS_Scheduler(const std::string &inputFile):inputFile_{inputFile} {

}


void VieVS_Scheduler::run() {
    cout << "writing header log file to: log_initializer.txt;\n";

    VieVS::Initializer init(inputFile_);
    std::size_t found = inputFile_.find_last_of("/\\");
    std::string path;
    if(found == inputFile_.npos){
        path = "";
    }else{
        path = inputFile_.substr(0,found+1);
    }



    ofstream headerLog(path+"log_initializer.txt");
    ofstream statisticsLog(path+"statistics.csv");

    VieVS::SkdCatalogReader skdCatalogReader = init.createSkdCatalogReader();

    init.initializeObservingMode(skdCatalogReader, headerLog);
    init.initializeLookup();
    init.initializeSkyCoverages();

    init.createStations(skdCatalogReader, headerLog);
    init.createSkyCoverages(headerLog);

    init.createSources(skdCatalogReader, headerLog);
    init.precalcSubnettingSrcIds();

    init.initializeSourceSequence();
    init.initializeCalibrationBlocks( headerLog );
    init.initializeWeightFactors();

    init.initializeOptimization(headerLog);

    bool flag_multiSched = false;
    unsigned long nsched = 1;

    init.statisticsLogHeader(statisticsLog);

    vector<VieVS::MultiScheduling::Parameters> all_multiSched_PARA = init.readMultiSched();
    if (!all_multiSched_PARA.empty()) {
        flag_multiSched = true;
        nsched = all_multiSched_PARA.size();
        headerLog << "multi scheduling found ... creating " << nsched << " schedules!\n";
        cout << "multi scheduling found ... creating " << nsched << " schedules!;\n";
    }

    headerLog.close();

    unsigned long numberOfCreatedScans = 0;


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
    omp_set_num_threads(nThreads);
#pragma omp parallel for reduction(+:numberOfCreatedScans)
#else
    if(nsched > 1){
            cout << "VLBI Scheduler was not compiled with OpenMP! Recompile it with OpenMP for multi core support!";
        }
#endif
    for (int i = 0; i < nsched; ++i) {

        VieVS::Scheduler scheduler;

        ofstream bodyLog;
        string threadNumberPrefix;
        if (flag_multiSched) {
            VieVS::Initializer newinit = init;

            string fname = init.getXml().get("master.output.experimentName","schedule");
            fname.append((boost::format("V%03d.log") % (i+1)).str());

            bodyLog.open(path+fname);


#ifdef  _OPENMP
            threadNumberPrefix = (boost::format("thread %d: ") % omp_get_thread_num()).str();
#endif
            string txt = threadNumberPrefix + (boost::format("creating multi scheduling version %d of %d;\n") % (i + 1) %
                                               nsched).str();

            string txt2 = (boost::format("version %d: writing log file to: %s;\n") % (i+1) % fname).str();
            cout << txt;
            cout << txt2;

            newinit.initializeGeneral(bodyLog);

            newinit.initializeStations();
            newinit.initializeSources();
            newinit.initializeBaselines();

            newinit.initializeWeightFactors();
            newinit.initializeSourceSequence();
            newinit.initializeCalibrationBlocks( headerLog );

            newinit.applyMultiSchedParameters(all_multiSched_PARA[i], bodyLog);

            newinit.initializeNutation();
            newinit.initializeEarth();


            scheduler = VieVS::Scheduler(newinit);
        } else {

            string fname = init.getXml().get("master.output.experimentName","schedule");
            fname.append(".log");
            bodyLog.open(path+fname);

            init.initializeGeneral(bodyLog);

            init.initializeStations();
            init.initializeSources();
            init.initializeBaselines();

            init.initializeWeightFactors();

            init.initializeNutation();
            init.initializeEarth();

            scheduler = VieVS::Scheduler(init);
        }

        scheduler.start(bodyLog);
        scheduler.statistics(bodyLog);

        unsigned long createdScans = scheduler.numberOfCreatedScans();

        numberOfCreatedScans += createdScans;

        bodyLog.close();

        VieVS::Output output(scheduler,path);
        if (flag_multiSched) {
            output.setIsched(i + 1);
        } else {
            output.setIsched(0);
        }

        output.createAllOutputFiles(statisticsLog, skdCatalogReader);


        if(flag_multiSched){
            string txt3 = threadNumberPrefix+(boost::format("version %d finished;\n") % (i + 1)).str();
            cout << txt3;
        }

    }
    statisticsLog.close();
    cout << "everything finally finished!!!;\n";
    cout << "created total number of scans: " << numberOfCreatedScans << ";\n";

}

