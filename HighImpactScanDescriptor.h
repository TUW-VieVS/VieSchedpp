/* 
 *  VieSched++ Very Long Baseline Interferometry (VLBI) Scheduling Software
 *  Copyright (C) 2018  Matthias Schartner
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

        void addAzElDescriptor(double az, double el, double margin, const std::vector<unsigned long> &staids);

        double highImpactScore(const PointingVector &pv) const;

        double highImpactScore(const Scan& scan) const;

        unsigned int getInterval() const {
            return interval_;
        }

        unsigned int getMinTimeBetweenScans() const {
            return minTimeBetweenScans_;
        }

        std::vector<unsigned long> getStationIds() const;

        void possibleHighImpactScans(unsigned int time, const Network &network, const std::vector<Source> &sources);

        void updateHighImpactScans(const Network &network, const std::vector<Source> &sources,
                                   const boost::optional<Subnetting> &subnetting);

        std::vector<Scan> highestImpactScans(Network &network, const std::vector<Source> &sources);

        bool isCorrectHighImpactScan(const Scan &target, const std::vector<Scan> &scans, const Source &source);

        bool hasMoreScans(){
            return highImpactScans_.getNumberSingleScans()>0 || highImpactScans_.getNumberSubnettingScans()>0;
        }

        void updateLogfile(std::ofstream &of);

    private:
        class AzElDescriptor;

        static unsigned long nextId;
        unsigned int interval_;
        unsigned int minTimeBetweenScans_;
        std::vector<AzElDescriptor> azElDescritors_;

        std::vector< std::map<unsigned long, double> > scores_;
        Subcon highImpactScans_;

    };

    class HighImpactScanDescriptor::AzElDescriptor: public VieVS_Object {
    public:
        AzElDescriptor(double az, double el, double margin, std::vector<unsigned long> staids);

        double highImpactScore(const PointingVector &pv) const;

        const std::vector<unsigned long> &getStaids() const;

    private:
        static unsigned long nextId;
        double az_;
        double el_;
        double margin_;
        std::vector<unsigned long> staids_;
    };



}


#endif //VLBI_SCHEDULER_HIGHIMPACTSCANDESCRIPTOR_H
