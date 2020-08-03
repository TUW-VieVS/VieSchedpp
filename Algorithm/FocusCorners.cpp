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

//
// Created by mschartn on 08.11.19.
//

#include "FocusCorners.h"

using namespace VieVS;
using namespace std;

bool VieVS::FocusCorners::flag = false;
bool thread_local VieVS::FocusCorners::startFocusCorner = false;
std::vector<double> thread_local VieVS::FocusCorners::lastCornerAzimuth = std::vector<double>();
std::vector<std::pair<int, double>> thread_local VieVS::FocusCorners::backupWeight =
    std::vector<std::pair<int, double>>();
unsigned int thread_local VieVS::FocusCorners::nextStart = numeric_limits<unsigned int>::max();
unsigned int thread_local VieVS::FocusCorners::interval = 900;
std::vector<int> thread_local VieVS::FocusCorners::staid2groupid = std::vector<int>();


void VieVS::FocusCorners::initialize( const Network &network, ofstream &of ) {
    unsigned long nsta = network.getNSta();
    lastCornerAzimuth = std::vector<double>( nsta, std::numeric_limits<double>::quiet_NaN() );
    staid2groupid = std::vector<int>( nsta, 0 );
    FocusCorners::startFocusCorner = true;
    FocusCorners::nextStart = 0;

    auto getDxy = []( const Station &sta1, const Station &sta2 ) {
        double dx = sta2.getPosition()->getX() - sta1.getPosition()->getX();
        double dy = sta2.getPosition()->getY() - sta1.getPosition()->getY();
        return sqrt( dx * dx + dy * dy );
    };

    vector<double> dxys;
    for ( const auto &bl : network.getBaselines() ) {
        const Station &sta1 = network.getStation( bl.getStaid1() );
        const Station &sta2 = network.getStation( bl.getStaid2() );
        dxys.push_back( getDxy( sta1, sta2 ) );
    }

    unsigned long maxBlid = distance( dxys.begin(), max_element( dxys.begin(), dxys.end() ) );
    double maxBl = dxys[maxBlid];
    unsigned long staid1 = network.getBaseline( maxBlid ).getStaid1();
    const Station &sta1 = network.getStation( staid1 );
    unsigned long staid2 = network.getBaseline( maxBlid ).getStaid2();
    const Station &sta2 = network.getStation( staid2 );

    of << "longest baseline in xy-direction: " << sta1.getName() << "-" << sta2.getName() << "\n";
    of << "dividing stations in two groups:\n";


    for ( unsigned long i = 0; i < network.getNSta(); ++i ) {
        const Station &sta = network.getStation( i );
        double dxy1 = getDxy( sta1, sta );
        if ( dxy1 < maxBl * 0.33 ) {
            staid2groupid[i] = 1;
        }
        double dxy2 = getDxy( sta2, sta );
        if ( dxy2 < maxBl * 0.33 ) {
            staid2groupid[i] = 2;
        }
    }

    for ( unsigned long i = 0; i < nsta; ++i ) {
        int idx = staid2groupid[i];
        if ( idx == 1 ) {
            of << boost::format( "    %-8s: group 1\n" ) % network.getStation( i ).getName();
        } else if ( idx == 2 ) {
            of << boost::format( "    %-8s: group 2\n" ) % network.getStation( i ).getName();
        } else {
            of << boost::format( "    %-8s: no group\n" ) % network.getStation( i ).getName();
        }
    }
}

void VieVS::FocusCorners::reweight( const Subcon &subcon, SourceList &sourceList, std::ofstream &of, double fraction,
                                    int iteration ) {
    const auto &scans = subcon.getSingleSourceScans();
    vector<double> sumEl = vector<double>( scans.size() );

    for ( int iscan = 0; iscan < scans.size(); ++iscan ) {
        const Scan &scan = scans[iscan];

        double mel1 = 0;
        int nsta1 = 0;
        double mel2 = 0;
        int nsta2 = 0;
        bool valid = true;
        for ( int ista = 0; ista < scan.getNSta(); ++ista ) {
            const auto &pv = scan.getPointingVector( ista, VieVS::Timestamp::start );
            unsigned long staid = pv.getStaid();
            double az = pv.getAz();
            constexpr double threshold = 45 * deg2rad;

            int group = staid2groupid[staid];
            if ( group == 0 ) {
                continue;
            }
            double el = pv.getEl();
            if ( !isnan( lastCornerAzimuth[staid] ) && abs( az - lastCornerAzimuth[staid] ) < threshold ) {
                valid = false;
                break;
            }

            if ( group == 1 ) {
                mel1 += el;
                ++nsta1;
            } else {
                mel2 += el;
                ++nsta2;
            }
        }
        double mel;
        if ( valid && nsta1 > 0 && nsta2 > 0 ) {
            mel = ( mel1 / nsta1 + mel2 / nsta2 ) / 2;
        } else {
            mel = numeric_limits<double>::max();
        }
        sumEl[iscan] = mel;
    }

    vector<int> bestElements = findBestIndices( sumEl, 20 );
    if ( bestElements.empty() ) {
        return;
    }

    double minimum = sumEl[bestElements[0]];
    for ( unsigned long i = 0; i < bestElements.size(); ++i ) {
        int idx = bestElements[i];
        if ( sumEl[idx] == numeric_limits<double>::max() ) {
            bestElements.resize( i );
        }
        if ( sumEl[idx] / minimum > fraction ) {
            bestElements.resize( i );
            break;
        }
    }
    if ( bestElements.empty() ) {
        of << "error: no valid scan found to focus in corner!\n";
        return;
    }

    if ( bestElements.size() > 5 ) {
        double newFraction = sqrt( fraction ) * 1.1;
        if ( iteration < 8 && newFraction > 1.25 ) {
            of << boost::format(
                      "| readjust source selection at corner (fraction %5.3f)                                    "
                      "                                                     |\n" ) %
                      newFraction;
            reweight( subcon, sourceList, of, newFraction, ++iteration );
            return;
        }
    } else if ( bestElements.size() < 3 ) {
        double newFraction = fraction * 1.20;
        if ( iteration < 10 && newFraction < 3.5 ) {
            of << boost::format(
                      "| readjust source selection at corner (fraction %5.3f)                                    "
                      "                                                     |\n" ) %
                      newFraction;
            reweight( subcon, sourceList, of, newFraction, ++iteration );
            return;
        }
    }

    for ( int idx : bestElements ) {
        unsigned long srcid = scans[idx].getSourceId();
        backupWeight.emplace_back( srcid, sourceList.getSource( srcid )->getPARA().weight );
        double newWeight = 1000 / ( sumEl[idx] / minimum );
        of << boost::format(
                  "|     increase weight of source %-8s to %7.2f                                             "
                  "                                               |\n" ) %
                  sourceList.getSource( srcid )->getName() % newWeight;
        sourceList.refSource( srcid )->referencePARA().weight = newWeight;
    }
    of << boost::format( "|%|143T-||\n" );
}

void FocusCorners::reset( const std::vector<Scan> &bestScans, SourceList &sourceList ) {
    for ( const auto &any : backupWeight ) {
        sourceList.refSource( any.first )->referencePARA().weight = any.second;
    }
    backupWeight.clear();

    for ( const auto &scan : bestScans ) {
        for ( int i = 0; i < scan.getNSta(); ++i ) {
            const PointingVector &pv = scan.getPointingVector( i );
            unsigned long staid = pv.getStaid();
            lastCornerAzimuth[staid] = pv.getAz();
        }
    }
    FocusCorners::startFocusCorner = false;
}


vector<int> FocusCorners::findBestIndices( const vector<double> &values, int n ) {
    if ( values.size() < n ) {
        n = values.size();
    }
    if ( n == 0 ) {
        return vector<int>();
    }

    vector<int> indices( values.size() );
    std::iota( indices.begin(), indices.end(), 0 );  // [0, 1, 2, 3 ... n-1]

    std::partial_sort( indices.begin(), indices.begin() + n, indices.end(),
                       [&values]( int i, int j ) { return values[i] < values[j]; } );

    return vector<int>( indices.begin(), indices.begin() + n );
}
