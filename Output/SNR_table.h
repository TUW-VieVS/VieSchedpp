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
* @file SNR_table.h
* @brief class SNR table
*
* @author Matthias Schartner
* @date 04.02.2017
*/


#ifndef VIESCHEDPP_SNR_TABLE_H
#define VIESCHEDPP_SNR_TABLE_H

#include "../Scan/Scan.h"
#include "../ObservingMode/ObservingMode.h"

namespace VieVS {
    /**
     * @class SNR_table
     * @brief produces a SNR table for comparison with correlator
     *
     * @author Matthias Schartner
     * @date 04.02.2017
     */
    class SNR_table : public VieVS_Object {

    public:

        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param file file name
         */
        explicit SNR_table(const std::string &file);

        /**
         * @brief create the SNR table
         * @author Matthias Schartner
         *
         * @param network station network
         * @param sources list of all souces
         * @param scans list of all scans
         * @param obsModes observin mode
         */
        void writeTable(const Network &network,
                        const std::vector<Source>& sources,
                        const std::vector<Scan> & scans,
                        const std::shared_ptr<const ObservingMode> &obsModes);


    private:
        static unsigned long nextId; ///< next id for this object type

        std::ofstream of; ///< output stream object *filename*.snr

    };
}

#endif //VIESCHEDPP_SNR_TABLE_H
