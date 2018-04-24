/**
 * @file FillinmodeEndposition.h
 * @brief class FillinmodeEndposition
 *
 *
 * @author Matthias Schartner
 * @date 12.08.2017
 */
#ifndef FILLINMODEENDPOSITION_H
#define FILLINMODEENDPOSITION_H

#include <vector>
#include <limits>
#include "Station.h"

namespace VieVS {
    /**
     * @class FillinmodeEndposition
     * @brief Information about which station can be used for fillin mode and how much time is available
     *
     * The available time is the time between the end of the slew time to the next source and the start of the scan of
     * the next scan. If a station is not participating in all following next scans the available time is the time
     * between the end of the last scan the station was participating and the earliest start of the next scan.
     *
     * @author Matthias Schartner
     * @date 12.08.2017
     */
    class FillinmodeEndposition: public VieVS_Object {
    public:

        enum class change{
            start,
            end
        };

//        /**
//         * @brief constructor
//         *
//         * @param bestScans list of all next scheduled scans
//         * @param stations list of all stations
//         */
//        FillinmodeEndposition(const std::vector<Scan> &bestScans, const std::vector<Station> &stations);

        explicit FillinmodeEndposition(int nsta);

        void addPointingVectorAsEndposition(const PointingVector &pv);

        void checkStationPossibility(const Station &station);

        bool checkStationPossibility(const std::vector<Station> &station);

        bool getStationPossible(int staid) const noexcept{
            bool possible = stationPossible_[staid];
            return possible;
        }

        bool hasEndposition(int staid) const noexcept{
            return finalPosition_[staid].is_initialized();
        }

        unsigned int requiredEndpositionTime(int staid) const;

        /**
         * @brief getter for all desired positions at the end of the fillin scans.
         *
         * This end positions are the start positions of the following scan. If a station is not participating in the
         * following scans a default pointing vector is used. This information is than also stored in the
         * getStationUnused() function.
         *
         * @return vector of desired end positions per station
         */
        const std::vector<boost::optional<PointingVector>> &getFinalPosition() const noexcept {
            return finalPosition_;
        }

        const boost::optional<PointingVector> &getFinalPosition(int staid) const noexcept {
            return finalPosition_[staid];
        }

        bool getStationAvailable(int staid) const noexcept {
            return stationAvailable_[staid];
        }

        bool everyStationInitialized() const noexcept{
            return std::all_of(finalPosition_.begin(), finalPosition_.end(),
                               [](const boost::optional<PointingVector> &p){ return p.is_initialized(); });
        }


    private:
        static int nextId;

        std::vector<char> stationAvailable_; ///< saves available state of station
        std::vector<char> stationPossible_; ///< true if it is possible to use this station
        std::vector<boost::optional<PointingVector> > finalPosition_; ///< final required end position for all stations
        unsigned int earliestScanStart_;
    };
}


#endif //FILLINMODEENDPOSITION_H
