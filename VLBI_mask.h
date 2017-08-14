/**
 * @file VLBI_mask.h
 * @brief class VLBI_mask
 *
 *
 * @author Matthias Schartner
 * @date 27.06.2017
 */

#ifndef VLBI_MASK_H
#define VLBI_MASK_H
#include <vector>
#include <cmath>

#include "VieVS_constants.h"
#include "VLBI_pointingVector.h"

using namespace std;

namespace VieVS{
    class VLBI_mask {
    public:
        /**
         * @brief horizon mask type
         */
        enum class category {
            step, ///< step function
            line, ///< line function
            none ///< no mask
        };

        /**
         * @brief empty default constructor
         */
        VLBI_mask();

        /**
         * @brief constructor
         *
         * @param el_mask_deg horizon mask information in degrees
         */
        VLBI_mask(vector<double> el_mask_deg);

        /**
         * @brief destructor
         */
        virtual ~VLBI_mask();

        /**
         * checks if a pointing vector is visible
         *
         * @param pv pointing vector whose azimuth and elevation is to check
         * @return true if visible, false if not visible
         */
        bool visible(const VLBI_pointingVector &pv);
        
    private:
        vector<double> azimuth; ///< horizon mask knots in radians
        vector<double> elevation; ///< minimum elevation values in radians
        category type; ///< horizon mask type
    };
}
#endif /* VLBI_MASK_H */

