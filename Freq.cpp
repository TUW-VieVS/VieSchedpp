//
// Created by mschartn on 10.09.18.
//

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
