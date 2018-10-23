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
     * @brief observing mode
     *
     * CURRENTLY UNDER DEVELOPMENT AND UNUSED
     *
     * @author Matthias Schartner
     * @date 17.09.2018
     */
    class Mode: public VieVS_NamedObject {
    public:
        Mode(std::string name, unsigned long nsta);

        void readFromSkedCatalogs(const SkdCatalogReader &skd);

        void addIf(const If &newIf, const std::vector<unsigned long> &staids);

        void addBbc(const Bbc &newBbc, const std::vector<unsigned long> &staids);

        void addFreq(const Freq &newFreq, const std::vector<unsigned long> &staids);

        void addTrack(const Track &newTrack, const std::vector<unsigned long> &staids);

        void addTrackFrameFormat(const std::string &newTrackFrameFormat, const std::vector<unsigned long> &staids);

        void calcRecordingRates();



        boost::optional<const If &>getIf(unsigned long staid);

        boost::optional<const Bbc &>getBbc(unsigned long staid);

        boost::optional<const Freq &>getFreq(unsigned long staid);

        boost::optional<const Track &>getTrack(unsigned long staid);

        boost::optional<const std::string &>getTrackFrameFormat(unsigned long staid);



        boost::optional<const std::vector<unsigned long> &>getAllStationsWithIf(std::string name);

        boost::optional<const std::vector<unsigned long> &>getAllStationsWithBbc(std::string name);

        boost::optional<const std::vector<unsigned long> &>getAllStationsWithFreq(std::string name);

        boost::optional<const std::vector<unsigned long> &>getAllStationsWithTrack(std::string name);

        boost::optional<const std::vector<unsigned long> &>getAllStationsWithTrackFrameFormat(std::string name);

    private:
        static unsigned long nextId;

        unsigned long nsta_;

        std::vector<std::pair<If, std::vector<unsigned long>>> ifs_;
        std::vector<std::pair<Bbc, std::vector<unsigned long>>> bbcs_;
        std::vector<std::pair<Freq, std::vector<unsigned long>>> freqs_;
        std::vector<std::pair<Track, std::vector<unsigned long>>> tracks_;
        std::vector<std::pair<std::string, std::vector<unsigned long>>> track_frame_formats_;

        std::unordered_map< std::pair<unsigned long, unsigned long>,
                                      std::unordered_map<std::string,double>,
                                      boost::hash<std::pair<unsigned long, unsigned long>>>
                staids2recordingRate_;

        /**
         * @brief create FREQ block from skd catalogs
         * @author Matthias Schartner
         *
         * @param skd skd catalogs
         */
        void readSkdFreq(const SkdCatalogReader &skd, const std::map<int,int> &channelNr2Bbc);

        /**
         * @brief create TRACKS block from skd catalogs
         * @author Matthias Schartner
         *
         * @param skd skd catalogs
         * @return channel number to bbc number map
         */
        std::map<int,int> readSkdTracks(const SkdCatalogReader &skd);

        /**
         * @brief create IF block from skd catalogs
         * @author Matthias Schartner
         *
         * @param skd skd catalogs
         */
        void readSkdIf(const SkdCatalogReader &skd);

        /**
         * @brief create BBC block from skd catalogs
         * @author Matthias Schartner
         *
         * @param skd skd catalogs
         */
        void readSkdBbc(const SkdCatalogReader &skd);


        /**
         * @brief create track frame format from skd catalogs
         * @author Matthias Schartner
         *
         * @param skd skd catalogs
         */
        void readSkdTrackFrameFormat(const SkdCatalogReader &skd);

    };

}


#endif //VIESCHEDPP_MODE_H
