//
// Created by mschartn on 23.08.17.
//

#include "VieVS_xmlWriter.h"

namespace VieVS {
    VieVS::VieVS_xmlWriter::VieVS_xmlWriter() {

    }

    void VieVS_xmlWriter::software(const std::string &name, const std::string &version,
                                   const boost::posix_time::ptime &created) {
        boost::property_tree::ptree software;
        software.add("software.name", name);
        software.add("software.version", version);
        software.add("software.created", created);

        master.add_child("master.software", software.get_child("software"));

    }

    void VieVS_xmlWriter::general(const std::string &experimentName, const std::string &experimentDescription,
                                  const boost::posix_time::ptime &startTime, const boost::posix_time::ptime &endTime,
                                  int maxDistanceTwinTeleskopes, bool subnetting, bool fillinmode,
                                  const std::vector<std::string> &stations) {
        boost::property_tree::ptree general;
        general.add("general.experimentName", experimentName);
        general.add("general.experimentDescription", experimentDescription);
        general.add("general.startTime", startTime);
        general.add("general.endTime", endTime);
        general.add("general.maxDistanceTwinTeleskopes", maxDistanceTwinTeleskopes);
        general.add("general.subnetting", subnetting);
        general.add("general.fillinmode", fillinmode);

        boost::property_tree::ptree all_stations;
        for (const auto &any: stations) {
            boost::property_tree::ptree tmp;
            tmp.add("station", any);
            all_stations.add_child("stations.station", tmp.get_child("station"));
        }
        general.add_child("general.stations", all_stations.get_child("stations"));

        master.add_child("master.general", general.get_child("general"));
    }

    void VieVS_xmlWriter::catalogs(const std::string &root, const std::string &antenna, const std::string &equip,
                                   const std::string &flux, const std::string &freq, const std::string &hdpos,
                                   const std::string &loif, const std::string &mask, const std::string &modes,
                                   const std::string &position, const std::string &rec, const std::string &rx,
                                   const std::string &source) {
        boost::property_tree::ptree catalogs;
        catalogs.add("catalogs.root", root);
        catalogs.add("catalogs.antenna", antenna);
        catalogs.add("catalogs.equip", equip);
        catalogs.add("catalogs.flux", flux);
        catalogs.add("catalogs.freq", freq);
        catalogs.add("catalogs.hdpos", hdpos);
        catalogs.add("catalogs.loif", loif);
        catalogs.add("catalogs.mask", mask);
        catalogs.add("catalogs.modes", modes);
        catalogs.add("catalogs.position", position);
        catalogs.add("catalogs.rec", rec);
        catalogs.add("catalogs.rx", rx);
        catalogs.add("catalogs.source", source);

        master.add_child("master.catalogs", catalogs.get_child("catalogs"));
    }

    void VieVS_xmlWriter::group(VieVS_xmlWriter::TYPE type, VieVS_parameterGroup group) {
        boost::property_tree::ptree pt_group;
        pt_group.add("group.<xmlattr>.name", group.name);

        for (const auto &any:group.members) {
            boost::property_tree::ptree tmp;
            tmp.add("member", any);
            pt_group.add_child("group.member", tmp.get_child("member"));
        }

        if (type == VieVS_xmlWriter::TYPE::station) {
            master.add_child("master.station.group", pt_group.get_child("group"));
            group_station[group.name] = group.members;
        } else if (type == VieVS_xmlWriter::TYPE::source) {
            master.add_child("master.source.group", pt_group.get_child("group"));
            group_source[group.name] = group.members;
        } else if (type == VieVS_xmlWriter::TYPE::baseline) {
            master.add_child("master.baseline.group", pt_group.get_child("group"));
            group_baseline[group.name] = group.members;
        }
    }

    const std::vector<std::string> &
    VieVS_xmlWriter::getGroupMembers(VieVS_xmlWriter::TYPE type, std::string groupName) {

        if (type == VieVS_xmlWriter::TYPE::station) {
            return group_station[groupName];
        } else if (type == VieVS_xmlWriter::TYPE::source) {
            return group_source[groupName];
        } else if (type == VieVS_xmlWriter::TYPE::baseline) {
            return group_baseline[groupName];
        }

    }

    void VieVS_xmlWriter::parameters(const std::string &name, VLBI_station::PARAMETERS PARA) {
        boost::property_tree::ptree parameters;
        if (PARA.available.is_initialized()) {
            parameters.add("parameters.available", PARA.available);
        }
        if (PARA.firstScan.is_initialized()) {
            parameters.add("parameters.firstScan", PARA.firstScan);
        }

        if (PARA.weight.is_initialized()) {
            parameters.add("parameters.weight", PARA.weight);
        }

        if (PARA.minScan.is_initialized()) {
            parameters.add("parameters.minScan", PARA.minScan);
        }
        if (PARA.maxScan.is_initialized()) {
            parameters.add("parameters.maxScan", PARA.maxScan);
        }
        if (PARA.maxSlewtime.is_initialized()) {
            parameters.add("parameters.maxSlewtime", PARA.maxSlewtime);
        }
        if (PARA.maxWait.is_initialized()) {
            parameters.add("parameters.maxWait", PARA.maxWait);
        }

        if (PARA.wait_calibration.is_initialized()) {
            parameters.add("parameters.wait_calibration", PARA.wait_calibration);
        }
        if (PARA.wait_corsynch.is_initialized()) {
            parameters.add("parameters.wait_corsynch", PARA.wait_corsynch);
        }
        if (PARA.wait_setup.is_initialized()) {
            parameters.add("parameters.wait_setup", PARA.wait_setup);
        }
        if (PARA.wait_source.is_initialized()) {
            parameters.add("parameters.wait_source", PARA.wait_source);
        }
        if (PARA.wait_tape.is_initialized()) {
            parameters.add("parameters.wait_tape", PARA.wait_tape);
        }
        if (!PARA.minSNR.empty()) {
            for (const auto &any:PARA.minSNR) {
                boost::property_tree::ptree minSNR;
                minSNR.add("minSNR", any.second);
                minSNR.put("minSNR.<xmlattr>.band", any.first);
                parameters.add_child("parameters.minSNR", minSNR.get_child("minSNR"));
            }
        }

        if (!PARA.ignoreSources_str.empty()) {
            boost::property_tree::ptree ignoreSources;
            for (const auto &any:PARA.ignoreSources_str) {
                boost::property_tree::ptree sourceName;
                sourceName.add("source", any);
                ignoreSources.add_child("ignoreSources.source", sourceName.get_child("source"));
            }
            parameters.add_child("parameters.ignoreSources", ignoreSources.get_child("ignoreSources"));
        }

        parameters.add("parameters.<xmlattr>.name", name);

        master.add_child("master.station.parameters", parameters.get_child("parameters"));
    }

    void VieVS_xmlWriter::parameters(const std::string &name, VLBI_source::PARAMETERS PARA) {
        boost::property_tree::ptree parameters;

        if (!PARA.available) {
            parameters.add("parameters.available", PARA.available);
        }

        if (PARA.weight.is_initialized()) {
            parameters.add("parameters.weight", PARA.weight);
        }

        if (PARA.minScan.is_initialized()) {
            parameters.add("parameters.minScan", PARA.minScan);
        }
        if (PARA.maxScan.is_initialized()) {
            parameters.add("parameters.maxScan", PARA.maxScan);
        }

        if (PARA.minNumberOfStations.is_initialized()) {
            parameters.add("parameters.minNumberOfStations", PARA.minNumberOfStations);
        }
        if (PARA.minRepeat.is_initialized()) {
            parameters.add("parameters.minRepeat", PARA.minRepeat);
        }
        if (PARA.minFlux.is_initialized()) {
            parameters.add("parameters.minFlux", PARA.minFlux);
        }

        if (PARA.fixedScanDuration.is_initialized()) {
            parameters.add("parameters.fixedScanDuration", *PARA.fixedScanDuration);
        }
        if (PARA.minNumberOfStations.is_initialized()) {
            parameters.add("parameters.minNumberOfStations", *PARA.minNumberOfStations);
        }
        if (PARA.tryToFocusIfObservedOnce.is_initialized()) {
            parameters.add("parameters.tryToFocusIfObservedOnce", *PARA.tryToFocusIfObservedOnce);
        }

        for (const auto &any:PARA.minSNR) {
            boost::property_tree::ptree minSNR;
            minSNR.add("minSNR", any.second);
            minSNR.put("minSNR.<xmlattr>.band", any.first);
            parameters.add_child("parameters.minSNR", minSNR.get_child("minSNR"));
        }

        if (!PARA.ignoreStations_str.empty()) {
            boost::property_tree::ptree ignoreStations;
            for (const auto &any:PARA.ignoreStations_str) {
                boost::property_tree::ptree stationName;
                stationName.add("station", any);
                ignoreStations.add_child("ignoreStations.station", stationName.get_child("station"));
            }
            parameters.add_child("parameters.ignoreStations", ignoreStations.get_child("ignoreStations"));
        }

        if (!PARA.requiredStations_str.empty()) {
            boost::property_tree::ptree requiredStations;
            for (const auto &any:PARA.requiredStations_str) {
                boost::property_tree::ptree stationName;
                stationName.add("station", any);
                requiredStations.add_child("requiredStations.station", stationName.get_child("station"));
            }
            parameters.add_child("parameters.requiredStations", requiredStations.get_child("requiredStations"));
        }

        if (!PARA.ignoreBaselines_str.empty()) {
            boost::property_tree::ptree ignoreBaselines;
            for (const auto &any:PARA.ignoreBaselines_str) {
                boost::property_tree::ptree baselineName;
                string bname = any.first + "-" + any.second;
                baselineName.add("baseline", bname);
                ignoreBaselines.add_child("ignoreBaselines.baseline", baselineName.get_child("baseline"));
            }
            parameters.add_child("parameters.ignoreBaselines", ignoreBaselines.get_child("ignoreBaselines"));
        }


        parameters.add("parameters.<xmlattr>.name", name);

        master.add_child("master.source.parameters", parameters.get_child("parameters"));

    }

    void VieVS_xmlWriter::parameters(const std::string &name, VLBI_baseline::PARAMETERS PARA) {
        boost::property_tree::ptree parameters;

        if (PARA.ignore.is_initialized()) {
            parameters.add("parameters.ignore", PARA.ignore);
        }
        if (PARA.minScan.is_initialized()) {
            parameters.add("parameters.minScan", PARA.minScan);
        }
        if (PARA.maxScan.is_initialized()) {
            parameters.add("parameters.maxScan", PARA.maxScan);
        }
        if (PARA.weight.is_initialized()) {
            parameters.add("parameters.weight", PARA.weight);
        }

        for (const auto &any: PARA.minSNR) {
            boost::property_tree::ptree minSNR;
            minSNR.add("minSNR", any.second);
            minSNR.put("minSNR.<xmlattr>.band", any.first);
            parameters.add_child("parameters.minSNR", minSNR.get_child("minSNR"));
        }

        parameters.add("parameters.<xmlattr>.name", name);

        master.add_child("master.baseline.parameters", parameters.get_child("parameters"));

    }

    void VieVS_xmlWriter::setup(VieVS_xmlWriter::TYPE type, const VLBI_setup &setup) {

        boost::property_tree::ptree root;

        if (setup.getChildren().size() > 0) {
            for (const auto &any:setup.getChildren()) {
                boost::property_tree::ptree thisChildTree = getChildTree(any);
                root.add_child("setup", thisChildTree.get_child("root"));
            }
        }

        if (type == VieVS_xmlWriter::TYPE::station) {
            for (const auto &it:root) {
                master.add_child("master.station.setup", it.second);
            }
        } else if (type == VieVS_xmlWriter::TYPE::source) {
            for (const auto &it:root) {
                master.add_child("master.source.setup", it.second);
            }
        } else if (type == VieVS_xmlWriter::TYPE::baseline) {
            for (const auto &it:root) {
                master.add_child("master.baseline.setup", it.second);
            }
        }
    }

    boost::property_tree::ptree VieVS_xmlWriter::getChildTree(const VLBI_setup &setup) {
        boost::property_tree::ptree root;
        boost::posix_time::ptime start = master.get<boost::posix_time::ptime>("master.general.startTime");
        boost::posix_time::ptime end = master.get<boost::posix_time::ptime>("master.general.endTime");

        boost::posix_time::time_duration a = end - start;
        int sec = a.total_seconds();
        unsigned int duration = (unsigned int) sec;

        if (setup.getChildren().size() > 0) {
            const std::vector<string> &members = setup.getMembers();
            const string &memberName = setup.getMemberName();
            if (members.size() == 1 && members[0] == memberName) {
                root.add("root.member", setup.getMemberName());
            } else {
                root.add("root.group", setup.getMemberName());
            }
            root.add("root.parameter", setup.getParameterName());

            unsigned int thisStart = setup.getStart();
            if (thisStart != 0) {
                root.add("root.start", start + boost::posix_time::seconds(thisStart));
            }

            unsigned int thisEnd = setup.getEnd();
            if (thisEnd < duration) {
                root.add("root.end", start + boost::posix_time::seconds(thisEnd));
            }

            if (setup.getTransition() != VLBI_setup::TRANSITION::soft) {
                root.add("root.transition", "hard");
            }

            for (const auto &any:setup.getChildren()) {
                boost::property_tree::ptree thisChildTree = getChildTree(any);
                root.add_child("root.setup", thisChildTree.get_child("root"));
            }
        } else {
            const std::vector<string> &members = setup.getMembers();
            const string &memberName = setup.getMemberName();

            if (members.size() == 1 && members[0] == memberName) {
                root.add("root.member", setup.getMemberName());
            } else {
                root.add("root.group", setup.getMemberName());
            }
            root.add("root.parameter", setup.getParameterName());
            unsigned int thisStart = setup.getStart();
            if (thisStart != 0) {
                root.add("root.start", start + boost::posix_time::seconds(thisStart));
            }

            unsigned int thisEnd = setup.getEnd();
            if (thisEnd < duration) {
                root.add("root.end", start + boost::posix_time::seconds(thisEnd));
            }
            if (setup.getTransition() != VLBI_setup::TRANSITION::soft) {
                root.add("root.transition", "hard");
            }

        }
        return root;
    }


    void VieVS_xmlWriter::skyCoverage(double influenceDistance, double influenceInterval) {
        boost::property_tree::ptree skyCoverage;
        skyCoverage.add("skyCoverage.influenceDistance", influenceDistance);
        skyCoverage.add("skyCoverage.influenceInterval", influenceInterval);

        master.add_child("master.skyCoverage", skyCoverage.get_child("skyCoverage"));
    }

    void
    VieVS_xmlWriter::weightFactor(double weight_skyCoverage, double weight_numberOfObservations, double weight_duration,
                                  double weight_averageSources, double weight_averageStations) {
        boost::property_tree::ptree weightFactor;
        weightFactor.add("weightFactor.skyCoverage", weight_skyCoverage);
        weightFactor.add("weightFactor.numberOfObservations", weight_numberOfObservations);
        weightFactor.add("weightFactor.duration", weight_duration);
        weightFactor.add("weightFactor.averageSources", weight_averageSources);
        weightFactor.add("weightFactor.averageStations", weight_averageStations);

        master.add_child("master.weightFactor", weightFactor.get_child("weightFactor"));
    }

    void VieVS_xmlWriter::mode(unsigned int bandwith, unsigned int sampleRate, unsigned int fanout, unsigned int bits) {
        boost::property_tree::ptree mode;
        mode.add("mode.bandwith", bandwith);
        mode.add("mode.sampleRate", sampleRate);
        mode.add("mode.fanout", fanout);
        mode.add("mode.bits", bits);

        master.add_child("master.mode", mode.get_child("mode"));
    }

    void VieVS_xmlWriter::mode_band(const std::string &name, double wavelength, VLBI_obsMode::PROPERTY flag,
                                    unsigned int chanels) {
        boost::property_tree::ptree band;
        band.add("band.wavelength", wavelength);
        if (flag == VLBI_obsMode::PROPERTY::required) {
            band.add("band.property", "required");
        } else if (flag == VLBI_obsMode::PROPERTY::optional) {
            band.add("band.property", "optional");
        }
        band.add("band.chanels", chanels);
        band.add("band.<xmlattr>.name", name);

        master.add_child("master.mode.band", band.get_child("band"));

    }

    void VieVS_xmlWriter::write(const std::string &name) {
        std::ofstream os;
        os.open(name);
        boost::property_tree::xml_parser::write_xml(os, master,
                                                    boost::property_tree::xml_writer_make_settings<string>('\t', 1));
        os.close();
    }

    void VieVS_xmlWriter::multisched(const VLBI_multiSched &ms) {
        boost::property_tree::ptree ms_tree = ms.createPropertyTree();

        master.add_child("master.multisched", ms_tree.get_child("multisched"));

    }


}
