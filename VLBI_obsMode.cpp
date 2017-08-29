//
// Created by mschartn on 21.08.17.
//

#include "VLBI_obsMode.h"

unsigned int VieVS::VLBI_obsMode::bandwith = 0;
unsigned int VieVS::VLBI_obsMode::sampleRate = 0;
unsigned int VieVS::VLBI_obsMode::fanout = 0;
unsigned int VieVS::VLBI_obsMode::bits = 0;
std::unordered_map<std::string,unsigned int> VieVS::VLBI_obsMode::num_channels {};
std::unordered_map<std::string,double> VieVS::VLBI_obsMode::wavelength {};
std::unordered_map<std::string,VieVS::VLBI_obsMode::PROPERTY> VieVS::VLBI_obsMode::property {};

