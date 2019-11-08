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
    static void initialize( unsigned long nsta );

    static void reweight( const Subcon &subcon, std::vector<Source> &sources, std::ofstream &of );

    static void resetWeights( const std::vector<Scan> &bestScans, std::vector<Source> &sources );

    static bool flag;
    static thread_local unsigned int nextStart;
    static unsigned int interval;

   private:
    static thread_local std::vector<double> lastCornerAzimuth;
    static thread_local std::vector<std::pair<int, double>> backupWeight;

    static std::vector<int> findBestIndices( const std::vector<double> &values, int n );
};

}  // namespace VieVS
#endif  // FOCUSCORNERS_H
