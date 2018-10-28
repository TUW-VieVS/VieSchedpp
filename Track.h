#include <utility>

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
#include <algorithm>
#include <fstream>
#include <boost/format.hpp>
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

        std::string toString(Bitstream b) const{
            switch(b){
                case Bitstream::sign: return "sign";
                case Bitstream::mag: return "mag";
            }
        }

        explicit Track(std::string name);

        void setBits(int bits){
            bits_ = bits;
        }

        int numberOfBits(const std::shared_ptr<const Track> &other) const{
            return std::min({bits_, other->bits_});
        }

        void addFanout(std::string subpass, std::string trksId, Bitstream bitstream, int headstack_number,
                       int first_multiplex_track, int second_multiplex_track = -999, int third_multiplex_track = -999,
                       int fourth_multiplex_track = -999);

        void toVexTracksDefinition( std::ofstream &of, const std::string &comment = "" ) const;

    private:
        static unsigned long nextId;


        class Fanout_definition: public VieVS_Object{
        public:
            Fanout_definition(std::string subpass,
                              std::string trksId,
                              Bitstream bitstream,
                              int headstack_number,
                              int first_multiplex_track,
                              int second_multiplex_track = -999,
                              int third_multiplex_track = -999,
                              int fourth_multiplex_track = -999):

                          VieVS_Object{nextId++},

                          subpass_{std::move(subpass)},
                          trksid_{std::move(trksId)},
                          bitstream_{bitstream},
                          headstack_number_{headstack_number},

                          first_multiplex_track_{first_multiplex_track},
                          second_multiplex_track_{second_multiplex_track},
                          third_multiplex_track_{third_multiplex_track},
                          fourth_multiplex_track_{fourth_multiplex_track}{};

            std::string subpass_;
            std::string trksid_;
            Bitstream bitstream_;
            int headstack_number_;

            int first_multiplex_track_;
            int second_multiplex_track_;
            int third_multiplex_track_;
            int fourth_multiplex_track_;

        private:
            static unsigned long nextId;
        };

        std::vector<Fanout_definition> fanout_definitions_;
        int bits_ = 1;

    };
}


#endif //VIESCHEDPP_TRACKS_H
