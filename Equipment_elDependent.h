//
// Created by matth on 11.04.2018.
//

#ifndef EQUIPMENT_ELDEPENDENT_H
#define EQUIPMENT_ELDEPENDENT_H

#include "Equipment.h"

namespace VieVS{
    class Equipment_elDependent: public Equipment {
    public:
        explicit Equipment_elDependent(const std::unordered_map<std::string, double> &SEFDs,
                                       const std::unordered_map<std::string, double> &SEFD_y,
                                       const std::unordered_map<std::string, double> &SEFD_c0,
                                       const std::unordered_map<std::string, double> &SEFD_c1);

        double getSEFD(const std::string &band, double el) const noexcept override ;

    private:
        std::unordered_map<std::string,double> y_; ///< elevation dependent SEFD parameter "y"
        std::unordered_map<std::string,double> c0_; ///< elevation dependent SEFD parameter "c0"
        std::unordered_map<std::string,double> c1_; ///< elevation dependent SEFD parameter "c1"

    };
}


#endif //EQUIPMENT_ELDEPENDENT_H
