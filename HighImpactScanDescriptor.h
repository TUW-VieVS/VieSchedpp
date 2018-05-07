//
// Created by mschartn on 26.04.18.
//

#ifndef VLBI_SCHEDULER_HIGHIMPACTSCANDESCRIPTOR_H
#define VLBI_SCHEDULER_HIGHIMPACTSCANDESCRIPTOR_H

#include <vector>

#include "VieVS_Object.h"
#include "Scan.h"
#include "Subcon.h"
#include "Subnetting.h"

namespace VieVS{

    class HighImpactScanDescriptor: public VieVS_Object {
    public:
        HighImpactScanDescriptor(unsigned int interval, unsigned int minTimeBetweenScans);

        void addAzElDescriptor(double az, double el, double margin, const std::vector<int> &staids);

        double highImpactScore(const PointingVector &pv) const;

        double highImpactScore(const Scan& scan) const;

        unsigned int getInterval() const {
            return interval_;
        }

        unsigned int getMinTimeBetweenScans() const {
            return minTimeBetweenScans_;
        }

        std::vector<int> getStationIds() const;

        void possibleHighImpactScans(unsigned int time, const std::vector<Station> &stations, const std::vector<Source> &sources);

        void updateHighImpactScans(const std::vector<Station> &stations, const std::vector<Source> &sources,
                                   const boost::optional<Subnetting> &subnetting);

        std::vector<Scan> highestImpactScans(const std::vector<Station> &stations, const std::vector<Source> &sources);

        bool isCorrectHighImpactScan(const Scan &target, const std::vector<Scan> &scans);

        bool hasMoreScans(){
            return highImpactScans_.getNumberSingleScans()>0 || highImpactScans_.getNumberSubnettingScans()>0;
        }

        void updateLogfile(std::ofstream &bodyLog);

    private:
        class AzElDescriptor;

        static int nextId;
        unsigned int interval_;
        unsigned int minTimeBetweenScans_;
        std::vector<AzElDescriptor> azElDescritors_;

        std::vector< std::map<int,double> > scores_;
        Subcon highImpactScans_;

    };

    class HighImpactScanDescriptor::AzElDescriptor: public VieVS_Object {
    public:
        AzElDescriptor(double az, double el, double margin, std::vector<int> staids);

        double highImpactScore(const PointingVector &pv) const;

        const std::vector<int> &getStaids() const;

    private:
        static int nextId;
        double az_;
        double el_;
        double margin_;
        std::vector<int> staids_;
    };



}


#endif //VLBI_SCHEDULER_HIGHIMPACTSCANDESCRIPTOR_H
