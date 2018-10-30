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

        Mode(std::string name, unsigned long nsta);

        void addIf(const std::shared_ptr<const If> &newIf, const std::vector<unsigned long> &staids){
            ifs_.emplace_back(newIf, staids);
        }

        void addBbc(const std::shared_ptr<const Bbc> &newBbc, const std::vector<unsigned long> &staids){
            bbcs_.emplace_back(newBbc, staids);
        }

        void addFreq(const std::shared_ptr<const Freq> &newFreq, const std::vector<unsigned long> &staids){
            freqs_.emplace_back(newFreq, staids);
            const auto &tmp = newFreq->getBands();
            bands_.insert(tmp.begin(), tmp.end());
        }

        void addTrack(const std::shared_ptr<const Track> &newTrack, const std::vector<unsigned long> &staids){
            tracks_.emplace_back(newTrack, staids);
        }

        void addTrackFrameFormat(const std::shared_ptr<const std::string> &newTrackFrameFormat, const std::vector<unsigned long> &staids){
            track_frame_formats_.emplace_back(newTrackFrameFormat, staids);
        }

        void calcRecordingRates();

        void calcRecordingRates(const std::string &band, double recRate);

        void setBands(const std::set<std::string> &bands){
            bands_ = bands;
        }

        double recordingRate(unsigned long staid1, unsigned long staid2, const std::string &band) const;

        boost::optional<const std::shared_ptr<const If> &>getIf(unsigned long staid) const;

        boost::optional<const std::shared_ptr<const Bbc> &>getBbc(unsigned long staid) const;

        boost::optional<const std::shared_ptr<const Freq> &>getFreq(unsigned long staid) const;

        boost::optional<const std::shared_ptr<const Track> &>getTracks(unsigned long staid) const;

        boost::optional<const std::shared_ptr<const std::string> &> getTrackFrameFormat(unsigned long staid) const;


        boost::optional<const std::vector<unsigned long> &>getAllStationsWithIf(const std::shared_ptr<const If> &this_if)const ;

        boost::optional<const std::vector<unsigned long> &>getAllStationsWithBbc(const std::shared_ptr<const Bbc> &bbc)const ;

        boost::optional<const std::vector<unsigned long> &>getAllStationsWithFreq(const std::shared_ptr<const Freq> &freq)const ;

        boost::optional<const std::vector<unsigned long> &>getAllStationsWithTrack(const std::shared_ptr<const Track> &track)const ;

        boost::optional<const std::vector<unsigned long> &>getAllStationsWithTrackFrameFormat(const std::shared_ptr<const std::string> &trackFrameFormat)const ;

        void summary( std::ofstream &of, const std::vector<std::string> &stations) const;

        void toVexModeDefiniton(std::ofstream &of, const std::vector<std::string> &stations) const;

        const std::set<std::string> getAllBands() const{
            return bands_;
        }

    private:
        static unsigned long nextId;

        unsigned long nsta_;


        std::vector<std::pair<std::shared_ptr<const If>, std::vector<unsigned long>>> ifs_;
        std::vector<std::pair<std::shared_ptr<const Bbc>, std::vector<unsigned long>>> bbcs_;
        std::vector<std::pair<std::shared_ptr<const Freq>, std::vector<unsigned long>>> freqs_;
        std::vector<std::pair<std::shared_ptr<const Track>, std::vector<unsigned long>>> tracks_;
        std::vector<std::pair<std::shared_ptr<const std::string>, std::vector<unsigned long>>> track_frame_formats_;

        std::unordered_map< std::pair<unsigned long, unsigned long>,
                                      std::unordered_map<std::string,double>,
                                      boost::hash<std::pair<unsigned long, unsigned long>>> staids2recordingRate_;


        std::set<std::string> bands_;

    };

}


#endif //VIESCHEDPP_MODE_H
