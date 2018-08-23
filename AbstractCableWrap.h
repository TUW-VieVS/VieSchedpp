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

#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#endif

#include "PointingVector.h"
#include "Constants.h"
#include "VieVS_Object.h"

namespace VieVS{
    /**
     * @class CableWrap
     * @brief representation of an VLBI cable wrap
     *
     * @author Matthias Schartner
     * @date 27.06.2017
     */
    class AbstractCableWrap : public VieVS_Object {
    public:


        /**
         * @brief constructor
         *
         * @param axis1_low_deg lower limit for first axis in degrees
         * @param axis1_up_deg upper limit for first axis in degrees
         * @param axis2_low_deg lower limit for second axis in degrees
         * @param axis2_up_deg upper limit for second axis in degrees
         */
        AbstractCableWrap(double axis1_low_deg, double axis1_up_deg, double axis2_low_deg, double axis2_up_deg);

        enum class CableWrapFlag{
            ccw,
            n,
            cw
        };

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

        std::pair<double, double> getLimits(char section) const;

        /**
         * @brief checks if this pointing vectors azimuth and elevation are inside the axis limits
         *
         * @param p pointing vector
         * @return true if inside, otherwise false
         */
        virtual bool anglesInside(const PointingVector &p) const noexcept = 0;

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
         * calcUnwrappedAz(VLBI_pointingVector& old_pointingVector, VLBI_pointingVector& new_pointingVector) INSTEAD
         *
         * @param new_pointingVector new pointing vector whose azimuth should be unwrapped
         * @param az_old current antenna azimuth
         */
        virtual void unwrapAzNearAz(PointingVector &new_pointingVector, double az_old) const noexcept = 0;

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
        void calcUnwrappedAz(const PointingVector &old_pointingVector,
                                     PointingVector &new_pointingVector) const noexcept;


        virtual bool unwrapAzInSection(PointingVector &pv, char section) const noexcept = 0;

        CableWrapFlag cableWrapFlag(const PointingVector &pointingVector) const noexcept{
            return cableWrapFlag(pointingVector.getAz());
        }

        virtual CableWrapFlag cableWrapFlag(double unaz) const noexcept = 0;

        virtual std::pair<std::string, std::string> getMotions() const noexcept = 0;

        virtual std::string vexPointingSectors() const noexcept = 0;

    protected:
        enum class Axis {
            axis1,
            axis2,
        };

        bool axisInsideCableWrap(double ax1, double ax2) const noexcept;

        std::string pointingSector(const std::string &motion1, const std::string &motion2, char section) const noexcept;

        double minLow(Axis axis) const;

        double maxUp(Axis axis) const;
    private:

        static unsigned long nextId;

        double axis1Low_; ///< lower limit of first axis in radians
        double axis1Up_; ///< upper limit of first axis in radians
        double axis2Low_; ///< lower limit of second axis in radians
        double axis2Up_; ///< upper limit of second axis in radians

        double axis1LowOffset_ = 0; ///< safety margin for lower limit for first axis in radians
        double axis1UpOffset_ = 0; ///< safety margin for upper limit for first axis in radians
        double axis2LowOffset_ = 0; ///< safety margin for lower limit for second axis in radians
        double axis2UpOffset_ = 0; ///< safety margin for upper limit for second axis in radians

        double nLow_; ///< lower limit for neutral cable wrap range
        double nUp_; ///< upper limit for neutral cable wrap range
        double cLow_; ///< lower limit for clockwise cable wrap range
        double cUp_; ///< upper limit for clockwise cable wrap range
        double wLow_; ///< lower limit for counter clockwise cable wrap range
        double wUp_; ///< upper limit for counter clockwise cable wrap range
    };
}
#endif /* CABLEWRAP_H */

