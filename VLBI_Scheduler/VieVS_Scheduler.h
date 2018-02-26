//
// Created by matth on 28.01.2018.
//
#ifndef VIEVS_SCHEDULER_H
#define VIEVS_SCHEDULER_H

#include <cstdlib>
#include <chrono>
#include <vector>
#include <boost/format.hpp>
#include <iostream>


#include "Initializer.h"
#include "Scheduler.h"
#include "Output.h"
#include "ParameterSettings.h"

#ifdef _OPENMP
#include <omp.h>
#endif

namespace VieVS {

    class VieVS_Scheduler {
    public:
        VieVS_Scheduler() = default;

        explicit VieVS_Scheduler(const std::string &inputFile);

        void run();

    private:
        std::string inputFile_;
    };
}

#endif //VIEVS_SCHEDULER_H
