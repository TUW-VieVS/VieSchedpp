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

#ifndef SKDREADER_H
#define SKDREADER_H

#include "SkdCatalogReader.h"
#include "Scheduler.h"

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
    class SkdParser: public VieVS_Object {
    public:

        explicit SkdParser(const std::string &filename);

        void setLogFiles();

        void read();

        Scheduler createScheduler();

        std::map<std::string, std::vector<double>> getFrequencies(){
            return freqs_;
        };

        std::vector<std::vector<unsigned int>> getScheduledTimes(const std::string &station);

    private:
        static unsigned long nextId;

        std::string filename_;
        unsigned int fieldSystemTimes_ = 0;
        unsigned int preob_ = 0;
        unsigned int midob_ = 0;
        unsigned int postob_ = 0;

        SkdCatalogReader skd_;

        Network network_;
        std::vector<Source> sources_; ///< all sources
        std::vector<Scan> scans_; ///< all scans in schedule

        std::map<std::string, std::vector<double>> freqs_;

        void createScans(std::ofstream &of);

        void copyScanMembersToObjects(std::ofstream &of);

    };

}


#endif //SKDREADER_H
