//
// Created by mschartn on 12.03.18.
//

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
