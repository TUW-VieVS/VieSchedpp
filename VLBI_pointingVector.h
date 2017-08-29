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

#include "VieVS_constants.h"

using namespace std;

namespace VieVS{
    class VLBI_pointingVector {
    public:
        /**
         * @brief empty default constructor
         */
        VLBI_pointingVector();

        /**
         * @brief constructor
         *
         * @param staid id of station
         * @param srcid id of observed source
         */
        VLBI_pointingVector(int staid, int srcid);

        /**
         * @brief default copy constructor
         *
         * @param other other pointing vector
         */
        VLBI_pointingVector(const VLBI_pointingVector &other) = default;

        /**
         * @brief default move constructor
         *
         * @param other other pointing vector
         */
        VLBI_pointingVector(VLBI_pointingVector &&other) = default;

        /**
         * @brief default copy assignment operator
         *
         * @param other other pointing vector
         * @return copy of other pointing vector
         */
        VLBI_pointingVector &operator=(const VLBI_pointingVector &other) = default;

        /**
         * @brief default move assignment operator
         *
         * @param other other pointing vector
         * @return moved other pointing vector
         */
        VLBI_pointingVector &operator=(VLBI_pointingVector &&other) = default;

        /**
         * @brief destructor
         */
        virtual ~VLBI_pointingVector() {}

        /**
         * @brief getter for station id
         *
         * @return id of station
         */
        int getStaid() const noexcept {
            return staid;
        }

        /**
         * @brief getter for source id
         *
         * @return id of source
         */
        int getSrcid() const noexcept {
            return srcid;
        }

        /**
         * @brief getter for azimuth
         *
         * @return azimth in radians
         */
        double getAz() const noexcept {
            return az;
        }

        /**
         * @brief getter for elevation
         *
         * @return elevation in radians
         */
        double getEl() const noexcept {
            return el;
        }

        /**
         * @brief getter for time for which azimuth and elevation is calculated
         *
         * @return time in seconds since session start
         */
        unsigned int getTime() const noexcept {
            return time;
        }

        /**
         * @brief sets azimth
         *
         * @param new_az new azimuth
         */
        void setAz(double new_az) noexcept {
            az = new_az;
        };

        /**
         * @brief sets elevation
         *
         * @param new_el new elevation
         */
        void setEl(double new_el) noexcept {
            el = new_el;
        };

        /**
         * @brief sets time for which azimth and elevation is calculated
         *
         * @param new_time new time
         */
        void setTime(unsigned int new_time) noexcept {
            time = new_time;
        };
        
    private:
        int staid; ///< station id
        int srcid; ///< source id
        double az; ///< azimth
        double el; ///< elevation
        unsigned int time; ///< time in seconds since session start for which azimth and elevation is calculated
    };
}
#endif /* VLBI_POINTINGVECTOR_H */

