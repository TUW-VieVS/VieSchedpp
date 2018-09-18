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

#include <chrono>
#include "VieSchedpp.h"
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
 * @brief namespace VieVS is used for all defined classes.
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
    std::cout << "Processing file: " << file << "\n";
    VieVS::VieSchedpp mainScheduler(file);
    mainScheduler.run();

//    V2: parse skd and log files
//    VieVS::SkdParser mySkdParser("/home/mschartn/build-scheduling_GUI-Desktop_Qt_5_9_1_GCC_64bit-Debug/out/20180709143715_C1701/c1701.skd");
//    mySkdParser.read();
//    VieVS::Scheduler sched = mySkdParser.createScheduler();
//
//
////    VieVS::LogParser htLogParser1("/data/Daten/Schedules/EINT05/log/eint05sa.log");
////    htLogParser1.parseLogFile("#flagr#flagr/antenna,new-source","#flagr#flagr/antenna,acquired");
////    htLogParser1.addScheduledTimes(mySkdParser.getScheduledTimes("RAEGSMAR"));
////    htLogParser1.output("/data/Daten/Schedules/EINT05/log/times_sa.txt");
//
//
//    std::ofstream of("/data/CONT17/c1701.log");
//    sched.checkAndStatistics(of);
//
//    VieVS::Output out(sched,"/data/CONT17/",0);
//    out.writeNGS();
//    out.writeOperationsNotes();
//


    auto finish = std::chrono::high_resolution_clock::now();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
    long long int usec = microseconds.count();

    auto milliseconds = usec/1000            % 1000;
    auto seconds =      usec/1000/1000       % 60;
    auto minutes =      usec/1000/1000/60    % 60;
    auto hours =        usec/1000/1000/60/60;
    std::stringstream t;
    t << "execution time: ";
    if(hours > 0){
        t << hours << "h ";
    }
    if(minutes > 0){
        t << minutes << "m ";
    }
    if(seconds > 0){
        t << seconds << "s ";
    }
    if(milliseconds > 0){
        t << milliseconds << "ms ";
    }
    #ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(info) << t.str();
    #else
    std::cout << "[info] " << t.str();
    #endif
    std::cout << std::endl;

    return 0;
}
