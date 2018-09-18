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

#ifndef VLBI_SCHEDULER_VIEVS_OBJECT_H
#define VLBI_SCHEDULER_VIEVS_OBJECT_H

#include <string>

namespace VieVS{
    class VieVS_Object {
    public:

        explicit VieVS_Object(unsigned long id): id_{id}{};

        const unsigned long getId() const {
            return id_;
        }

        bool hasValidId() const {
            return id_>0;
        }

        bool hasId(unsigned long id) const {
            return id_==id;
        }

        void setId(unsigned long id){
            id_ = id;
        }

        std::string printId() const {
            return std::string("(id: ").append(std::to_string(id_)).append(")");
        }

    private:
        unsigned long id_;
    };
}


#endif //VLBI_SCHEDULER_VIEVS_OBJECT_H
