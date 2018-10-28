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

#ifndef VIESCHEDPP_OBSMODES_H
#define VIESCHEDPP_OBSMODES_H

#include "Mode.h"

/**
* @file ObsModes.h
* @brief class observing modes
*
* @author Matthias Schartner
* @date 27.10.2018
*/
namespace VieVS{
    /**
     * @class Mode
     * @brief observing modes
     *
     * CURRENTLY UNDER DEVELOPMENT
     *
     * @author Matthias Schartner
     * @date 27.10.2018
     */
    class ObsModes : public VieVS_Object {
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
        static std::set<std::string> bands;

        static std::unordered_map<std::string, double> minSNR; ///< minimum signal to noise ration per band

        static std::unordered_map<std::string, Property> stationProperty; ///< is band required or optional for station
        static std::unordered_map<std::string, Backup> stationBackup; ///< backup version for station
        static std::unordered_map<std::string, double> stationBackupValue; ///< backup value for station

        static std::unordered_map<std::string, Property> sourceProperty; ///< is band required or optional for source
        static std::unordered_map<std::string, Backup> sourceBackup; ///< backup version for source
        static std::unordered_map<std::string, double> sourceBackupValue; ///< backup value for source

        ObsModes();

        void setStationNames(const std::vector<std::string> &names){
            stationNames_ = names;
        }

        void readFromSkedCatalogs(const SkdCatalogReader &skd);


        void addMode(const std::shared_ptr<const Mode> &newMode){
            modes_.push_back(newMode);
        }

        void addIf(const std::shared_ptr<const If> &newIf){
            ifs_.push_back(newIf);
        }

        void addBbc(const std::shared_ptr<const Bbc> &newBbc){
            bbcs_.push_back(newBbc);
        }

        void addFreq(const std::shared_ptr<const Freq> &newFreq){
            freqs_.push_back(newFreq);
            const auto &tmp = newFreq->getBands();
            ObsModes::bands.insert(tmp.begin(), tmp.end());
        }

        void addTrack(const std::shared_ptr<const Track> &newTrack){
            tracks_.push_back(newTrack);
        }

        void addTrackFrameFormat(const std::string &newTrackFrameFormat){
            trackFrameFormats_.emplace_back(newTrackFrameFormat);
        }

        const std::shared_ptr<const Mode> &getMode(unsigned long id) const{
            return modes_.at(id);
        }

        void toVexModeBlock(std::ofstream &of) const;

        void toVexFreqBlock(std::ofstream &of) const;

        void toVexBbcBlock(std::ofstream &of) const;

        void toVexIfBlock(std::ofstream &of) const;

        void toVexTracksBlock(std::ofstream &of) const;

        void summary(std::ofstream &of) const;

    private:
        static unsigned long nextId;
        std::vector<std::string> stationNames_;

        std::vector<std::shared_ptr<const Mode>> modes_;
        std::vector<std::shared_ptr<const If>> ifs_;
        std::vector<std::shared_ptr<const Bbc>> bbcs_;
        std::vector<std::shared_ptr<const Freq>> freqs_;
        std::vector<std::shared_ptr<const Track>> tracks_;
        std::vector<std::shared_ptr<const std::string>> trackFrameFormats_;



        /**
         * @brief create FREQ block from skd catalogs
         * @author Matthias Schartner
         *
         * @param skd skd catalogs
         */
        void readSkdFreq(const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd, const std::map<int,int> &channelNr2Bbc);

        /**
         * @brief create TRACKS block from skd catalogs
         * @author Matthias Schartner
         *
         * @param skd skd catalogs
         * @return channel number to bbc number map
         */
        std::map<int,int> readSkdTracks(const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd);

        /**
         * @brief create IF block from skd catalogs
         * @author Matthias Schartner
         *
         * @param skd skd catalogs
         */
        void readSkdIf(const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd);

        /**
         * @brief create BBC block from skd catalogs
         * @author Matthias Schartner
         *
         * @param skd skd catalogs
         */
        void readSkdBbc(const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd);


        /**
         * @brief create track frame format from skd catalogs
         * @author Matthias Schartner
         *
         * @param skd skd catalogs
         */
        void readSkdTrackFrameFormat(const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd);

        void toTrackFrameFormatDefinitions(std::ofstream &of) const;

    };
}


#endif //VIESCHEDPP_OBSMODES_H
