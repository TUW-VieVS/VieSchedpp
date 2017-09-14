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

        /**
         * @brief summary of all weight factors
         *
         * @param of out stream object
         */
        static void summary(std::ofstream &of) {
            of << "############################### WEIGHT FACTORS ###############################\n";
            of << "skyCoverage:          " << weightSkyCoverage << "\n";
            of << "numberOfObservations: " << weightNumberOfObservations << "\n";
            of << "duration:             " << weightDuration << "\n";
            of << "averageSources:       " << weightAverageSources << "\n";
            of << "averageStations:      " << weightAverageStations << "\n";
        }
    };
}
#endif //WEIGHTFACTORS_H
