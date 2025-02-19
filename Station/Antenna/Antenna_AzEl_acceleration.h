//
// Created by mschartner on 12/3/21.
//

#ifndef VIESCHEDPP_ANTENNA_AZEL_ACCELERATION_H
#define VIESCHEDPP_ANTENNA_AZEL_ACCELERATION_H


#include "AbstractAntenna.h"
namespace VieVS {
/**
 * @class Antenna_AzEl_acceleration
 * @brief azimuth, elevation antenna including acceleration and deceleration
 *
 * @author Matthias Schartner
 * @date 12.04.2018
 */

class Antenna_AzEl_acceleration : public AbstractAntenna {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param offset_m offset of antenna axis intersection in meters
     * @param diam_m diameter of antenna dish in meters
     * @param az_rate_deg_per_sec slew rate of azimuth in degrees/seconds
     * @param az_acceleration_deg_per_sec_sec azimuth acceleration in degrees/seconds^2
     * @param az_deceleration_deg_per_sec_sec azimuth deceleration in degrees/seconds^2
     * @param az_settle_s constant overhead for azimuth slew time in seconds
     * @param el_rate_deg_per_sec slew rate of elevation in degrees/seconds
     * @param el_acceleration_deg_per_sec_sec elevation acceleration in degrees/seconds^2
     * @param el_deceleration_deg_per_sec_sec elevation deceleration in degrees/seconds^2
     * @param el_settle_s constant overhead for elevation slew time in seconds
     */
    Antenna_AzEl_acceleration( double offset_m, double diam_m, double az_rate_deg_per_sec,
                               double az_acceleration_deg_per_sec_sec, double az_deceleration_deg_per_sec_sec,
                               unsigned int az_settle_s, double el_rate_deg_per_sec,
                               double el_acceleration_deg_per_sec_sec, double el_deceleration_deg_per_sec_sec,
                               unsigned int el_settle_s );


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


    /**
     * @brief get mount name
     * @author Matthias Schartner
     *
     * @return mount name
     */
    std::string getMount() const noexcept override { return "ALTAZ"; };

    std::string toVex( Axis axis ) const override;

    double getAcc1() const override { return ( az_acelleration + az_deceleration ) / 2; }
    double getAcc2() const override { return ( el_acelleration + el_deceleration ) / 2; }


   private:
    double az_acelleration;
    double az_deceleration;
    double el_acelleration;
    double el_deceleration;

    static unsigned int calc_slew_times( double delta, double rate, double acceleration, double deceleration,
                                         double settle );
};
#endif  // VIESCHEDPP_ANTENNA_AZEL_ACCELERATION_H
}