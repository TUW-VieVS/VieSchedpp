/**
 * @file CableWrap.h
 * @brief class CableWrap
 *
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 */

#ifndef CABLEWRAP_H
#define CABLEWRAP_H
#include <iostream>
#include <boost/format.hpp>
#include <cmath>

#include "PointingVector.h"
#include "Constants.h"

namespace VieVS{
    /**
     * @class CableWrap
     * @brief representation of an VLBI cable wrap
     *
     * @author Matthias Schartner
     * @date 27.06.2017
     */
    class CableWrap {
    public:

        /**
         * @brief cable wrap type
         *
         */
        enum class CableWrapType {
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
         * @brief empty default constructor
         */
        CableWrap();

        /**
         * @brief constructor
         *
         * @param axis1_low_deg lower limit for first axis in degrees
         * @param axis1_up_deg upper limit for first axis in degrees
         * @param axis2_low_deg lower limit for second axis in degrees
         * @param axis2_up_deg upper limit for second axis in degrees
         * @param cwt cable wrap type
         */
        CableWrap(double axis1_low_deg, double axis1_up_deg,
                       double axis2_low_deg, double axis2_up_deg, std::string cwt);


        /**
         * @brief sets safety margins for axis limits
         *
         * @param axis1_low_offset safety margin for lower limit for first axis in degrees
         * @param axis1_up_offset safety margin for upper limit for first axis in degrees
         * @param axis2_low_offset safety margin for lower limit for second axis in degrees
         * @param axis2_up_offset safety margin for upper limit for second axis in degrees
         */
        void setMinimumOffsets(double axis1_low_offset, double axis1_up_offset,
                               double axis2_low_offset, double axis2_up_offset) noexcept;

        /**
         * @brief returns axis limits neutral point
         *
         * The neutral point of an axis is the middle between its lower and upper limit
         *
         * @param axis index of axis, 1 for first axis, 2 for second axis
         * @return neutral point of axis limits in radiants
         */
        double neutralPoint(int axis) const noexcept;

        /**
         * @brief checks if this pointing vectors azimuth and elevation are inside the axis limits
         *
         * @param p pointing vector
         * @return true if inside, otherwise false
         */
        bool anglesInside(const PointingVector &p) const noexcept;

        /**
         * @brief unwraps the current azimuth and elevation of pointing vector
         *
         * !!! This function changes new_pointingVector !!!
         *
         * The azimuth of one pointing vector is first calculated in the range between [-pi,pi]. This function
         * adds an factor of 2*pi so that the azimuth is inside the axis limits. If there are possible ambigurities,
         * for example if the azimuth axis range is bigger than 360 degrees, the value, which is closest to the current
         * antenna position is used.
         *
         * THIS FUNCTION IS ONLY USED FOR INTERNAL CHECKS IN NICHE SCENARIOS. USE
         * calcUnwrappedAz(VLBI_pointingVector& old_pointingVector, VLBI_pointingVector& new_pointingVector) INSTEAD
         *
         * @param new_pointingVector new pointing vector whose azimuth should be unwrapped
         * @return true if it is secure to use the new azimuth (it is far enough away from cable wrap limits)
         */
        bool unwrapAzNearNeutralPoint(PointingVector &new_pointingVector) const noexcept;


        /**
         * @brief unwraps the current azimuth and elevation of pointing vector
         *
         * !!! This function changes new_pointingVector !!!
         *
         * The azimuth of one pointing vector is first calculated in the range between [-pi,pi]. This function
         * adds an factor of 2*pi so that the azimuth is inside the axis limits. If there are possible ambigurities,
         * for example if the azimuth axis range is bigger than 360 degrees, the value, which is closest to the second
         * parameters value is used.
         *
         * THIS FUNCTION IS ONLY USED FOR INTERNAL CHECKS IN NICHE SCENARIOS. USE
         * calcUnwrappedAz(VLBI_pointingVector& old_pointingVector, VLBI_pointingVector& new_pointingVector) INSTEAD
         *
         * @param new_pointingVector new pointing vector whose azimuth should be unwrapped
         * @param az_old current antenna azimuth
         */
        void unwrapAzNearAz(PointingVector &new_pointingVector, double az_old) const noexcept;

        /**
         * @brief unwraps the current azimuth and elevation of pointing vector
         *
         * !!! This function changes new_pointingVector !!!
         *
         * The azimuth of one pointing vector is first calculated in the range between [-pi,pi]. This function
         * adds an factor of 2*pi so that the azimuth is inside the axis limits. If there are possible ambigurities,
         * for example if the azimuth axis range is bigger than 360 degrees, the value, which is closest to the old
         * pointing vector is used.
         *
         * @param old_pointingVector current antenna pointing vector
         * @param new_pointingVector new pointing vector whose azimuth should be unwrapped
         */
        void
        calcUnwrappedAz(const PointingVector &old_pointingVector,
                        PointingVector &new_pointingVector) const noexcept;


        bool unwrapAzInSection(PointingVector &pv, char section) const noexcept;

        std::string cableWrapFlag(const PointingVector &pointingVector) const noexcept;

        /**
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param cw cable wrap information that should be printed to stream
         * @return stream object
         */
        friend std::ostream &operator<<(std::ostream &out, const CableWrap &cw) noexcept;

        double getNLow() const {
            return nLow_;
        }

        double getNUp() const {
            return nUp_;
        }

        double getCLow() const {
            return cLow_;
        }

        double getCUp() const {
            return cUp_;
        }

        double getWLow() const {
            return wLow_;
        }

        double getWUp() const {
            return wUp_;
        }

        double getAxis2Low() const {
            return axis2Low_;
        }

        double getAxis2Up() const {
            return axis2Up_;
        }

        std::pair<double, double> getLimits(char section) const;

        CableWrapType getCableWrapType() const {
            return cableWrapType_;
        }

    private:
        CableWrapType cableWrapType_;

        double axis1Low_; ///< lower limit of first axis in radians
        double axis1Up_; ///< upper limit of first axis in radians
        double axis2Low_; ///< lower limit of second axis in radians
        double axis2Up_; ///< upper limit of second axis in radians

        double axis1LowOffset_ = 5 * deg2rad; ///< safety margin for lower limit for first axis in radians
        double axis1UpOffset_ = 5 * deg2rad; ///< safety margin for upper limit for first axis in radians
        double axis2LowOffset_ = 0 * deg2rad; ///< safety margin for lower limit for second axis in radians
        double axis2UpOffset_ = 0 * deg2rad; ///< safety margin for upper limit for second axis in radians

        double nLow_; ///< lower limit for neutral cable wrap range
        double nUp_; ///< upper limit for neutral cable wrap range
        double cLow_; ///< lower limit for clockwise cable wrap range
        double cUp_; ///< upper limit for clockwise cable wrap range
        double wLow_; ///< lower limit for counter clockwise cable wrap range
        double wUp_; ///< upper limit for counter clockwise cable wrap range
    };
}
#endif /* CABLEWRAP_H */

