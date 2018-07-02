//
// Created by mschartn on 04.05.18.
//

#ifndef SUBNETTING_H
#define SUBNETTING_H

#include <vector>

namespace VieVS{
    struct Subnetting{
        std::vector<std::vector<unsigned long>> subnettingSrcIds;
        unsigned int subnettingMinNSta = 2;
    };
}

#endif //SUBNETTING_H
