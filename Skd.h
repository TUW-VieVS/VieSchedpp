//
// Created by mschartn on 15.12.17.
//

#ifndef SKD_H
#define SKD_H

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
    class Skd: public VieVS_Object {

    public:

        explicit Skd(const std::string &file);

        /**
         * @brief creates a skd file
         * @param skdCatalogReader skd catalogs
         */
        void writeSkd(const std::vector<Station>& stations,
                      const std::vector<Source>& sources,
                      const std::vector<Scan> & scans,
                      const SkdCatalogReader &skdCatalogReader,
                      const boost::property_tree::ptree &xml);

    private:
        static int nextId;

        std::ofstream of;

        /**
         * @brief write skd $PARAM block
         *
         * @param skdCatalogReader catalog reader
         * @param ofstream out stream
         */
        void skd_PARAM(const std::vector<Station>& stations, const boost::property_tree::ptree &xml,
                       const SkdCatalogReader &skdCatalogReader);

        /**
         * @brief write skd $OP block
         *
         * @param ofstream out stream
         */
        void skd_OP();

        /**
         * @brief write skd $DOWNTIME block
         *
         * @param ofstream out stream
         */
        void skd_DOWNTIME();

        /**
         * @brief write skd $MAJOR block
         *
         * @param skdCatalogReader catalog reader
         * @param ofstream out stream
         */
        void skd_MAJOR(const std::vector<Station> &stations, const std::vector<Source> &sources,
                       const boost::property_tree::ptree &xml, const SkdCatalogReader &skdCatalogReader);

        /**
         * @brief write skd $MINOR block
         *
         * @param ofstream out stream
         */
        void skd_MINOR();

        /**
         * @brief write skd $ASTROMETRIC block
         *
         * @param ofstream out stream
         */
        void skd_ASTROMETRIC();

        /**
         * @brief write skd %STATWT block
         *
         * @param of out stream
         */
        void skd_STATWT(const std::vector<Station>& stations);

        /**
         * @brief write skd $SRCWT block
         *
         * @param of out stream
         */
        void skd_SRCWT(const std::vector<Source> &sources);

        /**
         * @brief write skd $CATALOG_USED block
         *
         * @param of out stream
         */
        void skd_CATALOG_USED(const boost::property_tree::ptree &xml, const SkdCatalogReader &skdCatalogReader);

        /**
         * @brief write skd $BROADBAND block
         *
         * @param ofstream out stream
         */
        void skd_BROADBAND();

        /**
         * @brief write skd $SOURCES block
         *
         * @param skdCatalogReader catalog reader
         * @param of out stream
         */
        void skd_SOURCES(const std::vector<Source> &sources, const SkdCatalogReader &skdCatalogReader);

        /**
         * @brief write skd $STATIONS block
         * @param skdCatalogReader catalog reader
         * @param of out stream
         */
        void skd_STATIONS(const std::vector<Station>& stations, const SkdCatalogReader &skdCatalogReader);

        /**
         * @brief write skd $FLUX block
         *
         * @param skdCatalogReader catalog reader
         * @param of out stream
         */
        void skd_FLUX(const std::vector<Source> &sources, const boost::property_tree::ptree &xml,
                      const SkdCatalogReader &skdCatalogReader);

        /**
         * @brief write skd $SKED block
         *
         * @param skdCatalogReader catalog reader
         * @param of out stream
         */
        void skd_SKED(const std::vector<Station> &stations,
                      const std::vector<Source> &sources,
                      const std::vector<Scan> &scans,
                      const SkdCatalogReader &skdCatalogReader);

        /**
         * @brief write skd $CODES block
         *
         * @param skdCatalogReader catalog reader
         * @param of out stream
         */
        void skd_CODES(const std::vector<Station> &stations, const SkdCatalogReader &skdCatalogReader);
    };
}

#endif //SKD_H
