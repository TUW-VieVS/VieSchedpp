//
// Created by mschartn on 30.01.18.
//

#ifndef SKDREADER_H
#define SKDREADER_H

#include "SkdCatalogReader.h"
#include "Scheduler.h"

#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/named_scope.hpp>
#endif


namespace VieVS{
    class SkdParser: public VieVS_Object {
    public:

        explicit SkdParser(const std::string &filename);

        void setLogFiles();

        void read();

        Scheduler createScheduler();

        std::map<std::string, std::vector<double>> getFrequencies(){
            return freqs_;
        };

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

        std::map<std::string, std::vector<double>> freqs_;

        void createScans(std::ofstream &of);

        void copyScanMembersToObjects(std::ofstream &of);

    };

}


#endif //SKDREADER_H
