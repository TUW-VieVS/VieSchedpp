/**
 * @file VLBI_cableWrap.h
 * @brief class VLBI_cableWrap
 *
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 */

#ifndef VLBI_CABLEWRAP_H
#define VLBI_CABLEWRAP_H
#include <iostream>
#include <boost/format.hpp>
#include <cmath>

#include "VLBI_pointingVector.h"
#include "VieVS_constants.h"
using namespace std;

namespace VieVS{
    class VLBI_cableWrap {
    public:
        /**
         * @brief empty default constructor
         */
        VLBI_cableWrap();

        /**
         * @brief constructor
         *
         * @param axis1_low_deg lower limit for first axis in degrees
         * @param axis1_up_deg upper limit for first axis in degrees
         * @param axis2_low_deg lower limit for second axis in degrees
         * @param axis2_up_deg upper limit for second axis in degrees
         */
        VLBI_cableWrap(double axis1_low_deg, double axis1_up_deg,
                       double axis2_low_deg, double axis2_up_deg);

        /**
         * @brief sets safety margins for axis limits
         *
         * @param axis1_low_offset safety margin for lower limit for first axis in degrees
         * @param axis1_up_offset safety margin for upper limit for first axis in degrees
         * @param axis2_low_offset safety margin for lower limit for second axis in degrees
         * @param axis2_up_offset safety margin for upper limit for second axis in degrees
         */
        void setMinimumOffsets(double axis1_low_offset, double axis1_up_offset,
                               double axis2_low_offset, double axis2_up_offset);

        /**
         * @brief returns axis limits neutral point
         *
         * The neutral point of an axis is the middle between its lower and upper limit
         *
         * @param axis index of axis, 1 for first axis, 2 for second axis
         * @return neutral point of axis limits in radiants
         */
        double neutralPoint(int axis);

        /**
         * @brief destructor
         */
        virtual ~VLBI_cableWrap();

        /**
         * @brief checks if this pointing vectors azimuth and elevation are inside the axis limits
         *
         * @param p pointing vector
         * @return true if inside, otherwise false
         */
        bool anglesInside(VLBI_pointingVector& p);

        /**
         * @brief unwraps the current azimuth and elevation of pointing vector
         *
         * The azimuth of one pointing vector is first calculated in the range between [-pi,pi]. This function
         * adds an factor of 2*pi so that the azimuth is inside the axis limits. If there are possible ambigurities,
         * for example if the azimuth axis range is bigger than 360 degrees, the value, which is closest to the current
         * antenna position is used.
         *
         * THIS FUNCTION IS ONLY USED FOR INTERNAL CHECKS IN NICHE SCENARIOS. USE
         * calcUnwrappedAz(VLBI_pointingVector& old_pointingVector, VLBI_pointingVector& new_pointingVector) INSTEAD
         *
         * @param new_pointingVector
         * @return
         */
        bool unwrapAzNearNeutralPoint(VLBI_pointingVector &new_pointingVector);


        /**
         * @brief unwraps the current azimuth and elevation of pointing vector
         *
         * The azimuth of one pointing vector is first calculated in the range between [-pi,pi]. This function
         * adds an factor of 2*pi so that the azimuth is inside the axis limits. If there are possible ambigurities,
         * for example if the azimuth axis range is bigger than 360 degrees, the value, which is closest to the second
         * parameters value is used.
         *
         * THIS FUNCTION IS ONLY USED FOR INTERNAL CHECKS IN NICHE SCENARIOS. USE
         * calcUnwrappedAz(VLBI_pointingVector& old_pointingVector, VLBI_pointingVector& new_pointingVector) INSTEAD
         *
         * @param new_pointingVector
         * @param az_old
         * @return
         */
        void unwrapAzNearAz(VLBI_pointingVector &new_pointingVector, double az_old);

        /**
         * @brief unwraps the current azimuth and elevation of pointing vector
         *
         * The azimuth of one pointing vector is first calculated in the range between [-pi,pi]. This function
         * adds an factor of 2*pi so that the azimuth is inside the axis limits. If there are possible ambigurities,
         * for example if the azimuth axis range is bigger than 360 degrees, the value, which is closest to the old
         * pointing vector is used.
         *
         * @param new_pointingVector
         * @return
         */
        void calcUnwrappedAz(VLBI_pointingVector& old_pointingVector, VLBI_pointingVector& new_pointingVector);

        /**
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param cw cable wrap information that should be printed to stream
         * @return stream object
         */
        friend ostream &operator<<(ostream &out, const VLBI_cableWrap &cw);
        
    private:
        double axis1_low; ///< lower limit of first axis in radians
        double axis1_up; ///< upper limit of first axis in radians
        double axis2_low; ///< lower limit of second axis in radians
        double axis2_up; ///< upper limit of second axis in radians

        double axis1_low_offset; ///< safety margin for lower limit for first axis in radians
        double axis1_up_offset; ///< safety margin for upper limit for first axis in radians
        double axis2_low_offset; ///< safety margin for lower limit for second axis in radians
        double axis2_up_offset; ///< safety margin for upper limit for second axis in radians

        double n_low; ///< lower limit for neutral cable wrap range
        double n_up; ///< upper limit for neutral cable wrap range
        double c_low; ///< lower limit for clockwise cable wrap range
        double c_up; ///< upper limit for clockwise cable wrap range
        double w_low; ///< lower limit for counter clockwise cable wrap range
        double w_up; ///< upper limit for counter clockwise cable wrap range
    };
}
#endif /* VLBI_CABLEWRAP_H */

