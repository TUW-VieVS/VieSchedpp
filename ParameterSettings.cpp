//
// Created by mschartn on 23.08.17.
//

#include "ParameterSettings.h"
using namespace std;
using namespace VieVS;

VieVS::ParameterSettings::ParameterSettings() {

}

void ParameterSettings::software(const std::string &name, const std::string &version,
                               const boost::posix_time::ptime &created) {
    boost::property_tree::ptree software;
    software.add("software.name", name);
    software.add("software.version", version);
    software.add("software.created", created);

    master_.add_child("master.software", software.get_child("software"));

}

void ParameterSettings::general(const boost::posix_time::ptime &startTime, const boost::posix_time::ptime &endTime,
                                bool subnetting, bool fillinmode, bool fillinmodeInfluenceOnSchedule, double minElevation,
                                const std::vector<std::string> &stations) {
    boost::property_tree::ptree general;
    general.add("general.startTime", startTime);
    general.add("general.endTime", endTime);
    general.add("general.subnetting", subnetting);
    general.add("general.fillinmode", fillinmode);
    general.add("general.fillinmodeInfluenceOnSchedule", fillinmodeInfluenceOnSchedule);
    general.add("general.minElevation", minElevation);

    boost::property_tree::ptree all_stations;
    for (const auto &any: stations) {
        boost::property_tree::ptree tmp;
        tmp.add("station", any);
        all_stations.add_child("stations.station", tmp.get_child("station"));
    }
    general.add_child("general.stations", all_stations.get_child("stations"));

    master_.add_child("master.general", general.get_child("general"));
}

void ParameterSettings::catalogs(const std::string &antenna, const std::string &equip, const std::string &flux,
                                 const std::string &freq, const std::string &hdpos, const std::string &loif,
                                 const std::string &mask, const std::string &modes, const std::string &position,
                                 const std::string &rec, const std::string &rx, const std::string &source,
                                 const std::string &tracks) {
    boost::property_tree::ptree catalogs;
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
    catalogs.add("catalogs.tracks", tracks);

    master_.add_child("master.catalogs", catalogs.get_child("catalogs"));
}

void ParameterSettings::group(ParameterSettings::Type type, ParameterGroup group) {
    boost::property_tree::ptree pt_group;
    pt_group.add("group.<xmlattr>.name", group.name);

    for (const auto &any:group.members) {
        boost::property_tree::ptree tmp;
        tmp.add("member", any);
        pt_group.add_child("group.member", tmp.get_child("member"));
    }

    if (type == ParameterSettings::Type::station) {
        master_.add_child("master.station.group", pt_group.get_child("group"));
        groupStations_[group.name] = group.members;
    } else if (type == ParameterSettings::Type::source) {
        master_.add_child("master.source.group", pt_group.get_child("group"));
        groupSources_[group.name] = group.members;
    } else if (type == ParameterSettings::Type::baseline) {
        master_.add_child("master.baseline.group", pt_group.get_child("group"));
        groupBaselines_[group.name] = group.members;
    }
}

const std::vector<std::string> &
ParameterSettings::getGroupMembers(ParameterSettings::Type type, std::string groupName) {

    if (type == ParameterSettings::Type::station) {
        return groupStations_[groupName];
    } else if (type == ParameterSettings::Type::source) {
        return groupSources_[groupName];
    } else if (type == ParameterSettings::Type::baseline) {
        return groupBaselines_[groupName];
    }

}

void ParameterSettings::parameters(const std::string &name, ParametersStations PARA) {
    boost::property_tree::ptree parameters;
    if (PARA.available.is_initialized()) {
        parameters.add("parameters.available", PARA.available);
    }
    if (PARA.firstScan.is_initialized()) {
        parameters.add("parameters.firstScan", PARA.firstScan);
    }
    if (PARA.tagalong.is_initialized()) {
        parameters.add("parameters.tagalong", PARA.tagalong);
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

    master_.add_child("master.station.parameters", parameters.get_child("parameters"));
}

void ParameterSettings::parameters(const std::string &name, ParametersSources PARA) {
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

    if (PARA.tryToObserveXTimesEvenlyDistributed.is_initialized()){
        parameters.add("parameters.tryToObserveXTimesEvenlyDistributed", *PARA.tryToObserveXTimesEvenlyDistributed);
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

    if (!PARA.ignoreStationsString.empty()) {
        boost::property_tree::ptree ignoreStations;
        for (const auto &any:PARA.ignoreStationsString) {
            boost::property_tree::ptree stationName;
            stationName.add("station", any);
            ignoreStations.add_child("ignoreStations.station", stationName.get_child("station"));
        }
        parameters.add_child("parameters.ignoreStations", ignoreStations.get_child("ignoreStations"));
    }

    if (!PARA.requiredStationsString.empty()) {
        boost::property_tree::ptree requiredStations;
        for (const auto &any:PARA.requiredStationsString) {
            boost::property_tree::ptree stationName;
            stationName.add("station", any);
            requiredStations.add_child("requiredStations.station", stationName.get_child("station"));
        }
        parameters.add_child("parameters.requiredStations", requiredStations.get_child("requiredStations"));
    }

    if (!PARA.ignoreBaselinesString.empty()) {
        boost::property_tree::ptree ignoreBaselines;
        for (const auto &any:PARA.ignoreBaselinesString) {
            boost::property_tree::ptree baselineName;
            string bname = any.first + "-" + any.second;
            baselineName.add("baseline", bname);
            ignoreBaselines.add_child("ignoreBaselines.baseline", baselineName.get_child("baseline"));
        }
        parameters.add_child("parameters.ignoreBaselines", ignoreBaselines.get_child("ignoreBaselines"));
    }


    parameters.add("parameters.<xmlattr>.name", name);

    master_.add_child("master.source.parameters", parameters.get_child("parameters"));

}

void ParameterSettings::parameters(const std::string &name, ParametersBaselines PARA) {
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

    master_.add_child("master.baseline.parameters", parameters.get_child("parameters"));

}

void ParameterSettings::setup(ParameterSettings::Type type, const ParameterSetup &setup) {

    boost::property_tree::ptree root;

    if (!setup.getChildren().empty()) {
        for (const auto &any:setup.getChildren()) {
            boost::property_tree::ptree thisChildTree = getChildTree(any);
            root.add_child("setup", thisChildTree.get_child("root"));
        }
    }

    if (type == ParameterSettings::Type::station) {
        for (const auto &it:root) {
            master_.add_child("master.station.setup", it.second);
        }
    } else if (type == ParameterSettings::Type::source) {
        for (const auto &it:root) {
            master_.add_child("master.source.setup", it.second);
        }
    } else if (type == ParameterSettings::Type::baseline) {
        for (const auto &it:root) {
            master_.add_child("master.baseline.setup", it.second);
        }
    }
}

void
ParameterSettings::stationWaitTimes(const std::string &name, unsigned int setup, unsigned int source, unsigned int tape,
                                    unsigned int calibration, unsigned int corsynch) {

    vector<string> members;
    if (groupStations_.find(name) != groupStations_.end()) {
        members.insert(members.end(), groupStations_[name].begin(), groupStations_[name].end());
    } else {
        members.push_back(name);
    }

    vector<string> membersAlreadyUsed;
    boost::property_tree::ptree PARA_station = master_.get_child("master.station");
    for (const auto &any:PARA_station) {
        if (any.first == "waitTimes") {
            if (name == "__all__") {
                cerr << "ERROR: double use of station/group " << name
                     << " in cable wrap buffer block! This whole block is ignored!;\n";
                return;
            }
            string memberName = any.second.get_child("<xmlattr>.member").data();
            if (groupStations_.find(memberName) != groupStations_.end()) {
                membersAlreadyUsed.insert(membersAlreadyUsed.end(), groupStations_[memberName].begin(),
                                          groupStations_[memberName].end());
            } else if (memberName == "__all__") {
                cerr << "ERROR: double use of station/group " << name
                     << " in wait time block! This whole block is ignored!;\n";
                return;
            } else {
                membersAlreadyUsed.push_back(memberName);
            }
        }
    }
    for (const auto &any:members) {
        if (find(membersAlreadyUsed.begin(), membersAlreadyUsed.end(), any) != membersAlreadyUsed.end()) {
            cerr << "ERROR: double use of station/group " << name
                 << " in wait times block! This whole block is ignored!;\n";
            return;
        }
    }

    boost::property_tree::ptree wtimes;
    wtimes.add("waitTimes.setup", setup);
    wtimes.add("waitTimes.source", source);
    wtimes.add("waitTimes.tape", tape);
    wtimes.add("waitTimes.calibration", calibration);
    wtimes.add("waitTimes.corsynch", corsynch);
    wtimes.add("waitTimes.<xmlattr>.member", name);

    master_.add_child("master.station.waitTimes", wtimes.get_child("waitTimes"));
}

void ParameterSettings::stationCableWrapBuffer(const std::string &name, double axis1LowOffset, double axis1UpOffset,
                                               double axis2LowOffset, double axis2UpOffset) {
    vector<string> members;
    if (groupStations_.find(name) != groupStations_.end()) {
        members.insert(members.end(), groupStations_[name].begin(), groupStations_[name].end());
    } else {
        members.push_back(name);
    }

    vector<string> membersAlreadyUsed;
    boost::property_tree::ptree PARA_station = master_.get_child("master.station");
    for (const auto &any:PARA_station) {
        if (any.first == "cableWrapBuffer") {
            if (name == "__all__") {
                cerr << "ERROR: double use of station/group " << name
                     << " in cable wrap buffer block! This whole block is ignored!;\n";
                return;
            }
            string memberName = any.second.get_child("<xmlattr>.member").data();
            if (groupStations_.find(memberName) != groupStations_.end()) {
                membersAlreadyUsed.insert(membersAlreadyUsed.end(), groupStations_[memberName].begin(),
                                          groupStations_[memberName].end());
            } else if (memberName == "__all__") {
                cerr << "ERROR: double use of station/group " << name
                     << " in cable wrap buffer block! This whole block is ignored!;\n";
                return;
            } else {
                membersAlreadyUsed.push_back(memberName);
            }
        }
    }

    for (const auto &any:members) {
        if (find(membersAlreadyUsed.begin(), membersAlreadyUsed.end(), any) != membersAlreadyUsed.end()) {
            cerr << "ERROR: double use of station/group " << name
                 << " in cable wrap buffer block! This whole block is ignored!;\n";
            return;
        }
    }

    boost::property_tree::ptree cable;
    cable.add("cableWrapBuffer.axis1LowOffset", axis1LowOffset);
    cable.add("cableWrapBuffer.axis1UpOffset", axis1UpOffset);
    cable.add("cableWrapBuffer.axis2LowOffset", axis2LowOffset);
    cable.add("cableWrapBuffer.axis2UpOffset", axis2UpOffset);
    cable.add("cableWrapBuffer.<xmlattr>.member", name);

    master_.add_child("master.station.cableWrapBuffer", cable.get_child("cableWrapBuffer"));
}


boost::property_tree::ptree ParameterSettings::getChildTree(const ParameterSetup &setup) {
    boost::property_tree::ptree root;
    boost::posix_time::ptime start = master_.get<boost::posix_time::ptime>("master.general.startTime");
    boost::posix_time::ptime end = master_.get<boost::posix_time::ptime>("master.general.endTime");

    boost::posix_time::time_duration a = end - start;
    int sec = a.total_seconds();
    auto duration = static_cast<unsigned int>(sec);

    if (!setup.getChildren().empty()) {
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

        if (setup.getTransition() != ParameterSetup::Transition::soft) {
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
        if (setup.getTransition() != ParameterSetup::Transition::soft) {
            root.add("root.transition", "hard");
        }

    }
    return root;
}


void ParameterSettings::skyCoverage(double influenceDistance, unsigned int influenceInterval, double maxTwinTelecopeDistance) {
    boost::property_tree::ptree skyCoverage;
    skyCoverage.add("skyCoverage.influenceDistance", influenceDistance);
    skyCoverage.add("skyCoverage.influenceInterval", influenceInterval);
    skyCoverage.add("skyCoverage.maxTwinTelecopeDistance", maxTwinTelecopeDistance);


    master_.add_child("master.skyCoverage", skyCoverage.get_child("skyCoverage"));
}

void
ParameterSettings::weightFactor(double weight_skyCoverage, double weight_numberOfObservations, double weight_duration,
                                double weight_averageSources, double weight_averageStations, double weightDeclination,
                                double declinationSlopeStart, double declinationSlopeEnd, double weightLowElevation,
                                double lowElevationSlopeStart, double lowElevationSlopeEnd) {
    boost::property_tree::ptree weightFactor;
    weightFactor.add("weightFactor.skyCoverage", weight_skyCoverage);
    weightFactor.add("weightFactor.numberOfObservations", weight_numberOfObservations);
    weightFactor.add("weightFactor.duration", weight_duration);
    weightFactor.add("weightFactor.averageSources", weight_averageSources);
    weightFactor.add("weightFactor.averageStations", weight_averageStations);

    weightFactor.add("weightFactor.weightDeclination", weightDeclination);
    weightFactor.add("weightFactor.declinationStartWeight", declinationSlopeStart);
    weightFactor.add("weightFactor.declinationFullWeight", declinationSlopeEnd);

    weightFactor.add("weightFactor.weightLowElevation", weightLowElevation);
    weightFactor.add("weightFactor.lowElevationStartWeight", lowElevationSlopeStart);
    weightFactor.add("weightFactor.lowElevationFullWeight", lowElevationSlopeEnd);

    master_.add_child("master.weightFactor", weightFactor.get_child("weightFactor"));
}

void ParameterSettings::mode(const std::string &skdMode) {
    boost::property_tree::ptree mode;
    mode.add("mode.skdMode", skdMode);

    master_.add_child("master.mode", mode.get_child("mode"));
}


void ParameterSettings::mode(double sampleRate, unsigned int bits) {
    boost::property_tree::ptree mode;
    mode.add("mode.sampleRate", sampleRate);
    mode.add("mode.bits", bits);

    master_.add_child("master.mode", mode.get_child("mode"));
}

void ParameterSettings::mode_band(const std::string &name, double wavelength, ObservationModeProperty station,
                                ObservationModeBackup stationBackup, double stationBackupValue, ObservationModeProperty source,
                                ObservationModeBackup sourceBackup, double sourceBackupValue, unsigned int chanels) {
    boost::property_tree::ptree band;
    band.add("band.wavelength", wavelength);
    band.add("band.chanels", chanels);


    if (station == ObservationModeProperty::required) {
        band.add("band.station.tag", "required");
    } else if (station == ObservationModeProperty::optional) {
        band.add("band.station.tag", "optional");
    }
    if (stationBackup == ObservationModeBackup::maxValueTimes) {
        band.add("band.station.backup_maxValueTimes", stationBackupValue);
    } else if (stationBackup == ObservationModeBackup::minValueTimes) {
        band.add("band.station.backup_minValueTimes", stationBackupValue);
    } else if (stationBackup == ObservationModeBackup::value) {
        band.add("band.station.backup_value", stationBackupValue);
    } else if (stationBackup == ObservationModeBackup::none) {
    }

    if (source == ObservationModeProperty::required) {
        band.add("band.source.tag", "required");
    } else if (source == ObservationModeProperty::optional) {
        band.add("band.source.tag", "optional");
    }

    if (sourceBackup == ObservationModeBackup::maxValueTimes) {
        band.add("band.source.backup_maxValueTimes", sourceBackupValue);
    } else if (sourceBackup == ObservationModeBackup::minValueTimes) {
        band.add("band.source.backup_minValueTimes", sourceBackupValue);
    } else if (sourceBackup == ObservationModeBackup::value) {
        band.add("band.source.backup_value", sourceBackupValue);
    } else if (sourceBackup == ObservationModeBackup::none) {
    }

    band.add("band.<xmlattr>.name", name);

    master_.add_child("master.mode.band", band.get_child("band"));

}

void ParameterSettings::write(const std::string &name) {
    std::ofstream os;
    os.open(name);
    boost::property_tree::xml_parser::write_xml(os, master_,
                                                boost::property_tree::xml_writer_make_settings<string>('\t', 1));
    os.close();
}

void ParameterSettings::multisched(const boost::property_tree::ptree &ms_tree) {
    master_.add_child("master.multisched", ms_tree.get_child("multisched"));
}

void
ParameterSettings::output(const string &experimentName, const string &experimentDescription, const string &scheduler,
                          const string &correlator, bool createSummary, bool createNGS, bool createSKD,
                          bool createSkyCoverage) {
    boost::property_tree::ptree output;
    output.add("output.experimentName", experimentName);
    output.add("output.experimentDescription", experimentDescription);
    output.add("output.scheduler", scheduler);
    output.add("output.correlator", correlator);
    output.add("output.createSummary", createSummary);
    output.add("output.createNGS", createNGS);
    output.add("output.createSKD", createSKD);
    output.add("output.createSkyCoverage", createSkyCoverage);

    master_.add_child("master.output", output.get_child("output"));

}

void ParameterSettings::ruleScanSequence(unsigned int cadence, const vector<unsigned int> &modulo,
                                         const vector<string> &member) {
    boost::property_tree::ptree rules;
    rules.add("rules.sourceSequence.cadence",cadence);

    unsigned long n = modulo.size();
    for (int i = 0; i < n; ++i) {
        boost::property_tree::ptree sourceSelection;

        sourceSelection.add("sequence.modulo",modulo[i]);
        sourceSelection.add("sequence.member",member[i]);

        rules.add_child("rules.sourceSequence.sequence", sourceSelection.get_child("sequence"));
    }

    master_.add_child("master.rules.sourceSequence", rules.get_child("rules.sourceSequence"));

}

void ParameterSettings::ruleCalibratorBlockTime(unsigned int cadence, const std::string &member,
                                                const std::vector<std::pair<double, double> > &between_elevation,
                                                unsigned int nMaxScans, unsigned int scanTime) {
    boost::property_tree::ptree rules;

    rules.add("calibratorBlock.cadence_seconds",cadence);
    rules.add("calibratorBlock.member",member);
    rules.add("calibratorBlock.nMaxScans",nMaxScans);
    rules.add("calibratorBlock.fixedScanTime",scanTime);
    for(const auto &any:between_elevation){
        boost::property_tree::ptree be;
        double el1 = any.first;
        double el2 = any.second;
        if(el1>el2){
            swap(el1,el2);
        }

        be.add("targetElevation.lower_limit",el1);
        be.add("targetElevation.upper_limit",el1);

        rules.add_child("calibratorBlock.targetElevations",be.get_child("targetElevations"));
    }

    master_.add_child("master.rules.calibratorBlock", rules.get_child("calibratorBlock"));
}

void ParameterSettings::ruleCalibratorBlockNScanSelections(unsigned int cadence, const std::string &member,
                                                           unsigned int nMaxScans, unsigned int scanTime) {

    boost::property_tree::ptree rules;

    rules.add("calibratorBlock.cadence_nScanSelections",cadence);
    rules.add("calibratorBlock.member",member);
    rules.add("calibratorBlock.nMaxScans",nMaxScans);
    rules.add("calibratorBlock.fixedScanTime",scanTime);

    master_.add_child("master.rules.calibratorBlock", rules.get_child("calibratorBlock"));

}




