//
// Created by matth on 11.04.2018.
//

#ifndef HORIZONMASK_STEP_H
#define HORIZONMASK_STEP_H

#include "HorizonMask.h"

namespace VieVS{
    class HorizonMask_step: public HorizonMask {
    public:
        HorizonMask_step(const std::vector<double> &azimuths, const std::vector<double> &elevations);

        bool visible(const PointingVector &pv) const noexcept override;

        std::string vexOutput() const noexcept override;

        std::pair<std::vector<double>, std::vector<double>> getHorizonMask() const noexcept override;

    private:
        std::vector<double> azimuth_; ///< horizon mask knots in radians
        std::vector<double> elevation_; ///< minimum elevation values in radians

        double az2el(double az) const noexcept;

    };
}


#endif //VLBI_SCHEDULER_HORIZONMASK_STEP_H
