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
