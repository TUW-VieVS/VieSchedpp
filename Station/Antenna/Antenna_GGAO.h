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

#ifndef VIESCHEDPP_ANTENNA_GGAO_H
#define VIESCHEDPP_ANTENNA_GGAO_H

#include "AbstractAntenna.h"

namespace VieVS {
/**
 * @class Antenna_AzEl
 * @brief azimuth, elevation antenna
 *
 * special slew model for GGAO12M avoiding slewing through radar
 *
 * @author Matthias Schartner
 * @date 22.02.2021
 */
class Antenna_GGAO : public AbstractAntenna {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param offset_m offset of antenna axis intersection in meters
     * @param diam_m diameter of antenna dish in meters
     * @param rateAz_deg_per_min slew rate of azimuth in degrees/seconds
     * @param constantOverheadAz_s constant overhead for azimuth slew time in seconds
     * @param rateEl_deg_per_min slew rate of elevation in degrees/secondds
     * @param constantOverheadEl_s constant overhead for elevation slew time in seconds
     */
    Antenna_GGAO( double offset_m, double diam_m, double rateAz_deg_per_min, unsigned int constantOverheadAz_s,
                  double rateEl_deg_per_min, unsigned int constantOverheadEl_s );


    /**
     * @brief calculates slew time
     * @author Matthias Schartner
     *
     * @param old_pointingVector slew start point
     * @param new_pointingVector slew end point
     * @return slew time in seconds
     */
    unsigned int slewTime( const PointingVector &old_pointingVector,
                           const PointingVector &new_pointingVector ) const noexcept override;


    /**
     * @brief get mount name
     * @author Matthias Schartner
     *
     * @return mount name
     */
    std::string getMount() const noexcept override { return "ALTAZ"; };

    std::string toVex( Axis axis ) const override;


   private:
    /**
     * @brief This is a helper function to be consistend with the implementation of this model in SKED
     * @author Matthias Schartner
     *
     * @param x1 start
     * @param x2 end
     * @param vel velocity
     * @param acc acceleration
     * @return
     */
    static double slew_time(double x1, double x2, double vel, double acc);

    /**
     * @brief calculates the slewtime between azimuth and elevation of two pointing vectors in tracking mode
     * @author Matthias Schartner
     *
     * tracking mode means that the constant overhead time is not added
     *
     * @param old_pointingVector start pointing vector
     * @param new_pointingVector end pointing vector
     * @return slewtime between start pointing vector and end pointing vector in seconds
     */
    unsigned int slewTimeTracking( const PointingVector &old_pointingVector,
                                   const PointingVector &new_pointingVector ) const noexcept;

    double getAcc1() const override { return getRate1(); }
    double getAcc2() const override { return getRate2(); }
};
}  // namespace VieVS


#endif  // VIESCHEDPP_ANTENNA_GGAO_H
