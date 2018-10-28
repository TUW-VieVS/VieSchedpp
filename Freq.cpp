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

    bands_.insert(bandId);
    chan_defs_.emplace_back(bandId, sky_freq, net_sideband, chan_bandwidth, chan_id, bbc_id, phase_cal_id);

}

std::unordered_map<std::string, double> Freq::observingRate(const std::shared_ptr<const Freq> &other, int bits) const {

    unordered_map<string, double> band2observingRate;
    for(const auto &band : bands_){
        band2observingRate[band] = 0;
    }

    if(other->hasName(getName())){
        for(const auto &channel : chan_defs_){
            band2observingRate[channel.bandId_] +=  bits * sample_rate_ * 1e6;
        }
    }else if(sample_rate_ == other->sample_rate_){
        for(const auto &channelA : chan_defs_){
            for(const auto &channelB : other->chan_defs_){
                if(channelA.bandId_ == channelB.bandId_){

                    auto lower_upper_A = lower_upper_bound(channelA.sky_freq_, channelA.chan_bandwidth_, channelA.net_sideband_);
                    auto lower_upper_B = lower_upper_bound(channelB.sky_freq_, channelB.chan_bandwidth_, channelB.net_sideband_);

                    double overlapping = overlappingBandwidth(lower_upper_A.first, lower_upper_A.second, lower_upper_B.first, lower_upper_B.second);
                    band2observingRate[channelA.bandId_] += overlapping * bits * sample_rate_ * 1e6;
                }
            }
        }
    }

    return band2observingRate;
}

std::pair<double, double> Freq::lower_upper_bound(double skyFreq, double bandwidth, Freq::Net_sideband net_sideband) const {

    double lower = 0;
    double upper = 0;
    switch(net_sideband){
        case Net_sideband::U:{
            lower = skyFreq;
            upper = skyFreq + bandwidth;
            break;
        }
        case Net_sideband::L:{
            lower = skyFreq - bandwidth;
            upper = skyFreq;
            break;
        }
        case Net_sideband::UC:{
            lower = skyFreq - bandwidth/2;
            upper = skyFreq + bandwidth/2;
            break;
        }
        case Net_sideband::LC:{
            lower = skyFreq - bandwidth/2;
            upper = skyFreq + bandwidth/2;
            break;
        }
    }

    return {lower, upper};
}

double Freq::overlappingBandwidth(double low1, double up1, double low2, double up2) const {
    // no overlap
    if(up1 <= low2 || up2 <= low1){
        return 0;
    }
    // inside
    if(low1 >= low2 && up1 <= up2){
        return up1-low1;
    }
    if(low2 >= low1 && up2 <= up1){
        return up2-low2;
    }
    // overlap lower side
    if(low1 <= low2 && up1 <= up2){
        return up1-low2;
    }
    if(low2 <= low1 && up2 <= up1){
        return up2-low1;
    }
    // overlap upper side
    if(low1 >= low2 && up1 >= up2){
        return up2-low1;
    }
    if(low2 >= low1 && up2 >= up1){
        return up1-low2;
    }

    return 0;
}

std::vector<double> Freq::getFrequencies(const string &band) const{
    vector<double> freq;
    for(const auto &channel : chan_defs_){
        if(channel.bandId_ == band){
            freq.push_back(channel.sky_freq_);
        }
    }
    return freq;
}

void Freq::toVexFreqDefinition(std::ofstream &of, const std::string &comment) const{

    of << "    def " << getName() << ";    " << comment <<"\n";
    of << "*                 Band    Sky freq    Net    Chan       Chan     BBC   Phase-cal\n"
          "*                  Id    at 0Hz BBC    SB     BW         ID       ID       ID\n";
    for (const auto &any : chan_defs_) {
        of << boost::format("        chan_def = &%1s : %8s MHz : %1s : %6.3f MHz : %5s : %6s : %6s;\n") %
                            any.bandId_ % any.sky_freq_ % toString(any.net_sideband_) % any.chan_bandwidth_ % any.chan_id_ %
                            any.bbc_id_ % any.phase_cal_id_;
    }
    of << boost::format("        sample_rate = %.2f Ms/sec;\n") % sample_rate_;
    of << "    enddef;\n";

}
