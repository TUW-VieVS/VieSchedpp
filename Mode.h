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
     * CURRENTLY UNDER DEVELOPMENT
     *
     * @author Matthias Schartner
     * @date 17.09.2018
     */
    class Mode: public VieVS_NamedObject {
    public:
        /**
         * @brief all possible flux information type
         * @author Matthias Schartner
         */
        enum class Property {
            required,    ///< this band information is required. If this information is missing this object is not used.
            optional,    ///< this band information is only optional. If information is available it is used, otherwise it is calculated based on backup model
        };

        /**
         * @brief all possible backup models
         * @author Matthias Schartner
         */
        enum class Backup {
            minValueTimes, ///< use minimum value found in other bands times a factor
            maxValueTimes, ///< use maximum value found in other bands times a factor
            value, ///< use specific value
            none, ///< no backup model
        };

        static bool simple; ///< flag if manual observation mode was selected

        static std::set<std::string> bands; ///< list of all bands
        static std::unordered_map<std::string, double> minSNR; ///< minimum signal to noise ration per band

        static std::unordered_map<std::string, Property> stationProperty; ///< is band required or optional for station
        static std::unordered_map<std::string, Backup> stationBackup; ///< backup version for station
        static std::unordered_map<std::string, double> stationBackupValue; ///< backup value for station

        static std::unordered_map<std::string, Property> sourceProperty; ///< is band required or optional for source
        static std::unordered_map<std::string, Backup> sourceBackup; ///< backup version for source
        static std::unordered_map<std::string, double> sourceBackupValue; ///< backup value for source


        Mode(std::string name, unsigned long nsta);

        void setStationNames(const std::vector<std::string> &names){
            stationNames_ = names;
        }

        void readFromSkedCatalogs(const SkdCatalogReader &skd);

        void addIf(const If &newIf, const std::vector<unsigned long> &staids);

        void addBbc(const Bbc &newBbc, const std::vector<unsigned long> &staids);

        void addFreq(const Freq &newFreq, const std::vector<unsigned long> &staids);

        void addTrack(const Track &newTrack, const std::vector<unsigned long> &staids);

        void addTrackFrameFormat(const std::string &newTrackFrameFormat, const std::vector<unsigned long> &staids);

        void calcRecordingRates();

        void calcMeanWavelength();

        double getWavelength(const std::string &band) const {
            return band2meanWavelength_.at(band);
        }

        double recordingRate(unsigned long staid1, unsigned long staid2, const std::string &band) const;

        std::set<std::string> getAllBands() const;

        boost::optional<const If &>getIf(unsigned long staid);

        boost::optional<const Bbc &>getBbc(unsigned long staid);

        boost::optional<const Freq &>getFreq(unsigned long staid);

        boost::optional<const Track &>getTracks(unsigned long staid);

        boost::optional<const std::string &>getTrackFrameFormat(unsigned long staid);



        boost::optional<const std::vector<unsigned long> &>getAllStationsWithIf(std::string name);

        boost::optional<const std::vector<unsigned long> &>getAllStationsWithBbc(std::string name);

        boost::optional<const std::vector<unsigned long> &>getAllStationsWithFreq(std::string name);

        boost::optional<const std::vector<unsigned long> &>getAllStationsWithTrack(std::string name);

        boost::optional<const std::vector<unsigned long> &>getAllStationsWithTrackFrameFormat(std::string name);

        void summary( std::ofstream &of) const;

        void toVexModeBlock(std::ofstream &of) const;

        void toVexFreqBlock(std::ofstream &of) const;

        void toVexBbcBlock(std::ofstream &of) const;

        void toVexIfBlock(std::ofstream &of) const;

        void toVexTracksBlock(std::ofstream &of) const;


    private:
        static unsigned long nextId;

        unsigned long nsta_;

        std::vector<std::string> stationNames_;

        std::vector<std::pair<If, std::vector<unsigned long>>> ifs_;
        std::vector<std::pair<Bbc, std::vector<unsigned long>>> bbcs_;
        std::vector<std::pair<Freq, std::vector<unsigned long>>> freqs_;
        std::vector<std::pair<Track, std::vector<unsigned long>>> tracks_;
        std::vector<std::pair<std::string, std::vector<unsigned long>>> track_frame_formats_;

        std::unordered_map< std::pair<unsigned long, unsigned long>,
                                      std::unordered_map<std::string,double>,
                                      boost::hash<std::pair<unsigned long, unsigned long>>>
                staids2recordingRate_;

        std::unordered_map<std::string, double> band2meanWavelength_;

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

        void toTrackFrameFormatDefinitions(std::ofstream &of) const;

    };

}


#endif //VIESCHEDPP_MODE_H
