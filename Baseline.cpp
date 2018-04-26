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
int Baseline::nextId = 0;


Baseline::Baseline(int srcid, int staid1, int staid2, unsigned int startTime): VieVS_Object(nextId++),
        srcid_(srcid), staid1_(staid1), staid2_(staid2), startTime_{startTime}{
}

void
Baseline::checkForNewEvent(unsigned int time, bool &hardBreak, bool output, std::ofstream &bodyLog) noexcept {
    unsigned long nsta = Baseline::nextEvent.size();
    for (int i = 0; i < nsta; ++i) {
        for (int j = i + 1; j < nsta; ++j) {

            unsigned int thisNextEvent = Baseline::nextEvent.at(i).at(j);

            while (thisNextEvent < EVENTS[i][j].size() && EVENTS[i][j][thisNextEvent].time <= time) {

                Baseline::Parameters newPARA = EVENTS[i][j][thisNextEvent].PARA;
                hardBreak = hardBreak || !EVENTS[i][j][thisNextEvent].softTransition;

                Baseline::PARA.ignore[i][j] = newPARA.ignore;
                Baseline::PARA.maxScan[i][j] = newPARA.maxScan;
                Baseline::PARA.minScan[i][j] = newPARA.minScan;
                Baseline::PARA.weight[i][j] = newPARA.weight;
                for (const auto &any:newPARA.minSNR) {
                    Baseline::PARA.minSNR.at(any.first).at(i).at(j) = any.second;
                }
                if (output && time < TimeSystem::duration) {
                    bodyLog << "###############################################\n";
                    bodyLog << "## changing parameters for baseline: " << boost::format("%2d") % i << "-"
                              << boost::format("%2d") % j << "   ##\n";
                    bodyLog << "###############################################\n";
                }
                ++nextEvent[i][j];
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
