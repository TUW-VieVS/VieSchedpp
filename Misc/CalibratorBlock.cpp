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


#include "CalibratorBlock.h"

#include <utility>

using namespace std;
using namespace VieVS;

unsigned long CalibratorBlock::nextId = 0;
string CalibratorBlock::intent_;
bool CalibratorBlock::tryToIncludeAllStationFlag = false;
bool CalibratorBlock::subnetting = false;

double CalibratorBlock::tryToIncludeAllStations_factor = 3;
int CalibratorBlock::stationOverlap = 2;
bool CalibratorBlock::rigorosStationOverlap = false;
double CalibratorBlock::numberOfObservations_factor = 5.0;
double CalibratorBlock::numberOfObservations_offset = 0.0;
double CalibratorBlock::averageStations_factor = 100.0;
double CalibratorBlock::averageStations_offset = 1.0;
double CalibratorBlock::duration_factor = 0.2;
double CalibratorBlock::duration_offset = 1.0;
double CalibratorBlock::averageBaseline_factor = 0.0;
double CalibratorBlock::averageBaseline_offset = 1.0;


std::vector<int> thread_local CalibratorBlock::stationFlag = std::vector<int>();

CalibratorBlock::CalibratorBlock( unsigned int startTime, unsigned int nScans, unsigned int duration,
                                  std::string allowedSourceGroup, int overlap, bool rigorosOverlap )
    : VieVS_Object( nextId++ ),
      startTime{ startTime },
      nScans{ nScans },
      duration{ duration },
      allowedSourceGroup{ std::move( allowedSourceGroup ) },
      overlap{ overlap },
      rigorosOverlap{ rigorosOverlap } {}

CalibratorBlock::CalibratorBlock( unsigned int startTime, unsigned int nScans, unsigned int duration,
                                  std::vector<std::string> allowedSources, int overlap, bool rigorosOverlap )
    : VieVS_Object( nextId++ ),
      startTime{ startTime },
      nScans{ nScans },
      duration{ duration },
      allowedSources{ std::move( allowedSources ) },
      overlap{ overlap },
      rigorosOverlap{ rigorosOverlap } {}


std::vector<int> CalibratorBlock::findBestIndices( const vector<vector<double>>& elevations ) {
    int m = elevations.size();
    if ( m == 0 ) {
        return {};
    }
    int n = elevations[0].size();

    vector<int> best_subset;
    tuple<int, int, double> best_score = { m + 1, 0, 0 };  // Initialize with worst

    // Iterate over all subsets
    for ( int mask = 1; mask < ( 1 << m ); ++mask ) {
        vector<int> subset;
        for ( int i = 0; i < m; ++i ) {
            if ( mask & ( 1 << i ) ) {
                subset.push_back( i );
            }
        }

        if ( !covers_all_columns( subset, elevations, n ) ) {
            continue;
        }
        if ( !has_required_overlap( subset, elevations ) ) {
            continue;
        }

        auto score = compute_stats( subset, elevations );
        if ( score < best_score ) {
            best_score = score;
            best_subset = subset;
        }
    }
    return best_subset;
}

bool is_nan( double x ) { return std::isnan( x ); }

bool CalibratorBlock::covers_all_columns( const vector<int>& subset, const vector<vector<double>>& elevations, int n ) {
    vector<bool> covered( n, false );
    for ( int idx : subset ) {
        for ( int j = 0; j < n; ++j ) {
            if ( !is_nan( elevations[idx][j] ) ) {
                covered[j] = true;
            }
        }
    }
    return all_of( covered.begin(), covered.end(), []( bool v ) { return v; } );
}

tuple<int, int, double> CalibratorBlock::compute_stats( const vector<int>& subset,
                                                        const vector<vector<double>>& elevations ) {
    int non_nan_count = 0;
    double min_val = numeric_limits<double>::max();

    for ( int idx : subset ) {
        for ( double val : elevations[idx] ) {
            if ( !is_nan( val ) ) {
                non_nan_count++;
                min_val = min( min_val, val );
            }
        }
    }

    return { subset.size(), -non_nan_count, -min_val };  // Negate to sort descending for those
}
bool CalibratorBlock::has_required_overlap( const vector<int>& subset, const vector<std::vector<double>>& elevations ) {
    int n = elevations[0].size();
    vector<bool> overlap( n, true );  // Start assuming all indices are valid

    for ( int idx : subset ) {
        vector<bool> current( n, false );
        for ( int j = 0; j < n; ++j ) {
            if ( !isnan( elevations[idx][j] ) ) {
                current[j] = true;
            }
        }
        // AND current with overlap to find common non-NaN indices
        for ( int j = 0; j < n; ++j ) {
            overlap[j] = overlap[j] && current[j];
        }
    }

    // Count number of overlapping non-NaN indices
    int overlap_count = count( overlap.begin(), overlap.end(), true );
    return overlap_count >= stationOverlap;
}
