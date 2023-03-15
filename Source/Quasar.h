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
 * @author Matthias Schartner
 * @date 29.07.2020
 */

#ifndef VIESCHEDPP_QUASAR_H
#define VIESCHEDPP_QUASAR_H

#include "AbstractSource.h"

namespace VieVS {

class Quasar : public AbstractSource {
   public:
    /**
     * @brief pre calculated parameters
     * @author Matthias Schartner
     */
    struct PreCalculated {
        std::vector<double> sourceInCrs;  ///< source vector in celestrial reference frame
    };

    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param src_name name of the source
     * @param src_name2 alternative name of source
     * @param src_ra_deg right ascension in degrees
     * @param src_de_deg declination in degrees
     * @param src_flux flux information per band
     */
    Quasar( const std::string &src_name, const std::string &src_name2, double src_ra_deg, double src_de_deg,
            std::unordered_map<std::string, std::unique_ptr<AbstractFlux>> &src_flux );

    Quasar( const std::string &src_name, const std::string &src_name2, double src_ra_deg, double src_de_deg,
            std::unordered_map<std::string, std::unique_ptr<AbstractFlux>> &src_flux, double jet_angle,
            double jet_angle_std );

    double getDe() const noexcept { return de_; };

    double getRa() const noexcept { return ra_; };

    std::pair<double, double> getRaDe( unsigned int time,
                                       const std::shared_ptr<const Position> &sta_pos ) const noexcept override {
        return { ra_, de_ };
    }


    /**
     * @brief get source position in CRS
     * @author Matthias Schartner
     *
     * @return source position vector
     */
    std::pair<std::pair<double, double>, std::vector<double>> getSourceInCrs(
        unsigned int time, const std::shared_ptr<const Position> &sta_pos ) const override {
        return { getRaDe( time, sta_pos ), preCalculated_->sourceInCrs };
    }

    std::vector<double> getSourceInCrs() const { return preCalculated_->sourceInCrs; }

    /**
     * @brief this function checks if it is time to change the parameters
     * @author Matthias Schartner
     *
     * @param time current time
     * @param hardBreak flags this to true if a hard break was found
     * @return true if a new event was found
     */
    bool checkForNewEvent( unsigned int time, bool &hardBreak ) noexcept override;

    void toVex( std::ofstream &of ) const override;

    void toVex( std::ofstream &of, const std::vector<unsigned int> &times,
                const std::shared_ptr<const Position> &sta_pos ) const override {
        toVex( of );
    }


    void toNgsHeader( std::ofstream &of ) const override;

   private:
    double ra_;  ///< source right ascension
    double de_;  ///< source declination

    std::shared_ptr<PreCalculated> preCalculated_;  ///< pre calculated values
};

}  // namespace VieVS
#endif  // VIESCHEDPP_QUASAR_H
