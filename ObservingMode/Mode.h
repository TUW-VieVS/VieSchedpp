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
 * @file Mode.h
 * @brief class Mode
 *
 * @author Matthias Schartner
 * @date 17.09.2018
 */

#ifndef VIESCHEDPP_MODE_H
#define VIESCHEDPP_MODE_H


#include <algorithm>
#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>
#include <unordered_map>

#include "../Input/SkdCatalogReader.h"
#include "../Misc/VieVS_NamedObject.h"
#include "Bbc.h"
#include "Freq.h"
#include "If.h"
#include "Track.h"


namespace VieVS {

/**
 * @class Mode
 * @brief MODE block
 *
 * following vex standard
 *
 * @author Matthias Schartner
 * @date 17.09.2018
 */
class Mode : public VieVS_NamedObject {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param name observing mode name
     * @param nsta number of stations
     */
    Mode( std::string name, unsigned long nsta );


    /**
     * @brief converts object to property tree
     * @author Matthias Schartner
     *
     * @param stations station names
     * @return property tree
     */
    boost::property_tree::ptree toPropertytree( const std::vector<std::string> &stations ) const;


    /**
     * @brief add new ID block
     * @author Matthias Schartner
     *
     * @param newIf new IF block
     * @param staids corresponding station Ids
     */
    void addBlock( const std::shared_ptr<const If> &newIf, const std::vector<unsigned long> &staids ) {
        ifs_.emplace_back( newIf, staids );
    }


    /**
     * @brief add new BBC block
     * @author Matthias Schartner
     *
     * @param newBbc new BBC block
     * @param staids corresponding station Ids
     */
    void addBlock( const std::shared_ptr<const Bbc> &newBbc, const std::vector<unsigned long> &staids ) {
        bbcs_.emplace_back( newBbc, staids );
    }


    /**
     * @brief add new FREQ block
     * @author Matthias Schartner
     *
     * @param newFreq new FREQ block
     * @param staids corresponding station Ids
     */
    void addBlock( const std::shared_ptr<const Freq> &newFreq, const std::vector<unsigned long> &staids ) {
        freqs_.emplace_back( newFreq, staids );
        const auto &tmp = newFreq->getBands();
        bands_.insert( tmp.begin(), tmp.end() );
    }


    /**
     * @brief add new Tracks block
     * @author Matthias Schartner
     *
     * @param newTrack new Tracks block
     * @param staids corresponding station Ids
     */
    void addBlock( const std::shared_ptr<const Track> &newTrack, const std::vector<unsigned long> &staids ) {
        tracks_.emplace_back( newTrack, staids );
    }


    /**
     * @brief add new track frame format block
     * @author Matthias Schartner
     *
     * @param newTrackFrameFormat new track frame format block
     * @param staids corresponding station Ids
     */
    void addBlock( const std::shared_ptr<const std::string> &newTrackFrameFormat,
                   const std::vector<unsigned long> &staids ) {
        track_frame_formats_.emplace_back( newTrackFrameFormat, staids );
    }


    /**
     * @brief calculate recording rates
     * @author Matthias Schartner
     */
    void calcRecordingRates();


    /**
     * @brief set recording rates for simple manual model
     * @author Matthias Schartner
     *
     * @param band band name
     * @param recRate recording rate
     */
    void setRecordingRates( const std::string &band, double recRate );


    /**
     * @brief set efficiency factor for simple manual model
     * @author Matthias Schartner
     *
     * @param eff efficiency factor
     */
    void setEfficiencyFactor( double eff );


    /**
     * @brief set band names for simple manual model
     * @author Matthias Schartner
     *
     * @param bands list of all bands
     */
    void setBands( const std::set<std::string> &bands ) { bands_ = bands; }


    /**
     * @brief recording rate for observation
     * @author Matthias Schartner
     *
     * @param staid1 station 1
     * @param staid2 station 2
     * @param band observed band
     * @return recording rate
     */
    double recordingRate( unsigned long staid1, unsigned long staid2, const std::string &band ) const;


    /**
     * @brief efficiency factor between stations
     * @author Matthias Schartner
     *
     * efficiency is based on bit efficiency and correlator efficiency
     * Bit efficiency:
     * 1-bit: 0.5715
     * 2-bit: 0.6366
     * Correlator efficiency:
     * DiFX: 0.97
     *
     * @param staid1 station 1
     * @param staid2 station 2
     * @return efficiency factor
     */
    double efficiency( unsigned long staid1, unsigned long staid2 ) const;


    /**
     * @brief get IF block per station
     * @author Matthias Schartner
     *
     * @param staid station id
     * @return IF block
     */
    boost::optional<const std::shared_ptr<const If> &> getIf( unsigned long staid ) const;


    /**
     * @brief get BBC block per station
     * @author Matthias Schartner
     *
     * @param staid station id
     * @return BBC block
     */
    boost::optional<const std::shared_ptr<const Bbc> &> getBbc( unsigned long staid ) const;


    /**
     * @brief get FREQ block per station
     * @author Matthias Schartner
     *
     * @param staid station id
     * @return FREQ block
     */
    boost::optional<const std::shared_ptr<const Freq> &> getFreq( unsigned long staid ) const;


    /**
     * @brief get TRACKS block per station
     * @author Matthias Schartner
     *
     * @param staid station id
     * @return TRACKS block
     */
    boost::optional<const std::shared_ptr<const Track> &> getTracks( unsigned long staid ) const;


    /**
     * @brief get track frame format block per station
     * @author Matthias Schartner
     *
     * @param staid station id
     * @return track frame format
     */
    boost::optional<const std::shared_ptr<const std::string> &> getTrackFrameFormat( unsigned long staid ) const;


    /**
     * @brief get list of all station per IF block
     * @author Matthias Schartner
     *
     * @param this_if target IF block
     * @return list of stations
     */
    boost::optional<const std::vector<unsigned long> &> getAllStationsWithBlock(
        const std::shared_ptr<const If> &this_if ) const;


    /**
     * @brief get list of all station per BBC block
     * @author Matthias Schartner
     *
     * @param bbc target BBC block
     * @return list of stations
     */
    boost::optional<const std::vector<unsigned long> &> getAllStationsWithBlock(
        const std::shared_ptr<const Bbc> &bbc ) const;


    /**
     * @brief get list of all station per FREQ block
     * @author Matthias Schartner
     *
     * @param freq target FREQ block
     * @return list of stations
     */
    boost::optional<const std::vector<unsigned long> &> getAllStationsWithBlock(
        const std::shared_ptr<const Freq> &freq ) const;


    /**
     * @brief get list of all station per TRACKS block
     * @author Matthias Schartner
     *
     * @param track target TRACKS block
     * @return list of stations
     */
    boost::optional<const std::vector<unsigned long> &> getAllStationsWithBlock(
        const std::shared_ptr<const Track> &track ) const;


    /**
     * @brief get list of all station per track frame format
     * @author Matthias Schartner
     *
     * @param trackFrameFormat target track frame format
     * @return list of stations
     */
    boost::optional<const std::vector<unsigned long> &> getAllStationsWithBlock(
        const std::shared_ptr<const std::string> &trackFrameFormat ) const;


    /**
     * @brief create summary
     * @author Matthias Schartner
     *
     * @param of outstream
     * @param stations station names
     */
    void summary( std::ofstream &of, const std::vector<std::string> &stations ) const;


    /**
     * @brief create summary for operation notes
     * @author Matthias Schartner
     *
     * @param of outstream
     * @param stations station names
     */
    void operationNotesSummary( std::ofstream &of, const std::vector<std::string> &stations ) const;


    /**
     * @brief get recording rate per station
     * @author Matthias Schartner
     *
     * @param staid station id
     * @return recording rate
     */
    double recordingRate( unsigned long staid ) const;


    /**
     * @brief writes MODE block in vex format
     * @author Matthias Schartner
     *
     * @param of vex file stream
     * @param stations station names
     */
    void toVexModeDefiniton( std::ofstream &of, const std::vector<std::string> &stations ) const;


    /**
     * @brief get list of all bands
     * @author Matthias Schartner
     *
     * @return list of all bands
     */
    const std::set<std::string> getAllBands() const { return bands_; }


    /**
     * @brief getter for number of stations
     * @author Matthias Schartner
     *
     * @return number of stations
     */
    unsigned long getNSta() const { return nsta_; }


    /**
     * @brief change IF for station
     * @author Matthias Schartner
     *
     * This function is used for setting up manual observing mode in the GUI
     * It changes the IF for station staid
     *
     * @param idx index of new IF
     * @param staid target station
     */
    void changeIf( int idx, unsigned long staid );


    /**
     * @brief change BBC for station
     * @author Matthias Schartner
     *
     * This function is used for setting up manual observing mode in the GUI
     * It changes the BBC for station staid
     *
     * @param idx index of new BBC
     * @param staid target station
     */
    void changeBbc( int idx, unsigned long staid );


    /**
     * @brief change TRACK for station
     * @author Matthias Schartner
     *
     * This function is used for setting up manual observing mode in the GUI
     * It changes the TRACK for station staid
     *
     * @param idx index of new TRACK
     * @param staid target station
     */
    void changeTracks( int idx, unsigned long staid );


    /**
     * @brief change track frame format for station
     * @author Matthias Schartner
     *
     * This function is used for setting up manual observing mode in the GUI
     * It changes the track frame format for station staid
     *
     * @param idx index of new track frame format
     * @param staid target station
     */
    void changeTrackFrameFormat( int idx, unsigned long staid );


    /**
     * @brief change FREQ for station
     * @author Matthias Schartner
     *
     * This function is used for setting up manual observing mode in the GUI
     * It changes the FREQ for station staid
     *
     * @param idx index of new FREQ
     * @param staid target station
     */
    void changeFreq( int idx, unsigned long staid );


    /**
     * @brief remove FREQ block
     * @author Matthias Schartner
     *
     * This function is used for setting up manual observing mode in the GUI
     *
     * @param idx index of block which whould be removed
     */
    void removeFreq( int idx ) { freqs_.erase( freqs_.begin() + idx ); }


    /**
     * @brief remove BBC block
     * @author Matthias Schartner
     *
     * This function is used for setting up manual observing mode in the GUI
     *
     * @param idx index of block which whould be removed
     */
    void removeBbc( int idx ) { bbcs_.erase( bbcs_.begin() + idx ); }


    /**
     * @brief remove IF block
     * @author Matthias Schartner
     *
     * This function is used for setting up manual observing mode in the GUI
     *
     * @param idx index of block which whould be removed
     */
    void removeIf( int idx ) { ifs_.erase( ifs_.begin() + idx ); }


    /**
     * @brief remove TRACKS block
     * @author Matthias Schartner
     *
     * This function is used for setting up manual observing mode in the GUI
     *
     * @param idx index of block which whould be removed
     */
    void removeTracks( int idx ) { tracks_.erase( tracks_.begin() + idx ); }


    /**
     * @brief remove track frame format
     * @author Matthias Schartner
     *
     * This function is used for setting up manual observing mode in the GUI
     *
     * @param idx index of block which whould be removed
     */
    void removeTrackFrameFormats( int idx ) { track_frame_formats_.erase( track_frame_formats_.begin() + idx ); }


   private:
    static unsigned long nextId;  ///< next id for this object type

    unsigned long nsta_;  ///< number of stations

    std::vector<std::pair<std::shared_ptr<const If>, std::vector<unsigned long>>>
        ifs_;  ///< all IF blocks with corresponding station ids
    std::vector<std::pair<std::shared_ptr<const Bbc>, std::vector<unsigned long>>>
        bbcs_;  ///< all BBC blocks with corresponding station ids
    std::vector<std::pair<std::shared_ptr<const Freq>, std::vector<unsigned long>>>
        freqs_;  ///< all FREQ blocks with corresponding station ids
    std::vector<std::pair<std::shared_ptr<const Track>, std::vector<unsigned long>>>
        tracks_;  ///< all TRACKS blocks with corresponding station ids
    std::vector<std::pair<std::shared_ptr<const std::string>, std::vector<unsigned long>>>
        track_frame_formats_;  ///< all track frame format blocks with corresponding station ids

    std::unordered_map<std::pair<unsigned long, unsigned long>, std::unordered_map<std::string, double>,
                       boost::hash<std::pair<unsigned long, unsigned long>>>
        staids2recordingRate_;  ///< recording rate per station ids and band
    std::unordered_map<std::pair<unsigned long, unsigned long>, double,
                       boost::hash<std::pair<unsigned long, unsigned long>>>
        staids2efficiency_;  ///< efficiency per station ids

    std::set<std::string> bands_;  ///< list of all bands

    /**
     * @brief station ids to property tree
     * @author Matthias Schartner
     *
     * @param name block name
     * @param ids station ids
     * @param staNames station names
     * @return property tree
     */
    boost::property_tree::ptree staids2propertyTree( const std::string &name, const std::vector<unsigned long> &ids,
                                                     const std::vector<std::string> &staNames ) const;
};

}  // namespace VieVS

#endif  // VIESCHEDPP_MODE_H
