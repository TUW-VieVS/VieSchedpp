//
// Created by mschartn on 05.04.18.
//

#ifndef VLBI_SCHEDULER_FLUX_M_H
#define VLBI_SCHEDULER_FLUX_M_H

#include "AbstractFlux.h"
namespace VieVS {
    class Flux_M : public AbstractFlux {
    public:
        Flux_M(double wavelength, const std::vector<double> &flux, const std::vector<double> &majorAxis,
               const std::vector<double> &axialRatio, const std::vector<double> &positionAngle);

        double getMaximumFlux() const noexcept override;

        double observedFlux(double u, double v) const noexcept override;

    private:
//        Flux_M* do_clone() const override;

        std::vector<double> flux_; ///< flux density (type M)
        std::vector<double> majorAxis_; ///< major axis angle (type M)
        std::vector<double> axialRatio_; ///< axial ratio (type M)
        std::vector<double> positionAngle_; ///< position angle (type M)

        static double flcon1_; ///< constant precalculated value for model M
//        static double flcon2_; ///< constant precalculated value for model M

    };
}

#endif //VLBI_SCHEDULER_FLUX_M_H
