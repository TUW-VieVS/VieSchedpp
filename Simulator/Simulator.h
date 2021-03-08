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
 * @date 12.02.2020
 */

#ifndef VIESCHEDPP_SIMULATOR_H
#define VIESCHEDPP_SIMULATOR_H

// clang-format off
#include "../Eigen/Dense"
// clang-format on
#include "../Misc/VieVS_NamedObject.h"
#include "../Output/Output.h"

namespace VieVS {

/**
 * @class Simulator
 * @brief main class for simulations
 *
 * @author Matthias Schartner
 * @date 12.02.2020
 */
class Simulator : public VieVS_NamedObject {
   public:
    friend class Solver;
    struct SimPara {
        double wn = 20;
        double clockASD = 1e-14;
        double clockDur = 50;
        double tropo_Cn = 1.8e-7;
        double tropo_H = 2000;
        double tropo_ve = 8;
        double tropo_vn = 0;
        double tropo_dh = 200;
        double tropo_wzd0 = 150;
        double tropo_dhseg = 2;

        void fromXML( const boost::property_tree::ptree &tree );

        std::string toString() {
            return ( boost::format( "| %6.2f | %7.4fe-14 %4.0f  | %5.2fe-7 %5.0f %4.0f %5.2f %5.1f %5.1f %6.1f |" ) %
                     wn % ( clockASD * 1e14 ) % clockDur % ( tropo_Cn * 1e7 ) % tropo_H % tropo_dh % tropo_dhseg %
                     tropo_ve % tropo_vn % tropo_wzd0 )
                .str();
        }
    };


    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param output scheduler
     */
    explicit Simulator(Output &output);


    void start();

   private:
    static unsigned long nextId;  ///< next id for this object type
    std::ofstream of;             ///< output stream object
    unsigned long seed_;

    const boost::property_tree::ptree xml_;  ///< content of VieSchedpp.xml file

    const std::string path_;                                                  ///< path to output directory
    const int version_;                                                       ///< number of this schedule
    const Network network_;                                                   ///< network
    const SourceList sourceList_;                                             ///< all sources
    const std::vector<Scan> scans_;                                           ///< all scans in schedule
    const std::shared_ptr<const ObservingMode> &obsModes_;                    ///< observing mode
    boost::optional<MultiScheduling::Parameters> multiSchedulingParameters_;  ///< multi scheduling parameters

    std::vector<Eigen::MatrixXd> clk_;
    std::vector<Eigen::MatrixXd> tropo_;
    Eigen::MatrixXd obs_minus_com_;
    Eigen::VectorXd P_;
    bool simTropo_ = true;
    bool simClock_ = true;
    bool simWn_ = true;

    std::vector<SimPara> simpara_;
    int nsim = 1;
    std::default_random_engine generator_;

    void simClock();
    void simClockDummy();

    void simTropo();
    void simTropoDummy();

    void calcO_C();

    void setup();

    void parameterSummary();

    void dummyMatrixToFile( const Eigen::MatrixXd &M, const std::string &name ) {
        auto stream = std::ofstream( name );
        stream << M;
        stream.close();
    }

};
}  // namespace VieVS


#endif  // VIESCHEDPP_SIMULATOR_H
