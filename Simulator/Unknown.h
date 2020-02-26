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

/**
 * @file Unknown.h
 * @brief class Unknown
 *
 * @author Matthias Schartner
 * @date 26.02.2020
 */

#ifndef VIESCHEDPP_UNKNOWN_H
#define VIESCHEDPP_UNKNOWN_H

#include <limits>
#include <string>

#include "../Misc/VieVS_Object.h"

namespace VieVS {

/**
 * @class Unknown
 * @brief This class defines one unknown parameter used during parameter estimation
 *
 * @author Matthias Schartner
 * @date 26.02.2020
 */

class Unknown : public VieVS_Object {
   public:
    enum class Type {
        XPO,
        YPO,
        dUT1,
        NUTX,
        NUTY,
        ZWD,
        NGR,
        EGR,
        CLK,
        CLK_linear,
        CLK_quad,
        COORD_X,
        COORD_Y,
        COORD_Z,
        RA,
        DEC,
        undefined,
    };

    explicit Unknown( Type type, int refTime = std::numeric_limits<int>::min(), std::string member = "" )
        : VieVS_Object( nextId++ ), type{type}, refTime{refTime}, member{std::move( member )} {}

    Unknown( Type type, std::string member )
        : VieVS_Object( nextId++ ), type{type}, refTime{std::numeric_limits<int>::min()}, member{std::move( member )} {}

    bool defined() { return type != Type::undefined; }

    bool operator==( const Unknown &o ) { return type == o.type && refTime == o.refTime && member == o.member; }

    const Type type = Type::undefined;
    const int refTime = std::numeric_limits<int>::min();
    const std::string member = "";

   private:
    static unsigned long nextId;  ///< next id for this object type
};


}  // namespace VieVS

#endif  // VIESCHEDPP_UNKNOWN_H
