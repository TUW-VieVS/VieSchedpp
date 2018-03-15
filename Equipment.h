/**
 * @file Equipment.h
 * @brief class Equipment
 *
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 */

#ifndef EQUIPMENT_H
#define EQUIPMENT_H

#include <vector>
#include <unordered_map>
#include <iostream>
#include <limits>
#include <cmath>
#include "VieVS_Object.h"


namespace VieVS{
    /**
     * @class Equipment
     * @brief representation of VLBI equipment
     *
     * @author Matthias Schartner
     * @date 27.06.2017
     */
    class Equipment: public VieVS_Object {
    public:

        /**
         * @brief constructor
         *
         * @param SEFDs SEFD per band - key is band name, value is SEFD
         */
        explicit Equipment(const std::unordered_map<std::string, double> &SEFDs);


        /**
         * @brief getter function for antenna elevation dependent SEFD information
         *
         * if the elevation dependent SEFD is smaller than the zenith SEFD, the zenith SEFD is returned.
         *
         * @param band name of band
         * @param el elevation
         * @return SEFD of this band
         */
        double getSEFD(const std::string &band, double el) const noexcept;

        /**
         * @brief getter function for antenna SEFD information
         *
         * @param band name of band
         * @return SEFD of this band
         */
        double getSEFD(const std::string & band) const noexcept {
            return SEFD_.at(band);
        }

        /**
         * @brief returns maximum SEFD of this antenna
         *
         * @return maximum SEFD of this antenna
         */
        double getMaxSEFD() const noexcept;

        /**
         * @brief getter for flag if station has elevation dependent SEFD
         * @return
         */
        bool hasElevationDependentSEFD() const noexcept {
            return elevationDependentSEFD_;
        }


        /**
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param equip equipment information that should be printed to stream
         * @return stream object
         */
        friend std::ostream &operator<<(std::ostream &out, const Equipment &equip) noexcept;

        void setElevationDependentSEFD(const std::unordered_map<std::string, double> &SEFD_y,
                                   const std::unordered_map<std::string, double> &SEFD_c0,
                                   const std::unordered_map<std::string, double> &SEFD_c1);

    private:
        static int nextId;

        std::unordered_map<std::string,double> SEFD_; ///< SEFD information per band

        bool elevationDependentSEFD_;
        std::unordered_map<std::string,double> y_; ///< elevation dependent SEFD parameter "y"
        std::unordered_map<std::string,double> c0_; ///< elevation dependent SEFD parameter "c0"
        std::unordered_map<std::string,double> c1_; ///< elevation dependent SEFD parameter "c1"

    };
}
#endif /* EQUIPMENT_H */

