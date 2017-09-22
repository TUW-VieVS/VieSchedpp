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
        static thread_local double declinationSlopeStart; ///< start declination of additional weight (everything above has factor 0)
        static thread_local double declinationSlopeEnd; ///< end declination of additional declination weight slope (everything below has factor 1)

        static thread_local double weightLowElevation; ///< weight factor for low elevation scans
        static thread_local double lowElevationSlopeStart; ///< start elevation of additional weight (everything above has factor 0)
        static thread_local double lowElevationSlopeEnd; ///< end elevation of additional declination weight slope (everything below has factor 1)
        /**
         * @brief summary of all weight factors
         *
         * @param of out stream object
         */
        static void summary(std::ofstream &of) {
            double rad2deg = 180 / 3.141592653589793;
            of << "############################### WEIGHT FACTORS ###############################\n";
            of << "skyCoverage:          " << weightSkyCoverage << "\n";
            of << "numberOfObservations: " << weightNumberOfObservations << "\n";
            of << "duration:             " << weightDuration << "\n";
            of << "averageSources:       " << weightAverageSources << "\n";
            of << "averageStations:      " << weightAverageStations << "\n";
            of << "declination           " << weightDeclination << "from 90 to " << declinationSlopeStart * rad2deg
               << " = 0; from "
               << declinationSlopeStart * rad2deg << " to " << declinationSlopeEnd * rad2deg << " = linear; from "
               << declinationSlopeEnd * rad2deg << " to -90" << " = 1;\n";
            of << "elevation             " << weightLowElevation << "from 90 to " << lowElevationSlopeStart * rad2deg
               << " = 0; from "
               << lowElevationSlopeStart * rad2deg << " to " << lowElevationSlopeEnd * rad2deg << " = linear; from "
               << lowElevationSlopeEnd * rad2deg << " to -90" << " = 1;\n";

        }
    };
}
#endif //WEIGHTFACTORS_H
