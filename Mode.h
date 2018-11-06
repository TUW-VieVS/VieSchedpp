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

#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/functional/hash.hpp>
#include <algorithm>

#include "VieVS_NamedObject.h"
#include "SkdCatalogReader.h"
#include "Freq.h"
#include "Track.h"
#include "Bbc.h"
#include "If.h"

namespace VieVS{

    /**
     * @class Mode
     * @brief MODE block
     *
     * following vex standard
     *
     * @author Matthias Schartner
     * @date 17.09.2018
     */
    class Mode: public VieVS_NamedObject {
    public:

        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param name observing mode name
         * @param nsta number of stations
         */
        Mode(std::string name, unsigned long nsta);

        /**
         * @brief add new ID block
         * @author Matthias Schartner
         *
         * @param newIf new IF block
         * @param staids corresponding station Ids
         */
        void addIf(const std::shared_ptr<const If> &newIf, const std::vector<unsigned long> &staids){
            ifs_.emplace_back(newIf, staids);
        }

        /**
         * @brief add new BBC block
         * @author Matthias Schartner
         *
         * @param newBbc new BBC block
         * @param staids corresponding station Ids
         */
        void addBbc(const std::shared_ptr<const Bbc> &newBbc, const std::vector<unsigned long> &staids){
            bbcs_.emplace_back(newBbc, staids);
        }

        /**
         * @brief add new FREQ block
         * @author Matthias Schartner
         *
         * @param newFreq new FREQ block
         * @param staids corresponding station Ids
         */
        void addFreq(const std::shared_ptr<const Freq> &newFreq, const std::vector<unsigned long> &staids){
            freqs_.emplace_back(newFreq, staids);
            const auto &tmp = newFreq->getBands();
            bands_.insert(tmp.begin(), tmp.end());
        }

        /**
         * @brief add new Tracks block
         * @author Matthias Schartner
         *
         * @param newTrack new Tracks block
         * @param staids corresponding station Ids
         */
        void addTrack(const std::shared_ptr<const Track> &newTrack, const std::vector<unsigned long> &staids){
            tracks_.emplace_back(newTrack, staids);
        }

        /**
         * @brief add new track frame format block
         * @author Matthias Schartner
         *
         * @param newTrackFrameFormat new track frame format block
         * @param staids corresponding station Ids
         */
        void addTrackFrameFormat(const std::shared_ptr<const std::string> &newTrackFrameFormat, const std::vector<unsigned long> &staids){
            track_frame_formats_.emplace_back(newTrackFrameFormat, staids);
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
        void setRecordingRates(const std::string &band, double recRate);

        /**
         * @brief set band names for simple manual model
         * @author Matthias Schartner
         *
         * @param bands list of all bands
         */
        void setBands(const std::set<std::string> &bands){
            bands_ = bands;
        }

        /**
         * @brief recording rate for observation
         * @author Matthias Schartner
         *
         * @param staid1 station 1
         * @param staid2 station 2
         * @param band observed band
         * @return recording rate
         */
        double recordingRate(unsigned long staid1, unsigned long staid2, const std::string &band) const;

        /**
         * @brief get IF block per station
         * @author Matthias Schartner
         *
         * @param staid station id
         * @return IF block
         */
        boost::optional<const std::shared_ptr<const If> &>getIf(unsigned long staid) const;

        /**
         * @brief get BBC block per station
         * @author Matthias Schartner
         *
         * @param staid station id
         * @return BBC block
         */
        boost::optional<const std::shared_ptr<const Bbc> &>getBbc(unsigned long staid) const;

        /**
         * @brief get FREQ block per station
         * @author Matthias Schartner
         *
         * @param staid station id
         * @return FREQ block
         */
        boost::optional<const std::shared_ptr<const Freq> &>getFreq(unsigned long staid) const;

        /**
         * @brief get TRACKS block per station
         * @author Matthias Schartner
         *
         * @param staid station id
         * @return TRACKS block
         */
        boost::optional<const std::shared_ptr<const Track> &>getTracks(unsigned long staid) const;

        /**
         * @brief get track frame format block per station
         * @author Matthias Schartner
         *
         * @param staid station id
         * @return track frame format
         */
        boost::optional<const std::shared_ptr<const std::string> &> getTrackFrameFormat(unsigned long staid) const;

        /**
         * @brief get list of all station per IF block
         * @author Matthias Schartner
         *
         * @param this_if target IF block
         * @return list of stations
         */
        boost::optional<const std::vector<unsigned long> &>getAllStationsWithIf(const std::shared_ptr<const If> &this_if)const ;

        /**
         * @brief get list of all station per BBC block
         * @author Matthias Schartner
         *
         * @param bbc target BBC block
         * @return list of stations
         */
        boost::optional<const std::vector<unsigned long> &>getAllStationsWithBbc(const std::shared_ptr<const Bbc> &bbc)const ;

        /**
         * @brief get list of all station per FREQ block
         * @author Matthias Schartner
         *
         * @param freq target FREQ block
         * @return list of stations
         */
        boost::optional<const std::vector<unsigned long> &>getAllStationsWithFreq(const std::shared_ptr<const Freq> &freq)const ;

        /**
         * @brief get list of all station per TRACKS block
         * @author Matthias Schartner
         *
         * @param track target TRACKS block
         * @return list of stations
         */
        boost::optional<const std::vector<unsigned long> &>getAllStationsWithTrack(const std::shared_ptr<const Track> &track)const ;

        /**
         * @brief get list of all station per track frame format
         * @author Matthias Schartner
         *
         * @param trackFrameFormat target track frame format
         * @return list of stations
         */
        boost::optional<const std::vector<unsigned long> &>getAllStationsWithTrackFrameFormat(const std::shared_ptr<const std::string> &trackFrameFormat)const ;

        /**
         * @brief create summary
         * @author Matthias Schartner
         *
         * @param of outstream
         * @param stations station names
         */
        void summary( std::ofstream &of, const std::vector<std::string> &stations) const;

        /**
         * @brief writes MODE block in vex format
         * @author Matthias Schartner
         *
         * @param of vex file stream
         * @param stations station names
         */
        void toVexModeDefiniton(std::ofstream &of, const std::vector<std::string> &stations) const;

        /**
         * @brief get list of all bands
         * @author Matthias Schartner
         *
         * @return list of all bands
         */
        const std::set<std::string> getAllBands() const{
            return bands_;
        }

    private:
        static unsigned long nextId; ///< next id for this object type

        unsigned long nsta_; ///< number of stations


        std::vector<std::pair<std::shared_ptr<const If>, std::vector<unsigned long>>> ifs_; ///< all IF blocks with corresponding station ids
        std::vector<std::pair<std::shared_ptr<const Bbc>, std::vector<unsigned long>>> bbcs_; ///< all BBC blocks with corresponding station ids
        std::vector<std::pair<std::shared_ptr<const Freq>, std::vector<unsigned long>>> freqs_; ///< all FREQ blocks with corresponding station ids
        std::vector<std::pair<std::shared_ptr<const Track>, std::vector<unsigned long>>> tracks_; ///< all TRACKS blocks with corresponding station ids
        std::vector<std::pair<std::shared_ptr<const std::string>, std::vector<unsigned long>>> track_frame_formats_; ///< all track frame format blocks with corresponding station ids

        std::unordered_map< std::pair<unsigned long, unsigned long>,
                                      std::unordered_map<std::string,double>,
                                      boost::hash<std::pair<unsigned long, unsigned long>>> staids2recordingRate_; ///< recording rate per station ids and band


        std::set<std::string> bands_; ///< list of all bands

    };

}


#endif //VIESCHEDPP_MODE_H
