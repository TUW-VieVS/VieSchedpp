#include <utility>

//
// Created by mschartn on 10.09.18.
//

#ifndef VIESCHEDPP_FREQ_H
#define VIESCHEDPP_FREQ_H

#include <utility>
#include <vector>
#include "VieVS_NamedObject.h"

namespace VieVS{
    class Freq: public VieVS_NamedObject {
    public:

        enum class Net_sideband{
            U,
            L,
            UC,
            LC,
        };

        explicit Freq(std::string name);

        void addChannel(std::string bandId, double sky_freq, Net_sideband net_sideband, double chan_bandwidth,
                        std::string chan_id, std::string bbc_id, std::string phase_cal_id);

        void setSampleRate(double sample_rate){
            sample_rate_ = sample_rate;
        }



    private:
        static unsigned long nextId;


        class Chan_def: public VieVS_NamedObject{
        public:
            Chan_def(std::string bandId,
                     double sky_freq,
                     Net_sideband net_sideband,
                     double chan_bandwidth,
                     std::string chan_id,
                     std::string bbc_id,
                     std::string phase_cal_id):
                VieVS_NamedObject{std::move(bandId), nextId++},
                sky_freq_{sky_freq},
                net_sideband_{net_sideband},
                chan_bandwidth_{chan_bandwidth},
                chan_id_{std::move(chan_id)},
                bbc_id_{std::move(bbc_id)},
                phase_cal_id_{std::move(phase_cal_id)}{};

        private:
            static unsigned long nextId;

            double sky_freq_;
            Net_sideband net_sideband_;
            double chan_bandwidth_;
            std::string chan_id_;
            std::string bbc_id_;
            std::string phase_cal_id_;
        };

        double sample_rate_;
        std::vector<Chan_def> chan_defs_;
    };
}


#endif //VIESCHEDPP_FREQ_H
