#include <chrono>
#include "VieVS_Scheduler.h"
#include "SkdParser.h"
#include "LogParser.h"

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
        file ="/home/mschartn/build-scheduling_GUI-Desktop_Qt_5_9_1_GCC_64bit-Debug/out/20180411094110_AOV022/parameters.xml";
//        file ="/home/mschartn/build-scheduling_GUI-Desktop_Qt_5_9_1_GCC_64bit-Debug/out/20180129150754_AUA037/AUA037V035.skd";
    }

    if(argc == 2){
        auto start = std::chrono::high_resolution_clock::now();

        // V1: standard usage:
        std::cout << "Processing file: " << file << ";\n";
        VieVS::VieVS_Scheduler mainScheduler(file);
        mainScheduler.run();

        // V2: parse skd and log files
//        VieVS::SkdParser mySkdParser("/data/Daten/wettzell/1stSession/v012.skd");
//        mySkdParser.createObjects();
//        mySkdParser.createScans();
//        mySkdParser.copyScanMembersToObjects();
//
//        VieVS::LogParser htLogParser("/data/Daten/wettzell/1stSession/v012sa.log");
//        htLogParser.parseLogFile();
//        htLogParser.addScheduledTimes(mySkdParser.getScheduledTimes("RAEGSMAR"));
//        htLogParser.output("/data/Daten/wettzell/1stSession/times_sa.txt");


        auto finish = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
        std::cout << "execution time: " << static_cast<double>(microseconds.count()) / 1e6 << " [s];\n";
    }else{
        std::cout << "please add path to parameters.xml file as input argument!;";
    }


    return 0;
}
