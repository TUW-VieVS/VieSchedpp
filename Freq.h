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
* @file Freq.h
* @brief class Freq
*
* @author Matthias Schartner
* @date 17.09.2018
*/

#ifndef VIESCHEDPP_FREQ_H
#define VIESCHEDPP_FREQ_H

#include <utility>
#include <vector>
#include "VieVS_NamedObject.h"

namespace VieVS{

    /**
     * @class Freq
     * @brief freq section of observing mode
     *
     * CURRENTLY UNDER DEVELOPMENT AND UNUSED
     *
     * @author Matthias Schartner
     * @date 17.09.2018
     */
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
