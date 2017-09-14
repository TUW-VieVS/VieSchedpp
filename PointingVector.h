/**
 * @file VLBI_pointingVector.h
 * @brief class VLBI_pointingVector
 *
 *
 * @author Matthias Schartner
 * @date 06.07.2017
 */

#ifndef VLBI_POINTINGVECTOR_H
#define VLBI_POINTINGVECTOR_H
#include <iostream>
#include <boost/date_time.hpp>

#include "Constants.h"

namespace VieVS{
    /**
     * @class VLBI_pointingVector
     * @brief representation of VLBI pointing vector
     *
     * @author Matthias Schartner
     * @date 06.07.2017
     */
    class PointingVector {
    public:
        /**
         * @brief empty default constructor
         */
        PointingVector();

        /**
         * @brief constructor
         *
         * @param staid id of station
         * @param srcid id of observed source
         */
        PointingVector(int staid, int srcid);

        /**
         * @brief default copy constructor
         *
         * @param other other pointing vector
         */
        PointingVector(const PointingVector &other) = default;

        /**
         * @brief default move constructor
         *
         * @param other other pointing vector
         */
        PointingVector(PointingVector &&other) = default;

        /**
         * @brief default copy assignment operator
         *
         * @param other other pointing vector
         * @return copy of other pointing vector
         */
        PointingVector &operator=(const PointingVector &other) = default;

        /**
         * @brief default move assignment operator
         *
         * @param other other pointing vector
         * @return moved other pointing vector
         */
        PointingVector &operator=(PointingVector &&other) = default;

        /**
         * @brief destructor
         */
        virtual ~PointingVector() {}

        /**
         * @brief getter for station id
         *
         * @return id of station
         */
        int getStaid() const noexcept {
            return staid_;
        }

        /**
         * @brief getter for source id
         *
         * @return id of source
         */
        int getSrcid() const noexcept {
            return srcid_;
        }

        /**
         * @brief getter for azimuth
         *
         * @return azimth in radians
         */
        double getAz() const noexcept {
            return az_;
        }

        /**
         * @brief getter for elevation
         *
         * @return elevation in radians
         */
        double getEl() const noexcept {
            return el_;
        }

        /**
         * @brief getter for time for which azimuth and elevation is calculated
         *
         * @return time in seconds since session start
         */
        unsigned int getTime() const noexcept {
            return time_;
        }

        /**
         * @brief sets azimuth
         *
         * @param new_az new azimuth
         */
        void setAz(double new_az) noexcept {
            az_ = new_az;
        };

        /**
         * @brief sets elevation
         *
         * @param new_el new elevation
         */
        void setEl(double new_el) noexcept {
            el_ = new_el;
        };

        /**
         * @brief sets time for which azimth and elevation is calculated
         *
         * @param new_time new time
         */
        void setTime(unsigned int new_time) noexcept {
            time_ = new_time;
        };
        
    private:
        int staid_; ///< station id
        int srcid_; ///< source id
        double az_; ///< azimth
        double el_; ///< elevation
        unsigned int time_; ///< time in seconds since session start for which azimth and elevation is calculated
    };
}
#endif /* VLBI_POINTINGVECTOR_H */

