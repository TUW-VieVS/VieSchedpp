//
// Created by mschartn on 23.08.17.
//

#ifndef VIEVS_XMLWRITER_H
#define VIEVS_XMLWRITER_H

#include <ostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/date_time.hpp>
#include <utility>

#include "VLBI_station.h"
#include "VLBI_source.h"
#include "VLBI_baseline.h"
#include "VLBI_obsMode.h"
#include "VLBI_setup.h"

namespace VieVS {
    class VieVS_xmlWriter {
    public:
        enum class MEMBER {
            single,
            group,
        };

        enum class TYPE {
            station,
            source,
            baseline,
        };


        VieVS_xmlWriter();

        void software(const std::string &name, const std::string &version, const boost::posix_time::ptime &created);

        void general(const std::string &experimentName, const std::string &experimentDescription,
                     const boost::posix_time::ptime &startTime, const boost::posix_time::ptime &endTime,
                     int maxDistanceTwinTeleskopes, bool subnetting, bool fillinmode,
                     const std::vector<std::string> &stations);


        void group(TYPE type, const std::string &name, const std::vector<std::string> &members);

        void parameters(const std::string &name, VLBI_station::PARAMETERS PARA);

        void parameters(const std::string &name, VLBI_source::PARAMETERS PARA);

        void parameters(const std::string &name, VLBI_baseline::PARAMETERS PARA);

        void setup(TYPE type, const VLBI_setup &setup);

        void skyCoverage(double influenceDistance, double influenceInterval);

        void weightFactor(double weight_skyCoverage, double weight_numberOfObservations, double weight_duration,
                          double weight_averageSources, double weight_averageStations);

        void mode(unsigned int bandwith, unsigned int sampleRate, unsigned int fanout, unsigned int bits);

        void mode_band(const std::string &name, double wavelength, VLBI_obsMode::PROPERTY flag, unsigned int chanels);

        void write(const std::string &name);

    private:
        boost::property_tree::ptree master;

        boost::property_tree::ptree getChildTree(const VLBI_setup &setup);
    };
}


#endif //VIEVS_XMLWRITER_H
