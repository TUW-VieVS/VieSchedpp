//
// Created by mschartn on 14.08.17.
//

#ifndef VIEVS_SCHEDULER_CLION_VLBI_WEIGHTFACTORS_H
#define VIEVS_SCHEDULER_CLION_VLBI_WEIGHTFACTORS_H

namespace VieVS{
    class VLBI_weightFactors {
    public:
        static double weight_skyCoverage;
        static double weight_numberOfObservations;
        static double weight_duration;
        static double weight_averageSources;
        static double weight_averageStations;
    };
}


#endif //VIEVS_SCHEDULER_CLION_VLBI_WEIGHTFACTORS_H
