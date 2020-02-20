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


#include "../Misc/VieVS_NamedObject.h"
#include "../Output/Output.h"
#include "Eigen/Dense"

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
    struct SimPara {
        double wn = 20;
        double clockASD = 1e-14;
        double clockDur = 50;
        double tropo_Cn = 1.8;
        double tropo_H = 2000;
        double tropo_ve = 8;
        double tropo_vn = 0;
        double tropo_dh = 200;
        double tropo_wzd0 = 150;
        double tropo_dhseg = 2;

        void fromXML( const boost::property_tree::ptree &tree );

        std::string toString() {
            return ( boost::format( "| %5.1f | %7.4fe-14 %4.0f  | %5.2fe-7 %5.0f %4.0f %5.2f %5.1f %5.1f %6.1f |" ) %
                     wn % clockASD % clockDur % tropo_Cn % tropo_H % tropo_dh % tropo_dhseg % tropo_ve % tropo_vn %
                     tropo_wzd0 )
                .str();
        }
    };


    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param sched scheduler
     * @param path path to output directory
     * @param fname file name
     * @param version version number
     */
    Simulator( Output &sched, std::string path, std::string fname, int version );

    void start();

   private:
    static unsigned long nextId;  ///< next id for this object type
    std::ofstream of;             ///< output stream object

    boost::property_tree::ptree xml_;  ///< content of VieSchedpp.xml file

    std::string path_;             ///< path to output directory
    int version_;                  ///< number of this schedule
    Network network_;              ///< network
    std::vector<Source> sources_;  ///< all sources
    std::vector<Scan> scans_;      ///< all scans in schedule
    std::vector<Observation> obs_;
    const std::shared_ptr<const ObservingMode> &obsModes_;  ///< observing mode

    std::vector<Eigen::MatrixXd> clk_;
    std::vector<Eigen::MatrixXd> tropo_;
    Eigen::MatrixXd obs_minus_com_;
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
};
}  // namespace VieVS


#endif  // VIESCHEDPP_SIMULATOR_H
