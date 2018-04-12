//
// Created by mschartn on 05.04.18.
//

#ifndef VLBI_SCHEDULER_FLUX_B_H
#define VLBI_SCHEDULER_FLUX_B_H

#include "Flux.h"

namespace VieVS{
    class Flux_B: public Flux {
    public:
        Flux_B(double wavelength, const std::vector<double> &knots, const std::vector<double> &values);

        double getMaximumFlux()  const noexcept override;

        double observedFlux(double u, double v) const noexcept override;

    private:
//        Flux_B* do_clone() const override;

        std::vector<double> knots_; ///< baseline length of flux information (type B)
        std::vector<double> values_; ///< corresponding flux information for baseline length (type B)
    };
}


#endif //VLBI_SCHEDULER_FLUX_B_H
