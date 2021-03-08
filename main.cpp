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
// clang-format off
#include "VieSchedpp.h"
// clang-format on
#include "Input/LogParser.h"
#include "Input/SkdParser.h"
#include "Simulator/Solver.h"


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


    if ( argc == 1 ) {
        welcome();
        return 0;

    } else if ( argc == 2 ) {
        std::string arg = argv[1];
        if ( arg == "--version" || arg == "-v" ) {
            std::string versionNr = VieVS::util::version();
            std::cout << versionNr << std::endl;
            return 0;
        }
        if ( arg == "--help" || arg == "-h" ) {
            std::cout << "pass path to a VieSchedpp.xml file as first input argument" << std::endl;
            std::cout << "e.g. \"./VieSchedpp path/to/VieSchedpp.xml\"" << std::endl;
            return 0;
        }


        // main scheduling program


        auto start = std::chrono::high_resolution_clock::now();

        // V1: standard usage:
        std::cout << "Processing file: " << arg << "\n";
        VieVS::VieSchedpp mainScheduler( arg );
        mainScheduler.run();

        auto finish = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>( finish - start );
        long long int usec = microseconds.count();

        std::string t = "execution time: " + VieVS::util::milliseconds2string( usec );
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( info ) << t;
#else
        std::cout << "[info] " << t;
#endif
        std::cout << std::endl;

    } else if ( argc == 3 ) {
        std::string flag = argv[1];
        std::string file = argv[2];

        if ( flag == "--snr" ) {
            VieVS::SkdParser mySkdParser( file );
            mySkdParser.read();

            VieVS::Scheduler sched = mySkdParser.createScheduler();

            VieVS::Output out(sched);

            out.writeSnrTable();

        }
    } else if (argc == 4) {
        std::string xml = argv[1];
        std::string flag = argv[2];
        std::string file = argv[3];

        if (flag == "--sim") {
            VieVS::SkdParser mySkdParser(file);
            mySkdParser.read();

            boost::property_tree::ptree tree;
            std::ifstream is(xml);
            boost::property_tree::read_xml(is, tree, boost::property_tree::xml_parser::trim_whitespace);

            VieVS::Scheduler sched = mySkdParser.createScheduler(tree);

            VieVS::Output out(sched);

            VieVS::Simulator simulator(out);
            simulator.start();

            VieVS::Solver solver(simulator);
            solver.start();

            solver.simSummary();
        }
    }

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