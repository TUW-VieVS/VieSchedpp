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


namespace VieVS{
    /**
     * @class Equipment
     * @brief representation of VLBI equipment
     *
     * @author Matthias Schartner
     * @date 27.06.2017
     */
    class Equipment {
    public:
        /**
         * @brief empty default constructor
         */
        Equipment();

        /**
         * @brief constructor
         *
         * @param SEFDs all channel SEFDs - key is band name, value is SEFD
         */
        explicit Equipment(const std::unordered_map<std::string, double> SEFDs);

        /**
         * @brief default copy constructor
         *
         * @param other other equip
         */
        Equipment(const Equipment &other) = default;

        /**
         * @brief default move constructor
         *
         * @param other other equip
         */
        Equipment(Equipment &&other) = default;

        /**
         * @brief default copy assignment operator
         *
         * @param other other equip
         * @return copy of other equip
         */
        Equipment &operator=(const Equipment &other) = default;

        /**
         * @brief default move assignment operator
         *
         * @param other other equip
         * @return moved other equip
         */
        Equipment &operator=(Equipment &&other) = default;


        /**
         * @brief destructor
         */
        virtual ~Equipment();

        /**
         * @brief getter function for antenna SEFD information
         *
         * @param band name of band
         * @return SEFD of this band
         */
        double getSEFD(std::string band) const noexcept {
            return SEFD_.at(band);
        }

        /**
         * @brief returns maximum SEFD of this antenna
         *
         * @return maximum SEFD of this antenna
         */
        double getMaxSEFD() const noexcept;

        /**
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param equip equipment information that should be printed to stream
         * @return stream object
         */
        friend std::ostream &operator<<(std::ostream &out, const Equipment &equip) noexcept;
        
    private:
        std::unordered_map<std::string,double> SEFD_; ///< SEFD information per band
    };
}
#endif /* EQUIPMENT_H */

