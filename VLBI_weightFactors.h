/**
 * @file VLBI_weightFactors.h
 * @brief class VLBI_weightFactors
 *
 *
 * @author Matthias Schartner
 * @date 14.08.2017
 */


#ifndef VLBI_WEIGHTFACTORS_H
#define VLBI_WEIGHTFACTORS_H

namespace VieVS{
    class VLBI_weightFactors {
    public:
        static double weight_skyCoverage; ///< weight factor for sky Coverage
        static double weight_numberOfObservations; ///< weight factor for number of observations
        static double weight_duration; ///< weight factor for duration
        static double weight_averageSources; ///< weight factor for average out sources
        static double weight_averageStations; ///< weight factor for average out stations
    };
}
#endif //VLBI_WEIGHTFACTORS_H
