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
 * @file Scan.h
 * @brief class Scan
 *
 * @author Matthias Schartner
 * @date 08.11.19.
 */

#ifndef FOCUSCORNERS_H
#define FOCUSCORNERS_H

#include "../Scan/Subcon.h"

namespace VieVS {
/**
 * @class focusCorners
 * @brief algorithm which focuses corners of commonly visible sky. Primary use-case for intensive sessions.
 *
 * @author Matthias Schartner
 * @date 08.11.19.
 */

class FocusCorners {
   public:
    /**
     * @brief initialize Focus Corner algorithm
     * @author Matthias Schartner
     *
     * define group and set variables
     *
     * @param network station network
     * @param of log file stream
     */
    static void initialize( const Network &network, std::ofstream &of );

    /**
     * @brief reweight sources
     * @author Matthias Schartner
     *
     * @param subcon list of all scans
     * @param sourceList list of all sources
     * @param of log file stream
     * @param fraction fraction between best and worst mean elevation (used during iteration)
     * @param iteration iteration number
     */
    static void reweight( const Subcon &subcon, SourceList &sourceList, std::ofstream &of, double fraction = 2.0,
                          int iteration = 0 );

    /**
     * @brief rest all weights and set new time for next algorithm start
     * @author Matthias Schartner
     *
     * @param bestScans list of all best scans (to see which corner was observed)
     * @param sourceList list of all sources
     */
    static void reset( const std::vector<Scan> &bestScans, SourceList &sourceList );

    static bool flag;                            ///< use focus corner algorithm
    static thread_local bool startFocusCorner;   ///< focus observation at corner
    static thread_local unsigned int nextStart;  ///< next time when algorithm starts
    static thread_local unsigned int interval;   ///< time intervall between algorithm

   private:
    static thread_local std::vector<double> lastCornerAzimuth;  ///< last observed azimuth (which corner was observed)
    static thread_local std::vector<std::pair<int, double>> backupWeight;  ///< backup weights
    static thread_local std::vector<int> staid2groupid;                    ///< station id to corner group id

    /**
     * @brief find n lowest values
     * @author Matthias Schartner
     *
     * @param values list of values
     * @param n how many lowest are searched
     * @return list of lowest indices
     */
    static std::vector<int> findBestIndices( const std::vector<double> &values, int n );
};

}  // namespace VieVS
#endif  // FOCUSCORNERS_H
