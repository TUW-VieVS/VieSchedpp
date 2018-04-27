//
// Created by mschartn on 26.04.18.
//

#ifndef VLBI_SCHEDULER_HIGHIMPACTSCANDESCRIPTOR_H
#define VLBI_SCHEDULER_HIGHIMPACTSCANDESCRIPTOR_H

#include <vector>

#include "VieVS_Object.h"
#include "Scan.h"

namespace VieVS{
    class HighImpactScanDescriptor: public VieVS_Object {
    public:
        HighImpactScanDescriptor(unsigned int interval, unsigned int minTimeBetweenScans);

        void addAzElDescriptor(double az, double azMargin, double el, double elMargin, const std::vector<int> &staids);

        bool isHighImpactScan(const Scan& scan) const;

    private:
        class AzElDescriptor: public VieVS_Object {
        public:
            AzElDescriptor(double az, double azMargin, double el, double elMargin, std::vector<int> staids);

            bool isHighImpactPv(const PointingVector &pv) const;


        private:
            static int nextId;
            double az_;
            double azMargin_;
            double el_;
            double elMargin_;
            std::vector<int> staids_;
        };

        static int nextId;
        unsigned int interval_;
        unsigned int minTimeBetweenScans_;
        std::vector<AzElDescriptor> azElDescritors_;



    };
}


#endif //VLBI_SCHEDULER_HIGHIMPACTSCANDESCRIPTOR_H
