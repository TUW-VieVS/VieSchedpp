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
Solver::Solver( Simulator &simulator, std::string fname )
    : VieVS_NamedObject( move( fname ), nextId++ ),
      xml_{simulator.xml_},
      network_{std::move( simulator.network_ )},
      sources_{std::move( simulator.sources_ )},
      scans_{std::move( simulator.scans_ )},
      of{std::move( simulator.of )} {
    dWdx_ << 0, 0, -1, 0, 0, 0, 1, 0, 0;
    dWdy_ << 0, 0, 0, 0, 0, 1, 0, -1, 0;
}

void Solver::start() {
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
            // partials stations
            partials( obs, t2c, dQdx, dQdy, dQdut, dQdX, dQdY );
        }
    }
}

void Solver::partials( const Observation &obs, const Matrix3d &t2c, const Matrix3d &dQdx, const Matrix3d &dQdy,
                       const Matrix3d &dQdut, const Matrix3d &dQdX, const Matrix3d &dQdY ) {
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
    double x1 = -B( 0 );
    double y1 = -B( 1 );
    double z1 = -B( 2 );
    double x2 = B( 0 );
    double y2 = B( 1 );
    double z2 = B( 2 );

    // EOP
    Vector3d b_trs( sta2.getPosition().getX() - sta1.getPosition().getX(),
                    sta2.getPosition().getY() - sta1.getPosition().getY(),
                    sta2.getPosition().getZ() - sta1.getPosition().getZ() );

    double pdx = K.dot( dQdx * b_trs ) / speedOfLight;
    double pdy = K.dot( dQdy * b_trs ) / speedOfLight;
    double put = K.dot( dQdut * b_trs ) / speedOfLight;
    double pdX = K.dot( dQdX * b_trs ) / speedOfLight;
    double pdY = K.dot( dQdY * b_trs ) / speedOfLight;

    // sources
    double sid = sin( src.getDe() );
    double cod = cos( src.getDe() );
    double sir = sin( src.getRa() );
    double cor = cos( src.getRa() );

    Vector3d drqdra( -cod * sir, cod * cor, 0 );
    Vector3d drqdde( -sid * cor, sid * sir, cod );

    double parra = drqdra.dot( M ) * pi / 180 / 3600000 * 100;
    double parde = drqdde.dot( M ) * pi / 180 / 3600000 * 100;
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