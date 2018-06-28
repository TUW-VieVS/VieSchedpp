//
// Created by matth on 23.06.2018.
//

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
