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

#ifndef VIEVS_SCHEDULER_H
#define VIEVS_SCHEDULER_H

#include <cstdlib>
#include <chrono>
#include <vector>
#include <boost/format.hpp>
#include <iostream>
#include <thread>

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

#include "Initializer.h"
#include "Scheduler.h"
#include "Output.h"
#include "ParameterSettings.h"
#include "HighImpactScanDescriptor.h"


#ifdef _OPENMP
#include <omp.h>
#endif

namespace VieVS {

    class VieSchedpp {
    public:
        VieSchedpp() = default;

        explicit VieSchedpp(const std::string &inputFile);

        void run();


    private:
        std::string inputFile_;
        std::string path_;
        std::string fileName_;
        boost::property_tree::ptree xml_; ///< content of parameters.xml file

        SkdCatalogReader skdCatalogs_;
        std::vector<VieVS::MultiScheduling::Parameters> multiSchedParameters_;

        void readSkdCatalogs();

        #ifdef _OPENMP
        void multiCoreSetup();
        #endif

        #ifdef VIESCHEDPP_LOG
        void init_log();
        #endif

    };
}

#endif //VIEVS_SCHEDULER_H
