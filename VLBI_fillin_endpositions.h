/**
 * @file VLBI_fillin_endpositions.h
 * @brief class VLBI_fillin_endpositions
 *
 *
 * @author Matthias Schartner
 * @date 12.08.2017
 */
#ifndef VLBI_FILLIN_ENDPOSITIONS_H
#define VLBI_FILLIN_ENDPOSITIONS_H

#include <vector>
#include <limits>
#include "VLBI_scan.h"
#include "VLBI_station.h"

namespace VieVS {
    /**
     * @class VLBI_fillin_endpositions
     * @brief Information about which station can be used for fillin mode and how much time is available
     *
     * The available time is the time between the end of the slew time to the next source and the start of the scan of
     * the next scan. If a station is not participating in all following next scans the available time is the time
     * between the end of the last scan the station was participating and the earliest start of the next scan.
     *
     * @author Matthias Schartner
     * @date 12.08.2017
     */
    class VLBI_fillin_endpositions {
    public:
        /**
         * @brief empty default constructor
        */
        VLBI_fillin_endpositions();

        /**
         * @brief constructor
         *
         * @param bestScans list of all next scheduled scans
         * @param stations list of all stations
         */
        VLBI_fillin_endpositions(std::vector<VLBI_scan> &bestScans, std::vector<VLBI_station> &stations);

        /**
         * @brief getter for a vector of flags which represents if a station is used in the following next scans.
         *
         * If the flag for a station is false this station has no predetermined desired position that is requested at
         * the end of the fillin mode. If the flag for a station is true this station has to be at the pointing vector
         * stored in the finalPosition vector at this index.
         *
         * @return vector of flags
         */
        const std::vector<char> &getStationUnused() const {
            return stationUnused;
        }

        /**
         * @brief getter for a vector of flags which represents if a station is useable for fillin mode.
         *
         * Useable means the station at this index has at least enough time for calibration and for all required
         * calculation in the field system plus the minimum scan time of this station and an assumed slew time of five
         * seconds.
         *
         * @return vector of flags
         */
        const std::vector<char> &getStationPossible() const {
            return stationPossible;
        }

        /**
         * @brief getter for all desired positions at the end of the fillin scans.
         *
         * This end positions are the start positions of the following scan. If a station is not participating in the
         * following scans a default pointing vector is used. This information is than also stored in the
         * getStationUnused() function.
         *
         * @return vector of desired end positions
         */
        const vector<VLBI_pointingVector> &getFinalPosition() const {
            return finalPosition;
        }

        /**
         * @brief getter for number which can be used for fillin mode
         * @return number of possible stations
         */
        int getNumberOfPossibleStations() const {
            return numberOfPossibleStations;
        }

    private:
        std::vector<char> stationUnused; ///< true if station is unused in the following next scans (no end position)
        std::vector<char> stationPossible; ///< true if it is possible to use this station for fillin mode (with endposition)
        std::vector<VLBI_pointingVector> finalPosition; ///< final required end position of all stations
        int numberOfPossibleStations; ///< number of possible stations for fillin mode
    };
}


#endif //VLBI_FILLIN_ENDPOSITIONS_H
