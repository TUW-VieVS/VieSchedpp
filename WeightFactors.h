/**
 * @file WeightFactors.h
 * @brief class WeightFactors
 *
 *
 * @author Matthias Schartner
 * @date 14.08.2017
 */

#ifndef WEIGHTFACTORS_H
#define WEIGHTFACTORS_H

#include <fstream>
#include "Constants.h"

namespace VieVS{
    /**
     * @class WeightFactors
     * @brief storage of all VLBI weight factors
     *
     * @author Matthias Schartner
     * @date 14.08.2017
     */
    class WeightFactors {
    public:
        static thread_local double weightSkyCoverage; ///< weight factor for sky Coverage
        static thread_local double weightNumberOfObservations; ///< weight factor for number of observations
        static thread_local double weightDuration; ///< weight factor for duration
        static thread_local double weightAverageSources; ///< weight factor for average out sources
        static thread_local double weightAverageStations; ///< weight factor for average out stations

        static thread_local double weightDeclination; ///< weight factor for declination
        static thread_local double declinationStartWeight; ///< start declination of additional weight (everything above has factor 0)
        static thread_local double declinationFullWeight; ///< end declination of additional declination weight slope (everything below has factor 1)

        static thread_local double weightLowElevation; ///< weight factor for low elevation scans
        static thread_local double lowElevationStartWeight; ///< start elevation of additional weight (everything above has factor 0)
        static thread_local double lowElevationFullWeight; ///< end elevation of additional declination weight slope (everything below has factor 1)
        /**
         * @brief summary of all weight factors
         *
         * @param of out stream object
         */
        static void summary(std::ofstream &of) {
            of << "weight factors: \n";
            of << "    skyCoverage:          " << weightSkyCoverage << "\n";
            of << "    numberOfObservations: " << weightNumberOfObservations << "\n";
            of << "    duration:             " << weightDuration << "\n";
            of << "    averageSources:       " << weightAverageSources << "\n";
            of << "    averageStations:      " << weightAverageStations << "\n";
            if(weightDeclination != 0){
                of << "    declination           " << weightDeclination << " from 90 to " << declinationStartWeight * rad2deg
                   << " = 0; from "
                   << declinationStartWeight * rad2deg << " to " << declinationFullWeight * rad2deg << " = linear; from "
                   << declinationFullWeight * rad2deg << " to -90" << " = 1;\n";
            }
            if(weightLowElevation != 0){
                of << "    elevation             " << weightLowElevation << " from 90 to " << lowElevationStartWeight * rad2deg
                   << " = 0; from "
                   << lowElevationStartWeight * rad2deg << " to " << lowElevationFullWeight * rad2deg << " = linear; from "
                   << lowElevationFullWeight * rad2deg << " to -90" << " = 1;\n";
            }
            of << "\n";
        }
    };
}
#endif //WEIGHTFACTORS_H
