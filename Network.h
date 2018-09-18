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

#ifndef NETWORK_H
#define NETWORK_H

#include <vector>

#include "VieVS_Object.h"
#include "Station.h"
#include "Baseline.h"
#include "SkyCoverage.h"

namespace VieVS {

    class Network: public VieVS_Object {
    public:
        Network();

        void addStation(Station station);

        const Station &getStation(unsigned long id) const noexcept;
        const Station &getStation(const std::string &name) const noexcept;
        const std::vector<Station> &getStations() const noexcept;

        const Baseline &getBaseline(unsigned long id) const noexcept;
        const Baseline &getBaseline(unsigned long staid1, unsigned long staid2) const noexcept;
        const Baseline &getBaseline(const std::pair<unsigned long, unsigned long> &staids) const noexcept;
        const Baseline &getBaseline(const std::string &name) const noexcept;
        const std::vector<Baseline> &getBaselines() const noexcept;

        const SkyCoverage &getSkyCoverage(unsigned long id) const noexcept;
        const std::vector<SkyCoverage> &getSkyCoverages() const noexcept;

        Station &refStation(unsigned long id);
        Station &refStation(const std::string &name);
        std::vector<Station> &refStations();

        Baseline &refBaseline(unsigned long id);
        Baseline &refBaseline(unsigned long staid1, unsigned long staid2);
        Baseline &refBaseline(const std::pair<unsigned long, unsigned long> &staids);
        Baseline &refBaseline(const std::string &name);
        std::vector<Baseline> &refBaselines();

        SkyCoverage &refSkyCoverage(unsigned long id);
        std::vector<SkyCoverage> &refSkyCoverages();

        unsigned long getBlid(const std::pair<unsigned long, unsigned long> &staids) const noexcept ;
        unsigned long getBlid(unsigned long staid1, unsigned long staid2) const noexcept;

        void setMaxDistBetweenCorrespondingTelescopes(double maxDistBetweenCorrespondingTelescopes) {
            maxDistBetweenCorrespondingTelescopes_ = maxDistBetweenCorrespondingTelescopes;
        }

        unsigned long getNSta() const noexcept {
            return nsta_;
        }

        unsigned long getNBls() const noexcept {
            return nbls_;
        }

        void update(unsigned long nObs, const PointingVector &pointingVector, bool influence = true);


        void update(unsigned long blid, bool influence = true);

        const std::vector<double> &getDxyz(unsigned long staid1, unsigned long staid2) const;

        double calcScore_skyCoverage(const std::vector<PointingVector> &pvs) const;

        double calcScore_skyCoverage(const std::vector<PointingVector> &pvs,
                                     std::unordered_map<unsigned long, double> &staids2skyCoverageScore) const;

        double calcScore_skyCoverage_subnetting(const std::vector<PointingVector> &pvs,
                                                const std::unordered_map<unsigned long, double> &staids2skyCoverageScore) const;

        const std::map<unsigned long, unsigned long> &getStaid2skyCoverageId() const{
            return staids2skyCoverageId_;
        };

    private:
        unsigned long nsta_;
        unsigned long nbls_;
        std::vector<Station> stations_;
        std::vector<Baseline> baselines_;
        std::vector<SkyCoverage> skyCoverages_;

        std::map<std::pair<unsigned long, unsigned long>, unsigned long> staids2blid_;

        static unsigned long nextId;


        std::map<std::pair<unsigned long, unsigned long>, std::vector<double>> staids2dxyz_;

        double maxDistBetweenCorrespondingTelescopes_;
        std::map<unsigned long, unsigned long> staids2skyCoverageId_;

    };
}

#endif //NETWORK_H