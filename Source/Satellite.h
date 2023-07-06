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
 * @file SourceList.h
 * @brief class SourceList
 *
 * @author Helene Wolf and Matthias Schartner
 * @date 29.07.2020
 */

#ifndef VIESCHEDPP_SATELLITE_H
#define VIESCHEDPP_SATELLITE_H

#include <memory>
#include <utility>

#include "../SGP4/CoordTopocentric.h"
#include "../SGP4/Observer.h"
#include "../SGP4/SGP4.h"
#include "../SGP4/Tle.h"
#include "../Scan/PointingVector.h"
#include "../Station/Network.h"
#include "../Station/Station.h"
#include "AbstractSource.h"

namespace VieVS {

class Satellite : public AbstractSource {
   public:

    /**
     * @brief constructor
     * @author Helene Wolf and Matthias Schartner
     *
     * @param hdr header line of TLE data
     * @param l1 first line of TLE data
     * @param l2 second line of TLE data
     */
    Satellite( const std::string &hdr, const std::string &l1, const std::string &l2,
               std::unordered_map<std::string, std::unique_ptr<AbstractFlux>> &src_flux );


    /**
     * @brief get source postion in celestial reference frame
     * @author Matthias Schartner
     *
     * @param time reference time
     * @return source position in celestial reference frame
     */
    std::pair<std::pair<double, double>, std::vector<double>> getSourceInCrs(
        unsigned int time, const std::shared_ptr<const Position> &sta_pos ) const override;

    std::pair<double, double> getRaDe( unsigned int time,
                                       const std::shared_ptr<const Position> &sta_pos ) const noexcept override {
        return calcRaDe( time, sta_pos );
    }

    void toVex( std::ofstream &of ) const override;

    void toVex( std::ofstream &of, const std::vector<unsigned int> &times,
                const std::shared_ptr<const Position> &sta_pos ) const override;

    void toNgsHeader( std::ofstream &of ) const override;

    std::pair<double, double> calcRaDe( unsigned int time, const std::shared_ptr<const Position> &sta_pos ) const;

    std::tuple<double, double, double, double> calcRaDeDistTime(
        unsigned int time, const std::shared_ptr<const Position> &sta_pos ) const noexcept override;


    void addpSGP4Data( const std::string &hdr, const std::string &l1, const std::string &l2 ) {
        auto epoch = extractReferenceEpoch( l1 );
        pSGP4Data_.emplace_back( std::make_tuple( epoch, SGP4( Tle( hdr, l1, l2 ) ), hdr + "\n" + l1 + "\n" + l2 ) );
        //        std::string tmp = TimeSystem::time2string(epoch);
        //        std::cout << tmp;
    }

    std::string getNameTime( unsigned int t ) const {
        return ( boost::format( "%s<=>%s" ) % getName() % TimeSystem::time2string_doy_minus( t ) ).str();
    }

    static boost::posix_time::ptime extractReferenceEpoch( const std::string &l1 );

    const std::tuple<boost::posix_time::ptime, SGP4, std::string> &getSGP4( unsigned int epoch ) const;

   private:
    static unsigned long nextId;                                                      ///< next id for this object type
    std::string header_;                                                              ///< header line of TLE Data
    std::string line1_;                                                               ///< first line of TLE Data
    std::string line2_;                                                               ///< second line of TLE Data
    std::vector<std::tuple<boost::posix_time::ptime, SGP4, std::string>> pSGP4Data_;  ///< pointer to SGP4 Data + epoch
};
}  // namespace VieVS
#endif  // VIESCHEDPP_SATELLITE_H
