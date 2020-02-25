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
 * @file Simulator.h
 * @brief class Simulator
 *
 * @author Matthias Schartner
 * @date 24.02.2020
 */

#ifndef VIESCHEDPP_SOLVER_H
#define VIESCHEDPP_SOLVER_H

// clang-format off
#include <utility>

#include "Simulator.h"
// clang-format on
#include "../Misc/AstronomicalParameters.h"


namespace VieVS {

class PWL;
/**
 * @class Solver
 * @brief main class for least squares solver
 *
 * @author Matthias Schartner
 * @date 24.02.2020
 */
class Solver : public VieVS_NamedObject {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param simulator scheduler
     * @param fname file name
     */
    Solver( Simulator &simulator, std::string fname );

    void start();

   private:
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
        CLK_quad,
        COORD_X,
        COORD_Y,
        COORD_Z,
        RA,
        DEC,
        undefined,
    };

    struct Unknown {
        Unknown( Type type, int refTime, std::string member = "" )
            : type{type}, refTime{refTime}, member{std::move( member )} {}

        bool operator==( const Unknown &o ) { return type == o.type && refTime == o.refTime && member == o.member; }

        const Type type;
        const int refTime;
        const std::string member;
    };

    struct PWL {
        PWL() = default;

        PWL( Type type, int interval, double constraint )
            : type{type}, interval{interval}, flag_const{true}, constraint{constraint} {}

        PWL( Type type, int interval ) : type{type}, interval{interval} {}

        bool estimate() const { return type != Type::undefined; }

        const Type type = Type::undefined;
        const int interval = 0;
        const bool flag_const = false;
        const double constraint = 0;
    };

    struct EstimationParamGlobal {
        PWL XPO;
        PWL YPO;
        PWL dUT1;
        PWL NUTX;
        PWL NUTY;
    };

    struct EstimationParamStation {
        bool refClock = false;

        PWL ZWD;
        PWL NGR;
        PWL EGR;
        PWL CLK;

        bool coord = true;
        bool NNR_NNT = true;
    };

    struct EstimationParamSource {
        bool coord = true;
        bool NNR_NNT = true;
    };

    enum class Axis {
        X,
        Y,
        Z,
    };

    static unsigned long nextId;  ///< next id for this object type
    std::ofstream of;             ///< output stream object

    const boost::property_tree::ptree xml_;  ///< content of VieSchedpp.xml file

    const Network network_;              ///< network
    const std::vector<Source> sources_;  ///< all sources
    const std::vector<Scan> scans_;      ///< all scans in schedule

    Eigen::Matrix3d dWdx_;
    Eigen::Matrix3d dWdy_;


    std::vector<EstimationParamStation> estimationParamStations_;
    std::vector<EstimationParamSource> estimationParamSources_;
    EstimationParamGlobal estimationParamGlobal_;
    Eigen::Matrix3d A;
    std::vector<Unknown> unknowns;


    void setup();

    void partials( const Observation &obs, const Eigen::Matrix3d &t2c, const Eigen::Matrix3d &dQdx,
                   const Eigen::Matrix3d &dQdy, const Eigen::Matrix3d &dQdut, const Eigen::Matrix3d &dQdX,
                   const Eigen::Matrix3d &dQdY );

    static Eigen::Matrix3d rotm( double angle, Axis ax );

    static Eigen::Matrix3d drotm( double angle, Axis ax );
};


}  // namespace VieVS


#endif  // VIESCHEDPP_SOLVER_H
