#include <utility>

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
#include <unordered_map>
#include <set>
#include "VieVS_NamedObject.h"
#include "util.h"

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

        std::string toString( Net_sideband n) const{
            switch(n){
                case Net_sideband::U: return "U";
                case Net_sideband::L: return "L";
                case Net_sideband::UC: return "UC";
                case Net_sideband::LC: return "LC";
            }
        }

        explicit Freq(std::string name);

        void addChannel(std::string bandId, double sky_freq, Net_sideband net_sideband, double chan_bandwidth,
                        std::string chan_id, std::string bbc_id, std::string phase_cal_id);

        std::vector<double> getFrequencies(const std::string &band) const;

        void setSampleRate(double sample_rate){
            sample_rate_ = sample_rate;
        }

        const std::set<std::string> &getBands() const {
            return bands_;
        }

        std::unordered_map<std::string,double> observingRate(const Freq &other, int bits) const;

        void toVexFreqDefinition(std::ofstream &of) const;


    private:
        static unsigned long nextId;


        class Chan_def: public VieVS_Object{
        public:
            Chan_def(std::string bandId,
                     double sky_freq,
                     Net_sideband net_sideband,
                     double chan_bandwidth,
                     std::string chan_id,
                     std::string bbc_id,
                     std::string phase_cal_id):
                VieVS_Object{nextId++},
                bandId_{std::move(bandId)},
                sky_freq_{sky_freq},
                net_sideband_{net_sideband},
                chan_bandwidth_{chan_bandwidth},
                chan_id_{std::move(chan_id)},
                bbc_id_{std::move(bbc_id)},
                phase_cal_id_{std::move(phase_cal_id)}{

                wavelength_ = util::freqency2wavelenth(sky_freq*1e6);
            };

            std::string bandId_;
            double sky_freq_;
            double wavelength_;
            Net_sideband net_sideband_;
            double chan_bandwidth_;
            std::string chan_id_;
            std::string bbc_id_;
            std::string phase_cal_id_;

        private:
            static unsigned long nextId;
        };

        double sample_rate_;
        std::vector<Chan_def> chan_defs_;
        std::set<std::string> bands_;


        /**
         * @brief calculate lower and upper frequency limit of particular channel
         * @author Matthias Schartner
         *
         * @param skyFreq sky frequency in MHz
         * @param bandwidth bandwidth in MHz
         * @param net_sideband net sideband
         * @return lower and upper limit
         */
        std::pair<double, double> lower_upper_bound(double skyFreq, double bandwidth, Net_sideband net_sideband) const;

        /**
         * @brief calculate overlapping bandwidth
         * @author Matthias Schartner
         *
         * @param low1 lower limit of first channel in MHz
         * @param up1 upper limit of first channel in MHz
         * @param low2 lower limit of second channel in MHz
         * @param up2 upper limit of second channel in MHz
         * @return overlapping bandwidth in MHz
         */
        double overlappingBandwidth(double low1, double up1, double low2, double up2) const;

    };
}


#endif //VIESCHEDPP_FREQ_H
