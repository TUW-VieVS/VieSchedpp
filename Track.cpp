//
// Created by mschartn on 10.09.18.
//

#include "Track.h"

using namespace VieVS;
using namespace std;

unsigned long VieVS::Track::nextId = 0;
unsigned long VieVS::Track::Fanout_definition::nextId = 0;

Track::Track(std::string name): VieVS_NamedObject{std::move(name), nextId++} {

}

void Track::addFanout(std::string name, Track::Bitstream bitstream, double total_lo, int headstack_number,
                      int first_multiplex_track, int second_multiplex_track, int third_multiplex_track,
                      int fourth_multiplex_track) {

    fanout_definitions_.emplace_back(name, bitstream, total_lo, headstack_number, first_multiplex_track,
            second_multiplex_track, third_multiplex_track, fourth_multiplex_track);
}
