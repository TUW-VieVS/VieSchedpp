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

#include "Simulator.h"

unsigned long VieVS::Simulator::nextId = 0;

using namespace std;
using namespace VieVS;
using namespace Eigen;

Simulator::Simulator( Output &sched, std::string path, std::string fname, int version )
    : VieVS_NamedObject( move( fname ), nextId++ ),
      xml_{sched.xml_},
      network_{std::move( sched.network_ )},
      sources_{std::move( sched.sources_ )},
      scans_{std::move( sched.scans_ )},
      obsModes_{sched.obsModes_},
      path_{std::move( path )},
      version_{version},
      simpara_{vector<SimPara>( network_.getNSta() )} {
    unsigned long seed = std::chrono::system_clock::now().time_since_epoch().count();
    generator_ = default_random_engine( seed );
}

void Simulator::start() {
    simWn();
    simClock();
    simTropo();
    calcO_C();
}

void Simulator::generateObsVector() {
    for ( auto &scan : scans_ ) {
        for ( auto &obs : scan.refObservations() ) {
            unsigned long staidx1 = *scan.findIdxOfStationId( obs.getStaid1() );
            unsigned long staidx2 = *scan.findIdxOfStationId( obs.getStaid2() );

            obs.setStartTime( scan.getTimes().getObservingTime( staidx1, staidx2, Timestamp::start ) );
            obs.setObservingTime( scan.getTimes().getObservingDuration( staidx1, staidx2 ) );
            obs_.push_back( obs );
        }
    }
    unsigned long nSta = network_.getNSta();
    unsigned long nScans = scans_.size();
    unsigned long nObs = obs_.size();
}


void Simulator::simWn() {
    vector<normal_distribution<double>> distributions;
    for ( int i = 0; i < network_.getNSta(); ++i ) {
        distributions.emplace_back( normal_distribution<double>( 0.0, simpara_[i].wn ) );
    }

    wn_ = Eigen::VectorXd( obs_.size() );
    for ( int i = 0; i < obs_.size(); ++i ) {
        const auto &obs = obs_[i];
        double wn = distributions[obs.getStaid1()]( generator_ ) + distributions[obs.getStaid2()]( generator_ );
        wn_( i ) = wn;
    }
}
void Simulator::simClock() {
    unsigned long nsta = network_.getNSta();

    // loop over all stations
    for ( int ista = 0; ista < nsta; ++ista ) {
        unsigned int refTime = 0;
        VectorXd rw = VectorXd::Zero( nsim );
        VectorXd irw = VectorXd::Zero( nsim );

        const auto &simpara = simpara_[ista];
        double phic_rw = simpara.clockASD * simpara.clockASD * simpara.clockDur * 60;
        double phic_irw = simpara.clockASD * simpara.clockASD / ( simpara.clockDur * 60 ) * 3;
        normal_distribution<double> wn_rw = normal_distribution<double>( 0.0, sqrt( phic_rw ) );
        auto wn_rw_f = [this, &wn_rw]() { return wn_rw( generator_ ); };
        normal_distribution<double> wn_irw = normal_distribution<double>( 0.0, sqrt( phic_irw ) );
        auto wn_irw_f = [this, &wn_irw]() { return wn_irw( generator_ ); };
        VectorXd v = VectorXd::Zero( nsim, 1 );

        Eigen::MatrixXd clk( nsim, scans_.size() );

        // loop over all scans
        for ( int iscan = 0; iscan < scans_.size(); ++iscan ) {
            const auto &scan = scans_[iscan];
            unsigned int startTime = scan.getTimes().getScanTime( Timestamp::start );
            auto dt = static_cast<double>( startTime - refTime );

            // for the first scan, set noise to zero
            if ( iscan == 0 ) {
                clk.col( iscan ).array() = 0;
                continue;
            }

            if ( dt != 0 ) {
                rw += VectorXd::NullaryExpr( nsim, wn_rw_f ) * sqrt( dt );

                VectorXd tmp = VectorXd::NullaryExpr( nsim, wn_irw_f ) * sqrt( dt );
                irw += v * dt + tmp / 2 * dt;
                v += tmp;
            }
            clk.col( iscan ) = rw + irw;

            refTime = startTime;
        }
        clk_.emplace_back( clk.transpose() );
    }
}

void Simulator::simTropo() {
    const unsigned long nsta = network_.getNSta();
    const double L23 = pow( 3e6, 2 / 3 );
    normal_distribution<double> normalDistribution = normal_distribution<double>( 0.0, 1.0 );

    auto normalDist = [this, &normalDistribution]() { return normalDistribution( generator_ ); };


    // loop over all stations
    for ( int staid = 0; staid < nsta; ++staid ) {
        const auto &simpara = simpara_[staid];

        const double Cn = simpara.tropo_Cn * 1e-7;
        const Eigen::Vector3d v( simpara.tropo_vn * 3600, simpara.tropo_ve * 3600, 0 );
        const double Cnall = Cn * Cn / 2 * 1e6 * simpara.tropo_dh * simpara.tropo_dh;

        const int nh = floor( simpara.tropo_H / simpara.tropo_dh );
        VectorXd rho4x = VectorXd( ( nh + 1 ) * ( nh + 1 ) );
        VectorXd z = VectorXd( ( nh + 1 ) * ( nh + 1 ) );
        VectorXd zs = VectorXd( ( nh + 1 ) * ( nh + 1 ) );
        int c = 0;
        for ( int i = 0; i < nh + 1; ++i ) {
            double zs_s = simpara.tropo_dh * i;
            for ( int j = 0; j < nh + 1; ++j ) {
                double z_s = simpara.tropo_dh * j;
                double rho4 = abs( z_s - zs_s );
                double tmp = pow( rho4, 2 / 3 );
                double rho4x_s = tmp / ( 1 + tmp / L23 );

                rho4x( c ) = rho4x_s;
                z( c ) = z_s;
                zs( c ) = zs_s;
            }
        }


        int segments = ceil( TimeSystem::duration / ( simpara.tropo_dhseg * 3600 ) );
        VectorXd tn = VectorXd::Zero( segments );
        vector<PointingVector> pvs;
        for ( const auto &any : scans_ ) {
            if ( any.findIdxOfStationId( staid ).is_initialized() ) {
                PointingVector pv = any.getPointingVector( *any.findIdxOfStationId( staid ) );
                ++tn( pv.getTime() / simpara.tropo_dhseg );
                pvs.push_back( pv );
            }
        }

        auto calcR = []( const PointingVector &pv ) {
            Vector3d r( cos( pv.getAz() ) / tan( pv.getEl() ), sin( pv.getAz() ) / tan( pv.getEl() ), 1 );
            return r;
        };

        vector<MatrixXd> Ds;
        MatrixXd C11 = MatrixXd::Zero( 0, 0 );
        for ( int i2 = 0; i2 < segments - 1; ++i2 ) {
            int i1 = i2 - 1;
            int num1;
            i1 >= 0 ? num1 = tn( i1 ) : num1 = 0;
            int num2 = tn( i2 );
            int num3 = num1 + num2;

            int k = tn.head( i2 ).sum();
            MatrixXd C( num3, num3 );
            C.topLeftCorner( num1, num1 ) = C11;

            for ( int i = 0; i < num3; ++i ) {
                const PointingVector &pv1 = pvs[k + i];
                double t1_h = pv1.getTime() / 3600.;

                auto r1 = calcR( pv1 );
                auto rz1 = r1 * z.transpose();

                MatrixXd dd1 = rz1;
                dd1.row( 2 ) - zs.transpose();
                dd1 = dd1.colwise() + v * t1_h;

                VectorXd rho1 = ( dd1.array() * dd1.array() ).colwise().sum().pow( 1 / 3 );
                VectorXd rho1x = rho1.array() / ( 1 + rho1.array() / L23 );

                for ( int j = max( i, num1 ); j < num3; ++j ) {
                    const PointingVector &pv2 = pvs[k + j];

                    double t2_h = pv2.getTime() / 3600.;
                    Vector3d r2 = calcR( pv2 );
                    MatrixXd rz2 = r2 * z.transpose();
                    rz2.row( 2 ) -= zs;

                    MatrixXd dd2 = rz2;
                    dd2.row( 2 ) - zs.transpose();
                    dd2 = dd2.colwise() + v * t2_h;

                    auto rho2 = ( dd2.array() * dd2.array() ).colwise().sum().pow( 1 / 3 );


                    double dt12_h = t2_h - t1_h;
                    VectorXd rzs2 = r2 * zs.transpose();

                    MatrixXd dd3 = rz1 - rzs2;
                    dd2 = dd2.colwise() - v * dt12_h;
                    VectorXd rho3 = ( dd3.array() * dd3.array() ).colwise().sum().pow( 1 / 3 );

                    double result = ( rho1x.array() + rho2.array() / ( 1 + rho2.array() / L23 ) -
                                      rho3.array() / ( 1 + rho3.array() / L23 ) - rho4x.array() )
                                        .sum();
                    C( i, j ) = Cnall * result;
                }
            }

            Ds.emplace_back( C.llt().matrixU().transpose() );
            C11 = C.bottomRightCorner( num2, num2 );
        }


        MatrixXd l( pvs.size(), nsim );
        for ( int i2 = 0; i2 < segments - 1; ++i2 ) {
            int i1 = i2 - 1;
            int num1;
            i1 >= 0 ? num1 = tn( i1 ) : num1 = 0;
            int num2 = tn( i2 );

            const MatrixXd &D = Ds[i2];
            MatrixXd D11 = D.topLeftCorner( num1, num1 );
            MatrixXd D21 = D.bottomLeftCorner( num2, num1 );
            MatrixXd D22 = D.bottomRightCorner( num2, num2 );
            MatrixXd x = MatrixXd::NullaryExpr( num2, nsim, normalDist );
            MatrixXd l1;
            if ( D21.size() > 0 ) {
                l1 = D21 * D11.triangularView<Lower>().solve( l1 ) + D22 * x;
            } else {
                l1 = D22 * x;
            }

            int k = tn.head( i2 ).sum();
            l.block( k, 0, num2, nsim ) = l1;
        }
        double mfw = 1;
        tropo_.emplace_back( ( l.array() + simpara.tropo_wzd0 ) * mfw * 1e-3 * speedOfLight );
    }
}
void Simulator::calcO_C() {}
