//
// Created by mschartn on 10.09.18.
//

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
