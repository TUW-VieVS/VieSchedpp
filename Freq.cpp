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


#include "Freq.h"

using namespace VieVS;
using namespace std;

unsigned long VieVS::Freq::nextId = 0;
unsigned long VieVS::Freq::Chan_def::nextId = 0;

Freq::Freq(std::string name): VieVS_NamedObject{std::move(name), nextId++} {

}

void Freq::addChannel(std::string bandId, double sky_freq, Freq::Net_sideband net_sideband, double chan_bandwidth,
                      std::string chan_id, std::string bbc_id, std::string phase_cal_id) {

    chan_defs_.emplace_back(bandId, sky_freq, net_sideband, chan_bandwidth, chan_id, bbc_id, phase_cal_id);

}
