//
// Created by mschartn on 10.09.18.
//

#ifndef VIESCHEDPP_MODE_H
#define VIESCHEDPP_MODE_H

#include <unordered_map>
#include <boost/optional.hpp>
#include <boost/functional/hash.hpp>


#include "VieVS_NamedObject.h"
#include "Freq.h"
#include "Track.h"
#include "Bbc.h"
#include "If.h"

namespace VieVS{
    class Mode: public VieVS_NamedObject {
    public:
        explicit Mode(std::string name);

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

        std::vector<std::pair<If, std::vector<unsigned long>>> ifs_;
        std::vector<std::pair<Bbc, std::vector<unsigned long>>> bbcs_;
        std::vector<std::pair<Freq, std::vector<unsigned long>>> freqs_;
        std::vector<std::pair<Track, std::vector<unsigned long>>> tracks_;
        std::vector<std::pair<std::string, std::vector<unsigned long>>> track_frame_formats_;

        std::unordered_map< std::pair<unsigned long, unsigned long>, std::unordered_map<std::string,double>, boost::hash_value<std::pair<unsigned long, unsigned long>>> staids2recordingRatemMap_;

    };

}


#endif //VIESCHEDPP_MODE_H
