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
    if(argc != 2){
        std::cout << "please add path to parameters.xml file as input argument!;";
        return 0;
    }

    std::string file = argv[1];


    auto start = std::chrono::high_resolution_clock::now();

    // V1: standard usage:
    std::cout << "Processing file: " << file << ";\n";
    VieVS::VieVS_Scheduler mainScheduler(file);
    mainScheduler.run();

//    V2: parse skd and log files
//    VieVS::SkdParser mySkdParser("C:/Users/matth/Desktop/VGOS/b17341.skd");
//    mySkdParser.createObjects();
//    mySkdParser.createScans();
//    mySkdParser.copyScanMembersToObjects();

//    VieVS::LogParser htLogParser1("/data/Daten/Schedules/EINT05/log/eint05sa.log");
//    htLogParser1.parseLogFile("#flagr#flagr/antenna,new-source","#flagr#flagr/antenna,acquired");
//    htLogParser1.addScheduledTimes(mySkdParser.getScheduledTimes("RAEGSMAR"));
//    htLogParser1.output("/data/Daten/Schedules/EINT05/log/times_sa.txt");

//    VieVS::Scheduler sched = mySkdParser.createScheduler();
//
//    std::ofstream of("C:/Users/matth/Desktop/VGOS/b17341_summary.log");
//    sched.checkAndStatistics(of);
//
//    VieVS::Output out(sched,"C:/Users/matth/Desktop/VGOS/",0);
//    out.writeNGS();
//    out.writeOperationsNotes();

//


    auto finish = std::chrono::high_resolution_clock::now();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
    std::cout << "execution time: " << static_cast<double>(microseconds.count()) / 1e6 << " [s];\n";


    return 0;
}
