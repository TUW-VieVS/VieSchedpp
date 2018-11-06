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
* @file Bbc.h
* @brief class Bbc
*
* @author Matthias Schartner
* @date 17.09.2018
*/

#ifndef VIESCHEDPP_BBC_H
#define VIESCHEDPP_BBC_H

#include <utility>
#include <vector>
#include <fstream>
#include <boost/format.hpp>
#include "VieVS_NamedObject.h"


namespace VieVS{

    /**
     * @class Bbc
     * @brief BBC section of observing mode
     *
     * following vex standard
     * The $BBC block connects physical BBC's to the 'logical' BBC's defined in the $FREQ section, and also specifies the connection of the BBC to a 'logical' IF.
     *
     * @author Matthias Schartner
     * @date 17.09.2018
     */
    class Bbc: public VieVS_NamedObject {
    public:

        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param name BBC name
         */
        explicit Bbc(std::string name);

        /**
         * @brief add new BBC assignment
         * @author Matthias Schartner
         *
         * @param name Logical BBC 'link' with chan_def statement in $FREQ block
         * @param physical_bbc_number Physical BBC# or DBE channel#
         * @param if_name Logical IF 'link' with if_def statement in $IF block
         */
        void addBbc(std::string name, unsigned int physical_bbc_number, std::string if_name);

        /**
         * @brief writes BBC block in vex format
         * @author Matthias Schartner
         *
         * @param of vex file stream
         * @param comment optional comment line
         */
        void toVexBbcDefinition( std::ofstream &of, const std::string &comment = "") const;

    private:
        static unsigned long nextId; ///< next id for this object type

        /**
         * @class Bbc_assign
         * @brief BBC assign section in BBC block
         *
         * @author Matthias Schartner
         * @date 17.09.2018
         */
        class Bbc_assign: public VieVS_NamedObject{
        public:
            /**
             * @brief constructor
             * @author Matthias Schartner
             *
             * @param name Logical BBC 'link' with chan_def statement in $FREQ block
             * @param physical_bbc_number Physical BBC# or DBE channel#
             * @param if_name Logical IF 'link' with if_def statement in $IF block
             */
            Bbc_assign(std::string name,
                       unsigned int physical_bbc_number,
                       std::string if_name):
                    VieVS_NamedObject{std::move(name), nextId++},
                    physical_bbc_number_{physical_bbc_number},
                    if_name_{std::move(if_name)}{};

            unsigned int physical_bbc_number_; ///< Physical BBC# or DBE channel#
            std::string if_name_; ///< Logical IF 'link' with if_def statement in $IF block
        private:
            static unsigned long nextId; ///< next id for this object type

        };

        std::vector<Bbc_assign> bbc_assigns_; ///< list of all BBC assigns
    };
}


#endif //VIESCHEDPP_BBC_H
