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
 * File:   skyCoverage.cpp
 * Author: mschartn
 *
 * Created on June 29, 2017, 11:28 AM
 */

#include "SkyCoverage.h"


using namespace std;
using namespace VieVS;

unsigned long VieVS::SkyCoverage::nextId = 0;

SkyCoverage::SkyCoverage( string name, double maxInfluenceTime, double maxInfluenceDistance,
                          Interpolation interpolationTime, Interpolation interpolationDistance )
    : VieVS_NamedObject( std::move( name ), nextId++ ),
      maxInfluenceTime{ maxInfluenceTime },
      maxInfluenceDistance{ maxInfluenceDistance * deg2rad },
      interpolationTime{ interpolationTime },
      interpolationDistance{ interpolationDistance } {}


double SkyCoverage::calcScore( const PointingVector &pv ) const {
    double score = 1;
    for ( const auto &pv_old : pointingVectors_ ) {
        if ( pv_old.getTime() > pv.getTime() ) {
            continue;
        }
        double thisScore = scorePerPointingVector( pv, pv_old );
        if ( thisScore < score ) {
            score = thisScore;
        }
    }

    return score;
}


void SkyCoverage::update( const PointingVector &pv ) noexcept { pointingVectors_.push_back( pv ); }


double SkyCoverage::scorePerPointingVector( const PointingVector &pv_new,
                                            const PointingVector &pv_old ) const noexcept {
    long deltaTime = (long)pv_new.getTime() - (long)pv_old.getTime();
    if ( deltaTime > maxInfluenceTime ) {
        return 1;
    }

    if ( abs( pv_new.getEl() - pv_old.getEl() ) > maxInfluenceDistance ) {
        return 1;
    }

    float distance = LookupTable::angularDistance( pv_new, pv_old );

    if ( distance > maxInfluenceDistance ) {
        return 1;
    }

    double saturationDistance;
    switch ( interpolationDistance ) {
        case Interpolation::constant: {
            saturationDistance = 1;
            break;
        }
        case Interpolation::linear: {
            saturationDistance = 1 - distance / maxInfluenceDistance;
            break;
        }
        case Interpolation::cosine: {
            saturationDistance = .5 + .5 * ( LookupTable::cosLookup( distance * pi / maxInfluenceDistance ) );
            break;
        }
        default:
            break;
    }

    double saturationTime;
    switch ( interpolationTime ) {
        case Interpolation::constant: {
            saturationTime = 1;
            break;
        }
        case Interpolation::linear: {
            saturationTime = 1 - deltaTime / maxInfluenceTime;
            break;
        }
        case Interpolation::cosine: {
            saturationTime = .5 + .5 * ( LookupTable::cosLookup( deltaTime * pi / maxInfluenceTime ) );
            break;
        }
        default:
            break;
    }

    return 1 - ( saturationDistance * saturationTime );
}


void SkyCoverage::clearObservations() { pointingVectors_.clear(); }


void SkyCoverage::calculateSkyCoverageScores() {
    std::sort(
        pointingVectors_.begin(), pointingVectors_.end(),
        []( const PointingVector &left, const PointingVector &right ) { return left.getTime() < right.getTime(); } );

    a13m15_ = skyCoverageScore_13( 900 );
    a25m15_ = skyCoverageScore_25( 900 );
    a37m15_ = skyCoverageScore_37( 900 );
    a13m30_ = skyCoverageScore_13( 1800 );
    a25m30_ = skyCoverageScore_25( 1800 );
    a37m30_ = skyCoverageScore_37( 1800 );
    a13m60_ = skyCoverageScore_13( 3600 );
    a25m60_ = skyCoverageScore_25( 3600 );
    a37m60_ = skyCoverageScore_37( 3600 );
}


double SkyCoverage::skyCoverageScore_13( unsigned int deltaTime ) const {
    double total_score = 0;

    if ( pointingVectors_.empty() ) {
        return 0;
    }

    auto thisPv = pointingVectors_.begin();
    int c = 0;
    for ( unsigned int startTime = 0; startTime < TimeSystem::duration; startTime += deltaTime / 2 ) {
        unsigned int endTime = startTime += deltaTime;

        set<int> areas1;
        set<int> areas2;
        while ( thisPv != pointingVectors_.end() && thisPv->getTime() < endTime ) {
            areas1.insert( areaIndex13_v1( *thisPv ) );
            areas2.insert( areaIndex13_v2( *thisPv ) );

            ++thisPv;
        }

        double score = static_cast<double>( areas1.size() ) / 2.0 +
                       static_cast<double>( areas2.size() ) / 2.0;  // average of both distributions
        total_score += score / 13.0;                                // normalize score
        ++c;
    }
    total_score /= c;

    return total_score;
}


double SkyCoverage::skyCoverageScore_25( unsigned int deltaTime ) const {
    double total_score = 0;

    if ( pointingVectors_.empty() ) {
        return 0;
    }

    auto thisPv = pointingVectors_.begin();
    int c = 0;
    for ( unsigned int startTime = 0; startTime < TimeSystem::duration; startTime += deltaTime / 2 ) {
        unsigned int endTime = startTime += deltaTime;

        set<int> areas1;
        set<int> areas2;
        while ( thisPv != pointingVectors_.end() && thisPv->getTime() < endTime ) {
            areas1.insert( areaIndex25_v1( *thisPv ) );
            areas2.insert( areaIndex25_v2( *thisPv ) );

            ++thisPv;
        }

        double score = static_cast<double>( areas1.size() ) / 2.0 +
                       static_cast<double>( areas2.size() ) / 2.0;  // average of both distributions
        total_score += score / 25.0;                                // normalize score
        ++c;
    }
    total_score /= c;

    return total_score;
}


double SkyCoverage::skyCoverageScore_37( unsigned int deltaTime ) const {
    double total_score = 0;

    if ( pointingVectors_.empty() ) {
        return 0;
    }

    auto thisPv = pointingVectors_.begin();
    int c = 0;
    for ( unsigned int startTime = 0; startTime < TimeSystem::duration; startTime += deltaTime / 2 ) {
        unsigned int endTime = startTime += deltaTime;

        set<int> areas1;
        set<int> areas2;
        while ( thisPv != pointingVectors_.end() && thisPv->getTime() < endTime ) {
            areas1.insert( areaIndex37_v1( *thisPv ) );
            areas2.insert( areaIndex37_v2( *thisPv ) );

            ++thisPv;
        }

        double score = static_cast<double>( areas1.size() ) / 2.0 +
                       static_cast<double>( areas2.size() ) / 2.0;  // average of both distributions
        total_score += score / 37.0;                                // normalize score
        ++c;
    }
    total_score /= c;

    return total_score;
}


int SkyCoverage::areaIndex13_v1( const PointingVector &pv ) noexcept {
    constexpr double el_space = halfpi / 2.;

    int row = static_cast<int>( floorl( pv.getEl() / el_space ) );
    int idx;
    switch ( row ) {
        case 0: {
            double n = 9;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = col;
            break;
        }

        default: {
            double n = 4;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = 9 + col;
            break;
        }
    }

    return idx;
}


int SkyCoverage::areaIndex13_v2( const PointingVector &pv ) noexcept {
    constexpr double el_space = halfpi / 2.75;

    int row = static_cast<int>( floorl( pv.getEl() / el_space ) );
    int idx;
    switch ( row ) {
        case 0: {
            double n = 8;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = col;
            break;
        }

        case 1: {
            double n = 4;
            double az_space = twopi / n;
            int col = static_cast<int>( floorl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = 8 + col;
            break;
        }

        default: {
            idx = 12;
            break;
        }
    }

    return idx;
}


int SkyCoverage::areaIndex25_v1( const PointingVector &pv ) noexcept {
    constexpr double el_space = halfpi / 3.;

    int row = static_cast<int>( floorl( pv.getEl() / el_space ) );

    int idx;
    switch ( row ) {
        case 0: {
            double n = 13;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = col;
            break;
        }

        case 1: {
            double n = 9;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = 13 + col;
            break;
        }

        default: {
            double n = 3;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = 22 + col;
            break;
        }
    }

    return idx;
}


int SkyCoverage::areaIndex25_v2( const PointingVector &pv ) noexcept {
    constexpr double el_space = halfpi / 3.75;

    int row = static_cast<int>( floorl( pv.getEl() / el_space ) );
    int idx;
    switch ( row ) {
        case 0: {
            double n = 12;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = col;
            break;
        }

        case 1: {
            double n = 8;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = 12 + col;
            break;
        }

        case 2: {
            double n = 4;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = 20 + col;
            break;
        }

        default: {
            idx = 24;
            break;
        }
    }

    return idx;
}


int SkyCoverage::areaIndex37_v1( const PointingVector &pv ) noexcept {
    constexpr double el_space = halfpi / 4.;

    int row = static_cast<int>( floorl( pv.getEl() / el_space ) );

    int idx;
    switch ( row ) {
        case 0: {
            double n = 14;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = col;
            break;
        }

        case 1: {
            double n = 12;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = 14 + col;
            break;
        }

        case 2: {
            double n = 8;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = 26 + col;
            break;
        }

        default: {
            double n = 3;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = 34 + col;
            break;
        }
    }

    return idx;
}


int SkyCoverage::areaIndex37_v2( const PointingVector &pv ) noexcept {
    constexpr double el_space = halfpi / 4.75;

    int row = static_cast<int>( floorl( pv.getEl() / el_space ) );
    int idx;
    switch ( row ) {
        case 0: {
            double n = 13;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = col;
            break;
        }

        case 1: {
            double n = 12;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = 13 + col;
            break;
        }

        case 2: {
            double n = 7;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = 25 + col;
            break;
        }

        case 3: {
            double n = 4;
            double az_space = twopi / n;
            int col = static_cast<int>( roundl( util::wrap2twoPi( pv.getAz() ) / az_space ) );
            if ( col > n - 1 ) {
                col = 0;
            }
            idx = 32 + col;
            break;
        }

        default: {
            idx = 36;
            break;
        }
    }

    return idx;
}

void SkyCoverage::generateDebuggingFiles( const std::string &filename, const std::string &stations ) const {
    string stas = boost::trim_copy( stations );
    std::vector<double> azs;
    std::vector<double> els;
    for ( double az = 0; az <= 360; az += 10 ) {
        for ( double el = 0; el <= 90; el += 5 ) {
            azs.push_back( az );
            els.push_back( el );
        }
    }
    unsigned long n = azs.size();

    PointingVector pv( 0, 0 );
    for ( int i = 0; i < TimeSystem::duration; i += 60 ) {
        string fname = ( boost::format( "%s_%s_%04d.sky" ) % filename % stas % i ).str();
        ofstream of( fname );
        of << "az,el,score\n";
        pv.setTime( i );
        for ( int j = 0; j < n; ++j ) {
            double az = azs[j];
            double el = els[j];
            pv.setAz( az * deg2rad );
            pv.setEl( el * deg2rad );
            double score = calcScore( pv );
            of << az << "," << el << "," << score << "\n";
        }
        of.close();

        string pfname = ( boost::format( "%s_%s_%04d.psky" ) % filename % stas % i ).str();
        ofstream pof( pfname );
        pof << "az,el,dt\n";
        for ( const auto &any : pointingVectors_ ) {
            if ( any.getTime() < i ) {
                double score = 1 - ( i - any.getTime() ) / maxInfluenceTime;
                if ( score < 0 ) {
                    continue;
                }
                pof << any.getAz() * rad2deg << "," << any.getEl() * rad2deg << "," << score << "\n";
            }
        }
        pof.close();
    }
    string sumname = ( boost::format( "%s_%s.allsky" ) % filename % stas ).str();
    ofstream sum( sumname );
    sum << "time,az,el\n";
    for ( const auto &any : pointingVectors_ ) {
        sum << TimeSystem::time2string( any.getTime() ) << "," << ( any.getAz() * rad2deg ) << ","
            << ( any.getEl() * rad2deg ) << "\n";
    }
}
