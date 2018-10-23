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

#include "Track.h"

using namespace VieVS;
using namespace std;

unsigned long VieVS::Track::nextId = 0;
unsigned long VieVS::Track::Fanout_definition::nextId = 0;

Track::Track(std::string name): VieVS_NamedObject{std::move(name), nextId++} {

}

void Track::addFanout(std::string subpass, std::string trksId, Bitstream bitstream, int headstack_number,
                      int first_multiplex_track, int second_multiplex_track, int third_multiplex_track,
                      int fourth_multiplex_track) {

    fanout_definitions_.emplace_back(subpass, trksId, bitstream, headstack_number, first_multiplex_track,
            second_multiplex_track, third_multiplex_track, fourth_multiplex_track);
}
