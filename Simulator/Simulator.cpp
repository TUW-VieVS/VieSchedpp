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

Simulator::Simulator(Output &output)
        : VieVS_NamedObject(output.getName(), nextId++),
          xml_{ output.xml_ },
          network_{ std::move( output.network_ ) },
          sourceList_{ std::move( output.sourceList_ ) },
          scans_{ std::move( output.scans_ ) },
          obsModes_{ output.obsModes_ },
          path_{std::move(output.path_)},
          version_{output.version_},
          multiSchedulingParameters_{ std::move( output.multiSchedulingParameters_ ) },
          simpara_{ vector<SimPara>( network_.getNSta() ) } {
    auto tmp = xml_.get_optional<int>( "VieSchedpp.simulator.seed" );
    if ( tmp.is_initialized() ) {
        seed_ = *tmp;
    } else {
        seed_ = std::chrono::system_clock::now().time_since_epoch().count();
    }

    generator_ = default_random_engine( seed_ );

    string file = path_;
    file.append( getName() ).append( "_simulator.txt" );
    of = ofstream( file );
}


void Simulator::start() {
    if ( scans_.empty() ) {
        return;
    }
    string prefix = util::version2prefix(version_);
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL(info) << prefix << "start simulator";
#else
    cout << "[info] " + prefix + "start simulator\n";
#endif

    setup();

    parameterSummary();

    if ( simClock_ ) {
        of << "simulation clocks:" << endl;
        auto start = std::chrono::high_resolution_clock::now();
        simClock();
        auto finish = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>( finish - start );
        long long int usec = microseconds.count();
        of << "total " << util::milliseconds2string( usec ) << endl << endl;
    } else {
        simClockDummy();
    }
    if ( simTropo_ ) {
        of << "simulation troposphere:" << endl;
        auto start = std::chrono::high_resolution_clock::now();
        simTropo();
        auto finish = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>( finish - start );
        long long int usec = microseconds.count();
        of << "total " << util::milliseconds2string( usec ) << endl << endl;
    } else {
        simTropoDummy();
    }
    calcO_C();
}

void Simulator::simClock() {
    unsigned long nsta = network_.getNSta();

    // loop over all stations
    for ( int ista = 0; ista < nsta; ++ista ) {
        auto start = std::chrono::high_resolution_clock::now();
        const auto &simpara = simpara_[ista];
        if ( simpara.clockASD < 1e-20 ) {
            clk_.emplace_back( MatrixXd::Zero( scans_.size(), nsim ) );
            continue;
        }
        of << boost::format( "    %-8s " ) % network_.getStation( ista ).getName();

        unsigned int refTime = 0;
        VectorXd rw = VectorXd::Zero( nsim );
        VectorXd irw = VectorXd::Zero( nsim );

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
            unsigned int startTime = scan.getTimes().getObservingTime();
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
        auto finish = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>( finish - start );
        long long int usec = microseconds.count();
        of << "(" << util::milliseconds2string( usec, true ) << ")" << endl;
    }
}

void Simulator::simTropo() {
    const unsigned long nsta = network_.getNSta();
    const double L23 = pow( 3e6, 2.0 / 3.0 );
    normal_distribution<double> normalDistribution = normal_distribution<double>( 0.0, 1.0 );

    auto normalDist = [this, &normalDistribution]() { return normalDistribution( generator_ ); };


    // loop over all stations
    for ( int staid = 0; staid < nsta; ++staid ) {
        auto start = std::chrono::high_resolution_clock::now();
        const auto &simpara = simpara_[staid];

        int segments = ceil( TimeSystem::duration / ( simpara.tropo_dhseg * 3600 ) );
        if ( segments == 1 ) {
            segments = 2;
        }
        VectorXd tn = VectorXd::Zero( segments );
        vector<PointingVector> pvs;

        unsigned int c = 0;
        for ( const auto &any : scans_ ) {
            if ( any.findIdxOfStationId( staid ).is_initialized() ) {
                PointingVector pv = any.getPointingVector( *any.findIdxOfStationId( staid ) );
                ++tn( pv.getTime() / ( simpara.tropo_dhseg * 3600 ) );
                pvs.push_back( pv );
                ++c;
            }
        }

        VectorXd mfw = VectorXd::Zero( c );
        c = 0;
        for ( const auto &any : scans_ ) {
            if ( any.findIdxOfStationId( staid ).is_initialized() ) {
                PointingVector pv = any.getPointingVector( *any.findIdxOfStationId( staid ) );
                mfw( c ) = 1 / sin( pv.getEl() );
                ++c;
            }
        }

        if ( simpara.tropo_Cn < 1e-20 ) {
            tropo_.emplace_back( MatrixXd::Zero( pvs.size(), nsim ) );
            continue;
        }
        of << boost::format( "    %-8s " ) % network_.getStation( staid ).getName();
        const double Cn = simpara.tropo_Cn;
        const Eigen::Vector3d v( simpara.tropo_vn * 3600, simpara.tropo_ve * 3600, 0 );
        const double Cnall = Cn * Cn / 2 * 1e6 * simpara.tropo_dh * simpara.tropo_dh;

        const int nh = floor( simpara.tropo_H / simpara.tropo_dh );
        VectorXd rho4x = VectorXd( ( nh + 1 ) * ( nh + 1 ) );
        VectorXd z = VectorXd( ( nh + 1 ) * ( nh + 1 ) );
        VectorXd zs = VectorXd( ( nh + 1 ) * ( nh + 1 ) );
        c = 0;
        for ( int i = 0; i < nh + 1; ++i ) {
            double zs_s = simpara.tropo_dh * i;
            for ( int j = 0; j < nh + 1; ++j ) {
                double z_s = simpara.tropo_dh * j;
                double rho4 = abs( z_s - zs_s );
                double tmp = cbrt( rho4 );
                tmp = tmp * tmp;
                double rho4x_s = tmp / ( 1 + tmp / L23 );

                rho4x( c ) = rho4x_s;
                z( c ) = z_s;
                zs( c ) = zs_s;
                ++c;
            }
        }


        auto calcR = []( const PointingVector &pv ) {
            Vector3d r( cos( pv.getAz() ) / tan( pv.getEl() ), sin( pv.getAz() ) / tan( pv.getEl() ), 1 );
            return r;
        };

        vector<MatrixXd> Ds;
        MatrixXd C11 = MatrixXd::Zero( 0, 0 );
        for ( int i1 = 0; i1 < segments - 1; ++i1 ) {
            int i2 = i1 + 1;
            int num1 = tn( i1 );
            int num2 = tn( i2 );
            int k = tn.head( i1 ).sum();
            if ( i2 == 1 ) {
                num2 += num1;
                num1 = 0;
                k = 0;
            }
            int num3 = num1 + num2;

            MatrixXd C = MatrixXd::Zero( num3, num3 );
            C.topLeftCorner( num1, num1 ) = C11;

            for ( int i = 0; i < num3; ++i ) {
                const PointingVector &pv1 = pvs[k + i];
                double t1_h = pv1.getTime() / 3600.;

                Vector3d r1 = calcR( pv1 );
                MatrixXd rz1 = r1 * z.transpose();

                MatrixXd dd1 = rz1;
                dd1.row( 2 ) -= zs;
                dd1 = dd1.colwise() + v * t1_h;

                VectorXd rho1 =
                    ( dd1.array() * dd1.array() ).colwise().sum().unaryExpr( []( double d ) { return cbrt( d ); } );
                VectorXd rho1x = rho1.array() / ( 1 + rho1.array() / L23 );

                for ( int j = max( i, num1 ); j < num3; ++j ) {
                    const PointingVector &pv2 = pvs[k + j];

                    double t2_h = pv2.getTime() / 3600.;
                    Vector3d r2 = calcR( pv2 );
                    MatrixXd rz2 = r2 * z.transpose();

                    MatrixXd dd2 = rz2;
                    dd2.row( 2 ) -= zs;
                    dd2 = dd2.colwise() + v * t2_h;

                    VectorXd rho2 =
                        ( dd2.array() * dd2.array() ).colwise().sum().unaryExpr( []( double d ) { return cbrt( d ); } );

                    double dt12_h = t2_h - t1_h;
                    MatrixXd rzs2 = r2 * zs.transpose();

                    MatrixXd dd3 = rz1 - rzs2;
                    dd3 = dd3.colwise() - v * dt12_h;
                    VectorXd rho3 =
                        ( dd3.array() * dd3.array() ).colwise().sum().unaryExpr( []( double d ) { return cbrt( d ); } );

                    double result = ( rho1x.array() + rho2.array() / ( 1 + rho2.array() / L23 ) -
                                      rho3.array() / ( 1 + rho3.array() / L23 ) - rho4x.array() )
                                        .sum();
                    C( j, i ) = Cnall * result;
                }
            }
            Ds.emplace_back( C.llt().matrixL() );
            // cout << Ds.back() << endl << endl;
            C11 = C.bottomRightCorner( tn( i2 ), tn( i2 ) );
        }


        MatrixXd l = MatrixXd::Zero( pvs.size(), nsim );
        MatrixXd l1;
        for ( int i1 = 0; i1 < segments - 1; ++i1 ) {
            int i2 = i1 + 1;
            int num1 = tn( i1 );
            int num2 = tn( i2 );
            int k = tn.head( i2 ).sum();
            if ( i1 == 0 ) {
                num2 += num1;
                num1 = 0;
                k = 0;
            }

            const MatrixXd &D = Ds[i1];
            MatrixXd D11 = D.topLeftCorner( num1, num1 );
            //            cout << "D11" << endl << D11 << endl << endl;
            MatrixXd D21 = D.bottomLeftCorner( num2, num1 );
            //            cout << "D21" << endl << D21 << endl << endl;
            MatrixXd D22 = D.bottomRightCorner( num2, num2 );
            //            cout << "D22" << endl << D22 << endl << endl;
            MatrixXd x = MatrixXd::NullaryExpr( num2, nsim, normalDist );

            if ( i1 == 0 ) {
                MatrixXd tmp = D22 * x;
                l.block( k, 0, num2, nsim ) = tmp;
                l1 = tmp.block( tn( i1 ), 0, tn( i2 ), nsim );
            } else {
                l1 = D21 * D11.triangularView<Lower>().solve( l1 ) + D22 * x;
                l.block( k, 0, num2, nsim ) = l1;
            }
        }
        tropo_.emplace_back( ( l.array() + simpara.tropo_wzd0 ).array().colwise() * mfw.array() * 1e-3 / speedOfLight );
        auto finish = std::chrono::high_resolution_clock::now();
        auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>( finish - start );
        long long int usec = microseconds.count();
        of << "(" << util::milliseconds2string( usec, true ) << ")" << endl;
    }
}

void Simulator::calcO_C() {
    int nobs = 0;
    for ( const Scan &scan : scans_ ) {
        for ( const Observation &obs : scan.getObservations() ) {
            ++nobs;
        }
    }

    obs_minus_com_ = Eigen::MatrixXd( nobs, nsim );
    P_ = Eigen::VectorXd( nobs );

    double constexpr constNoise = ( 0.005 / speedOfLight ) * ( 0.005 / speedOfLight );
    unsigned int iobs = 0;
    vector<int> tropoCounter( network_.getNSta(), -1 );

    for ( int iscan = 0; iscan < scans_.size(); ++iscan ) {
        const Scan &scan = scans_[iscan];

        for ( int ista = 0; ista < network_.getNSta(); ++ista ) {
            if ( scan.findIdxOfStationId( ista ).is_initialized() ) {
                ++tropoCounter[ista];
            }
        }

        for ( const Observation &obs : scan.getObservations() ) {
            unsigned long staid1 = obs.getStaid1();
            unsigned long staid2 = obs.getStaid2();
            unsigned long tropoId_staid1 = tropoCounter[staid1];
            unsigned long tropoId_staid2 = tropoCounter[staid2];
            const auto &p1 = simpara_[staid1];
            const auto &p2 = simpara_[staid2];


            VectorXd wn1;
            VectorXd wn2;
            if ( simWn_ ) {
                if ( simpara_[staid1].wn > 1e-10 ) {
                    auto dist1 = normal_distribution<double>( 0.0, simpara_[staid1].wn * 1e-12 );
                    auto normalDist1 = [this, &dist1]() { return dist1( generator_ ); };
                    wn1 = VectorXd::NullaryExpr( nsim, normalDist1 );
                } else {
                    wn1 = VectorXd::Zero( nsim );
                }

                if ( simpara_[staid2].wn > 1e-10 ) {
                    auto dist2 = normal_distribution<double>( 0.0, simpara_[staid2].wn * 1e-12 );
                    auto normalDist2 = [this, &dist2]() { return dist2( generator_ ); };
                    wn2 = VectorXd::NullaryExpr( nsim, normalDist2 );
                } else {
                    wn2 = VectorXd::Zero( nsim );
                }
            } else {
                wn1 = VectorXd::Zero( nsim );
                wn2 = VectorXd::Zero( nsim );
            }

            obs_minus_com_.row( iobs ) = wn2.transpose() - wn1.transpose() + clk_[staid2].row( iscan ) -
                                         clk_[staid1].row( iscan ) + tropo_[staid2].row( tropoId_staid2 ) -
                                         tropo_[staid1].row( tropoId_staid1 );

            double varNoise = ( p1.wn * p1.wn + p2.wn * p2.wn ) * 1e-24;
            double sigma = 1 / ( ( constNoise + varNoise ) * speedOfLight * speedOfLight * 100 * 100 );
            P_( iobs ) = sigma;
            ++iobs;
        }
    }
}
void Simulator::simClockDummy() {
    unsigned long nsta = network_.getNSta();
    for ( int ista = 0; ista < nsta; ++ista ) {
        clk_.emplace_back( MatrixXd::Zero( scans_.size(), nsim ) );
    }
}

void Simulator::simTropoDummy() {
    const unsigned long nsta = network_.getNSta();
    for ( int staid = 0; staid < nsta; ++staid ) {
        int counter = 0;
        for ( const auto &any : scans_ ) {
            if ( any.findIdxOfStationId( staid ).is_initialized() ) {
                ++counter;
            }
        }
        tropo_.emplace_back( MatrixXd::Zero( counter, nsim ) );
    }
}
void Simulator::setup() {
    of << "setup simulator:\n";
    unsigned long nsta = network_.getNSta();
    const boost::property_tree::ptree &tree = xml_.get_child( "VieSchedpp.simulator" );
    nsim = tree.get( "number_of_simulations", 1000 );
    vector<SimPara> simparas;
    vector<string> names;
    bool all = false;
    for ( const auto &any : tree ) {
        if ( any.first == "station" ) {
            string name = any.second.get( "<xmlattr>.name", "" );
            SimPara tmp;
            tmp.fromXML( any.second );

            simparas.push_back( tmp );
            names.emplace_back( name );
            if ( name == "__all__" ) {
                all = true;
            }
        }
    }
    if ( all ) {
        for ( int i = 0; i < nsta; ++i ) {
            simpara_[i] = simparas[0];
        }
    } else {
        for ( int staid = 0; staid < nsta; ++staid ) {
            const Station &station = network_.getStation( staid );
            const string &staName = station.getName();
            auto p_found = find( names.begin(), names.end(), staName );
            if ( p_found == names.end() ) {
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( error ) << "station: " << staName << " not found in simulator parameters";
#endif
                cout << "[error] station: " << staName << " not found in simulator parameters";
                continue;
            }
            int idx = distance( names.begin(), p_found );

            simpara_[staid] = simparas[idx];
        }
    }
}
void Simulator::parameterSummary() {
    of << "random number generator seed: " << seed_ << endl;
    of << "number of simulations: " << nsim << endl;

    of << boost::format( ".%|87T-|.\n" );
    of << "|   NAME   |    wn  |     ASD       @   |    Cn      H    dH    dt    ve    vn   wzwd0 |\n"
          "|          |   [ps] |     [s]     [min] |  [m^-1/3] [m]   [m]  [h]  [m/s] [m/s]   [mm] |\n"
          "|----------|--------|-------------------|----------------------------------------------|\n";
    for ( int i = 0; i < network_.getNSta(); ++i ) {
        of << boost::format( "| %-8s %s \n" ) % network_.getStation( i ).getName() % simpara_[i].toString();
    }
    of << boost::format( "'%|87T-|'\n" );
    of << endl;
}

void Simulator::SimPara::fromXML( const boost::property_tree::ptree &tree ) {
    wn = tree.get( "wn", 25. );

    clockASD = tree.get( "clockASD", 1.0 ) * 1e-14;
    clockDur = tree.get( "clockDur", 50. );

    tropo_Cn = tree.get( "tropo_Cn", 1.8 ) * 1e-7;
    tropo_H = tree.get( "tropo_H", 2000. );
    tropo_dh = tree.get( "tropo_dH", 200. );
    tropo_dhseg = tree.get( "tropo_dHseg", 2. );
    tropo_ve = tree.get( "tropo_ve", 8. );
    tropo_vn = tree.get( "tropo_vn", 0. );
    tropo_wzd0 = tree.get( "tropo_wzd0", 150. );
}
