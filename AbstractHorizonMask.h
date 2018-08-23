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
#include "VieVS_Object.h"

namespace VieVS{
    /**
     * @class HorizonMask
     * @brief representation of VLBI horizon mask
     *
     * @author Matthias Schartner
     * @date 27.06.2017
     */
    class AbstractHorizonMask : public VieVS_Object {
    public:
        AbstractHorizonMask();

        /**
         * checks if a pointing vector is visible
         *
         * @param pv pointing vector whose azimuth and elevation is to check
         * @return true if visible, false if not visible
         */
        virtual bool visible(const PointingVector &pv) const noexcept = 0;

        virtual std::string vexOutput() const noexcept = 0;

        virtual std::pair<std::vector<double>, std::vector<double>> getHorizonMask() const noexcept  = 0;

    private:
        static unsigned long nextId;
    };
}
#endif /* HORIOZONMASK_H */

