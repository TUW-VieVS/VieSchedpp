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

bool VieVS::FocusCorners::flag = true;
std::vector<double> thread_local VieVS::FocusCorners::lastCornerAzimuth = std::vector<double>();
std::vector<std::pair<int, double>> thread_local VieVS::FocusCorners::backupWeight =
    std::vector<std::pair<int, double>>();
unsigned int thread_local VieVS::FocusCorners::nextStart = 0;
unsigned int VieVS::FocusCorners::interval = 900;


void VieVS::FocusCorners::initialize( unsigned long nsta ) {
    lastCornerAzimuth = std::vector<double>( nsta, std::numeric_limits<double>::quiet_NaN() );
}

void VieVS::FocusCorners::reweight( const Subcon &subcon, std::vector<Source> &sources, std::ofstream &of ) {
    const auto &scans = subcon.getSingleSourceScans();
    vector<double> sumEl = vector<double>( scans.size() );
    //    vector<vector<double>> azimuths =
    //    vector<vector<double>>(scans.size(),vector<double>(lastCornerAzimuth.size()));

    for ( int iscan = 0; iscan < scans.size(); ++iscan ) {
        const Scan &scan = scans[iscan];

        double mel = 0;
        for ( int ista = 0; ista < scan.getNSta(); ++ista ) {
            const auto &pv = scan.getPointingVector( ista, VieVS::Timestamp::start );
            unsigned long staid = pv.getStaid();
            double az = pv.getAz();
            constexpr double threshold = 30 * deg2rad;
            if ( !isnan( lastCornerAzimuth[staid] ) && abs( az - lastCornerAzimuth[staid] ) < threshold ) {
                mel += numeric_limits<double>::max();
                break;
            } else {
                mel += pv.getEl();
            }
            //            azimuths[iscan][pv.getStaid()] = pv.getAz();
        }
        sumEl[iscan] = mel / static_cast<double>( scan.getNSta() );
    }

    vector<int> bestElements = findBestIndices( sumEl, 20 );
    if ( bestElements.empty() ) {
        return;
    }

    double minimum = sumEl[bestElements[0]];
    for ( int i = 0; i < bestElements.size(); ++i ) {
        int idx = bestElements[i];
        if ( sumEl[idx] / minimum > 2 ) {
            bestElements.resize( i );
            break;
        }
    }

    for ( int idx : bestElements ) {
        unsigned long srcid = scans[idx].getSourceId();
        backupWeight.emplace_back( srcid, sources[srcid].getPARA().weight );
        of << "############ increase weight of source " << sources[srcid].getName() << " \n";
        sources[srcid].referencePARA().weight = 1000;
    }
}

void FocusCorners::resetWeights( const std::vector<Scan> &bestScans, std::vector<Source> &sources ) {
    for ( const auto &any : backupWeight ) {
        sources[any.first].referencePARA().weight = any.second;
    }
    backupWeight.clear();

    for ( const auto &scan : bestScans ) {
        for ( int i = 0; i < scan.getNSta(); ++i ) {
            const PointingVector &pv = scan.getPointingVector( i );
            unsigned long staid = pv.getStaid();
            lastCornerAzimuth[staid] = pv.getAz();
        }
    }
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
