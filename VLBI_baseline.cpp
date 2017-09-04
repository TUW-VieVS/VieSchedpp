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

#include "VLBI_baseline.h"

thread_local VieVS::VLBI_baseline::PARAMETER_STORAGE VieVS::VLBI_baseline::PARA;
std::vector<std::vector<std::vector<VieVS::VLBI_baseline::EVENT> > >  VieVS::VLBI_baseline::EVENTS;
std::vector<std::vector<unsigned int> >  VieVS::VLBI_baseline::nextEvent;

namespace VieVS{
    VLBI_baseline::VLBI_baseline(){
    }

    VLBI_baseline::VLBI_baseline(int srcid, int staid1, int staid2, unsigned int startTime)
            : srcid(srcid), staid1(staid1), staid2(staid2), startTime{startTime}{
    }

    void
    VLBI_baseline::checkForNewEvent(unsigned int time, bool &hardBreak, bool output, std::ofstream &bodyLog) noexcept {
        int nsta = VLBI_baseline::nextEvent.size();
        for (int i = 0; i < nsta; ++i) {
            for (int j = i + 1; j < nsta; ++j) {

                unsigned int thisNextEvent = VLBI_baseline::nextEvent[i][j];

                while (EVENTS[i][j][thisNextEvent].time <= time) {

                    VLBI_baseline::PARAMETERS newPARA = EVENTS[i][j][thisNextEvent].PARA;
                    hardBreak = hardBreak || !EVENTS[i][j][thisNextEvent].softTransition;

                    VLBI_baseline::PARA.ignore[i][j] = *newPARA.ignore;
                    VLBI_baseline::PARA.maxScan[i][j] = *newPARA.maxScan;
                    VLBI_baseline::PARA.minScan[i][j] = *newPARA.minScan;
                    VLBI_baseline::PARA.weight[i][j] = *newPARA.weight;
                    for (const auto &any:newPARA.minSNR) {
                        VLBI_baseline::PARA.minSNR[any.first][i][j] = any.second;
                    }
                    if (output) {
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
}
