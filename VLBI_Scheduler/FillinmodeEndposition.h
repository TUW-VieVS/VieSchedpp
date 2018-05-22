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
#include "Scan.h"
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
    class FillinmodeEndposition {
    public:
        /**
         * @brief empty default constructor
        */
        FillinmodeEndposition();

        /**
         * @brief constructor
         *
         * @param bestScans list of all next scheduled scans
         * @param stations list of all stations
         */
        FillinmodeEndposition(const std::vector<Scan> &bestScans, const std::vector<Station> &stations);

        /**
         * @brief getter for a vector of flags which represents if a station is used in the following next scans.
         *
         * If the flag for a station is false this station has no predetermined desired position that is requested at
         * the end of the fillin mode. If the flag for a station is true this station has to be at the pointing vector
         * stored in the finalPosition vector at this index.
         *
         * @return vector of flags which stations are unused
         */
        const std::vector<char> &getStationUnused() const noexcept {
            return stationUnused_;
        }

        /**
         * @brief getter for a vector of flags which represents if a station is useable for fillin mode.
         *
         * Usable means the station at this index has at least enough time for calibration and for all required
         * calculation in the field system plus the minimum scan time of this station and an assumed slew time of five
         * seconds.
         *
         * @return vector of flags which stations are possible for fillin mode
         */
        const std::vector<char> &getStationPossible() const noexcept {
            return stationPossible_;
        }

        /**
         * @brief getter for all desired positions at the end of the fillin scans.
         *
         * This end positions are the start positions of the following scan. If a station is not participating in the
         * following scans a default pointing vector is used. This information is than also stored in the
         * getStationUnused() function.
         *
         * @return vector of desired end positions per station
         */
        const std::vector<PointingVector> &getFinalPosition() const noexcept {
            return finalPosition_;
        }

        /**
         * @brief getter for number which can be used for fillin mode
         * @return number of possible stations
         */
        int getNumberOfPossibleStations() const noexcept {
            return numberOfPossibleStations_;
        }

    private:
        std::vector<char> stationUnused_; ///< true if station is unused in the following next scans (no end position)
        std::vector<char> stationPossible_; ///< true if it is possible to use this station for fillin mode (with endposition)
        std::vector<PointingVector> finalPosition_; ///< final required end position of all stations
        int numberOfPossibleStations_; ///< number of possible stations for fillin mode
    };
}


#endif //FILLINMODEENDPOSITION_H
