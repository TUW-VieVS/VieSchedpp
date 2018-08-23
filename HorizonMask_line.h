//
// Created by matth on 11.04.2018.
//

#ifndef HORIZONMASK_LINE_H
#define HORIZONMASK_LINE_H

#include "AbstractHorizonMask.h"

namespace VieVS{
    class HorizonMask_line : public AbstractHorizonMask {
    public:
        HorizonMask_line(const std::vector<double> &azimuths, const std::vector<double> &elevations);

        bool visible(const PointingVector &pv) const noexcept override;

        std::string vexOutput() const noexcept override;

        std::pair<std::vector<double>, std::vector<double>> getHorizonMask() const noexcept override;

    private:
        std::vector<double> azimuth_; ///< horizon mask knots in radians
        std::vector<double> elevation_; ///< minimum elevation values in radians

        double az2el(double az) const noexcept;
    };
}


#endif //VLBI_SCHEDULER_HORIZONMASK_LINE_H
