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
        virtual double getSEFD(const std::string &band, double el) const noexcept{
            return SEFD_.at(band);
        };

        /**
         * @brief returns maximum SEFD of this antenna
         *
         * @return maximum SEFD of this antenna
         */
        double getMaxSEFD() const noexcept;

    private:
        static int nextId;

        std::unordered_map<std::string,double> SEFD_; ///< SEFD information per band
    };
}
#endif /* EQUIPMENT_H */

