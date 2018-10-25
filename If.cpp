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


#include "If.h"

using namespace VieVS;
using namespace std;

unsigned long VieVS::If::nextId = 0;
unsigned long VieVS::If::If_def::nextId = 0;

If::If(std::string name): VieVS_NamedObject{std::move(name), nextId++} {

}

void If::addIf(std::string name, std::string physical_name, If::Polarization polarization, double total_lo,
               If::Net_sidband net_sidband, double phase_cal_freq_spacing, double phase_cal_base_freqency) {

    if_defs_.emplace_back(name, physical_name, polarization, total_lo, net_sidband, phase_cal_freq_spacing,
                          phase_cal_freq_spacing);

}

void If::toVecIfDefinition(std::ofstream &of) const {

    of << "    def " << getName() << ";\n";
    of << "*                  IF   Physical Pol    Total      Net     Phase-cal   P-cal base  \n"
          "*                  ID     Name            IO        SB   freq spacing     freq\n";
    for (const auto &any : if_defs_){
        of << boost::format("        if_def = %6s :   %2s : %2s : %7.2f MHz : %2s : %7.2f MHz : % 7.2f Hz;\n") % any.getName()
        % any.physical_name_ % toString(any.polarization_) % any.total_lo_ % toString(any.net_sidband_) % any.phase_cal_freq_spacing_
        % any.phase_cal_base_frequency_;
    }

    of << "    enddef;\n";

}
