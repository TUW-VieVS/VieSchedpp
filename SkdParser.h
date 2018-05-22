//
// Created by mschartn on 30.01.18.
//

#ifndef SKDREADER_H
#define SKDREADER_H

#include "SkdCatalogReader.h"
#include "Scheduler.h"

namespace VieVS{
    class SkdParser: public VieVS_Object {
    public:

        explicit SkdParser(const std::string &filename);

        void createObjects();

        void createScans();

        void copyScanMembersToObjects();

        Scheduler createScheduler();

        std::vector<std::vector<unsigned int>> getScheduledTimes(const std::string &station);

    private:
        static int nextId;

        std::string filename_;
        unsigned int fieldSystemTimes_ = 0;

        SkdCatalogReader skd_;

        std::vector<Station> stations_; ///< all stations
        std::vector<Source> sources_; ///< all sources
        std::vector<SkyCoverage> skyCoverages_; ///< all sky coverages
        std::vector<Scan> scans_; ///< all scans in schedule


    };

}


#endif //SKDREADER_H
