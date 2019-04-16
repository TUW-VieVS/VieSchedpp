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
 * @file Subnetting.h
 * @brief class Subnetting
 *
 * @author Matthias Schartner
 * @date 07.05.2018
 */

#ifndef SUBNETTING_H
#define SUBNETTING_H

#include <utility>
#include <vector>

namespace VieVS {
/**
 * @class Subnetting
 * @brief class Subnetting
 *
 * @author Matthias Schartner
 * @date 07.05.2018
 */
class Subnetting {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param subnettingSrcIds list of possible subnetting source ids
     */
    explicit Subnetting( std::vector<std::vector<unsigned long>> subnettingSrcIds )
        : subnettingSrcIds{std::move( subnettingSrcIds )} {

          };

    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param nsta number of scheduled stations
     * @param nsta_max number of available stations
     * @return true if subnetting scan is allowed
     */
    bool isAllowed( unsigned long nsta, unsigned long nsta_max ) { return allowedMinSta( nsta, nsta_max ); }

    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @return list of possible subnetting source ids
     */
    const std::vector<std::vector<unsigned long>> &getSubnettingSrcIds() const { return subnettingSrcIds; }

   private:
    /**
     * @brief check if minimum number of stations is reached
     * @author Matthias Schartner
     *
     * @param nsta number of scheduled stations
     * @param nsta_max number of available stations
     * @return true if subnetting scan is allowed
     */
    virtual bool allowedMinSta( unsigned long nsta, unsigned long nsta_max ) = 0;

    std::vector<std::vector<unsigned long>> subnettingSrcIds;  ///< list of possible subnetting source ids
};

/**
 * @class Subnetting_percent
 * @brief class Subnetting_percent
 *
 * @author Matthias Schartner
 * @date 22.02.2019
 */
class Subnetting_percent : public Subnetting {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param subnettingSrcIds list of possible subnetting source ids
     * @param percent minimum necessary percentage of stations (e.g.: = 0.8)
     */
    explicit Subnetting_percent( std::vector<std::vector<unsigned long>> subnettingSrcIds, double percent )
        : Subnetting( std::move( subnettingSrcIds ) ), percent_{percent} {}

   private:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param nsta number of scheduled stations
     * @param nsta_max number of available stations
     * @return true if subnetting scan is allowed
     */
    bool allowedMinSta( unsigned long nsta, unsigned long nsta_max ) override { return nsta >= nsta_max * percent_; }

    double percent_;  ///< minimum necessary percentage of stations
};

/**
 * @class Subnetting_minIdle
 * @brief class Subnetting_minIdle
 *
 * @author Matthias Schartner
 * @date 22.02.2019
 */
class Subnetting_minIdle : public Subnetting {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param subnettingSrcIds list of possible subnetting source ids
     * @param maxIdle maximum allowed number of idle stations
     */
    explicit Subnetting_minIdle( std::vector<std::vector<unsigned long>> subnettingSrcIds, int maxIdle )
        : Subnetting( std::move( subnettingSrcIds ) ), maxIdle_{maxIdle} {}

   private:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param nsta number of scheduled stations
     * @param nsta_max number of available stations
     * @return true if subnetting scan is allowed
     */
    bool allowedMinSta( unsigned long nsta, unsigned long nsta_max ) override { return nsta >= nsta_max - maxIdle_; }

    int maxIdle_;  ///< maximum allowed number of idle stations
};
}  // namespace VieVS

#endif  // SUBNETTING_H
