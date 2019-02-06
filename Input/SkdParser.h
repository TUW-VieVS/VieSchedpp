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
* @file SkdParser.h
* @brief class SkdParser
*
* @author Matthias Schartner
* @date 22.10.2017
*/


#ifndef SKDREADER_H
#define SKDREADER_H

#include "SkdCatalogReader.h"
#include "../Scheduler.h"

#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/named_scope.hpp>
#endif


namespace VieVS{

    /**
     * @class SkdParser
     * @brief skd output file parser
     *
     * @author Matthias Schartner
     * @date 31.01.2018
     */
    class SkdParser: public VieVS_Object {
    public:

        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param filename path to skd file
         */
        explicit SkdParser(const std::string &filename);

        /**
         * @brief enables log files for output
         * @author Matthias Schartner
         */
        void setLogFiles();

        /**
         * @brief read skd file
         * @author Matthias Schartner
         */
        void read();

        /**
         * @brief create schedule
         * @author Matthias Schartner
         *
         * @return schedule
         */
        Scheduler createScheduler();

        /**
         * @brief get observed frequencies
         * @author Matthias Schartner
         *
         * @return observed frequencies per band
         */
        std::map<std::string, std::vector<double>> getFrequencies(){
            return freqs_;
        };

        /**
         * @brief get scheduled times
         * @author Matthias Schartner
         *
         * @return vector with scheduled times
         */
        std::vector<std::vector<unsigned int>> getScheduledTimes(const std::string &station);

    private:
        static unsigned long nextId; ///< next id for this object type

        std::string filename_; ///< skd file name
        unsigned int fieldSystemTimes_ = 0; ///< scheduled field system time
        unsigned int preob_ = 0; ///< scheduled calibrator time
        unsigned int midob_ = 0; ///< scheduled correlator synchronization time
        unsigned int postob_ = 0; ///< scheduled postob time

        SkdCatalogReader skd_; ///< sked catalog reader

        Network network_; ///< station network
        std::vector<Source> sources_; ///< all sources
        std::vector<Scan> scans_; ///< all scans in schedule
        std::shared_ptr<ObservingMode> obsModes_ = nullptr; ///< observing mode


        std::map<std::string, std::vector<double>> freqs_; ///< all observed frequencies per band

        /**
         * @brief create scans
         * @author Matthias Schartner
         *
         * @param of output stream object
         */
        void createScans(std::ofstream &of);

        /**
         * @brief link created objects
         * @author Matthias Schartner
         *
         * @param of output stream object
         */
        void copyScanMembersToObjects(std::ofstream &of);

    };

}


#endif //SKDREADER_H
