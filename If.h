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
* @file If.h
* @brief class If
*
* @author Matthias Schartner
* @date 17.09.2018
*/

#ifndef VIESCHEDPP_IF_H
#define VIESCHEDPP_IF_H

#include <utility>
#include <vector>
#include <fstream>
#include <boost/format.hpp>
#include "VieVS_NamedObject.h"

namespace VieVS{

    /**
     * @class If
     * @brief if section of observing mode
     *
     * CURRENTLY UNDER DEVELOPMENT AND UNUSED
     *
     * @author Matthias Schartner
     * @date 17.09.2018
     */
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

        std::string toString(Polarization p) const {
            switch(p){
                case Polarization::R: return "R";
                case Polarization::L: return "L";
                case Polarization::X: return "X";
                case Polarization::Y: return "Y";
                case Polarization::H: return "H";
                case Polarization::V: return "V";
            }
        }

        std::string toString(Net_sidband n) const {
            switch(n){
                case Net_sidband::U: return "U";
                case Net_sidband::L: return "L";
                case Net_sidband::D: return "D";
            }
        }

        explicit If(std::string name);

        void addIf(std::string name, std::string physical_name, Polarization polarization,  double total_lo,
                   Net_sidband net_sidband, double phase_cal_freq_spacing, double phase_cal_base_freqency);

        void toVecIfDefinition( std::ofstream &of) const;

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

            std::string physical_name_;
            Polarization polarization_;
            double total_lo_;
            Net_sidband net_sidband_;
            double phase_cal_freq_spacing_;
            double phase_cal_base_frequency_;

        private:
            static unsigned long nextId;

        };


        std::vector<If_def> if_defs_;
    };
}


#endif //VIESCHEDPP_IF_H
