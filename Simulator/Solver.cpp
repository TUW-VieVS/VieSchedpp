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

/*
 * File:   Simulator.cpp
 * Author: mschartn
 *
 * 12.02.2020
 */

#include "Solver.h"

using namespace std;
using namespace VieVS;
using namespace Eigen;

unsigned long VieVS::Solver::nextId = 0;
unsigned long VieVS::Solver::PWL::nextId = 0;

Solver::Solver( Simulator &simulator, std::string fname )
    : VieVS_NamedObject( move( fname ), nextId++ ),
      xml_{simulator.xml_},
      network_{std::move( simulator.network_ )},
      sources_{std::move( simulator.sources_ )},
      scans_{std::move( simulator.scans_ )},
      multiSchedulingParameters_{std::move( simulator.multiSchedulingParameters_ )},
      version_{simulator.version_},
      obs_minus_com_{std::move( simulator.obs_minus_com_ )},
      P_A_{std::move( simulator.P_ )},
      nsim_{simulator.nsim},
      of{std::move( simulator.of )} {

    estimationParamStations_ = vector<EstimationParamStation>( network_.getNSta() );
    estimationParamSources_ = vector<EstimationParamSource>( sources_.size() );
}

void Solver::start() {
    setup();
    buildConstraintsMatrix();
    buildDesignMatrix();
    solve();
}

void Solver::setup() {
    readXML();
    setupSummary();

    unsigned long constraints = 0;
    unsigned long nobs = 0;
    for ( const auto &scan : scans_ ) {
        nobs += scan.getNObs();
    }
    int daySecOfSessionStart = TimeSystem::startTime.time_of_day().total_seconds();

    auto addPWL_params = [&daySecOfSessionStart, &constraints, this]( const PWL &p, const string &name = "" ) {
        if ( p.estimate() && p.getType() != Unknown::Type::undefined ) {
            int t = daySecOfSessionStart / p.getInterval() * p.getInterval() - daySecOfSessionStart;

            unknowns.emplace_back( p.getType(), t, name );
            t += p.getInterval();
            while ( t < static_cast<int>( TimeSystem::duration ) ) {
                unknowns.emplace_back( p.getType(), t, name );
                t += p.getInterval();
                ++constraints;
            }
            unknowns.emplace_back( p.getType(), t, name );
            ++constraints;
        }
    };


    for ( int i = 0; i < network_.getNSta(); ++i ) {
        const string &sta_name = network_.getStation( i ).getName();
        const auto &params = estimationParamStations_[i];
        if ( params.refClock ) {
            continue;
        }
        if ( params.CLK.estimate() && !params.refClock ) {
            name2startIdx[Unknown::typeString( Unknown::Type::CLK ) + sta_name] = unknowns.size();
            addPWL_params( params.CLK, sta_name );
        }
    }

    for ( int i = 0; i < network_.getNSta(); ++i ) {
        const string &sta_name = network_.getStation( i ).getName();
        const auto &params = estimationParamStations_[i];
        if ( params.refClock ) {
            continue;
        }
        name2startIdx[Unknown::typeString( Unknown::Type::CLK_linear ) + sta_name] = unknowns.size();
        unknowns.emplace_back( Unknown::Type::CLK_linear, sta_name );
        name2startIdx[Unknown::typeString( Unknown::Type::CLK_quad ) + sta_name] = unknowns.size();
        unknowns.emplace_back( Unknown::Type::CLK_quad, sta_name );
    }
    for ( int i = 0; i < network_.getNSta(); ++i ) {
        const string &sta_name = network_.getStation( i ).getName();
        const auto &params = estimationParamStations_[i];
        name2startIdx[Unknown::typeString( Unknown::Type::ZWD ) + sta_name] = unknowns.size();
        addPWL_params( params.ZWD, sta_name );
    }
    for ( int i = 0; i < network_.getNSta(); ++i ) {
        const string &sta_name = network_.getStation( i ).getName();
        const auto &params = estimationParamStations_[i];
        name2startIdx[Unknown::typeString( Unknown::Type::NGR ) + sta_name] = unknowns.size();
        addPWL_params( params.NGR, sta_name );
    }
    for ( int i = 0; i < network_.getNSta(); ++i ) {
        const string &sta_name = network_.getStation( i ).getName();
        const auto &params = estimationParamStations_[i];
        name2startIdx[Unknown::typeString( Unknown::Type::EGR ) + sta_name] = unknowns.size();
        addPWL_params( params.EGR, sta_name );
    }
    name2startIdx[Unknown::typeString( Unknown::Type::XPO )] = unknowns.size();
    addPWL_params( estimationParamEOP_.XPO );
    name2startIdx[Unknown::typeString( Unknown::Type::YPO )] = unknowns.size();
    addPWL_params( estimationParamEOP_.YPO );
    name2startIdx[Unknown::typeString( Unknown::Type::dUT1 )] = unknowns.size();
    addPWL_params( estimationParamEOP_.dUT1 );
    name2startIdx[Unknown::typeString( Unknown::Type::NUTX )] = unknowns.size();
    addPWL_params( estimationParamEOP_.NUTX );
    name2startIdx[Unknown::typeString( Unknown::Type::NUTY )] = unknowns.size();
    addPWL_params( estimationParamEOP_.NUTY );


    for ( int i = 0; i < network_.getNSta(); ++i ) {
        const string &sta_name = network_.getStation( i ).getName();
        const auto &params = estimationParamStations_[i];
        name2startIdx[Unknown::typeString( Unknown::Type::COORD_X ) + sta_name] = unknowns.size();
        if ( params.coord ) {
            unknowns.emplace_back( Unknown::Type::COORD_X, sta_name );
        }
    }
    for ( int i = 0; i < network_.getNSta(); ++i ) {
        const string &sta_name = network_.getStation( i ).getName();
        const auto &params = estimationParamStations_[i];
        name2startIdx[Unknown::typeString( Unknown::Type::COORD_Y ) + sta_name] = unknowns.size();
        if ( params.coord ) {
            unknowns.emplace_back( Unknown::Type::COORD_Y, sta_name );
        }
    }
    for ( int i = 0; i < network_.getNSta(); ++i ) {
        const string &sta_name = network_.getStation( i ).getName();
        const auto &params = estimationParamStations_[i];
        name2startIdx[Unknown::typeString( Unknown::Type::COORD_Z ) + sta_name] = unknowns.size();
        if ( params.coord ) {
            unknowns.emplace_back( Unknown::Type::COORD_Z, sta_name );
        }
    }


    for ( int i = 0; i < sources_.size(); ++i ) {
        const string &src_name = sources_[i].getName();
        const auto &params = estimationParamSources_[i];
        if ( params.coord ) {
            name2startIdx[Unknown::typeString( Unknown::Type::RA ) + src_name] = unknowns.size();
            unknowns.emplace_back( Unknown::Type::RA, src_name );
            name2startIdx[Unknown::typeString( Unknown::Type::DEC ) + src_name] = unknowns.size();
            unknowns.emplace_back( Unknown::Type::DEC, src_name );
        }
    }

    A_ = MatrixXd::Zero( nobs, unknowns.size() );
    B_ = MatrixXd::Zero( constraints, unknowns.size() );
    P_B_ = VectorXd::Zero( constraints );
}

void Solver::buildConstraintsMatrix() {
    unsigned long i = 0;
    auto f = [this, &i]( const PWL &pwl, const string &name = "" ) {
        if ( pwl.estimate() ) {
            unsigned long prev_idx = name2startIdx[Unknown::typeString( pwl.getType() ) + name];
            double v = 1 / ( pwl.getConstraint() * pwl.getConstraint() );
            unsigned long follow_idx = prev_idx + 1;


            while ( follow_idx < unknowns.size() &&
                    ( unknowns[follow_idx].type == pwl.getType() && unknowns[follow_idx].member == name ) ) {
                B_( i, prev_idx ) = 1;
                B_( i, follow_idx ) = -1;
                P_B_( i ) = v;
                ++prev_idx;
                ++follow_idx;
                ++i;
            }
        }
    };

    for ( int i_sta = 0; i_sta < network_.getNSta(); ++i_sta ) {
        const auto &sta = network_.getStation( i_sta ).getName();
        const auto &para = estimationParamStations_[i_sta];
        f( para.CLK, sta );
    }
    for ( int i_sta = 0; i_sta < network_.getNSta(); ++i_sta ) {
        const auto &sta = network_.getStation( i_sta ).getName();
        const auto &para = estimationParamStations_[i_sta];
        f( para.ZWD, sta );
    }
    for ( int i_sta = 0; i_sta < network_.getNSta(); ++i_sta ) {
        const auto &sta = network_.getStation( i_sta ).getName();
        const auto &para = estimationParamStations_[i_sta];
        f( para.NGR, sta );
    }
    for ( int i_sta = 0; i_sta < network_.getNSta(); ++i_sta ) {
        const auto &sta = network_.getStation( i_sta ).getName();
        const auto &para = estimationParamStations_[i_sta];
        f( para.EGR, sta );
    }

    f( estimationParamEOP_.XPO );
    f( estimationParamEOP_.YPO );
    f( estimationParamEOP_.dUT1 );
    f( estimationParamEOP_.NUTX );
    f( estimationParamEOP_.NUTY );
}

void Solver::buildDesignMatrix() {
    unsigned int iobs = 0;

    Eigen::Matrix3d dWdx;
    Eigen::Matrix3d dWdy;
    dWdx << 0, 0, -1, 0, 0, 0, 1, 0, 0;
    dWdy << 0, 0, 0, 0, 0, 1, 0, -1, 0;

    for ( const auto &scan : scans_ ) {
        double date1 = 2400000.5;
        unsigned int startTime = scan.getTimes().getObservingTime();
        double mjd = TimeSystem::mjdStart + static_cast<double>( startTime ) / 86400.0;

        // calculate EOP transformation and rotation matrizes:
        double era = iauEra00( date1, mjd );
        Matrix3d R = rotm( -era, Axis::Z );
        Matrix3d dR = -drotm( -era, Axis::Z ) * 1.00273781191135448;
        unsigned int nutIdx = AstronomicalParameters::getNutInterpolationIdx( startTime );
        double X = AstronomicalParameters::getNutX( startTime, nutIdx );
        double Y = AstronomicalParameters::getNutY( startTime, nutIdx );
        double S = AstronomicalParameters::getNutS( startTime, nutIdx );

        double X2Y2 = X * X + Y * Y;
        double v = sqrt( X2Y2 );
        double E = atan2( Y, X );
        double z = sqrt( 1 - ( X2Y2 ) );
        double d = atan2( v, z );
        Matrix3d PN = rotm( -E, Axis::Z ) * rotm( -d, Axis::Y ) * rotm( E + S, Axis::Z );
        double dEdX = -Y / X2Y2;
        double dEdY = X / X2Y2;

        double dddX = X / ( z * v );
        double dddY = Y / ( z * v );
        double dSdX = -Y / 2;
        double dSdY = -X / 2;

        Matrix3d rotm_Ez = rotm( E, Axis::Z );
        Matrix3d m_rotm_Ez = rotm( -E, Axis::Z );
        Matrix3d rotm_Sz = rotm( S, Axis::Z );
        Matrix3d m_rotm_d = rotm( -d, Axis::Y );

        Matrix3d dPNdX = drotm( -E, Axis::Z ) * m_rotm_d * rotm_Ez * rotm_Sz * -dEdX +
                         m_rotm_Ez * drotm( -d, Axis::Y ) * rotm_Ez * rotm_Sz * -dddX +
                         m_rotm_Ez * m_rotm_d * drotm( E, Axis::Z ) * rotm_Sz * dEdX +
                         m_rotm_Ez * m_rotm_d * rotm_Ez * drotm( S, Axis::Z ) * dSdX;

        Matrix3d dPNdY = drotm( -E, Axis::Z ) * m_rotm_d * rotm_Ez * rotm_Sz * -dEdY +
                         m_rotm_Ez * drotm( -d, Axis::Y ) * rotm_Ez * rotm_Sz * -dddY +
                         m_rotm_Ez * m_rotm_d * drotm( E, Axis::Z ) * rotm_Sz * dEdY +
                         m_rotm_Ez * m_rotm_d * rotm_Ez * drotm( S, Axis::Z ) * dSdY;

        Matrix3d t2c = PN * R;

        Matrix3d dQdx = t2c * dWdx;
        Matrix3d dQdy = t2c * dWdy;
        Matrix3d dQdut = PN * dR;
        Matrix3d dQdX = dPNdX * R;
        Matrix3d dQdY = dPNdY * R;

        for ( const auto &obs : scan.getObservations() ) {
            const PointingVector &pv1 = scan.getPointingVector( *scan.findIdxOfStationId( obs.getStaid1() ) );
            const PointingVector &pv2 = scan.getPointingVector( *scan.findIdxOfStationId( obs.getStaid2() ) );
            // partials stations
            Partials p = partials( obs, t2c, dQdx, dQdy, dQdut, dQdX, dQdY );
            partialsToA( iobs, obs, pv1, pv2, p );
            ++iobs;
        }
    }
}

void Solver::solve() {
    of << "\n";
    of << "Number of unknowns:       " << unknowns.size() << "\n";
    of << "Number of observations:   " << A_.rows() << "\n";
    of << "Number of constraints:    " << B_.rows() << endl;
    MatrixXd A( A_.rows() + B_.rows(), A_.cols() );
    A << A_, B_;
    VectorXd P( P_A_.size() + P_B_.size() );
    P << P_A_, P_B_;
    MatrixXd o_c( A.rows(), obs_minus_com_.cols() );
    o_c << obs_minus_com_, MatrixXd::Zero( B_.rows(), obs_minus_com_.cols() );
    o_c *= speedOfLight * 100;

    MatrixXd N = A.transpose() * P.asDiagonal() * A;
    MatrixXd n = A.transpose() * P.asDiagonal() * o_c;
    addDatum_stations( N, n );
    addDatum_sources( N, n );

    MatrixXd x = N.completeOrthogonalDecomposition().solve( n );

    MatrixXd v = A * x.topRows( A.cols() ) - o_c;
    auto fun_std = []( const VectorXd &v ) {
        return sqrt( ( v.array() - v.mean() ).square().sum() / ( v.size() - 1 ) );
    };


    VectorXd vTPv = ( v.transpose() * P.asDiagonal() * v ).diagonal();
    int red = A.rows() - unknowns.size();
    of << "vTPv:                     " << vTPv.mean() << " +/- " << fun_std( vTPv ) << endl;
    of << "redundancy:               " << red << endl;
    VectorXd m0 = ( vTPv / red ).array().sqrt();
    of << "chi^2:                    " << m0.mean() << " +/- " << fun_std( m0 ) << endl;

    VectorXd tmp = N.inverse().diagonal().array().sqrt();
    MatrixXd sigma_x = tmp * m0.transpose();

    mean_sig_ = sigma_x.rowwise().mean();
    rep_ = VectorXd::Zero( sigma_x.rows() );
    if ( sigma_x.cols() > 1 ) {
        Eigen::ArrayXXd s = x.topRows( A.cols() ).transpose().array();
        rep_ = ( ( ( s.rowwise() - s.colwise().mean() ).square().colwise().sum() / ( s.rows() - 1 ) ).sqrt() ).matrix();
    }

    listUnknowns();
}

void Solver::addDatum_stations( MatrixXd &N, MatrixXd &n ) {
    MatrixXd dat = MatrixXd::Zero( 6, N.cols() );
    bool stationInDatum = false;

    double cc = 0;
    for ( int i = 0; i < network_.getNSta(); ++i ) {
        const auto &sta = network_.getStation( i );
        cc += sta.getPosition().getX() * sta.getPosition().getX();
        cc += sta.getPosition().getY() * sta.getPosition().getY();
        cc += sta.getPosition().getZ() * sta.getPosition().getZ();
    }
    cc = sqrt( cc );
    int c = 0;
    for ( int i = 0; i < network_.getNSta(); ++i ) {
        const auto &sta = network_.getStation( i );
        const auto &para = estimationParamStations_[i];

        double xii = sta.getPosition().getX() / cc;
        double yii = sta.getPosition().getY() / cc;
        double zii = sta.getPosition().getZ() / cc;

        if ( para.coord && para.datum ) {
            MatrixXd B = MatrixXd::Zero( 6, 3 );
            B( 0, 0 ) = 1;
            B( 1, 1 ) = 1;
            B( 2, 2 ) = 1;

            B( 3, 1 ) = -zii;
            B( 3, 2 ) = yii;

            B( 4, 0 ) = zii;
            B( 4, 2 ) = -xii;

            B( 5, 0 ) = -yii;
            B( 5, 1 ) = xii;
            stationInDatum = true;

            unsigned long idx_x = name2startIdx[Unknown::typeString( Unknown::Type::COORD_X ) + sta.getName()];
            unsigned long idx_y = name2startIdx[Unknown::typeString( Unknown::Type::COORD_Y ) + sta.getName()];
            unsigned long idx_z = name2startIdx[Unknown::typeString( Unknown::Type::COORD_Z ) + sta.getName()];
            dat.col( idx_x ) = B.col( 0 );
            dat.col( idx_y ) = B.col( 1 );
            dat.col( idx_z ) = B.col( 2 );
            ++c;
        }
    }

    if ( stationInDatum ) {
        N.conservativeResize( N.rows() + 6, N.cols() + 6 );
        N.block( N.rows() - 6, 0, 6, dat.cols() ) = dat;
        N.block( 0, N.cols() - 6, dat.cols(), 6 ) = dat.transpose();
        N.block( N.rows() - 6, N.rows() - 6, 6, 6 ) = MatrixXd::Zero( 6, 6 );

        n.conservativeResize( n.rows() + 6, n.cols() );
        n.block( n.rows() - 6, 0, 6, n.cols() ) = MatrixXd::Zero( 6, n.cols() );
    }
    of << "Number of datum stations: " << c << endl;
}

void Solver::addDatum_sources( MatrixXd &N, MatrixXd &n ) {}

Solver::Partials Solver::partials( const Observation &obs, const Matrix3d &t2c, const Matrix3d &dQdx,
                                   const Matrix3d &dQdy, const Matrix3d &dQdut, const Matrix3d &dQdX,
                                   const Matrix3d &dQdY ) {
    Partials p;
    Vector3d vearth{AstronomicalParameters::earth_velocity[0], AstronomicalParameters::earth_velocity[1],
                    AstronomicalParameters::earth_velocity[2]};
    Vector3d beta = vearth / speedOfLight;

    unsigned long staid1 = obs.getStaid1();
    unsigned long staid2 = obs.getStaid2();
    unsigned long srcid = obs.getSrcid();
    const Station &sta1 = network_.getStation( staid1 );
    const Station &sta2 = network_.getStation( staid2 );
    const Source &src = sources_[srcid];
    Vector3d v2{-omega * sta2.getPosition().getY(), omega * sta2.getPosition().getX(), 0};
    Vector3d b2 = ( v2 + vearth ) / speedOfLight;
    double gam = 1 / sqrt( 1 - beta.dot( beta ) );

    Vector3d rq( src.getSourceInCrs()[0], src.getSourceInCrs()[1], src.getSourceInCrs()[2] );
    double rho = 1 + rq.dot( b2 );

    Vector3d psi = -( gam * ( 1 - beta.dot( b2 ) ) * rq / rho + gam * beta );
    Matrix3d E = Matrix3d::Identity() + ( ( gam - 1 ) * beta / ( beta.dot( beta ) ) - gam * b2 ) * beta.transpose();
    Vector3d K = E * psi;
    Vector3d b_gcrs =
        t2c * Vector3d( sta2.getPosition().getX(), sta2.getPosition().getY(), sta2.getPosition().getZ() ) -
        t2c * Vector3d( sta1.getPosition().getX(), sta1.getPosition().getY(), sta1.getPosition().getZ() );
    Vector3d M =
        ( Matrix3d::Identity() - ( rq * b2.transpose() ) ) * ( -gam * ( 1 - b2.dot( beta ) ) * ( E * b_gcrs ) / rho );

    Vector3d B = t2c.transpose() * K;


    // stations
    p.coord_x = -B( 0 );
    p.coord_y = -B( 1 );
    p.coord_z = -B( 2 );

    // EOP
    Vector3d b_trs( sta2.getPosition().getX() - sta1.getPosition().getX(),
                    sta2.getPosition().getY() - sta1.getPosition().getY(),
                    sta2.getPosition().getZ() - sta1.getPosition().getZ() );

    p.xpo = K.dot( dQdx * b_trs ) / speedOfLight;
    p.ypo = K.dot( dQdy * b_trs ) / speedOfLight;
    p.dut1 = K.dot( dQdut * b_trs ) / speedOfLight;
    p.nutx = K.dot( dQdX * b_trs ) / speedOfLight;
    p.nuty = K.dot( dQdY * b_trs ) / speedOfLight;

    // sources
    double sid = sin( src.getDe() );
    double cod = cos( src.getDe() );
    double sir = sin( src.getRa() );
    double cor = cos( src.getRa() );

    Vector3d drqdra( -cod * sir, cod * cor, 0 );
    Vector3d drqdde( -sid * cor, sid * sir, cod );

    p.src_ra = drqdra.dot( M ) * pi / 180 / 3600000 * 100;
    p.src_de = drqdde.dot( M ) * pi / 180 / 3600000 * 100;

    return p;
}

void Solver::partialsToA( unsigned int iobs, const Observation &obs, const PointingVector &pv1,
                          const PointingVector &pv2, const Partials &p ) {
    unsigned long staid1 = obs.getStaid1();
    unsigned long staid2 = obs.getStaid2();
    string sta1 = network_.getStation( staid1 ).getName();
    string sta2 = network_.getStation( staid2 ).getName();
    const auto &para1 = estimationParamStations_[staid1];
    const auto &para2 = estimationParamStations_[staid2];
    unsigned int time = obs.getStartTime();

    auto partialsPWL = [iobs, time, this]( Unknown::Type type, double val, const string &name = "" ) {
        if ( !isnan( val ) ) {
            unsigned long idx = name2startIdx[Unknown::typeString( type ) + name];
            unsigned long prev = findStartIdxPWL( time, idx );
            unsigned long follow = prev + 1;
            int rs = unknowns[prev].refTime;
            int re = unknowns[follow].refTime;
            auto dt = static_cast<double>( re - rs );
            double f2 = ( static_cast<int>( time ) - rs ) / ( dt );
            double f1 = 1. - f2;
            A_( iobs, prev ) = f1 * val;
            A_( iobs, follow ) = f2 * val;
        }
    };

    // station coordinates
    if ( para1.coord ) {
        A_( iobs, name2startIdx[Unknown::typeString( Unknown::Type::COORD_X ) + sta1] ) = p.coord_x;
        A_( iobs, name2startIdx[Unknown::typeString( Unknown::Type::COORD_Y ) + sta1] ) = p.coord_y;
        A_( iobs, name2startIdx[Unknown::typeString( Unknown::Type::COORD_Z ) + sta1] ) = p.coord_z;
    }
    if ( para2.coord ) {
        A_( iobs, name2startIdx[Unknown::typeString( Unknown::Type::COORD_X ) + sta2] ) = -p.coord_x;
        A_( iobs, name2startIdx[Unknown::typeString( Unknown::Type::COORD_Y ) + sta2] ) = -p.coord_y;
        A_( iobs, name2startIdx[Unknown::typeString( Unknown::Type::COORD_Z ) + sta2] ) = -p.coord_z;
    }

    // EOP
    partialsPWL( Unknown::Type::XPO, p.xpo * speedOfLight * 100 / rad2mas );
    partialsPWL( Unknown::Type::YPO, p.ypo * speedOfLight * 100 / rad2mas );
    partialsPWL( Unknown::Type::dUT1, p.dut1 * speedOfLight * 100 / rad2mas );
    partialsPWL( Unknown::Type::NUTX, p.nutx * speedOfLight * 100 / rad2mas );
    partialsPWL( Unknown::Type::NUTY, p.nuty * speedOfLight * 100 / rad2mas );

    // clock
    if ( !para1.refClock ) {
        double clk_lin1 = ( static_cast<int>( obs.getStartTime() ) -
                            unknowns[name2startIdx[Unknown::typeString( Unknown::Type::CLK ) + sta1]].refTime ) /
                          86400.;
        double clk_quad1 = clk_lin1 * clk_lin1;
        if ( para1.CLK.estimate() ) {
            partialsPWL( Unknown::Type::CLK, -1, sta1 );
        }
        if ( para1.linear_clk ) {
            unsigned long idx1 = name2startIdx[Unknown::typeString( Unknown::Type::CLK_linear ) + sta1];
            A_( iobs, idx1 ) = -clk_lin1;
        }
        if ( para1.quadratic_clk ) {
            unsigned long idx1 = name2startIdx[Unknown::typeString( Unknown::Type::CLK_quad ) + sta1];
            A_( iobs, idx1 ) = -clk_quad1;
        }
    }

    if ( !para2.refClock ) {
        double clk_lin2 = ( static_cast<int>( obs.getStartTime() ) -
                            unknowns[name2startIdx[Unknown::typeString( Unknown::Type::CLK ) + sta2]].refTime ) /
                          86400.;
        double clk_quad2 = clk_lin2 * clk_lin2;
        if ( para2.CLK.estimate() ) {
            partialsPWL( Unknown::Type::CLK, 1, sta2 );
        }
        if ( para2.linear_clk ) {
            unsigned long idx2 = name2startIdx[Unknown::typeString( Unknown::Type::CLK_linear ) + sta2];
            A_( iobs, idx2 ) = clk_lin2;
        }
        if ( para2.quadratic_clk ) {
            unsigned long idx2 = name2startIdx[Unknown::typeString( Unknown::Type::CLK_quad ) + sta2];
            A_( iobs, idx2 ) = clk_quad2;
        }
    }
    // zwd
    if ( para1.ZWD.estimate() ) {
        double val = -1 / sin( pv1.getEl() );
        partialsPWL( Unknown::Type::ZWD, val, sta1 );
    }
    if ( para2.ZWD.estimate() ) {
        double val = 1 / sin( pv2.getEl() );
        partialsPWL( Unknown::Type::ZWD, val, sta2 );
    }

    // ngr
    if ( para1.NGR.estimate() ) {
        double val = -1 / ( tan( pv1.getEl() ) * sin( pv1.getEl() ) + 0.0032 ) * cos( pv1.getAz() );
        partialsPWL( Unknown::Type::NGR, val, sta1 );
    }
    if ( para2.NGR.estimate() ) {
        double val = 1 / ( tan( pv2.getEl() ) * sin( pv2.getEl() ) + 0.0032 ) * cos( pv2.getAz() );
        partialsPWL( Unknown::Type::NGR, val, sta2 );
    }

    // egr
    if ( para1.EGR.estimate() ) {
        double val = -1 / ( tan( pv1.getEl() ) * sin( pv1.getEl() ) + 0.0032 ) * sin( pv1.getAz() );
        partialsPWL( Unknown::Type::EGR, val, sta1 );
    }
    if ( para2.EGR.estimate() ) {
        double val = 1 / ( tan( pv2.getEl() ) * sin( pv2.getEl() ) + 0.0032 ) * sin( pv2.getAz() );
        partialsPWL( Unknown::Type::EGR, val, sta2 );
    }
}


Matrix3d Solver::rotm( double angle, Axis ax ) {
    Matrix3d r = Matrix3d::Zero();
    double ca = cos( angle );
    double sa = sin( angle );
    switch ( ax ) {
        case Axis::X: {
            r( 0, 0 ) = 1;
            r( 1, 1 ) = ca;
            r( 1, 2 ) = sa;
            r( 2, 1 ) = -sa;
            r( 2, 2 ) = ca;
            break;
        }
        case Axis::Y: {
            r( 0, 0 ) = ca;
            r( 0, 2 ) = -sa;
            r( 1, 1 ) = 1;
            r( 2, 0 ) = sa;
            r( 2, 2 ) = ca;
            break;
        }
        case Axis::Z: {
            r( 0, 0 ) = ca;
            r( 0, 1 ) = sa;
            r( 1, 0 ) = -sa;
            r( 1, 1 ) = ca;
            r( 2, 2 ) = 1;
            break;
        }
    }
    return r;
}

Matrix3d Solver::drotm( double angle, Axis ax ) {
    Matrix3d r = Matrix3d::Zero();
    double ca = cos( angle );
    double sa = sin( angle );
    switch ( ax ) {
        case Axis::X: {
            r( 1, 1 ) = -sa;
            r( 1, 2 ) = ca;
            r( 2, 1 ) = -ca;
            r( 2, 2 ) = -sa;
            break;
        }
        case Axis::Y: {
            r( 0, 0 ) = -sa;
            r( 0, 2 ) = -ca;
            r( 2, 0 ) = ca;
            r( 2, 2 ) = -sa;
            break;
        }
        case Axis::Z: {
            r( 0, 0 ) = -sa;
            r( 0, 1 ) = ca;
            r( 1, 0 ) = -ca;
            r( 1, 1 ) = -sa;
            break;
        }
    }
    return r;
}

void Solver::readXML() {
    const auto &tree = xml_.get_child( "VieSchedpp.solver" );

    string refClock = tree.get( "reference_clock", "" );

    bool firstStation = true;
    for ( const auto &any : tree ) {
        if ( any.first == "EOP" ) {
            if ( any.second.get_child_optional( "XPO" ).is_initialized() ) {
                estimationParamEOP_.XPO = PWL( Unknown::Type::XPO, any.second.get<int>( "XPO.interval" ) * 3600,
                                               any.second.get<double>( "XPO.constraint" ) );
            }
            if ( any.second.get_child_optional( "YPO" ).is_initialized() ) {
                estimationParamEOP_.YPO = PWL( Unknown::Type::YPO, any.second.get<int>( "YPO.interval" ) * 3600,
                                               any.second.get<double>( "YPO.constraint" ) );
            }
            if ( any.second.get_child_optional( "dUT1" ).is_initialized() ) {
                estimationParamEOP_.dUT1 = PWL( Unknown::Type::dUT1, any.second.get<int>( "dUT1.interval" ) * 3600,
                                                any.second.get<double>( "dUT1.constraint" ) );
            }
            if ( any.second.get_child_optional( "NUTX" ).is_initialized() ) {
                estimationParamEOP_.NUTX = PWL( Unknown::Type::NUTX, any.second.get<int>( "NUTX.interval" ) * 3600,
                                                any.second.get<double>( "NUTX.constraint" ) );
            }
            if ( any.second.get_child_optional( "NUTY" ).is_initialized() ) {
                estimationParamEOP_.NUTY = PWL( Unknown::Type::NUTY, any.second.get<int>( "NUTY.interval" ) * 3600,
                                                any.second.get<double>( "NUTY.constraint" ) );
            }
        }

        if ( any.first == "station" ) {
            string name = any.second.get( "<xmlattr>.name", "" );
            EstimationParamStation tmp;
            tmp.coord = any.second.get( "coordinates", false );
            tmp.datum = any.second.get( "datum", true );
            if ( firstStation && refClock.empty() ) {
                refClock = name;
            }
            tmp.linear_clk = any.second.get( "linear_clock", true );
            tmp.quadratic_clk = any.second.get( "quadratic_clock", true );
            if ( any.second.get_child_optional( "PWL_clock" ).is_initialized() ) {
                tmp.CLK = PWL( Unknown::Type::CLK, any.second.get<int>( "PWL_clock.interval" ) * 60,
                               any.second.get<double>( "PWL_clock.constraint" ) );
            }
            if ( any.second.get_child_optional( "PWL_ZWD" ).is_initialized() ) {
                tmp.ZWD = PWL( Unknown::Type::ZWD, any.second.get<int>( "PWL_ZWD.interval" ) * 60,
                               any.second.get<double>( "PWL_ZWD.constraint" ) );
            }
            if ( any.second.get_child_optional( "PWL_NGR" ).is_initialized() ) {
                tmp.NGR = PWL( Unknown::Type::NGR, any.second.get<int>( "PWL_NGR.interval" ) * 60,
                               any.second.get<double>( "PWL_NGR.constraint" ) );
            }
            if ( any.second.get_child_optional( "PWL_EGR" ).is_initialized() ) {
                tmp.EGR = PWL( Unknown::Type::EGR, any.second.get<int>( "PWL_EGR.interval" ) * 60,
                               any.second.get<double>( "PWL_EGR.constraint" ) );
            }

            firstStation = false;
            if ( name == "__all__" ) {
                for ( int i = 0; i < network_.getNSta(); ++i ) {
                    estimationParamStations_[i] = tmp;
                    if ( network_.getStation( i ).getName() == refClock ) {
                        tmp.refClock = true;
                        estimationParamStations_[i] = tmp;
                        tmp.refClock = false;
                    } else {
                        estimationParamStations_[i] = tmp;
                    }
                }
            } else {
                for ( int i = 0; i < network_.getNSta(); ++i ) {
                    if ( network_.getStation( i ).getName() == name ) {
                        if ( network_.getStation( i ).getName() == refClock ) {
                            tmp.refClock = true;
                            estimationParamStations_[i] = tmp;
                            tmp.refClock = false;
                        } else {
                            estimationParamStations_[i] = tmp;
                        }
                        break;
                    }
                }
            }
        }

        if ( any.first == "source" ) {
            string name = any.second.get( "<xmlattr>.name", "" );
            EstimationParamSource tmp;
            tmp.coord = any.second.get( "coord", false );
            tmp.datum = any.second.get( "datum", true );

            if ( name == "__all__" ) {
                for ( int i = 0; i < sources_.size(); ++i ) {
                    estimationParamSources_[i] = tmp;
                }
            } else {
                for ( int i = 0; i < sources_.size(); ++i ) {
                    if ( sources_[i].getName() == name ) {
                        estimationParamSources_[i] = tmp;
                        break;
                    }
                }
            }
        }
    }
}

void Solver::setupSummary() {
    of << "Estimate EOP:\n";

    of << ".---------------------------------------------.\n";
    of << "|   type   | estimate | interval | constraint |\n";
    of << "|          |          |    [h]   |    [mas]   |\n";
    of << "|----------|----------|----------|------------|\n";

    auto f = [this]( const PWL &pwl ) {
        string flag;
        pwl.estimate() ? flag = "true" : flag = "false";
        of << boost::format( "| %-8s | %=8s | %8.2f | %10.4f |\n" ) % Unknown::typeString( pwl.getType() ) % flag %
                  ( pwl.getInterval() / 3600. ) % pwl.getConstraint();
    };
    f( estimationParamEOP_.XPO );
    f( estimationParamEOP_.YPO );
    f( estimationParamEOP_.dUT1 );
    f( estimationParamEOP_.NUTX );
    f( estimationParamEOP_.NUTY );
    of << "'---------------------------------------------'\n\n";

    of << "Estimate station parameters:\n"
          ".-----------------------------------------------------------------------------------------------------------"
          "--------------------.\n"
          "|   name   | coord | datum | lin CLK | quad CLK |      PWL CLK      |      PWL ZWD      |      PWL NGR      "
          "|      PWL EGT      |\n"
          "|          |       |       |         |          | interval | constr | interval | constr | interval | constr "
          "| interval | constr |\n"
          "|          |       |       |         |          |   [min]  |  [cm]  |   [min]  |  [cm]  |   [min]  |   [cm] "
          "|   [min]  |  [cm]  |\n"
          "|----------|-------|-------|---------|----------|----------|--------|----------|--------|----------|--------"
          "|----------|--------|\n";

    for ( int i = 0; i < network_.getNSta(); ++i ) {
        const auto &p = estimationParamStations_[i];
        const auto &name = network_.getStation( i ).getName();

        string flagCoord = p.coord ? "yes" : "no";
        string flagDatum = p.coord && p.datum ? "yes" : "no";
        string flagLinClk = p.refClock ? "ref" : p.linear_clk ? "yes" : "no";
        string flagQuadClk = p.refClock ? "ref" : p.quadratic_clk ? "yes" : "no";
        of << boost::format( "| %-8s | %=5s | %=5s | %=7s | %=8s " ) % name % flagCoord % flagDatum % flagLinClk %
                  flagQuadClk;
        auto fn = [this]( const PWL &pwl ) {
            if ( pwl.estimate() ) {
                of << boost::format( "| %8.2f | %6.3f " ) % ( pwl.getInterval() / 60. ) % pwl.getConstraint();
            } else {
                of << boost::format( "| %=8s | %=6s " ) % "--" % "--";
            }
        };
        if ( p.refClock ) {
            of << "|       -- |     -- ";
        } else {
            fn( p.CLK );
        }
        fn( p.ZWD );
        fn( p.NGR );
        fn( p.EGR );
        of << "|\n";
    }
    of << "'-----------------------------------------------------------------------------------------------------------"
          "--------------------'"
       << endl;
}

void Solver::listUnknowns() {
    of << "\nList of estimated parameters\n";
    of << ".---------------------------------------------------------------------------------------------------.\n";
    of << "|     # | Type     | member   | reference epoch     |      sigma [unit]     | repeatability [unit]  |\n"
          "|-------|----------|----------|---------------------|-----------------------|-----------------------|\n";
    for ( int i = 0; i < unknowns.size(); ++i ) {
        const auto &u = unknowns[i];
        double sig = mean_sig_[i];
        double rep = rep_[i];
        if ( rep != 0 ) {
            of << boost::format( "| %5d %s%11.5f %-10s |%11.5f %-10s |\n" ) % i % u.toString() % sig %
                      Unknown::getUnit( u.type ) % rep % Unknown::getUnit( u.type );
        } else {
            of << boost::format( "| %5d %s%11.5f %-10s |%11s %10s |\n" ) % i % u.toString() % sig %
                      Unknown::getUnit( u.type ) % "--" % "";
        }
    }
    of << "'---------------------------------------------------------------------------------------------------'\n";
}

unsigned long Solver::findStartIdxPWL( unsigned int time, unsigned long startIdx ) {
    unsigned long endIdx = startIdx + 1;
    auto it = unknowns.begin() + endIdx;
    while ( it != unknowns.end() && it->refTime < time ) {
        ++it;
    }
    return distance( unknowns.begin(), it ) - 1;
}

std::vector<double> Solver::getMeanSigma() { return summarizeResult( mean_sig_ ); }
std::vector<double> Solver::getRepeatabilities() { return summarizeResult( rep_ ); }

std::vector<double> Solver::summarizeResult( const Eigen::VectorXd &vec ) {
    vector<double> v;
    vector<Unknown::Type> types{Unknown::Type::dUT1, Unknown::Type::XPO, Unknown::Type::YPO, Unknown::Type::NUTX,
                                Unknown::Type::NUTY};

    for ( const auto &t : types ) {
        double val = 0;
        int c = 0;

        for ( int j = 0; j < unknowns.size(); ++j ) {
            const auto &u = unknowns[j];
            if ( u.type == t ) {
                val += vec[j];
                ++c;
            }
        }
        if ( c == 0 ) {
            v.push_back( numeric_limits<double>::quiet_NaN() );
        } else {
            if ( t == Unknown::Type::dUT1 ) {
                v.push_back( val / c / 15. * 1000 );
            } else {
                v.push_back( val / c * 1000 );
            }
        }
    }

    for ( const auto &sta : network_.getStations() ) {
        const string &name = sta.getName();
        double x = 0;
        double y = 0;
        double z = 0;
        for ( int j = 0; j < unknowns.size(); ++j ) {
            const auto &u = unknowns[j];
            if ( u.type == Unknown::Type::COORD_X && u.member == name ) {
                x = vec[j];
                break;
            }
        }
        for ( int j = 0; j < unknowns.size(); ++j ) {
            const auto &u = unknowns[j];
            if ( u.type == Unknown::Type::COORD_Y && u.member == name ) {
                y = vec[j];
                break;
            }
        }
        for ( int j = 0; j < unknowns.size(); ++j ) {
            const auto &u = unknowns[j];
            if ( u.type == Unknown::Type::COORD_Z && u.member == name ) {
                z = vec[j];
                break;
            }
        }
        double val = sqrt( x * x + y * y + z * z );
        if ( val == 0 ) {
            v.push_back( numeric_limits<double>::quiet_NaN() );
        } else {
            v.push_back( val * 10 );
        }
    }

    return v;
}

void Solver::writeStatistics( std::ofstream &stat_of ) {
    string oString;

    auto n_scans = static_cast<int>( scans_.size() );
    int n_standard = 0;
    int n_fillin = 0;
    int n_calibrator = 0;
    int n_single = 0;
    int n_subnetting = 0;
    int n_obs_total = 0;
    vector<unsigned int> nscan_sta( network_.getNSta(), 0 );
    vector<unsigned int> nobs_sta( network_.getNSta(), 0 );
    vector<unsigned int> nobs_bl( network_.getNBls(), 0 );
    vector<unsigned int> nscan_src( sources_.size(), 0 );
    vector<unsigned int> nobs_src( sources_.size(), 0 );

    for ( const auto &any : scans_ ) {
        switch ( any.getType() ) {
            case Scan::ScanType::fillin: {
                ++n_fillin;
                break;
            }
            case Scan::ScanType::astroCalibrator: {
                ++n_calibrator;
                break;
            }
            case Scan::ScanType::standard: {
                ++n_standard;
                break;
            }
            case Scan::ScanType::highImpact: {
                ++n_standard;
                break;
            }
        }
        switch ( any.getScanConstellation() ) {
            case Scan::ScanConstellation::single: {
                ++n_single;
                break;
            }
            case Scan::ScanConstellation::subnetting: {
                ++n_subnetting;
                break;
            }
        }
        auto n_obs = any.getNObs();
        n_obs_total += n_obs;
        for ( int ista = 0; ista < any.getNSta(); ++ista ) {
            const PointingVector &pv = any.getPointingVector( ista );
            unsigned long id = pv.getStaid();
            ++nscan_sta[id];
        }
        for ( int ibl = 0; ibl < any.getNObs(); ++ibl ) {
            const Observation &obs = any.getObservation( ibl );
            ++nobs_sta[obs.getStaid1()];
            ++nobs_sta[obs.getStaid2()];
            ++nobs_bl[network_.getBaseline( obs.getStaid1(), obs.getStaid2() ).getId()];
        }
        unsigned long id = any.getSourceId();
        ++nscan_src[id];
        nobs_src[id] += n_obs;
    }
    int n_src = static_cast<int>( count_if( nscan_src.begin(), nscan_src.end(), []( int i ) { return i > 0; } ) );

    auto totalTime = static_cast<double>( TimeSystem::duration );
    vector<double> obsPer;
    for ( const auto &station : network_.getStations() ) {
        int t = station.getStatistics().totalObservingTime;
        obsPer.push_back( static_cast<double>( t ) / totalTime * 100 );
    }
    double obsMean = accumulate( obsPer.begin(), obsPer.end(), 0.0 ) / ( network_.getNSta() );

    vector<double> preobPer;
    for ( const auto &station : network_.getStations() ) {
        int t = station.getStatistics().totalPreobTime;
        preobPer.push_back( static_cast<double>( t ) / totalTime * 100 );
    }
    double preobMean = accumulate( preobPer.begin(), preobPer.end(), 0.0 ) / ( network_.getNSta() );

    vector<double> slewPer;
    for ( const auto &station : network_.getStations() ) {
        int t = station.getStatistics().totalSlewTime;
        slewPer.push_back( static_cast<double>( t ) / totalTime * 100 );
    }
    double slewMean = accumulate( slewPer.begin(), slewPer.end(), 0.0 ) / ( network_.getNSta() );

    vector<double> idlePer;
    for ( const auto &station : network_.getStations() ) {
        int t = station.getStatistics().totalIdleTime;
        idlePer.push_back( static_cast<double>( t ) / totalTime * 100 );
    }
    double idleMean = accumulate( idlePer.begin(), idlePer.end(), 0.0 ) / ( network_.getNSta() );

    vector<double> fieldPer;
    for ( const auto &station : network_.getStations() ) {
        int t = station.getStatistics().totalFieldSystemTime;
        fieldPer.push_back( static_cast<double>( t ) / totalTime * 100 );
    }
    double fieldMean = accumulate( fieldPer.begin(), fieldPer.end(), 0.0 ) / ( network_.getNSta() );

    vector<double> a13m30;
    vector<double> a25m30;
    vector<double> a37m30;
    vector<double> a13m60;
    vector<double> a25m60;
    vector<double> a37m60;
    for ( const auto &station : network_.getStations() ) {
        unsigned long id = station.getId();
        const auto &map = network_.getStaid2skyCoverageId();
        unsigned long skyCovId = map.at( id );
        const auto &skyCov = network_.getSkyCoverage( skyCovId );
        a13m30.push_back( skyCov.getSkyCoverageScore_a13m30() );
        a25m30.push_back( skyCov.getSkyCoverageScore_a25m30() );
        a37m30.push_back( skyCov.getSkyCoverageScore_a37m30() );
        a13m60.push_back( skyCov.getSkyCoverageScore_a13m60() );
        a25m60.push_back( skyCov.getSkyCoverageScore_a25m60() );
        a37m60.push_back( skyCov.getSkyCoverageScore_a37m60() );
    }
    double a13m30Mean = accumulate( a13m30.begin(), a13m30.end(), 0.0 ) / ( network_.getNSta() );
    double a25m30Mean = accumulate( a25m30.begin(), a25m30.end(), 0.0 ) / ( network_.getNSta() );
    double a37m30Mean = accumulate( a37m30.begin(), a37m30.end(), 0.0 ) / ( network_.getNSta() );
    double a13m60Mean = accumulate( a13m60.begin(), a13m60.end(), 0.0 ) / ( network_.getNSta() );
    double a25m60Mean = accumulate( a25m60.begin(), a25m60.end(), 0.0 ) / ( network_.getNSta() );
    double a37m60Mean = accumulate( a37m60.begin(), a37m60.end(), 0.0 ) / ( network_.getNSta() );

    oString.append( std::to_string( version_ ) ).append( "," );
    oString.append( std::to_string( n_scans ) ).append( "," );
    oString.append( std::to_string( n_single ) ).append( "," );
    oString.append( std::to_string( n_subnetting ) ).append( "," );
    oString.append( std::to_string( n_fillin ) ).append( "," );
    oString.append( std::to_string( n_calibrator ) ).append( "," );
    oString.append( std::to_string( n_obs_total ) ).append( "," );
    oString.append( std::to_string( network_.getNSta() ) ).append( "," );
    oString.append( std::to_string( n_src ) ).append( "," );

    oString.append( std::to_string( obsMean ) ).append( "," );
    oString.append( std::to_string( preobMean ) ).append( "," );
    oString.append( std::to_string( slewMean ) ).append( "," );
    oString.append( std::to_string( idleMean ) ).append( "," );
    oString.append( std::to_string( fieldMean ) ).append( "," );

    oString.append( std::to_string( a13m30Mean ) ).append( "," );
    oString.append( std::to_string( a25m30Mean ) ).append( "," );
    oString.append( std::to_string( a37m30Mean ) ).append( "," );
    oString.append( std::to_string( a13m60Mean ) ).append( "," );
    oString.append( std::to_string( a25m60Mean ) ).append( "," );
    oString.append( std::to_string( a37m60Mean ) ).append( "," );

    oString.append( WeightFactors::statisticsValues() );

    for ( auto any : obsPer ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : preobPer ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : slewPer ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : idlePer ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : fieldPer ) {
        oString.append( std::to_string( any ) ).append( "," );
    }

    for ( auto any : a13m30 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : a25m30 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : a37m30 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : a13m60 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : a25m60 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : a37m60 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }

    for ( int i = 0; i < network_.getNSta(); ++i ) {
        oString.append( std::to_string( nscan_sta[i] ) ).append( "," );
    }
    for ( int i = 0; i < network_.getNSta(); ++i ) {
        oString.append( std::to_string( nobs_sta[i] ) ).append( "," );
    }
    for ( int i = 0; i < network_.getNBls(); ++i ) {
        oString.append( std::to_string( nobs_bl[i] ) ).append( "," );
    }
    for ( int i = 0; i < sources_.size(); ++i ) {
        oString.append( std::to_string( nscan_src[i] ) ).append( "," );
    }
    for ( int i = 0; i < sources_.size(); ++i ) {
        oString.append( std::to_string( nobs_src[i] ) ).append( "," );
    }

    if ( multiSchedulingParameters_.is_initialized() ) {
        oString.append( multiSchedulingParameters_->statisticsOutput() );
    }

    vector<double> msig = getMeanSigma();
    oString.append( std::to_string( nsim_ ) ).append( "," );
    for ( int i = 0; i < 5; ++i ) {
        double v = msig[i];
        if ( isnan( v ) ) {
            oString.append( "0," );
        } else {
            oString.append( std::to_string( v ) ).append( "," );
        }
    }
    double meanS = 0;
    for ( int i = 5; i < 5 + network_.getNSta(); ++i ) {
        meanS += msig[i];
    }
    meanS /= network_.getNSta();
    if ( isnan( meanS ) ) {
        oString.append( "0," );
    } else {
        oString.append( std::to_string( meanS ) ).append( "," );
    }
    for ( int i = 5; i < 5 + network_.getNSta(); ++i ) {
        double v = msig[i];
        if ( isnan( v ) ) {
            oString.append( "0," );
        } else {
            oString.append( std::to_string( v ) ).append( "," );
        }
    }


    vector<double> rep = getRepeatabilities();
    oString.append( std::to_string( nsim_ ) ).append( "," );
    for ( int i = 0; i < 5; ++i ) {
        double v = rep[i];
        if ( isnan( v ) ) {
            oString.append( "0," );
        } else {
            oString.append( std::to_string( v ) ).append( "," );
        }
    }
    double meanR = 0;
    for ( int i = 5; i < 5 + network_.getNSta(); ++i ) {
        meanR += rep[i];
    }
    meanR /= network_.getNSta();
    if ( isnan( meanR ) ) {
        oString.append( "0," );
    } else {
        oString.append( std::to_string( meanR ) ).append( "," );
    }
    for ( int i = 5; i < 5 + network_.getNSta(); ++i ) {
        double v = rep[i];
        if ( isnan( v ) ) {
            oString.append( "0," );
        } else {
            oString.append( std::to_string( v ) ).append( "," );
        }
    }


#ifdef _OPENMP
#pragma omp critical
#endif
    { stat_of << oString << endl; };
}
