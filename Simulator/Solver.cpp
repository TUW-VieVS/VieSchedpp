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
      obs_minus_com_{std::move( simulator.obs_minus_com_ )},
      of{std::move( simulator.of )} {
    dWdx_ << 0, 0, -1, 0, 0, 0, 1, 0, 0;
    dWdy_ << 0, 0, 0, 0, 0, 1, 0, -1, 0;

    estimationParamStations_ = vector<EstimationParamStation>( network_.getNSta() );
    estimationParamSources_ = vector<EstimationParamSource>( sources_.size() );
}

void Solver::start() {
    setup();
    buildConstraintsMatrix();
    buildDesignMatrix();
    solve();

    ofstream of_dummy( "A.txt" );
    if ( of_dummy.is_open() ) {
        of_dummy << A_;
    }
    ofstream of_dummy2( "B.txt" );
    if ( of_dummy2.is_open() ) {
        of_dummy2 << B_;
    }
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
        name2startIdx[Unknown::typeString( Unknown::Type::COORD_X ) + sta_name] = unknowns.size();
        if ( params.coord ) {
            unknowns.emplace_back( Unknown::Type::COORD_X, sta_name );
            unknowns.emplace_back( Unknown::Type::COORD_Y, sta_name );
            unknowns.emplace_back( Unknown::Type::COORD_Z, sta_name );
        }
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
        if ( params.refClock ) {
            continue;
        }
        name2startIdx[Unknown::typeString( Unknown::Type::CLK_linear ) + sta_name] = unknowns.size();
        unknowns.emplace_back( Unknown::Type::CLK_linear, sta_name );
    }
    for ( int i = 0; i < network_.getNSta(); ++i ) {
        const string &sta_name = network_.getStation( i ).getName();
        const auto &params = estimationParamStations_[i];
        if ( params.refClock ) {
            continue;
        }
        name2startIdx[Unknown::typeString( Unknown::Type::CLK_quad ) + sta_name] = unknowns.size();
        unknowns.emplace_back( Unknown::Type::CLK_quad, sta_name );
    }
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
    for ( int i = 0; i < sources_.size(); ++i ) {
        const string &src_name = sources_[i].getName();
        const auto &params = estimationParamSources_[i];
        if ( params.coord ) {
            name2startIdx[Unknown::typeString( Unknown::Type::RA ) + src_name] = unknowns.size();
            unknowns.emplace_back( Unknown::Type::RA, src_name );
            unknowns.emplace_back( Unknown::Type::DEC, src_name );
        }
    }

    A_ = MatrixXd::Zero( nobs, unknowns.size() );
    P_A_ = VectorXd::Zero( nobs );
    B_ = MatrixXd::Zero( constraints, unknowns.size() );
    P_B_ = VectorXd::Zero( constraints );

    listUnknowns();
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
                B_( i, prev_idx ) = -1;
                B_( i, follow_idx ) = 1;
                P_B_( i ) = v;
                ++prev_idx;
                ++follow_idx;
                ++i;
            }
        }
    };

    f( estimationParamEOP_.XPO );
    f( estimationParamEOP_.YPO );
    f( estimationParamEOP_.dUT1 );
    f( estimationParamEOP_.NUTX );
    f( estimationParamEOP_.NUTY );

    for ( int i_sta = 0; i_sta < network_.getNSta(); ++i_sta ) {
        const auto &sta = network_.getStation( i_sta ).getName();
        const auto &para = estimationParamStations_[i_sta];
        f( para.CLK, sta );
        f( para.ZWD, sta );
        f( para.NGR, sta );
        f( para.EGR, sta );
    }
}

void Solver::buildDesignMatrix() {
    unsigned int iobs = 0;
    for ( const auto &scan : scans_ ) {
        double date1 = 2400000.5;
        unsigned int startTime = scan.getTimes().getObservingTime();
        double mjd = TimeSystem::mjdStart + static_cast<double>( startTime ) / 86400.0;

        // calculate EOP transformation and rotation matrizes:
        double era = iauEra00( date1, mjd );
        Matrix3d R = rotm( -era, Axis::Z );
        Matrix3d dR = drotm( -era, Axis::Z ) * 1.00273781191135448;
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
        double dddY = X / ( z * v );
        double dSdX = -Y / 2;
        double dSdY = -X / 2;

        Matrix3d rotm_Ez = rotm( E, Axis::Z );
        Matrix3d rotm_Sz = rotm( S, Axis::Z );
        Matrix3d m_rotm_dy = rotm( -d, Axis::Y );

        Matrix3d dPNdX = drotm( -E, Axis::Z ) * m_rotm_dy * rotm_Ez * rotm_Sz * -dEdX +
                         -rotm_Ez * drotm( -d, Axis::Y ) * rotm_Ez * rotm_Sz * -dddX +
                         -rotm_Ez * m_rotm_dy * drotm( E, Axis::Z ) * rotm_Sz * dEdX +
                         -rotm_Ez * m_rotm_dy * rotm_Ez * drotm( S, Axis::Z ) * dSdX;

        Matrix3d dPNdY = drotm( -E, Axis::Z ) * m_rotm_dy * rotm_Ez * rotm_Sz * -dEdY +
                         -rotm_Ez * drotm( -d, Axis::Y ) * rotm_Ez * rotm_Sz * -dddY +
                         -rotm_Ez * m_rotm_dy * drotm( E, Axis::Z ) * rotm_Sz * dEdY +
                         -rotm_Ez * m_rotm_dy * rotm_Ez * drotm( S, Axis::Z ) * dSdY;

        Matrix3d t2c = PN * R;

        Matrix3d dQdx = t2c * dWdx_;
        Matrix3d dQdy = t2c * dWdy_;
        Matrix3d dQdut = PN * dR;
        Matrix3d dQdX = dPNdX * R;
        Matrix3d dQdY = dPNdY * R;

        for ( const auto &obs : scan.getObservations() ) {
            const PointingVector &pv1 = scan.getPointingVector( *scan.findIdxOfStationId( obs.getStaid1() ) );
            const PointingVector &pv2 = scan.getPointingVector( *scan.findIdxOfStationId( obs.getStaid1() ) );
            // partials stations
            Partials p = partials( obs, t2c, dQdx, dQdy, dQdut, dQdX, dQdY );
            partialsToA( iobs, obs, pv1, pv2, p );
            P_A_( iobs ) = .5;
            ++iobs;
        }
    }
}

void Solver::solve() {
    MatrixXd A( A_.rows() + B_.rows(), A_.cols() );
    A << A_, B_;
    VectorXd P( P_A_.size() + P_B_.size() );
    P << P_A_, P_B_;
    MatrixXd o_c( A.rows(), obs_minus_com_.cols() );
    MatrixXd dummy = MatrixXd::Zero( A.rows() - obs_minus_com_.rows(), obs_minus_com_.cols() );
    o_c << obs_minus_com_, dummy;

    MatrixXd N = A.transpose() * P.asDiagonal() * A;
    MatrixXd n = A.transpose() * P.asDiagonal() * o_c;
    addDatum_stations( N, n );
    addDatum_sources( N, n );

    MatrixXd x1 = N.ldlt().solve( n );

    MatrixXd x2 = N.colPivHouseholderQr().solve( n );
}

void Solver::addDatum_stations( MatrixXd &N, MatrixXd &n ) {}

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
            double f2 = ( time - rs ) / ( dt );
            double f1 = 1. - f2;
            A_( iobs, prev ) = f1 * val;
            A_( iobs, follow ) = f2 * val;
        }
    };

    // station coordinates
    if ( !isnan( p.coord_x ) ) {
        unsigned long idx1 = name2startIdx[Unknown::typeString( Unknown::Type::COORD_X ) + sta1];
        unsigned long idx2 = name2startIdx[Unknown::typeString( Unknown::Type::COORD_X ) + sta2];

        A_( iobs, idx1 ) = -p.coord_x;
        A_( iobs, idx1 + 1 ) = -p.coord_y;
        A_( iobs, idx1 + 2 ) = -p.coord_z;

        A_( iobs, idx2 ) = p.coord_x;
        A_( iobs, idx2 + 1 ) = p.coord_y;
        A_( iobs, idx2 + 2 ) = p.coord_z;
    }

    // EOP
    partialsPWL( Unknown::Type::XPO, p.xpo * speedOfLight * 1 / rad2mas );
    partialsPWL( Unknown::Type::YPO, p.ypo * speedOfLight * 1 / rad2mas );
    partialsPWL( Unknown::Type::dUT1, p.dut1 * speedOfLight * 1 / rad2mas );
    partialsPWL( Unknown::Type::NUTX, p.nutx * speedOfLight * 1 / rad2mas );
    partialsPWL( Unknown::Type::NUTY, p.nuty * speedOfLight * 1 / rad2mas );

    // clock
    double clk_lin = obs.getStartTime() / 86400.;
    double clk_quad = clk_lin * clk_lin;
    if ( para1.CLK.estimate() && !para1.refClock ) {
        partialsPWL( Unknown::Type::CLK, -1, sta1 );
    }
    if ( para1.linear_clk && !para1.refClock ) {
        unsigned long idx1 = name2startIdx[Unknown::typeString( Unknown::Type::CLK_linear ) + sta1];
        A_( iobs, idx1 ) = -clk_lin;
    }
    if ( para1.quadratic_clk && !para1.refClock ) {
        unsigned long idx1 = name2startIdx[Unknown::typeString( Unknown::Type::CLK_quad ) + sta1];
        A_( iobs, idx1 ) = -clk_quad;
    }

    if ( para2.CLK.estimate() && !para2.refClock ) {
        partialsPWL( Unknown::Type::CLK, 1, sta2 );
    }
    if ( para2.linear_clk && !para2.refClock ) {
        unsigned long idx2 = name2startIdx[Unknown::typeString( Unknown::Type::CLK_linear ) + sta2];
        A_( iobs, idx2 ) = clk_lin;
    }
    if ( para2.quadratic_clk && !para2.refClock ) {
        unsigned long idx2 = name2startIdx[Unknown::typeString( Unknown::Type::CLK_quad ) + sta2];
        A_( iobs, idx2 ) = clk_quad;
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
        double val = ( tan( pv1.getEl() ) * sin( pv1.getEl() ) + 0.0032 ) * cos( pv1.getAz() );
        partialsPWL( Unknown::Type::NGR, val, sta1 );
    }
    if ( para2.NGR.estimate() ) {
        double val = ( tan( pv2.getEl() ) * sin( pv2.getEl() ) + 0.0032 ) * cos( pv2.getAz() );
        partialsPWL( Unknown::Type::NGR, val, sta2 );
    }

    // egr
    if ( para1.EGR.estimate() ) {
        double val = ( tan( pv1.getEl() ) * sin( pv1.getEl() ) + 0.0032 ) * sin( pv1.getAz() );
        partialsPWL( Unknown::Type::EGR, val, sta1 );
    }
    if ( para2.EGR.estimate() ) {
        double val = ( tan( pv2.getEl() ) * sin( pv2.getEl() ) + 0.0032 ) * sin( pv2.getAz() );
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
                tmp.refClock = true;
            }
            if ( refClock == name ) {
                tmp.refClock = true;
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
                }
            } else {
                for ( int i = 0; i < network_.getNSta(); ++i ) {
                    if ( network_.getStation( i ).getName() == name ) {
                        estimationParamStations_[i] = tmp;
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
        string flagLinClk = p.linear_clk ? "yes" : "no";
        string flagQuadClk = p.quadratic_clk ? "yes" : "no";
        of << boost::format( "| %-8s | %=5s | %=5s | %=7s | %=8s " ) % name % flagCoord % flagDatum % flagLinClk %
                  flagQuadClk;
        auto fn = [this]( const PWL &pwl ) {
            if ( pwl.estimate() ) {
                of << boost::format( "| %8.2f | %6.3f " ) % ( pwl.getInterval() / 60. ) % pwl.getConstraint();
            } else {
                of << boost::format( "| %=8s | %=6s " ) % "--" % "--";
            }
        };
        fn( p.CLK );
        fn( p.ZWD );
        fn( p.NGR );
        fn( p.EGR );
        of << "|\n";
    }
    of << "'-----------------------------------------------------------------------------------------------------------"
          "--------------------'\n";
}

void Solver::listUnknowns() {
    int i = 0;

    of << "\nList of estimated parameters\n";
    of << ".---------------------------------------------------.\n";
    for ( const auto &u : unknowns ) {
        of << boost::format( "| %5d %s \n" ) % i++ % u.toString();
    }
    of << "'---------------------------------------------------'\n";
}

unsigned long Solver::findStartIdxPWL( unsigned int time, unsigned long startIdx ) {
    unsigned long endIdx = startIdx + 1;
    auto it = unknowns.begin() + endIdx;
    while ( it != unknowns.end() && it->refTime < time ) {
        ++it;
    }
    return distance( unknowns.begin(), it ) - 1;
}
