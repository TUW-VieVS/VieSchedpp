//
// Created by mschartn on 30.01.18.
//

#ifndef SKDREADER_H
#define SKDREADER_H

#include "SkdCatalogReader.h"
#include "Station.h"
#include "Source.h"
#include "Scan.h"
#include "SkyCoverage.h"
#include "Initializer.h"

namespace VieVS{
    class SkdParser {
    public:
        SkdParser() = default;

        explicit SkdParser(const std::string &filename);

        void createObjects();

        void createScans();

        void copyScanMembersToObjects();

        std::vector<unsigned int> getScheduledSlewTimes(const std::string &station);

    private:
        std::string filename_;

        SkdCatalogReader skd_;
        std::vector<Station> stations_; ///< all stations
        std::vector<Source> sources_; ///< all sources
        std::vector<SkyCoverage> skyCoverages_; ///< all sky coverages
        std::vector<Scan> scans_; ///< all scans in schedule


    };

}


#endif //SKDREADER_H
