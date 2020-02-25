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
#include "Simulator.h"
// clang-format on
#include "../Misc/AstronomicalParameters.h"


namespace VieVS {

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
   private:
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

    void partials( const Observation &obs, const Eigen::Matrix3d &t2c, const Eigen::Matrix3d &dQdx,
                   const Eigen::Matrix3d &dQdy, const Eigen::Matrix3d &dQdut, const Eigen::Matrix3d &dQdX,
                   const Eigen::Matrix3d &dQdY );

    static Eigen::Matrix3d rotm( double angle, Axis ax );

    static Eigen::Matrix3d drotm( double angle, Axis ax );
};


}  // namespace VieVS


#endif  // VIESCHEDPP_SOLVER_H
