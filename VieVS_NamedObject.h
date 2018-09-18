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

#ifndef VLBI_SCHEDULER_VIEVS_NAMEDOBJECT_H
#define VLBI_SCHEDULER_VIEVS_NAMEDOBJECT_H

#include "VieVS_Object.h"
#include <string>
#include <utility>

namespace VieVS{
    class VieVS_NamedObject: public VieVS_Object {
    public:
        VieVS_NamedObject(std::string name, unsigned long id):VieVS_Object(id), name_{std::move(name)}{};

        VieVS_NamedObject(std::string name, std::string alternativeName, unsigned long id):
                VieVS_Object(id), name_{std::move(name)}, alternativeName_{std::move(alternativeName)}{
        };


        const std::string &getName() const{
            return name_;
        }

        const std::string &getAlternativeName() const{
            return alternativeName_;
        }

        bool hasAlternativeName() const{
            return !alternativeName_.empty();
        }

        bool hasName(const std::string &name) const{
            return name_ == name || alternativeName_ == name;
        }

    private:
        std::string name_ = "";
        std::string alternativeName_ = "";
    };

}


#endif //VLBI_SCHEDULER_VIEVS_NAMEDOBJECT_H
