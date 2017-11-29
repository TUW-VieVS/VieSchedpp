#include <cstdlib>
#include <chrono>
#include <vector>
#include <boost/format.hpp>
#include <iostream>
#include <thread>
//#include <experimental/filesystem>
//#include <boost/filesystem.hpp>


#include "Initializer.h"
#include "Scheduler.h"
#include "Output.h"
#include "ParameterSettings.h"

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
 * creates the corresponding .xml file (will be replaced later by GUI.
 */
void createParameterFile();

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
        file = "/home/mschartn/build-scheduling_GUI-Desktop_Qt_5_9_1_GCC_64bit-Debug/out/20171128110916_test/parameters.xml";
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
 * First a VLBI_initializer is created, than the following steps are executed:
 * - stations are created
 * - sources are creted
 * - stations are initialized
 * - sources are initialized
 * - nutation is calculated
 * - earth velocity is calculated
 * - lookup tabels are created
 * - skyCoverage objects are created
 * After this, the VLBI Scheduler is created, some a prioiri calculations are done and the scheduler is started.
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

    ofstream headerLog(path+"header.txt");

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

    bool flag_multiSched = false;
    unsigned long nsched = 1;


    vector<VieVS::MultiScheduling::Parameters> all_multiSched_PARA = init.readMultiSched();
    if (!all_multiSched_PARA.empty()) {
        flag_multiSched = true;
        nsched = all_multiSched_PARA.size();
        headerLog << "multi scheduling found ... creating " << nsched << " schedules!\n";
    }

    headerLog.close();

    unsigned long numberOfCreatedScans = 0;


//    unsigned int concurentThreadsSupported = std::thread::hardware_concurrency();
//    omp_set_num_threads(concurentThreadsSupported);
//    omp_set_num_threads(1);
//    #pragma omp parallel for
    for (int i = 0; i < nsched; ++i) {

        VieVS::Scheduler scheduler;

        ofstream bodyLog;
        if (flag_multiSched) {
            VieVS::Initializer newinit = init;
            string fname = (boost::format("body_%04d.txt") % (i + 1)).str();
            bodyLog.open(path+fname);

            string txt = (boost::format("########## multiSched number: %4d of %4d ##########;\n") % (i + 1) %
                          nsched).str();
            string txt2 = (boost::format("log file is written in this file: %s;\n") % fname).str();
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
            bodyLog.open(path+"body.txt");
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

//        #pragma omp atomic
        numberOfCreatedScans += createdScans;

        bodyLog.close();

        VieVS::Output output(scheduler,path);
        if (flag_multiSched) {
            output.setIsched(i + 1);
        } else {
            output.setIsched(0);
        }

        output.writeStatistics(true, true, true, true, true);

        output.writeNGS();

        output.writeSkd(skdCatalogReader);

        string txt3 = (boost::format("thread %4d finished;\n") % (i + 1)).str();
        cout << txt3;

    }
    cout << "everything finally finished!!!;\n";
    cout << "created total number of scans: " << numberOfCreatedScans << ";\n";
}
