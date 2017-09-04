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
        static thread_local double weight_skyCoverage; ///< weight factor for sky Coverage
        static thread_local double weight_numberOfObservations; ///< weight factor for number of observations
        static thread_local double weight_duration; ///< weight factor for duration
        static thread_local double weight_averageSources; ///< weight factor for average out sources
        static thread_local double weight_averageStations; ///< weight factor for average out stations
    };
}
#endif //VLBI_WEIGHTFACTORS_H
