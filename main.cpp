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
#include "Input/LogParser.h"
#include "Input/SkdParser.h"
#include "VieSchedpp.h"

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
 * @author Matthias Schartner
 */

/**
 * @brief welcome message
 * @author Matthias Schartner
 *
 * Displays a welcome message in case you did not pass any arguments
 */
void welcome();


/**
 * @brief error message in case of termination
 * @author Matthias Schartner
 */
void VieSchedppTerminate() {
    std::cerr << "VieSched++ crashed. Check the log file for more information. In case you cannot solve the issue "
                 "yourself contact matthias.schartner@geo.tuwien.ac.at\n";
}


/**
 * @brief main function
 * @author Matthias Schartner
 *
 * pass path to VieSchedpp.xml file as argument.
 * This file will then be processed
 *
 * @param argc number of arguments
 * @param argv  argument list
 * @return return type
 */
int main( int argc, char *argv[] ) {
    std::set_terminate( VieSchedppTerminate );

    if ( argc != 2 ) {
        welcome();
        return 0;
    }

    std::string file = argv[1];

    auto start = std::chrono::high_resolution_clock::now();

    // V1: standard usage:
    std::cout << "Processing file: " << file << "\n";
    VieVS::VieSchedpp mainScheduler( file );
    mainScheduler.run();

    ////    V2: parse skd and log files
    //    VieVS::SkdParser mySkdParser("/home/mschartn/programming/out/20181029164853_/dummy.skd");
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
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>( finish - start );
    long long int usec = microseconds.count();

    auto milliseconds = usec / 1000 % 1000;
    auto seconds = usec / 1000 / 1000 % 60;
    auto minutes = usec / 1000 / 1000 / 60 % 60;
    auto hours = usec / 1000 / 1000 / 60 / 60;
    std::stringstream t;
    t << "execution time: ";
    if ( hours > 0 ) {
        t << hours << "h ";
    }
    if ( minutes > 0 ) {
        t << minutes << "m ";
    }
    if ( seconds > 0 ) {
        t << seconds << "s ";
    }
    if ( milliseconds > 0 ) {
        t << milliseconds << "ms ";
    }
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << t.str();
#else
    std::cout << "[info] " << t.str();
#endif
    std::cout << std::endl;

    return 0;
}


void welcome() {
    std::cout << " __     ___      ____       _              _             \n"
                 " \\ \\   / (_) ___/ ___|  ___| |__   ___  __| |  _     _   \n"
                 "  \\ \\ / /| |/ _ \\___ \\ / __| '_ \\ / _ \\/ _` |_| |_ _| |_ \n"
                 "   \\ V / | |  __/___) | (__| | | |  __/ (_| |_   _|_   _|\n"
                 "    \\_/  |_|\\___|____/ \\___|_| |_|\\___|\\__,_| |_|   |_|  \n"
                 "                                                         \n"
                 "Welcome to VieSched++\n\n"
                 "In case this was a test to verify the connection between the GUI and VieSched++ then you were "
                 "successful!\n\n"
                 "In case you want to run VieSched++ from a terminal pass the path to the VieSchedpp.xml file as an "
                 "argument to the executable. \n"
                 "e.g.: ./VieSchedpp path/to/VieSchedpp.xml\n";
}