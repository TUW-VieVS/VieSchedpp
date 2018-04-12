/**
 * @file ObservationMode.h
 * @brief class ObservationMode
 *
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
         */
        enum class Property {
            required,    ///< this band information is required. If this information is missing this object is not used.
            optional,    ///< this band information is only optional. If information is available it is used, otherwise it is interpolated
        };

        /**
        * @brief all possible backup models
        */
        enum class Backup {
            minValueTimes, ///< use minimum value found in other bands times a factor
            maxValueTimes, ///< use maximum value found in other bands times a factor
            value, ///< use specific value
            none, ///< no backup model
        };

        static double sampleRate; ///< sample rate of one channel
        static unsigned int bits; ///< number of recorded bits
        static bool manual;

        static std::vector<std::string> bands; ///< list of all bands
        static std::unordered_map<std::string, double> minSNR;

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
