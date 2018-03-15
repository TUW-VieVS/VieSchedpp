/**
 * @file Antenna.h
 * @brief class Antenna
 *
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 */

#ifndef ANTENNA_H
#define ANTENNA_H
#include <iostream>
#include <boost/format.hpp>

#include "PointingVector.h"
#include "VieVS_Object.h"

namespace VieVS{

    /**
     * @class Antenna
     * @brief representation of an VLBI antenna
     *
     * @author Matthias Schartner
     * @date 27.06.2017
     */
    class Antenna: public VieVS_Object {
    public:
        /**
         * @brief antenna type
         */
        enum class AxisType {
            AZEL, ///< azimuth elevation antenna
            HADC, ///< hour angle declination antenna
            XYNS, ///< x-y north south antenna
            XYEW, ///< x-y east west antenna
            RICH, ///< keine ahnung
            SEST, ///< keine ahnung
            ALGO, ///< keine ahnung
            undefined ///< undefined antenna type
        };


        /**
         * @brief constructor
         *
         * @param type axis type
         * @param offset_m offset of antenna axis intersection in meters
         * @param diam_m diameter of antenna dish in meters
         * @param rate1_deg_per_min slew rate of first axis in degrees/seconds
         * @param constantOverhead1_s constant overhead for first axis slew in seconds
         * @param rate2_deg_per_min slew rate of second axis in degrees/secondds
         * @param constantOverhead2_s constant overhead for second axis slew in seconds
         */
        Antenna(const std::string &type, double offset_m, double diam_m, double rate1_deg_per_min,
                double constantOverhead1_s, double rate2_deg_per_min, double constantOverhead2_s);

        /**
         * @brief getter for axis type
         *
         * @return axis type
         */
        AxisType getAxisType() const {
            return axisType_;
        }

        double getDiam() const {
            return diam_;
        }

        double getOffset() const {
            return offset_;
        }

        double getRate1() const {
            return rate1_;
        }

        double getCon1() const {
            return con1_;
        }

        double getRate2() const {
            return rate2_;
        }

        double getCon2() const {
            return con2_;
        }

        /**
         * @brief calculates the slewtime between azimuth and elevation of two pointing vectors
         * @param old_pointingVector start azimuth and elevation
         * @param new_pointingVector end azimuth and elevation
         * @return slewtime in seconds
         */
        unsigned int
        slewTime(const PointingVector &old_pointingVector,
                 const PointingVector &new_pointingVector) const noexcept;

        /**
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param antenna antenna information that should be printed to stream
         * @return stream object
         */
        friend std::ostream &operator<<(std::ostream &out, const Antenna &antenna) noexcept;



    private:
        static int nextId;

        AxisType axisType_; ///< station axis type

        double offset_; ///< offset of the antenna axis intersection in meters
        double diam_; ///< diameter of the antenna dish in meters
        double rate1_; ///< slew rate of first axis in radians/second
        double con1_; ///< constant overhead for first axis slew in seconds
        double rate2_; ///< slew rate of second axis in radians/second
        double con2_; ///< constant overhead for second axis slew in seconds

        /**
         * @brief calculates slew time per axis
         *
         * @param delta distance to slew
         * @param rate velocity
         * @param acc acceleration
         * @return slew time in seconds
         */
        double slewTimePerAxis(double delta, double rate, double acc) const noexcept ;
    };
}
#endif /* ANTENNA_H */

