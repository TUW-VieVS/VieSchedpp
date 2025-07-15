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
 * @file Skd.h
 * @brief class Skd
 *
 * @author Matthias Schartner
 * @date 15.12.2017
 */

#ifndef SKD_H
#define SKD_H


#include "../Input/SkdCatalogReader.h"
#include "../Scan/Scan.h"


namespace VieVS {

/**
 * @class Skd
 * @brief this is the VLBI skd-output class
 *
 * @author Matthias Schartner
 * @date 15.12.2017
 */
class Skd : public VieVS_Object {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param file file name
     */
    explicit Skd( const std::string &file );


    /**
     * @brief creates a skd file
     * @author Matthias Schartner
     *
     * @param network station network
     * @param sourceList source list
     * @param scans scheduled scans
     * @param skdCatalogReader skd catalog reader
     * @param xml VieSchedpp.xml file
     */
    void writeSkd( const Network &network, const SourceList &sourceList, const std::vector<Scan> &scans,
                   const SkdCatalogReader &skdCatalogReader, const boost::property_tree::ptree &xml );


   private:
    static unsigned long nextId;  ///< next id for this object type

    std::ofstream of;  ///< output stream object *filename*.skd

    /**
     * @brief write skd $PARAM block
     * @author Matthias Schartner
     *
     * @param network station network
     * @param xml VieSchedpp.xml file
     * @param skdCatalogReader catalog reader
     */
    void skd_PARAM( const Network &network, const boost::property_tree::ptree &xml,
                    const SkdCatalogReader &skdCatalogReader );


    /**
     * @brief write skd $OP block
     * @author Matthias Schartner
     */
    void skd_OP();


    /**
     * @brief write skd $DOWNTIME block
     * @author Matthias Schartner
     *
     * @param network station network
     */
    void skd_DOWNTIME( const Network &network );


    /**
     * @brief write skd $MAJOR block
     * @author Matthias Schartner
     *
     * @param stations list of all stations
     * @param sourceList list of all sources
     * @param xml paramters.xml file
     * @param skdCatalogReader catalog reader
     */
    void skd_MAJOR( const std::vector<Station> &stations, const SourceList &sourceList,
                    const boost::property_tree::ptree &xml, const SkdCatalogReader &skdCatalogReader );


    /**
     * @brief write skd $MINOR block
     * @author Matthias Schartner
     */
    void skd_MINOR();


    /**
     * @brief write skd $ASTROMETRIC block
     * @author Matthias Schartner
     */
    void skd_ASTROMETRIC();


    /**
     * @brief write skd %STATWT block
     * @author Matthias Schartner
     *
     * @param stations list of all stations
     */
    void skd_STATWT( const std::vector<Station> &stations );


    /**
     * @brief write skd $SRCWT block
     * @author Matthias Schartner
     *
     * @param sourceList list of all sources
     */
    void skd_SRCWT( const SourceList &sourceList );


    /**
     * @brief write skd $CATALOG_USED block
     * @author Matthias Schartner
     *
     * @param xml paramters.xml file
     * @param skdCatalogReader skd catalog reader
     */
    void skd_CATALOG_USED( const boost::property_tree::ptree &xml, const SkdCatalogReader &skdCatalogReader );


    /**
     * @brief write skd $BROADBAND block
     * @author Matthias Schartner
     *
     * @param stations list of all stations
     * @param skdCatalogReader skd catalog reader
     */
    void skd_BROADBAND( const std::vector<Station> &stations, const SkdCatalogReader &skdCatalogReader );


    /**
     * @brief write skd $SOURCES block
     * @author Matthias Schartner
     *
     * @param sourceList list of all sources
     * @param skdCatalogReader catalog reader
     */
    void skd_SOURCES( const SourceList &sourceList, const SkdCatalogReader &skdCatalogReader, const std::vector<Scan> &scans );


    /**
     * @brief write skd $STATIONS block
     * @author Matthias Schartner
     *
     * @param stations list of all stations
     * @param skdCatalogReader catalog reader
     */
    void skd_STATIONS( const std::vector<Station> &stations, const SkdCatalogReader &skdCatalogReader );


    /**
     * @brief write skd $FLUX block
     * @author Matthias Schartner
     *
     * @param sourceList list of all sources
     * @param skdCatalogReader catalog reader
     */
    void skd_FLUX( const SourceList &sourceList, const SkdCatalogReader &skdCatalogReader );


    /**
     * @brief write skd $SKED block
     * @author Matthias Schartner
     *
     * @param stations list of all stations
     * @param sourceList list of all sources
     * @param scans list of all scheduled scans
     * @param skdCatalogReader catalog reader
     */
    void skd_SKED( const std::vector<Station> &stations, const SourceList &sourceList, const std::vector<Scan> &scans,
                   const SkdCatalogReader &skdCatalogReader );


    /**
     * @brief write skd $CODES block
     * @author Matthias Schartner
     *
     * @param stations list of all stations
     * @param skdCatalogReader catalog reader
     */
    void skd_CODES( const std::vector<Station> &stations, const SkdCatalogReader &skdCatalogReader );

    /**
     * @brief write skd $HEAD block
     * @author Matthias Schartner
     *
     * This block is deprecated! I only provide it since drudge crashes in some rare circumstances in case this block is
     * not presetn.
     *
     * @param stations list of all stations
     * @param skdCatalogReader catalog reader
     */
    void skd_HEAD( const std::vector<Station> &stations, const SkdCatalogReader &skdCatalogReader );

    /**
     * @brief write skd $PROCS block
     * @author Matthias Schartner
     *
     * Copy PROCS block of stations that are in the session
     *
     * @param stations list of all stations
     * @param skdCatalogReader catalog reader
     */
    void skd_PROCS( const std::vector<Station> &stations, const SkdCatalogReader &skdCatalogReader );

    static std::string satName( std::string name, unsigned int time );
};
}  // namespace VieVS

#endif  // SKD_H
