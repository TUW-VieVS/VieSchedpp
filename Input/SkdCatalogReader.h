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
 * @file SkdCatalogReader.h
 * @brief class SkdCatalogReader
 *
 * @author Matthias Schartner
 * @date 22.10.2017
 */

#ifndef SKDCATALOGREADER_H
#define SKDCATALOGREADER_H


#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "../Misc/VieVS_Object.h"
#include "../Misc/util.h"
#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#endif


namespace VieVS {

/**
 * @class SkdCatalogReader
 * @brief sked catalog reader class
 *
 * @author Matthias Schartner
 * @date 07.12.2017
 */
class SkdCatalogReader : public VieVS_Object {
   public:
    /**
     * @brief All available and read sked catalog files which can be read with the same return type.
     * @author Matthias Schartner
     */
    enum class CATALOG {
        source,  ///< source.cat file
        flux,    ///< flux.cat file

        antenna,   ///< antenna.cat file
        position,  ///< position.cat file
        equip,     ///< equip.cat file
        mask,      ///< mask.cat file
    };


    /**
     * @brief default constructor
     * @author Matthias Schartner
     */
    SkdCatalogReader();


    /**
     * @brief setter for catalog pathes
     * @author Matthias Schartner
     *
     * @param ptreeWithPathes property tree with catalog pathes from .xml file
     */
    void setCatalogFilePathes( const boost::property_tree::ptree &ptreeWithPathes );


    /**
     * @brief set catalog file pathes
     * @author Matthias Schartner
     *
     * @param antenna antenna catalog name
     * @param equip equip catalog name
     * @param flux flux catalog name
     * @param freq freq catalog name
     * @param hdpos hdpos catalog name
     * @param loif loif catalog name
     * @param mask mask catalog name
     * @param modes modes catalog name
     * @param position position catalog name
     * @param rec rec catalog name
     * @param rx rx catalog name
     * @param source source catalog name
     * @param tracks tracks catalog name
     */
    void setCatalogFilePathes( const std::string &antenna, const std::string &equip, const std::string &flux,
                               const std::string &freq, const std::string &hdpos, const std::string &loif,
                               const std::string &mask, const std::string &modes, const std::string &position,
                               const std::string &rec, const std::string &rx, const std::string &source,
                               const std::string &tracks );


    /**
     * @brief
     * @author Matthias Schartner
     *
     * @param skdFile skd file name
     */
    void setCatalogFilePathes( const std::string &skdFile );


    /**
     * @brief reads all source specific catalogs
     * @author Matthias Schartner
     */
    void initializeSourceCatalogs();


    /**
     * @brief reads all station specific catalogs
     * @author Matthias Schartner
     */
    void initializeStationCatalogs();


    /**
     * @brief reads all modes specific catalogs
     * @author Matthias Schartner
     *
     * @param obsModeName name of the observation mode
     */
    void initializeModesCatalogs( const std::string &obsModeName );


    /**
     * @brief set station names
     * @author Matthias Schartner
     *
     * @param staNames all participating stations
     */
    void setStationNames( const std::vector<std::string> &staNames ) { staNames_ = staNames; }


    /**
     * @brief getter for all station names
     * @author Matthias Schartner
     *
     * @return all station names
     */
    const std::vector<std::string> &getStaNames() const { return staNames_; }


    /**
     * @brief getter for station one letter code
     * @author Matthias Schartner
     *
     * @return map with station name as key and one letter code as value
     */
    const std::map<std::string, char> &getOneLetterCode() const { return oneLetterCode_; }


    /**
     * @brief getter for station two letter code
     * @author Matthias Schartner
     *
     * @return map with station name as key and two letter code as value
     */
    const std::map<std::string, std::string> &getTwoLetterCode() const { return twoLetterCode_; }


    /**
     * @brief getter for source catalog
     * @author Matthias Schartner
     *
     * @return map with source name id as key and the corresponding catalog line as value
     */
    const std::map<std::string, std::vector<std::string>> &getSourceCatalog() const { return sourceCatalog_; }


    /**
     * @brief getter for flux catalog
     * @author Matthias Schartner
     *
     * @return map with source name as key and all flux infos as value
     */
    const std::map<std::string, std::vector<std::string>> &getFluxCatalog() const { return fluxCatalog_; }


    /**
     * @brief getter for antenna catalog
     * @author Matthias Schartner
     *
     * @return map with antenna name as key and the corresponding catalog line as value
     */
    const std::map<std::string, std::vector<std::string>> &getAntennaCatalog() const { return antennaCatalog_; }


    /**
     * @brief getter for position catalog
     * @author Matthias Schartner
     *
     * @return map with position id as key and the corresponding catalog line as value
     */
    const std::map<std::string, std::vector<std::string>> &getPositionCatalog() const { return positionCatalog_; }


    /**
     * @brief getter for equipment catalog
     * @author Matthias Schartner
     *
     * @return map with equipment id + "|" + station name as key and corresponding catalog line as value
     */
    const std::map<std::string, std::vector<std::string>> &getEquipCatalog() const { return equipCatalog_; }


    /**
     * @brief getter for mask catalog
     * @author Matthias Schartner
     *
     * @return map with mask id as key and corresponding catalog line as value
     */
    const std::map<std::string, std::vector<std::string>> &getMaskCatalog() const { return maskCatalog_; }


    /**
     * @brief getter for observing mode name
     * @author Matthias Schartner
     *
     * @return observing mode name
     */
    const std::string &getModeName() const { return modeName_; }


    /**
     * @brief getter for frequency sequence name
     * @author Matthias Schartner
     *
     * @return frequency sequence name
     */
    const std::string &getFreqName() const { return freqName_; }


    /**
     * @brief getter for bandwith
     * @author Matthias Schartner
     *
     * @return bandwidth
     */
    double getBandWidth() const { return bandWidth_; }


    /**
     * @brief getter for sample rate
     * @author Matthias Schartner
     *
     * @return sample rate
     */
    double getSampleRate() const { return sampleRate_; }


    /**
     * @brief getter for station hdpos
     * @author Matthias Schartner
     *
     * @return map with station name as key and hdpos name as value
     */
    const std::map<std::string, std::string> &getStaName2hdposMap() const { return staName2hdposMap_; }


    /**
     * @brief getter for station tracks
     * @author Matthias Schartner
     *
     * @return map wich station names as key and tracks name as value
     */
    const std::map<std::string, std::string> &getStaName2tracksMap() const { return staName2tracksMap_; }


    /**
     * @brief getter for all track ids
     * @author Matthias Schartner
     *
     * @return vector of track ids
     */
    const std::vector<std::string> &getTracksIds() const { return tracksIds_; }


    /**
     * @brief getter for all loif ids
     * @author Matthias Schartner
     *
     * @return vector of loif ids
     */
    const std::vector<std::string> &getLoifIds() const { return loifIds_; }


    /**
     * @brief getter for tracks fanout
     * @author Matthias Schartner
     *
     * @return map with tracks name as key and fanout as value
     */
    const std::map<std::string, int> &getTracksId2fanoutMap() const { return tracksId2fanoutMap_; }


    /**
     * @brief getter for tracks bit
     * @author Matthias Schartner
     *
     * @return map with tracks name as key and number of sampled bits as value
     */
    const std::map<std::string, int> &getTracksId2bits() const { return tracksId2bitsMap_; }


    /**
     * @brief getter for channel tracks info
     * @author Matthias Schartner
     *
     * @return map with tracks name as key and map with channel number as key and tracks infor as value as value
     */
    const std::map<std::string, std::map<int, std::string>> &getTracksId2channelNumber2tracksMap() const {
        return tracksId2channelNumber2tracksMap_;
    }


    /**
     * @brief getter for frequency two letter code
     * @author Matthias Schartner
     *
     * @return frequency two letter code
     */
    const std::string &getFreqTwoLetterCode() const { return freqTwoLetterCode_; }


    /**
     * @brief getter for cannel band
     * @author Matthias Schartner
     *
     * @return map with channel number as key and band name as value
     */
    const std::map<int, std::string> &getChannelNumber2band() const { return channelNumber2band_; }


    /**
     * @brief getter for channel sky frequency
     * @author Matthias Schartner
     *
     * @return map with channel number as key and sky frequency as value
     */
    const std::map<int, std::string> &getChannelNumber2skyFreq() const { return channelNumber2skyFreq_; }


    /**
     * @brief getter for channel phase cal frequency
     * @author Matthias Schartner
     *
     * @return map with channel number as key and phace cal frequency as value
     */
    const std::map<int, std::string> &getChannelNumber2phaseCalFrequency() const {
        return channelNumber2phaseCalFrequency_;
    }


    /**
     * @brief getter for station local oscillator frequency setup name
     * @author Matthias Schartner
     *
     * @return map with station name as key and locas oscillator frequency setup name as value
     */
    const std::map<std::string, std::string> &getStaName2loifId() const { return staName2loifId_; }


    /**
     * @brief getter for local oscillator frequency setup
     * @author Matthias Schartner
     *
     * @return map with loif name as key and loif info as value
     */
    const std::map<std::string, std::vector<std::string>> &getLoifId2loifInfo() const { return loifId2loifInfo_; }


    /**
     * @brief getter for channel bbc number
     * @author Matthias Schartner
     *
     * @return map with channel number as key and bbc number as value
     */
    const std::map<int, std::string> &getChannelNumber2BBC() const { return channelNumber2BBC_; }


    /**
     * @brief getter for recoreded bits
     * @author Matthias Schartner
     *
     * @return number of recorded bits
     */
    unsigned int getBits() const { return bits_; }


    /**
     * @brief get position key
     * @author Matthias Schartner
     *
     * @param staName station name
     * @return position key
     */
    const std::string &positionKey( const std::string &staName ) const { return antennaKey2positionKey_.at( staName ); }


    /**
     * @brief get equip key
     * @author Matthias Schartner
     *
     * @param staName station name
     * @return equip key
     */
    const std::string &equipKey( const std::string &staName ) const { return antennaKey2equipKey_.at( staName ); }


    /**
     * @brief get mask key
     * @author Matthias Schartner
     *
     * @param staName station name
     * @return mask key
     */
    const std::string &maskKey( const std::string &staName ) const { return antennaKey2maskKey_.at( staName ); }


    /**
     * @brief get sked catalog version number
     * @author Matthias Schartner
     *
     * @param name sked catalog
     * @return version number
     */
    std::string getVersion( const std::string &name ) const;


   private:
    static unsigned long nextId;  ///< next id for this object type

    std::vector<std::string> staNames_;  ///< list of all station

    std::string sourcePath_;  ///< source catalog name
    std::string fluxPath_;    ///< flux catalog name

    std::string antennaPath_;   ///< antenna catalog name
    std::string positionPath_;  ///< position catalog name
    std::string equipPath_;     ///< equipment catalog name
    std::string maskPath_;      ///< mask catalog name

    std::string modesPath_;   ///< modes catalog name
    std::string recPath_;     ///< rec catalog name
    std::string tracksPath_;  ///< tracks catalog name
    std::string freqPath_;    ///< frequency catalog name
    std::string rxPath_;      ///< rx catalog name
    std::string loifPath_;    ///< loif catalog name

    std::map<std::string, char> oneLetterCode_;         ///< map with station name as key and one letter codes as value
    std::map<std::string, std::string> twoLetterCode_;  ///< map with station name as key and two letter code as value

    std::map<std::string, std::vector<std::string>>
        sourceCatalog_;  ///< map with source name as key and catalog entry as value
    std::map<std::string, std::vector<std::string>>
        fluxCatalog_;  ///< map with source name as key and flux catalog entry as value

    std::map<std::string, std::vector<std::string>>
        antennaCatalog_;  ///< map with station name as key and catalog entry as value
    std::map<std::string, std::string> antennaKey2positionKey_;  ///< antenna key to position key
    std::map<std::string, std::vector<std::string>>
        positionCatalog_;  ///< map with position id as key and catalog entry as value
    std::map<std::string, std::string> antennaKey2equipKey_;  ///< antenna key to equip key
    std::map<std::string, std::vector<std::string>>
        equipCatalog_;  ///< map with equipment id + "|" + station name as key and catalog entry as value
    std::map<std::string, std::string> antennaKey2maskKey_;  ///< antenna key 2 mask key
    std::map<std::string, std::vector<std::string>>
        maskCatalog_;  ///< map with mask id as key and catalog entry as value

    std::string modeName_;  ///< observing mode name
    std::string freqName_;  ///< frequency sequence name
    std::string recName_;   ///< rec name
    double bandWidth_;      ///< bandwith
    double sampleRate_;     ///< sample rate
    unsigned int bits_;     ///< sample bits

    std::map<std::string, std::string> staName2hdposMap_;   ///< map with station name as key and hdpos name as value
    std::map<std::string, std::string> staName2tracksMap_;  ///< map with station name as key and tracks name as value
    std::vector<std::string> tracksIds_;                    ///< list of all track names
    std::map<std::string, std::string>
        staName2recFormatMap_;  ///< map with station name as key and rec format name as value

    std::map<std::string, int> tracksId2fanoutMap_;  ///< map with track name as key and fanout as value
    std::map<std::string, int> tracksId2bitsMap_;    ///< map with track name as key and number of sampled bits as value
    std::map<std::string, std::map<int, std::string>>
        tracksId2channelNumber2tracksMap_;  ///< map with channel number as key and tracks info as value

    std::string freqTwoLetterCode_;                  ///< two letter frequency code
    std::string rxName_;                             ///< rx name
    std::map<int, std::string> channelNumber2band_;  ///< map with channel number as key and band name as value
    std::map<int, std::string>
        channelNumber2polarization_;                     ///< map with channel number as key and polarization as value
    std::map<int, std::string> channelNumber2skyFreq_;   ///< map with channel number as key and sky frequency as value
    std::map<int, std::string> channelNumber2sideBand_;  ///< map with channel number as key and side band as value
    std::map<int, std::string> channelNumber2BBC_;       ///< map with channel number as key and bbc number as value
    std::map<int, std::string>
        channelNumber2phaseCalFrequency_;  ///< map with channel number as key and phase cal frequency as value

    std::map<std::string, std::string>
        staName2loifId_;  ///< map with station name as key and local oscillator frequency setup name as value
    std::vector<std::string> loifIds_;  ///< list of all local oscillator frequency setup names

    std::map<std::string, std::vector<std::string>>
        loifId2loifInfo_;  ///< map with loif name as key and loif infos as value

    std::map<std::string, std::string> catalogsVersion_;  ///< skd catalog versions

    /**
     * @brief This function reads a specific sked catalog file and stores the data in a map.
     * @author Matthias Schartner
     *
     * @param type catalog file which should be read
     * @return key is list of all Ids, value is corresponding catalog entry
     */
    std::map<std::string, std::vector<std::string>> readCatalog( CATALOG type ) noexcept;


    /**
     * @brief reads modes.cat
     * @author Matthias Schartner
     *
     * @param obsModeName mode name
     */
    void readModesCatalog( const std::string &obsModeName );


    /**
     * @brief read rec.cat
     * @author Matthias Schartner
     */
    void readRecCatalog();


    /**
     * @brief read tracks.cat
     * @author Matthias Schartner
     */
    void readTracksCatalog();


    /**
     * @brief read freq.cat
     * @author Matthias Schartner
     */
    void readFreqCatalog();


    /**
     * @brief read rx.cat
     * @author Matthias Schartner
     */
    void readRxCatalog();


    /**
     * @brief read loif.cat
     * @author Matthias Schartner
     */
    void readLoifCatalog();


    /**
     * @brief save one letter codes
     * @author Matthias Schartner
     */
    void saveOneLetterCode();


    /**
     * @brief save two letter codes
     * @author Matthias Schartner
     */
    void saveTwoLetterCode();
};
}  // namespace VieVS

#endif  // SKDCATALOGREADER_H
