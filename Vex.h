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
    class Vex {
    public:
        Vex();

        explicit Vex(const std::string &file);

        void writeVex(const std::vector<Station>& stations,
                           const std::vector<Source>& sources,
                           const std::vector<Scan> & scans,
                           const SkdCatalogReader &skdCatalogReader,
                           const boost::property_tree::ptree &xml);


    private:
        std::ofstream of;
        std::string eol = ";\n";

        void global_block(const std::string &expName);

        void exper_block(const std::string &expName, const std::string &expDescription, const std::string &schedulerName,
                         const std::string &schedulerEmail, const std::string &targetCorrelator);



        void station_block(const std::vector<Station>& stations, const SkdCatalogReader &skdCatalogReader);

        void sites_block(const std::vector<Station>& stations, const SkdCatalogReader &skdCatalogReader);

        void antenna_block(const std::vector<Station>& stations);

        void das_block(const std::vector<Station>& stations, const SkdCatalogReader &skdCatalogReader);



        void source_block(const std::vector<Source>& stations);



        void mode_block();

        void freq_block();

        void bbc_block();

        void if_block();

        void tracks_block();

        void head_pos_block();

        void pass_order_block();

        void roll_block();

        void phase_cal_detect_block();




        void sched_block(const std::vector<Scan>& scans, const std::vector<Station>& stations,
                         const std::vector<Source>& sources, const SkdCatalogReader &skdCatalogReader);

    };
}

#endif //VEX_H
