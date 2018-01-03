#include <cstdlib>
#include <chrono>
#include <vector>
#include <boost/format.hpp>
#include <iostream>


#include "Initializer.h"
#include "Scheduler.h"
#include "Output.h"
#include "ParameterSettings.h"

#ifdef _OPENMP
#include <omp.h>
#endif

/**
 * @file main.cpp
 * @brief main file
 *
 * @author Matthias Schartner
 * @date 21.06.2017
 */

/**
 * @namespace VieVS
 * @brief namespace VieVS is used for all "VieVS_*" and "VLBI_*" classes and files.
 */

using namespace std;
/**
 * starts the scheduling software
 */
void run(std::string file);

/**
 * Main function.
 *
 * @param argc currently unused
 * @param argv  currently unused
 * @return 0 if no error occurred
 */
int main(int argc, char *argv[])
{
    std::string file;
    if(argc == 2){
        file = argv[1];
    }else{
        argc = 2;
        file ="/home/mschartn/build-scheduling_GUI-Desktop_Qt_5_9_1_GCC_64bit-Debug/out/20180103123001_austral/parameters.xml";
    }


    if(argc == 2){
        auto start = std::chrono::high_resolution_clock::now();
        cout << "Processing file: " << file << ";\n";
        run(file);
        auto finish = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
        std::cout << "execution time: " << static_cast<double>(microseconds.count()) / 1e6 << " [s];\n";
    }else{
        std::cout << "please add path to parameters.xml file as input argument!;";
    }


    return 0;
}

/**
 * reads xml file and runs scheduling software accordingly
 */
void run(std::string file){

    VieVS::Initializer init(file);
    std::size_t found = file.find_last_of("/\\");
    std::string path;
    if(found == file.npos){
        path = "";
    }else{
        path = file.substr(0,found+1);
    }


    cout << "log file is written in this file: header.txt;\n";

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
            string fname = (boost::format("log_V%03d.txt") % (i + 1)).str();
            bodyLog.open(path+fname);


            #ifdef  _OPENMP
            threadNumberPrefix = (boost::format("thread %d: ") % omp_get_thread_num()).str();
            #endif
            string txt = threadNumberPrefix + (boost::format("creating multi scheduling version %d of %d;\n") % (i + 1) %
                          nsched).str();

            string txt2 = (boost::format("version %d: log file is written in this file: %s;\n") % (i+1) % fname).str();
            cout << txt;
            cout << txt2;

            newinit.initializeGeneral(bodyLog);

            newinit.initializeStations();
            newinit.initializeSources();
            newinit.initializeBaselines();

            newinit.initializeWeightFactors();

            newinit.applyMultiSchedParameters(all_multiSched_PARA[i], bodyLog);

            newinit.initializeNutation();
            newinit.initializeEarth();

            scheduler = VieVS::Scheduler(newinit);
        } else {
            cout << "log file is written in this file: body.txt;\n";
            bodyLog.open(path+"log.txt");
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

        unsigned long createdScans = scheduler.numberOfCreatedScans();

        numberOfCreatedScans += createdScans;

        bodyLog.close();

        VieVS::Output output(scheduler,path);
        if (flag_multiSched) {
            output.setIsched(i + 1);
        } else {
            output.setIsched(0);
        }

        output.writeStatistics(true, true, true, true, true, true, statisticsLog);

        output.writeNGS();

        output.writeSkd(skdCatalogReader);

        output.writeVex(skdCatalogReader);

        if(flag_multiSched){
            string txt3 = threadNumberPrefix+(boost::format("version %d finished;\n") % (i + 1)).str();
            cout << txt3;
        }

    }
    statisticsLog.close();
    cout << "everything finally finished!!!;\n";
    cout << "created total number of scans: " << numberOfCreatedScans << ";\n";
}
