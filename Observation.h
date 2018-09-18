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


#ifndef OBSERVATION_H
#define OBSERVATION_H

#include "VieVS_Object.h"

namespace VieVS{
    class Observation: public VieVS_Object {
    public:

        Observation(unsigned long blid, unsigned long staid1, unsigned long staid2, unsigned long srcid,
                    unsigned int startTime, unsigned int observingTime = 0);

        Observation(const Observation &other);


        unsigned long getBlid() const {
            return blid_;
        }

        unsigned long getStaid1() const {
            return staid1_;
        }

        unsigned long getStaid2() const {
            return staid2_;
        }

        unsigned long getSrcid() const {
            return srcid_;
        }

        unsigned int getStartTime() const {
            return startTime_;
        }

        unsigned int getObservingTime() const {
            return observingTime_;
        }

        bool containsStation(unsigned long staid) const noexcept;

        void setObservingTime(unsigned int observingTime) {
            observingTime_ = observingTime;
        }

        static unsigned long numberOfCreatedObjects() {
            return nextId;
        }

    private:
        static unsigned long nextId;

        unsigned long blid_;
        unsigned long staid1_;
        unsigned long staid2_;
        unsigned long srcid_;

        unsigned int startTime_ ;
        unsigned int observingTime_;

    };
}


#endif //OBSERVATION_H
