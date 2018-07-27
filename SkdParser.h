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

        void read();

        Scheduler createScheduler();

        std::vector<std::vector<unsigned int>> getScheduledTimes(const std::string &station);

    private:
        static unsigned long nextId;

        std::string filename_;
        unsigned int fieldSystemTimes_ = 0;
        unsigned int preob_ = 0;
        unsigned int midob_ = 0;
        unsigned int postob_ = 0;

        SkdCatalogReader skd_;

        Network network_;
        std::vector<Source> sources_; ///< all sources
        std::vector<Scan> scans_; ///< all scans in schedule

        void createScans(std::ofstream &of);

        void copyScanMembersToObjects(std::ofstream &of);

    };

}


#endif //SKDREADER_H
