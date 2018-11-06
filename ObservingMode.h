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
* @file ObservingMode.h
* @brief class observing mode
*
* @author Matthias Schartner
* @date 27.10.2018
*/
namespace VieVS{
    /**
     * @class ObservingMode
     * @brief observing modes
     *
     * following vex standard
     *
     * @author Matthias Schartner
     * @date 27.10.2018
     */
    class ObservingMode : public VieVS_Object {
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

        static std::unordered_map<std::string, double> minSNR; ///< minimum signal to noise ration per band

        static std::unordered_map<std::string, Property> stationProperty; ///< is band required or optional for station
        static std::unordered_map<std::string, Backup> stationBackup; ///< backup version for station
        static std::unordered_map<std::string, double> stationBackupValue; ///< backup value for station

        static std::unordered_map<std::string, Property> sourceProperty; ///< is band required or optional for source
        static std::unordered_map<std::string, Backup> sourceBackup; ///< backup version for source
        static std::unordered_map<std::string, double> sourceBackupValue; ///< backup value for source



        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         */
        ObservingMode();

        /**
         * @brief set station names
         * @author Matthias Schartner
         *
         * @param names list of station names
         */
        void setStationNames(const std::vector<std::string> &names){
            stationNames_ = names;
        }

        /**
         * @brief read observin mode from sked catalogs
         * @author Matthias Schartner
         *
         * @param skd sked catalog reader
         */
        void readFromSkedCatalogs(const SkdCatalogReader &skd);

        /**
         * @brief add observing mode from simple manual model
         * @author Matthias Schartner
         *
         * @param nsta number of stations
         * @param samplerate sample rate
         * @param bits sampling bits
         * @param band2channel band name to number of channels
         * @param band2wavelength band name to wavelength
         */
        void simpleMode( unsigned long nsta, double samplerate, unsigned int bits,
                const std::unordered_map<std::string, unsigned int> &band2channel,
                const std::unordered_map<std::string, double> &band2wavelength );

        /**
         * @brief add new observing mode
         * @author Matthias Schartner
         *
         * @param newMode new observing mode
         */
        void addMode(const std::shared_ptr<const Mode> &newMode){
            modes_.push_back(newMode);
        }

        /**
         * @brief add IF block
         * @author Matthias Schartner
         *
         * @param newIf IF block
         */
        void addIf(const std::shared_ptr<const If> &newIf){
            ifs_.push_back(newIf);
        }

        /**
         * @brief add BBC block
         * @author Matthias Schartner
         *
         * @param newBbc BBC block
         */
        void addBbc(const std::shared_ptr<const Bbc> &newBbc){
            bbcs_.push_back(newBbc);
        }

        /**
         * @brief add FREQ block
         * @author Matthias Schartner
         *
         * @param newFreq FREQ block
         */
        void addFreq(const std::shared_ptr<const Freq> &newFreq){
            freqs_.push_back(newFreq);
            const auto &tmp = newFreq->getBands();
            ObservingMode::bands_.insert(tmp.begin(), tmp.end());
        }

        /**
         * @brief add TRACKS block
         * @author Matthias Schartner
         *
         * @param newTrack TRACKS block
         */
        void addTrack(const std::shared_ptr<const Track> &newTrack){
            tracks_.push_back(newTrack);
        }

        /**
         * @brief add track frame format
         * @author Matthias Schartner
         *
         * @param newTrackFrameFormat track frame format
         */
        void addTrackFrameFormat(const std::string &newTrackFrameFormat){
            trackFrameFormats_.emplace_back(std::make_shared<const std::string>(newTrackFrameFormat));
        }

        /**
         * @brief get MODE block
         * @author Matthias Schartner
         *
         * @param id
         * @return MODE block
         */
        const std::shared_ptr<const Mode> &getMode(unsigned long id) const{
            return modes_.at(id);
        }

        /**
         * @brief write MODE section in vex format
         * @author Matthias Schartner
         *
         * @param of vex file stream
         */
        void toVexModeBlock(std::ofstream &of) const;

        /**
         * @brief write FREQ section in vex format
         * @author Matthias Schartner
         *
         * @param of vex file stream
         */
        void toVexFreqBlock(std::ofstream &of) const;

        /**
         * @brief write BBC section in vex format
         * @author Matthias Schartner
         *
         * @param of vex file stream
         */
        void toVexBbcBlock(std::ofstream &of) const;

        /**
         * @brief write IF section in vex format
         * @author Matthias Schartner
         *
         * @param of vex file stream
         */
        void toVexIfBlock(std::ofstream &of) const;

        /**
         * @brief write FREQ section in vex format
         * @author Matthias Schartner
         *
         * @param of vex file stream
         */
        void toVexTracksBlock(std::ofstream &of) const;

        /**
         * @brief write observing mode summary
         * @author Matthias Schartner
         *
         * @param of vex file stream
         */
        void summary(std::ofstream &of) const;

        /**
         * @brief calculate mean frequencies
         * @author Matthias Schartner
         *
         */
        void calcMeanFrequencies();

        /**
         * @brief get all bands
         * @author Matthias Schartner
         *
         * @return list of all bands
         */
        const std::set<std::string> &getAllBands() const{
            return bands_;
        }

        /**
         * @brief get wavelength per band
         * @author Matthias Schartner
         *
         * @param band band
         * @return wavelength
         */
        double getWavelength(const std::string &band) const{
            return wavelength_.at(band);
        }

        /**
         * @brief add bands manually
         * @author Matthias Schartner
         *
         * @param band list of bands and frequencies
         */
        void addDummyBands(const std::map<std::string, std::vector<double>> &band);

        /**
         * @brief getter for all station names
         * @author Matthias Schartner
         *
         * @return all station names
         */
        const std::vector<std::string> &getStationNames() const {
            return stationNames_;
        }

        /**
         * @brief getter for all MODE blocks
         * @author Matthias Schartner
         *
         * @return list of all MODE blocks
         */
        const std::vector<std::shared_ptr<const Mode>> &getModes() const {
            return modes_;
        }

        /**
         * @brief getter for all IF blocks
         * @author Matthias Schartner
         *
         * @return list of all IF blocks
         */
        const std::vector<std::shared_ptr<const If>> &getIfs() const {
            return ifs_;
        }

        /**
         * @brief getter for all BBC blocks
         * @author Matthias Schartner
         *
         * @return list of all BBC blocks
         */
        const std::vector<std::shared_ptr<const Bbc>> &getBbcs() const {
            return bbcs_;
        }

        /**
         * @brief getter for all FREQ blocks
         * @author Matthias Schartner
         *
         * @return list of all FREQ blocks
         */
        const std::vector<std::shared_ptr<const Freq>> &getFreqs() const {
            return freqs_;
        }

        /**
         * @brief getter for all TRACKS blocks
         * @author Matthias Schartner
         *
         * @return list of all TRACKS blocks
         */
        const std::vector<std::shared_ptr<const Track>> &getTracks() const {
            return tracks_;
        }

        /**
         * @brief getter for all track frame formats
         * @author Matthias Schartner
         *
         * @return list of all track frame formats
         */
        const std::vector<std::shared_ptr<const std::string>> &getTrackFrameFormats() const {
            return trackFrameFormats_;
        }

    private:
        static unsigned long nextId; ///< next id for this object type
        std::vector<std::string> stationNames_; ///< station names
        std::set<std::string> bands_; ///< list of all observed bands
        std::unordered_map<std::string, double> wavelength_; ///< list of mean wavelength per band


        std::vector<std::shared_ptr<const Mode>> modes_; ///< list of all MODE blocks
        std::vector<std::shared_ptr<const If>> ifs_; ///< list of all IF blocks
        std::vector<std::shared_ptr<const Bbc>> bbcs_; ///< list of all BBC blocks
        std::vector<std::shared_ptr<const Freq>> freqs_; ///< list of all FREQ blocks
        std::vector<std::shared_ptr<const Track>> tracks_; ///< list of all TRACKs blocks
        std::vector<std::shared_ptr<const std::string>> trackFrameFormats_; ///< list of all track frame formats



        /**
         * @brief create FREQ block from skd catalogs
         * @author Matthias Schartner
         *
         * @param mode MODE block
         * @param skd skd catalogs
         * @param channelNr2Bbc channel number to bbc
         */
        void readSkdFreq(const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd, const std::map<int,int> &channelNr2Bbc);

        /**
         * @brief create TRACKS block from skd catalogs
         * @author Matthias Schartner
         *
         * @param mode MODE block
         * @param skd skd catalogs
         * @return channel number to bbc number map
         */
        std::map<int,int> readSkdTracks(const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd);

        /**
         * @brief create IF block from skd catalogs
         * @author Matthias Schartner
         *
         * @param mode MODE block
         * @param skd skd catalogs
         */
        void readSkdIf(const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd);

        /**
         * @brief create BBC block from skd catalogs
         * @author Matthias Schartner
         *
         * @param mode MODE block
         * @param skd skd catalogs
         */
        void readSkdBbc(const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd);


        /**
         * @brief create track frame format from skd catalogs
         * @author Matthias Schartner
         *
         * @param mode MODE block
         * @param skd skd catalogs
         */
        void readSkdTrackFrameFormat(const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd);

        /**
         * @brief create track frame format blocks in vex format
         * @author Matthias Schartner
         *
         * @param of vex file stream
         */
        void toTrackFrameFormatDefinitions(std::ofstream &of) const;

    };
}


#endif //VIESCHEDPP_OBSMODES_H
