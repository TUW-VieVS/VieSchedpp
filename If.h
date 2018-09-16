#include <utility>

//
// Created by mschartn on 10.09.18.
//

#ifndef VIESCHEDPP_IF_H
#define VIESCHEDPP_IF_H

#include <utility>
#include <vector>
#include "VieVS_NamedObject.h"

namespace VieVS{
    class If: public VieVS_NamedObject {
    public:

        enum class Polarization{
            R,
            L,
            X,
            Y,
            H,
            V,
        };

        enum class Net_sidband{
            U,
            L,
            D,
        };

        explicit If(std::string name);

        void addIf(std::string name, std::string physical_name, Polarization polarization,  double total_lo,
                   Net_sidband net_sidband, double phase_cal_freq_spacing, double phase_cal_base_freqency);

    private:
        static unsigned long nextId;


        class If_def: public VieVS_NamedObject{
        public:
            If_def(std::string name,
                   std::string physical_name,
                   Polarization polarization,
                   double total_lo,
                   Net_sidband net_sidband,
                   double phase_cal_freq_spacing,
                   double phase_cal_base_freqency):
               VieVS_NamedObject{std::move(name), nextId++},
               physical_name_{std::move(physical_name)},
               polarization_{polarization},
               total_lo_{total_lo},
               net_sidband_{net_sidband},
               phase_cal_base_frequency_{phase_cal_base_freqency},
               phase_cal_freq_spacing_{phase_cal_freq_spacing}{};

        private:
            static unsigned long nextId;

            std::string physical_name_;
            Polarization polarization_;
            double total_lo_;
            Net_sidband net_sidband_;
            double phase_cal_freq_spacing_;
            double phase_cal_base_frequency_;
        };


        std::vector<If_def> if_defs_;
    };
}


#endif //VIESCHEDPP_IF_H
