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

/**
 * @file Output.h
 * @brief class Output
 *
 * @author Matthias Schartner
 * @date 22.08.2017
 */

#ifndef OUTPUT_H
#define OUTPUT_H

#include "../Scheduler.h"
#include "Ast.h"
#include "OperationNotes.h"
#include "SNR_table.h"
#include "Skd.h"
#include "SourceStatistics.h"
#include "Vex.h"
#include "boost/format.hpp"
#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#endif


namespace VieVS {

/**
 * @class Output
 * @brief this is the VLBI output class that creates all output files
 *
 * @author Matthias Schartner
 * @date 22.08.2017
 */
class Output : public VieVS_NamedObject {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param sched scheduler
     * @param path path to output directory
     * @param fname file name
     * @param version version number
     */
    Output( Scheduler &sched, std::string path, std::string fname, int version );


    /**
     * @brief writes a summary text file containing some basic statistics and overviews
     * @author Matthias Schartner
     */
    void writeSkdsum();


    /**
     * @brief write statistics line to statistics.csv file
     * @author Matthias Schartner
     *
     * @param of statistics.csv file
     */
    void writeStatistics( std::ofstream &of );


    /**
     * @brief create a ngs file
     * @author Matthias Schartner
     */
    void writeNGS();


    /**
     * @brief create a operations notes file
     * @author Matthias Schartner
     */
    void writeOperationsNotes();


    /**
     * @brief create a operations notes file
     * @author Matthias Schartner
     */
    void writeSourceStatistics();


    /**
     * @brief creates a skd file
     * @author Matthias Schartner
     *
     * @param skdCatalogReader sked catalogs
     */
    void writeSkd( const SkdCatalogReader &skdCatalogReader );


    /**
     * @brief creates a vex file
     * @author Matthias Schartner
     */
    void writeVex();


    /**
     * @brief creates a SNR overview file
     * @author Matthias Schartner
     */
    void writeSnrTable();


    /**
     * @brief creates a ast file
     * @author Matthias Schartner
     */
    void writeAstFile();


    /**
     * @brief create all output files
     * @author Matthias Schartner
     *
     * @param of statistics.csv file
     * @param skdCatalogReader sked catalogs
     */
    void createAllOutputFiles( std::ofstream &of, const SkdCatalogReader &skdCatalogReader );


   private:
    static unsigned long nextId;  ///< next id for this object type

    boost::property_tree::ptree xml_;  ///< content of VieSchedpp.xml file

    std::string path_;                                                        ///< path to output directory
    int version_;                                                             ///< number of this schedule
    Network network_;                                                         ///< network
    std::vector<Source> sources_;                                             ///< all sources
    std::vector<Scan> scans_;                                                 ///< all scans in schedule
    const std::shared_ptr<const ObservingMode> &obsModes_;                    ///< observing mode
    boost::optional<MultiScheduling::Parameters> multiSchedulingParameters_;  ///< multi scheduling parameters
};
}  // namespace VieVS

#endif  // OUTPUT_H
