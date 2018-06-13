//
// Created by mschartn on 12.03.18.
//

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

        bool hasId(int id) const {
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
