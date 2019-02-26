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
 * @file Ast.h
 * @brief class Ast
 *
 * @author Matthias Schartner
 * @date 26.02.2019
 */

#ifndef VIESCHEDPP_AST_H
#define VIESCHEDPP_AST_H

#include "../Scan/Scan.h"
#include "../ObservingMode/ObservingMode.h"

namespace VieVS{

    /**
     * @class Ast
     * @brief create Operation in Ast format
     *
     * @author Matthias Schartner
     * @date 26.02.2019
     */
    class Ast: public VieVS_Object{
    public:
        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param file file name
         */
        explicit Ast(const std::string &file);


        /**
         * @brief write ast file
         * @author Matthias Schartner
         *
         * @param network station network
         * @param sources list of all sources
         * @param scans list of all scans
         * @param xml paramters.xml file
         * @param obsModes observing mode
         */
        void writeAstFile(const Network &network,
                          const std::vector<Source>& sources,
                          const std::vector<Scan> & scans,
                          const boost::property_tree::ptree &xml,
                          const std::shared_ptr<const ObservingMode> &obsModes);


    private:
        static unsigned long nextId; ///< next id for this object type

        std::ofstream of; ///< output stream object

        /**
         * @brief write experiment block
         * @author Matthias Schartner
         *
         * @param xml paramters.xml file
         */
        void experiment(const boost::property_tree::ptree &xml);

        void stationParameters(const Station &station, double Mbps);

    };

}


#endif //VIESCHEDPP_AST_H
