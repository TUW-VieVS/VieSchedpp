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
#include "VieVS_constants.h"

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
        
    private:
        vector<double> knots; ///< horizon mask knots in radians
        vector<double> values; ///< minimum elevation values in radians
        category type; ///< horizon mask type
    };
}
#endif /* VLBI_MASK_H */

