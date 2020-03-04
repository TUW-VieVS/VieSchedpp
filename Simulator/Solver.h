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
#include "../Eigen/SparseCore"

#include "Simulator.h"
#include "Unknown.h"
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

    std::vector<double> getMeanSigma();

    std::vector<double> getRepeatabilities();

    /**
     * @brief write statistics line to statistics.csv file
     * @author Matthias Schartner
     *
     * @param of statistics.csv file
     */
    void writeStatistics( std::ofstream &of );

   private:
    class PWL : public VieVS_Object {
       public:
        PWL() : VieVS_Object( nextId++ ) {}

        explicit PWL( Unknown::Type type ) : VieVS_Object( nextId++ ), flag{false}, type{type} {}

        PWL( Unknown::Type type, int interval )
            : VieVS_Object( nextId++ ), flag{true}, type{type}, interval{interval} {}

        PWL( Unknown::Type type, int interval, double constraint )
            : VieVS_Object( nextId++ ),
              flag{true},
              type{type},
              interval{interval},
              flag_constraint{true},
              constraint{constraint} {}


        bool estimate() const { return flag; }

        Unknown::Type getType() const { return type; }
        int getInterval() const { return interval; }
        bool isFlagConstraint() const { return flag_constraint; }
        double getConstraint() const { return constraint; }

       private:
        static unsigned long nextId;  ///< next id for this object type
        Unknown::Type type = Unknown::Type::undefined;
        bool flag = false;
        int interval = 0;
        bool flag_constraint = false;
        double constraint = 0;
    };

    struct EstimationParamEOP {
        PWL XPO{Unknown::Type::XPO};
        PWL YPO{Unknown::Type::YPO};
        PWL dUT1{Unknown::Type::dUT1};
        PWL NUTX{Unknown::Type::NUTX};
        PWL NUTY{Unknown::Type::NUTY};
    };

    struct Partials {
        double coord_x = std::numeric_limits<double>::quiet_NaN();
        double coord_y = std::numeric_limits<double>::quiet_NaN();
        double coord_z = std::numeric_limits<double>::quiet_NaN();
        double xpo = std::numeric_limits<double>::quiet_NaN();
        double ypo = std::numeric_limits<double>::quiet_NaN();
        double dut1 = std::numeric_limits<double>::quiet_NaN();
        double nutx = std::numeric_limits<double>::quiet_NaN();
        double nuty = std::numeric_limits<double>::quiet_NaN();
        double src_ra = std::numeric_limits<double>::quiet_NaN();
        double src_de = std::numeric_limits<double>::quiet_NaN();
    };

    struct EstimationParamStation {
        bool refClock = false;
        bool linear_clk = true;
        bool quadratic_clk = true;
        PWL CLK{Unknown::Type::CLK};

        PWL ZWD{Unknown::Type::ZWD};
        PWL NGR{Unknown::Type::NGR};
        PWL EGR{Unknown::Type::EGR};

        bool coord = false;
        bool datum = true;
    };

    struct EstimationParamSource {
        bool coord = false;
        bool datum = true;
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
    const Eigen::MatrixXd obs_minus_com_;
    const int version_;                                                       ///< number of this schedule
    boost::optional<MultiScheduling::Parameters> multiSchedulingParameters_;  ///< multi scheduling parameters
    int nsim_;


    std::unordered_map<std::string, unsigned long> name2startIdx;

    std::vector<EstimationParamStation> estimationParamStations_;
    std::vector<EstimationParamSource> estimationParamSources_;
    EstimationParamEOP estimationParamEOP_;
    Eigen::MatrixXd A_;
    Eigen::VectorXd P_A_;
    Eigen::MatrixXd B_;
    Eigen::VectorXd P_B_;
    std::vector<Unknown> unknowns;

    Eigen::VectorXd mean_sig_;
    Eigen::VectorXd rep_;


    void setup();

    void setupSummary();

    void readXML();

    Partials partials( const Observation &obs, const Eigen::Matrix3d &t2c, const Eigen::Matrix3d &dQdx,
                       const Eigen::Matrix3d &dQdy, const Eigen::Matrix3d &dQdut, const Eigen::Matrix3d &dQdX,
                       const Eigen::Matrix3d &dQdY );

    void listUnknowns();

    void buildConstraintsMatrix();

    void buildDesignMatrix();

    void solve();

    void addDatum_stations( Eigen::MatrixXd &N, Eigen::MatrixXd &n );

    void addDatum_sources( Eigen::MatrixXd &N, Eigen::MatrixXd &n );


    static Eigen::Matrix3d rotm( double angle, Axis ax );

    static Eigen::Matrix3d drotm( double angle, Axis ax );

    void partialsToA( unsigned int iobs, const Observation &obs, const PointingVector &pv1, const PointingVector &pv2,
                      const Partials &p );

    unsigned long findStartIdxPWL( unsigned int time, unsigned long startIdx );

    std::vector<double> summarizeResult( const Eigen::VectorXd & );
};


}  // namespace VieVS


#endif  // VIESCHEDPP_SOLVER_H
