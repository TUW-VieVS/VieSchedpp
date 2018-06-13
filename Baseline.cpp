/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   baseline.cpp
 * Author: mschartn
 * 
 * Created on June 29, 2017, 3:25 PM
 */

#include "Baseline.h"
using namespace std;
using namespace VieVS;

thread_local VieVS::Baseline::ParameterStorage VieVS::Baseline::PARA;
std::vector<std::vector<std::vector<VieVS::Baseline::Event> > >  VieVS::Baseline::EVENTS;
std::vector<std::vector<unsigned int> >  VieVS::Baseline::nextEvent;
unsigned long Baseline::nextId = 0;


Baseline::Baseline(unsigned long srcid, unsigned long staid1, unsigned long staid2, unsigned int startTime): VieVS_Object(nextId++),
        srcid_(srcid), staid1_(staid1), staid2_(staid2), startTime_{startTime}{
}

void
Baseline::checkForNewEvent(unsigned int time, bool &hardBreak, bool output, std::ofstream &bodyLog) noexcept {
    unsigned long nsta = Baseline::nextEvent.size();
    for (unsigned long staid1 = 0; staid1 < nsta; ++staid1) {
        for (unsigned long staid2 = staid1 + 1; staid2 < nsta; ++staid2) {

            unsigned int thisNextEvent = Baseline::nextEvent.at(staid1).at(staid2);

            while (thisNextEvent < EVENTS[staid1][staid2].size() && EVENTS[staid1][staid2][thisNextEvent].time <= time) {

                Baseline::Parameters newPARA = EVENTS[staid1][staid2][thisNextEvent].PARA;
                hardBreak = hardBreak || !EVENTS[staid1][staid2][thisNextEvent].softTransition;

                Baseline::PARA.ignore[staid1][staid2] = newPARA.ignore;
                Baseline::PARA.maxScan[staid1][staid2] = newPARA.maxScan;
                Baseline::PARA.minScan[staid1][staid2] = newPARA.minScan;
                Baseline::PARA.weight[staid1][staid2] = newPARA.weight;
                for (const auto &any:newPARA.minSNR) {
                    Baseline::PARA.minSNR.at(any.first).at(staid1).at(staid2) = any.second;
                }
                if (output && time < TimeSystem::duration) {
                    bodyLog << "###############################################\n";
                    bodyLog << "## changing parameters for baseline: " << boost::format("%2d") % staid1 << "-"
                              << boost::format("%2d") % staid2 << "   ##\n";
                    bodyLog << "###############################################\n";
                }
                ++nextEvent[staid1][staid2];
                ++thisNextEvent;
            }
        }
    }
}

void Baseline::displaySummaryOfStaticMembersForDebugging(ofstream &log) {
    unsigned long nsta = PARA.ignore.size();
    log << "############################### BASELINES ###############################\n";
    for (int i = 0; i < nsta; ++i) {
        for (int j = i + 1; j < nsta; ++j) {
            log << "baseline " << i << "-" << j << ":\n";
            if (PARA.ignore[i][j]) {
                log << "    ignore: " << "TRUE\n";
            } else {
                log << "    ignore: " << "FALSE\n";
            }

            log << "    minScan: " << PARA.minScan[i][j] << "\n";
            log << "    maxScan: " << PARA.maxScan[i][j] << "\n";

            for (const auto &it:PARA.minSNR) {
                log << "    minSNR: " << it.first << " " << it.second[i][j] << "\n";
            }

        }
    }

}
