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
 * @file CalibratorBlock.h
 * @brief class CalibratorBlock
 *
 * @author Matthias Schartner
 * @date 27.01.2020
 *
 */
#ifndef VIESCHEDPP_CALIBRATORBLOCK_H
#define VIESCHEDPP_CALIBRATORBLOCK_H

#include <vector>

#include "TimeSystem.h"
#include "VieVS_Object.h"

namespace VieVS {
/**
 * @class CalibratorBlock
 * @brief define calibrator blocks
 *
 * @author Matthias Schartner
 * @date 27.01.2020
 */

class CalibratorBlock : public VieVS_Object {
   public:
    CalibratorBlock( unsigned int startTime, unsigned int nScans, unsigned int duration, std::string allowedSourceGroup,
                     int overlap, bool rigorosOverlap );
    CalibratorBlock( unsigned int startTime, unsigned int nScans, unsigned int duration,
                     std::vector<std::string> allowedSources, int overlap, bool rigorosOverlap );

    bool isAllowedSource( const std::string &name ) const {
        return find( allowedSources.begin(), allowedSources.end(), name ) != allowedSources.end();
    }

    void setAllowedSources( const std::vector<std::string> &names ) { allowedSources = names; }

    unsigned int getStartTime() const { return startTime; }
    unsigned int getNScans() const { return nScans; }
    unsigned int getDuration() const { return duration; }
    unsigned int getOverlap() const { return overlap; }
    bool hasRigorosOverlap() const { return rigorosOverlap; }
    const std::vector<std::string> &getAllowedSources() const { return allowedSources; }
    const std::string &getAllowedSourceGroup() const { return allowedSourceGroup; }

    static std::string intent_;
    static bool tryToIncludeAllStationFlag;
    static bool subnetting;

    static double tryToIncludeAllStations_factor;
    static int stationOverlap;
    static bool rigorosStationOverlap;
    static double numberOfObservations_factor;
    static double numberOfObservations_offset;
    static double averageStations_factor;
    static double averageStations_offset;
    static double duration_factor;
    static double duration_offset;
    static double averageBaseline_factor;
    static double averageBaseline_offset;

    static thread_local std::vector<int> stationFlag;
    static std::vector<int> findBestIndices( const std::vector<std::vector<double>> &elevations );

   private:
    static unsigned long nextId;  ///< next id for this object type
    unsigned int startTime;
    unsigned int nScans;
    unsigned int duration;
    int overlap;
    unsigned int rigorosOverlap;
    std::string allowedSourceGroup;
    std::vector<std::string> allowedSources;

    static bool covers_all_columns( const std::vector<int> &subset, const std::vector<std::vector<double>> &elevations,
                                    int n );

    static bool has_required_overlap( const std::vector<int> &subset,
                                      const std::vector<std::vector<double>> &elevations );

    static std::tuple<int, int, double> compute_stats( const std::vector<int> &subset,
                                                       const std::vector<std::vector<double>> &elevations );
};
}  // namespace VieVS


#endif  // VIESCHEDPP_CALIBRATORBLOCK_H
