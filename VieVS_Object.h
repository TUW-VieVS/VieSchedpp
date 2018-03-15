//
// Created by mschartn on 12.03.18.
//

#ifndef VLBI_SCHEDULER_VIEVS_OBJECT_H
#define VLBI_SCHEDULER_VIEVS_OBJECT_H

namespace VieVS{
    class VieVS_Object {
    public:

        explicit VieVS_Object(int id): id_{id}{};

        const int getId() const {
            return id_;
        }

        bool hasValidId() const {
            return id_>0;
        }

        bool hasID(int id) const {
            return id_==id;
        }

    protected:
        int id_;

    };
}


#endif //VLBI_SCHEDULER_VIEVS_OBJECT_H
