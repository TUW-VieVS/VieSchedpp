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
 * @file SourceStatistics.h
 * @brief class SourceStatistics
 *
 * @author Matthias Schartner
 * @date 04.05.2019
 */

#ifndef VIESCHEDPP_SOURCESTATISTICS_H
#define VIESCHEDPP_SOURCESTATISTICS_H


#include <fstream>
#include "../Misc/VieVS_Object.h"
#include "../Scan/Scan.h"
#include "../Station/Network.h"


namespace VieVS {

/**
 * @class SourceStatistics
 * @brief creates the source statistics output file
 *
 * @author Matthias Schartner
 * @date 28.06.2017
 */

class SourceStatistics : public VieVS_Object {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param file file name
     */
    explicit SourceStatistics( const std::string &file );

    /**
     * @brief write simple skdsum file
     * @author Matthias Schartner
     *
     * @param network station network
     * @param sources list of all sources
     * @param scans list of all scans
     * @param xml paramters.xml file
     */
    void writeFile( Network &network, std::vector<Source> &sources, const std::vector<Scan> &scans,
                    const boost::property_tree::ptree &xml );

   private:
    static unsigned long nextId;  ///< next id for this object type

    std::ofstream of;  ///< output stream object

    /**
     * @brief read all groups from VieSchedpp.xml file
     * @author Matthias Schartner
     *
     * @param root VieSchedpp.xml file
     * @param sources list of all sources
     * @return list of all groups
     */
    std::unordered_map<std::string, std::vector<std::string>> readGroups( boost::property_tree::ptree root,
                                                                          const std::vector<Source> &sources ) noexcept;


    /**
     * @brief calculate minutes where source is visible
     * @author Matthias Schartner
     *
     * @param network station network
     * @param source target source
     * @return pair of visible time periods
     */
    std::vector<std::pair<unsigned int, unsigned int>> minutesVisible( Network &network, const Source &source );
};
}  // namespace VieVS

#endif  // VIESCHEDPP_SOURCESTATISTICS_H
