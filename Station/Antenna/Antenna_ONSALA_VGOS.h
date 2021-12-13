//
// Created by mschartner on 12/8/21.
//

#ifndef VIESCHEDPP_ANTENNA_ONSALA_VGOS_H
#define VIESCHEDPP_ANTENNA_ONSALA_VGOS_H

#include "AbstractAntenna.h"

namespace VieVS {
/**
 * @class Antenna_ONSALA_VGOS
 * @brief azimuth, elevation antenna
 *
 * special slew model for ONSALA13NE and ONSALA13SW with slower slew rate near cable wrap limits
 *
 * @author Matthias Schartner
 * @date 08.12.2021
 */

class Antenna_ONSALA_VGOS : public AbstractAntenna {
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
    Antenna_ONSALA_VGOS( double offset_m, double diam_m, double rateAz_deg_per_min, unsigned int constantOverheadAz_s,
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
                                   const PointingVector &new_pointingVector ) const noexcept override;

    std::string toVex( Axis axis ) const override;

    /**
     * @brief get mount name
     * @author Matthias Schartner
     *
     * @return mount name
     */
    std::string getMount() const noexcept override { return "ALTAZ"; };

   private:
    double rate_slow_ = 66.0 / 60.0 * deg2rad;  // rad/sec
    double slow_unaz_lower_as_ = -64.0 * deg2rad;
    double slow_unaz_higher_as_ = 424.0 * deg2rad;

    unsigned int calcSlewTime( double az_start, double az_end ) const;
};

}  // namespace VieVS


#endif  // VIESCHEDPP_ANTENNA_ONSALA_VGOS_H
