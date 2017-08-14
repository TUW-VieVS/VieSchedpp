/**
 * @file VLBI_antenna.h
 * @brief class VLBI_antenna
 *
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 */

#ifndef VLBI_ANTENNA_H
#define VLBI_ANTENNA_H
#include <iostream>
#include <boost/format.hpp>

#include "VLBI_pointingVector.h"
#include "VieVS_constants.h"

using namespace std;
namespace VieVS{

    class VLBI_antenna {
    public:
        /**
         * @brief empty default constructor
         */
        VLBI_antenna();

        /**
         * @brief constructor
         *
         * @param offset_m offset of antenna axis intersection in meters
         * @param diam_m diameter of antenna dish in meters
         * @param rate1_deg_per_min slew rate of first axis in degrees/seconds
         * @param constantOverhead1_s constant overhead for first axis slew in seconds
         * @param rate2_deg_per_min slew rate of second axis in degrees/secondds
         * @param constantOverhead2_s constant overhead for second axis slew in seconds
         */
        VLBI_antenna(double offset_m, 
                     double diam_m, 
                     double rate1_deg_per_min,
                     double constantOverhead1_s, 
                     double rate2_deg_per_min,
                     double constantOverhead2_s);

        /**
         * destructor
         */
        virtual ~VLBI_antenna();

        /**
         * @brief calculates the slewtime between azimuth and elevation of two pointing vectors
         * @param old_pointingVector start azimuth and elevation
         * @param new_pointingVector end azimuth and elevation
         * @return slewtime in seconds
         */
        unsigned int
        slewTime(const VLBI_pointingVector &old_pointingVector, const VLBI_pointingVector &new_pointingVector) const;

        /**
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param antenna antenna information that should be printed to stream
         * @return stream object
         */
        friend ostream& operator<<(ostream& out, const VLBI_antenna& antenna);
        
    private:
        double offset; ///< offset of the antenna axis intersection in meters
        double diam; ///< diameter of the antenna dish in meters
        double rate1; ///< slew rate of first axis in radians/second
        double con1; ///< constant overhead for first axis slew in seconds
        double rate2; ///< slew rate of second axis in radians/second
        double con2; ///< constant overhead for second axis slew in seconds
    };
}
#endif /* VLBI_ANTENNA_H */

