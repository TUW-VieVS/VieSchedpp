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
 * @file ObservationMode.h
 * @brief class ObservationMode
 *
 * @author Matthias Schartner
 * @date 21.08.2017
 */

#ifndef OBSERVATIONMODE_H
#define OBSERVATIONMODE_H

#include <vector>
#include <unordered_map>

namespace VieVS{
    /**
     * @class ObservationMode
     * @brief representation of VLBI observation mode
     *
     * @author Matthias Schartner
     * @date 21.08.2017
     */
    class ObservationMode {
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

        static double sampleRate; ///< sample rate of one channel
        static unsigned int bits; ///< number of recorded bits
        static bool manual; ///< flag if manual observation mode was selected

        static std::vector<std::string> bands; ///< list of all bands
        static std::unordered_map<std::string, double> minSNR; ///< minimum signal to noise ration per band

        static std::unordered_map<std::string, unsigned int> nChannels; ///< number of channels per band
        static std::unordered_map<std::string, double> wavelength; ///< wavelength per band

        static std::unordered_map<std::string, Property> stationProperty; ///< is band required or optional for station
        static std::unordered_map<std::string, Backup> stationBackup; ///< backup version for station
        static std::unordered_map<std::string, double> stationBackupValue; ///< backup value for station

        static std::unordered_map<std::string, Property> sourceProperty; ///< is band required or optional for source
        static std::unordered_map<std::string, Backup> sourceBackup; ///< backup version for source
        static std::unordered_map<std::string, double> sourceBackupValue; ///< backup value for source

    };
}


#endif //OBSERVATIONMODE_H
