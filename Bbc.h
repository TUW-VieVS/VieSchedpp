//
// Created by mschartn on 10.09.18.
//

#ifndef VIESCHEDPP_BBC_H
#define VIESCHEDPP_BBC_H

#include <utility>
#include <vector>
#include "VieVS_NamedObject.h"


namespace VieVS{
    class Bbc: public VieVS_NamedObject {
    public:
        explicit Bbc(std::string name);

        void addBbc(std::string name, unsigned int physical_bbc_number, unsigned int if_name);

    private:
        static unsigned long nextId;

        enum class Net_sideband{
            U,
            L,
            D,
        };

        class Bbc_assign: public VieVS_NamedObject{
        public:
            Bbc_assign(std::string name,
                       unsigned int physical_bbc_number,
                       unsigned int if_name):
                    VieVS_NamedObject{std::move(name), nextId++},
                    physical_bbc_number_{physical_bbc_number},
                    if_name_{if_name}{};

        private:
            static unsigned long nextId;

            unsigned int physical_bbc_number_;
            unsigned int if_name_;
        };

        std::vector<Bbc_assign> bbc_assigns_;
    };
}


#endif //VIESCHEDPP_BBC_H
