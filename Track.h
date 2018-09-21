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

/**
* @file Track.h
* @brief class Track
*
* @author Matthias Schartner
* @date 17.09.2018
*/

#ifndef VIESCHEDPP_TRACKS_H
#define VIESCHEDPP_TRACKS_H

#include <utility>
#include <vector>
#include "VieVS_NamedObject.h"

namespace VieVS{

    /**
     * @class Track
     * @brief track section of observing mode
     *
     * CURRENTLY UNDER DEVELOPMENT AND UNUSED
     *
     * @author Matthias Schartner
     * @date 17.09.2018
     */
    class Track: public VieVS_NamedObject {
    public:
        enum class Bitstream{
            sign,
            mag,
        };

        explicit Track(std::string name);

        void addFanout(std::string name, Bitstream bitstream, double total_lo, int headstack_number,
                       int first_multiplex_track, int second_multiplex_track = -999, int third_multiplex_track = -999,
                       int fourth_multiplex_track = -999);

    private:
        static unsigned long nextId;


        class Fanout_definition: public VieVS_NamedObject{
        public:
            Fanout_definition(std::string name,
                              Bitstream bitstream,
                              double total_lo,
                              int headstack_number,
                              int first_multiplex_track,
                              int second_multiplex_track = -999,
                              int third_multiplex_track = -999,
                              int fourth_multiplex_track = -999):
                          VieVS_NamedObject{std::move(name), nextId++},
                          bitstream_{bitstream},
                          total_lo_{total_lo},
                          headstack_number_{headstack_number},
                          first_multiplex_track_{first_multiplex_track},
                          second_multiplex_track_{second_multiplex_track},
                          third_multiplex_track_{third_multiplex_track},
                          fourth_multiplex_track_{fourth_multiplex_track}{};

        private:
            static unsigned long nextId;

            Bitstream bitstream_;
            double total_lo_;
            int headstack_number_;
            int first_multiplex_track_;
            int second_multiplex_track_;
            int third_multiplex_track_;
            int fourth_multiplex_track_;
        };

        std::vector<Fanout_definition> fanout_definitions_;

    };
}


#endif //VIESCHEDPP_TRACKS_H
