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
 *
 * @author Matthias Schartner
 * @date 07.12.2017
 */

#ifndef VEX_H
#define VEX_H

#include "SkdCatalogReader.h"
#include "Station.h"
#include "Scan.h"

namespace VieVS {

    /**
     * @class Vex
     * @brief this is the VLBI vex-output class
     *
     * @author Matthias Schartner
     * @date 07.12.2017
     */
    class Vex: public VieVS_Object {
    public:

        explicit Vex(const std::string &file);

        void writeVex(const Network &network, const std::vector<Source>& sources, const std::vector<Scan> & scans,
                      const SkdCatalogReader &skdCatalogReader, const boost::property_tree::ptree &xml);


    private:
        static unsigned long nextId;

        std::ofstream of;
        std::string eol = ";\n";
        std::map<int,int> channelNr2Bbc_;

        void global_block(const std::string &expName);

        void exper_block(const std::string &expName, const std::string &expDescription,
                         const std::string &piName, const std::string &piEmail,
                         const std::string &contactName, const std::string &contactEmail,
                         const std::string &schedulerName, const std::string &schedulerEmail,
                         const std::string &notes, const std::string &targetCorrelator,
                         const std::string &gui_version);



        void station_block(const std::vector<Station>& stations, const SkdCatalogReader &skdCatalogReader);

        void sites_block(const std::vector<Station>& stations, const SkdCatalogReader &skdCatalogReader);

        void antenna_block(const std::vector<Station>& stations);

        void das_block(const std::vector<Station>& stations, const SkdCatalogReader &skdCatalogReader);



        void source_block(const std::vector<Source>& sources);



        void mode_block(const std::vector<Station>& stations, const SkdCatalogReader &skdCatalogReader);

        void freq_block(const SkdCatalogReader &skdCatalogReader);

        void bbc_block(const SkdCatalogReader &skdCatalogReader);

        void if_block(const SkdCatalogReader &skdCatalogReader);

        void tracks_block(const std::vector<Station> &stations, const SkdCatalogReader &skdCatalogReader);

        void head_pos_block();

        void pass_order_block();

        void roll_block();

        void phase_cal_detect_block();




        void sched_block(const std::vector<Scan>& scans, const std::vector<Station>& stations,
                         const std::vector<Source>& sources, const SkdCatalogReader &skdCatalogReader);

    };
}

#endif //VEX_H
