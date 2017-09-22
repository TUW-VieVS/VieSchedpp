/**
 * @file HorizonMask.h
 * @brief class HorizonMask
 *
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 */

#ifndef HORIOZONMASK_H
#define HORIOZONMASK_H
#include <vector>
#include <cmath>

#include "Constants.h"
#include "PointingVector.h"

namespace VieVS{
    /**
     * @class HorizonMask
     * @brief representation of VLBI horizon mask
     *
     * @author Matthias Schartner
     * @date 27.06.2017
     */
    class HorizonMask {
    public:
        /**
         * @brief horizon mask type
         */
        enum class Category {
            step, ///< step function
            line, ///< line function
            none ///< no mask
        };

        static double minElevation; ///< minimum required elevation

        /**
         * @brief empty default constructor
         */
        HorizonMask();

        /**
         * @brief constructor
         *
         * The information in el_mask_deg is always alternating azimuth and elevation
         * if there is an even number of elements in el_mask_deg a line function model is used, otherwise a step
         * function model is used.
         *
         * @param el_mask_deg horizon mask information in degrees
         */
        explicit HorizonMask(const std::vector<double> &el_mask_deg);


        /**
         * @brief default copy constructor
         *
         * @param other other horizon mask
         */
        HorizonMask(const HorizonMask &other) = default;

        /**
         * @brief default move constructor
         *
         * @param other other horizon mask
         */
        HorizonMask(HorizonMask &&other) = default;

        /**
         * @brief default copy assignment operator
         *
         * @param other other horizon mask
         * @return copy of other horizon mask
         */
        HorizonMask &operator=(const HorizonMask &other) = default;

        /**
         * @brief default move assignment operator
         *
         * @param other other horizon mask
         * @return moved other horizon mask
         */
        HorizonMask &operator=(HorizonMask &&other) = default;


        /**
         * @brief destructor
         */
        virtual ~HorizonMask();

        /**
         * checks if a pointing vector is visible
         *
         * @param pv pointing vector whose azimuth and elevation is to check
         * @return true if visible, false if not visible
         */
        bool visible(const PointingVector &pv) const noexcept;
        
    private:
        std::vector<double> azimuth_; ///< horizon mask knots in radians
        std::vector<double> elevation_; ///< minimum elevation values in radians
        Category type_; ///< horizon mask type
    };
}
#endif /* HORIOZONMASK_H */

