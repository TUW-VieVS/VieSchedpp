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

#include "ParameterSettings.h"


using namespace std;
using namespace VieVS;


VieVS::ParameterSettings::ParameterSettings() {}


void ParameterSettings::software( const std::string &name, const std::string &version ) {
    boost::property_tree::ptree software;
    software.add( "software.name", name );
    software.add( "software.GUI_version", version );

    //    master_.insert(master_.begin(),software.get_child("software"));
    master_.add_child( "VieSchedpp.software", software.get_child( "software" ) );
}


void ParameterSettings::general( const std::string &experimentName, const boost::posix_time::ptime &startTime,
                                 const boost::posix_time::ptime &endTime, bool subnetting, double subnettingMinAngle,
                                 bool useSubnettingPercent_otherwiseAllBut, double subnettingNumber,
                                 bool fillinmodeInfluenceOnSchedule, bool fillinmodeDuringScan,
                                 bool fillinmodeAPosteriori, bool idleToObservingTime,
                                 const std::vector<std::string> &stations, bool useSourcesFromParameter_otherwiseIgnore,
                                 const std::vector<std::string> &srcNames, const std::string &scanAlignment,
                                 const std::string &logConsole, const std::string &logFile,
                                 bool doNotObserveSourcesWithinMinRepeat ) {
    boost::property_tree::ptree general;

    if ( experimentName.empty() ) {
        general.add( "general.experimentName", "dummy" );
    } else {
        general.add( "general.experimentName", experimentName );
    }

    int smonth = startTime.date().month();
    string startTimeStr =
        ( boost::format( "%04d.%02d.%02d %02d:%02d:%02d" ) % startTime.date().year() % smonth % startTime.date().day() %
          startTime.time_of_day().hours() % startTime.time_of_day().minutes() % startTime.time_of_day().seconds() )
            .str();
    general.add( "general.startTime", startTimeStr );

    int emonth = endTime.date().month();
    string endTimeStr =
        ( boost::format( "%04d.%02d.%02d %02d:%02d:%02d" ) % endTime.date().year() % emonth % endTime.date().day() %
          endTime.time_of_day().hours() % endTime.time_of_day().minutes() % endTime.time_of_day().seconds() )
            .str();
    general.add( "general.endTime", endTimeStr );

    general.add( "general.subnetting", subnetting );
    if ( subnetting ) {
        general.add( "general.subnettingMinAngle", subnettingMinAngle );
        if ( useSubnettingPercent_otherwiseAllBut ) {
            general.add( "general.subnettingMinNStaPercent", subnettingNumber );
        } else {
            general.add( "general.subnettingMinNStaAllBut", subnettingNumber );
        }
    }

    if ( fillinmodeAPosteriori || fillinmodeDuringScan ) {
        if ( fillinmodeAPosteriori ) {
            general.add( "general.fillinmodeAPosteriori", fillinmodeAPosteriori );
        }
        if ( fillinmodeDuringScan ) {
            general.add( "general.fillinmodeDuringScanSelection", fillinmodeDuringScan );
        }
        general.add( "general.fillinmodeInfluenceOnSchedule", fillinmodeInfluenceOnSchedule );
    }

    if ( idleToObservingTime ) {
        general.add( "general.idleToObservingTime", idleToObservingTime );
    }

    boost::property_tree::ptree all_stations;
    for ( const auto &any : stations ) {
        boost::property_tree::ptree tmp;
        tmp.add( "station", any );
        all_stations.add_child( "stations.station", tmp.get_child( "station" ) );
    }
    if ( !all_stations.empty() ) {
        general.add_child( "general.stations", all_stations.get_child( "stations" ) );
    }

    if ( !srcNames.empty() ) {
        boost::property_tree::ptree all_sources;
        for ( const auto &any : srcNames ) {
            boost::property_tree::ptree tmp;
            tmp.add( "source", any );
            all_sources.add_child( "sources.source", tmp.get_child( "source" ) );
        }

        if ( useSourcesFromParameter_otherwiseIgnore ) {
            general.add_child( "general.onlyUseListedSources", all_sources.get_child( "sources" ) );
        } else {
            general.add_child( "general.ignoreListedSources", all_sources.get_child( "sources" ) );
        }
    }

    general.add( "general.scanAlignment", scanAlignment );

    general.add( "general.logSeverityConsole", logConsole );
    general.add( "general.logSeverityFile", logFile );
    general.add( "general.doNotObserveSourcesWithinMinRepeat", doNotObserveSourcesWithinMinRepeat );

    master_.add_child( "VieSchedpp.general", general.get_child( "general" ) );
}


void ParameterSettings::created( const boost::posix_time::ptime &time, string name, string email ) {
    boost::property_tree::ptree created;
    int smonth = time.date().month();
    string timeString =
        ( boost::format( "%04d.%02d.%02d %02d:%02d:%02d" ) % time.date().year() % smonth % time.date().day() %
          time.time_of_day().hours() % time.time_of_day().minutes() % time.time_of_day().seconds() )
            .str();
    created.add( "created.time", timeString );
    created.add( "created.name", name );
    created.add( "created.email", email );
    master_.add_child( "VieSchedpp.created", created.get_child( "created" ) );
}


void ParameterSettings::catalogs( const std::string &antenna, const std::string &equip, const std::string &flux,
                                  const std::string &freq, const std::string &hdpos, const std::string &loif,
                                  const std::string &mask, const std::string &modes, const std::string &position,
                                  const std::string &rec, const std::string &rx, const std::string &source,
                                  const std::string &tracks ) {
    boost::property_tree::ptree catalogs;
    catalogs.add( "catalogs.antenna", antenna );
    catalogs.add( "catalogs.equip", equip );
    catalogs.add( "catalogs.flux", flux );
    catalogs.add( "catalogs.freq", freq );
    catalogs.add( "catalogs.hdpos", hdpos );
    catalogs.add( "catalogs.loif", loif );
    catalogs.add( "catalogs.mask", mask );
    catalogs.add( "catalogs.modes", modes );
    catalogs.add( "catalogs.position", position );
    catalogs.add( "catalogs.rec", rec );
    catalogs.add( "catalogs.rx", rx );
    catalogs.add( "catalogs.source", source );
    catalogs.add( "catalogs.tracks", tracks );

    master_.add_child( "VieSchedpp.catalogs", catalogs.get_child( "catalogs" ) );
}


void ParameterSettings::group( ParameterSettings::Type type, ParameterGroup group ) {
    boost::property_tree::ptree pt_group;
    pt_group.add( "group.<xmlattr>.name", group.name );

    for ( const auto &any : group.members ) {
        boost::property_tree::ptree tmp;
        tmp.add( "member", any );
        pt_group.add_child( "group.member", tmp.get_child( "member" ) );
    }

    if ( type == ParameterSettings::Type::station ) {
        master_.add_child( "VieSchedpp.station.groups.group", pt_group.get_child( "group" ) );
        groupStations_[group.name] = group.members;
    } else if ( type == ParameterSettings::Type::source ) {
        master_.add_child( "VieSchedpp.source.groups.group", pt_group.get_child( "group" ) );
        groupSources_[group.name] = group.members;
    } else if ( type == ParameterSettings::Type::baseline ) {
        master_.add_child( "VieSchedpp.baseline.groups.group", pt_group.get_child( "group" ) );
        groupBaselines_[group.name] = group.members;
    }
}


const std::vector<std::string> &ParameterSettings::getGroupMembers( ParameterSettings::Type type,
                                                                    std::string groupName ) {
    if ( type == ParameterSettings::Type::station ) {
        return groupStations_[groupName];
    } else if ( type == ParameterSettings::Type::source ) {
        return groupSources_[groupName];
    } else if ( type == ParameterSettings::Type::baseline ) {
        return groupBaselines_[groupName];
    }
}


void ParameterSettings::parameters( const std::string &name, const ParametersStations &PARA ) {
    paraStations_[name] = PARA;
    const boost::property_tree::ptree &parameters = parameterStation2ptree( name, PARA );
    master_.add_child( "VieSchedpp.station.parameters.parameter", parameters.get_child( "parameters" ) );
}


boost::property_tree::ptree ParameterSettings::parameterStation2ptree( const string &name,
                                                                       const ParametersStations &PARA ) {
    boost::property_tree::ptree parameters;
    if ( PARA.available.is_initialized() ) {
        parameters.add( "parameters.available", PARA.available );
    }
    if ( PARA.availableForFillinmode.is_initialized() ) {
        parameters.add( "parameters.availableForFillinmode", PARA.availableForFillinmode );
    }
    if ( PARA.firstScan.is_initialized() ) {
        parameters.add( "parameters.firstScan", PARA.firstScan );
    }
    if ( PARA.tagalong.is_initialized() ) {
        parameters.add( "parameters.tagalong", PARA.tagalong );
    }

    if ( PARA.weight.is_initialized() ) {
        parameters.add( "parameters.weight", PARA.weight );
    }

    if ( PARA.minScan.is_initialized() ) {
        parameters.add( "parameters.minScan", PARA.minScan );
    }
    if ( PARA.maxScan.is_initialized() ) {
        parameters.add( "parameters.maxScan", PARA.maxScan );
    }
    if ( PARA.minSlewtime.is_initialized() ) {
        parameters.add( "parameters.minSlewtime", PARA.minSlewtime );
    }
    if ( PARA.maxSlewtime.is_initialized() ) {
        parameters.add( "parameters.maxSlewtime", PARA.maxSlewtime );
    }
    if ( PARA.maxSlewDistance.is_initialized() ) {
        parameters.add( "parameters.maxSlewDistance", PARA.maxSlewDistance );
    }
    if ( PARA.minSlewDistance.is_initialized() ) {
        parameters.add( "parameters.minSlewDistance", PARA.minSlewDistance );
    }
    if ( PARA.maxWait.is_initialized() ) {
        parameters.add( "parameters.maxWait", PARA.maxWait );
    }
    if ( PARA.minElevation.is_initialized() ) {
        parameters.add( "parameters.minElevation", PARA.minElevation );
    }
    if ( PARA.maxNumberOfScans.is_initialized() ) {
        parameters.add( "parameters.maxNumberOfScans", PARA.maxNumberOfScans );
    }
    if ( PARA.maxTotalObsTime.is_initialized() ) {
        parameters.add( "parameters.maxTotalObsTime", PARA.maxTotalObsTime );
    }
    if ( PARA.dataWriteRate.is_initialized() ) {
        parameters.add( "parameters.dataWriteRate", PARA.dataWriteRate );
    }

    if ( PARA.preob.is_initialized() ) {
        parameters.add( "parameters.preob", PARA.preob );
    }
    if ( PARA.midob.is_initialized() ) {
        parameters.add( "parameters.midob", PARA.midob );
    }
    if ( PARA.systemDelay.is_initialized() ) {
        parameters.add( "parameters.systemDelay", PARA.systemDelay );
    }

    if ( !PARA.minSNR.empty() ) {
        for ( const auto &any : PARA.minSNR ) {
            boost::property_tree::ptree minSNR;
            minSNR.add( "minSNR", any.second );
            minSNR.put( "minSNR.<xmlattr>.band", any.first );
            parameters.add_child( "parameters.minSNR", minSNR.get_child( "minSNR" ) );
        }
    }

    if ( !PARA.ignoreSourcesString.empty() ) {
        boost::property_tree::ptree ignoreSources;
        for ( const auto &any : PARA.ignoreSourcesString ) {
            boost::property_tree::ptree sourceName;
            sourceName.add( "source", any );
            ignoreSources.add_child( "ignoreSources.source", sourceName.get_child( "source" ) );
        }
        parameters.add_child( "parameters.ignoreSources", ignoreSources.get_child( "ignoreSources" ) );
    }

    parameters.add( "parameters.<xmlattr>.name", name );
    return parameters;
}


std::pair<string, ParameterSettings::ParametersStations> ParameterSettings::ptree2parameterStation(
    boost::property_tree::ptree ptree ) {
    string parameterName = ptree.get_child( "<xmlattr>.name" ).data();

    ParametersStations para;

    for ( auto &it : ptree ) {
        string paraName = it.first;
        if ( paraName == "<xmlattr>" ) {
            continue;
        } else if ( paraName == "available" ) {
            para.available = it.second.get_value<bool>();
        } else if ( paraName == "availableForFillinmode" ) {
            para.availableForFillinmode = it.second.get_value<bool>();
        } else if ( paraName == "tagalong" ) {
            para.tagalong = it.second.get_value<bool>();
        } else if ( paraName == "firstScan" ) {
            para.firstScan = it.second.get_value<bool>();
        } else if ( paraName == "weight" ) {
            para.weight = it.second.get_value<double>();
        } else if ( paraName == "minScan" ) {
            para.minScan = it.second.get_value<unsigned int>();
        } else if ( paraName == "maxScan" ) {
            para.maxScan = it.second.get_value<unsigned int>();
        } else if ( paraName == "minSlewtime" ) {
            para.minSlewtime = it.second.get_value<unsigned int>();
        } else if ( paraName == "maxSlewtime" ) {
            para.maxSlewtime = it.second.get_value<unsigned int>();
        } else if ( paraName == "maxSlewDistance" ) {
            para.maxSlewDistance = it.second.get_value<double>();
        } else if ( paraName == "minSlewDistance" ) {
            para.minSlewDistance = it.second.get_value<double>();
        } else if ( paraName == "maxWait" ) {
            para.maxWait = it.second.get_value<unsigned int>();
        } else if ( paraName == "maxNumberOfScans" ) {
            para.maxNumberOfScans = it.second.get_value<unsigned int>();
        } else if ( paraName == "maxTotalObsTime" ) {
            para.maxTotalObsTime = it.second.get_value<unsigned int>();
        } else if ( paraName == "minElevation" ) {
            para.minElevation = it.second.get_value<double>();
        } else if ( paraName == "dataWriteRate" ) {
            para.dataWriteRate = it.second.get_value<double>();
        } else if ( paraName == "preob" ) {
            para.preob = it.second.get_value<double>();
        } else if ( paraName == "midob" ) {
            para.midob = it.second.get_value<double>();
        } else if ( paraName == "systemDelay" ) {
            para.systemDelay = it.second.get_value<double>();
        } else if ( paraName == "minSNR" ) {
            string bandName = it.second.get_child( "<xmlattr>.band" ).data();
            auto value = it.second.get_value<double>();
            para.minSNR[bandName] = value;
        } else if ( paraName == "ignoreSources" ) {
            for ( auto &it2 : it.second ) {
                string srcName = it2.second.data();
                para.ignoreSourcesString.push_back( srcName );
            }
        } else {
            //            std::cerr << "<station> <parameter>: " << parameterName << ": parameter <" << name
            //                 << "> not understood! (Ignored)\n";
        }
    }
    return {parameterName, para};
}


void ParameterSettings::parameters( const std::string &name, const ParametersSources &PARA ) {
    paraSources_[name] = PARA;
    const boost::property_tree::ptree &parameters = parameterSource2ptree( name, PARA );
    master_.add_child( "VieSchedpp.source.parameters.parameter", parameters.get_child( "parameters" ) );
}


boost::property_tree::ptree ParameterSettings::parameterSource2ptree( const string &name,
                                                                      const ParametersSources &PARA ) {
    boost::property_tree::ptree parameters;
    if ( PARA.available.is_initialized() ) {
        parameters.add( "parameters.available", PARA.available );
    }
    if ( PARA.availableForFillinmode.is_initialized() ) {
        parameters.add( "parameters.availableForFillinmode", PARA.availableForFillinmode );
    }

    if ( PARA.weight.is_initialized() ) {
        parameters.add( "parameters.weight", PARA.weight );
    }
    if ( PARA.minElevation.is_initialized() ) {
        parameters.add( "parameters.minElevation", PARA.minElevation );
    }
    if ( PARA.minSunDistance.is_initialized() ) {
        parameters.add( "parameters.minSunDistance", PARA.minSunDistance );
    }

    if ( PARA.minScan.is_initialized() ) {
        parameters.add( "parameters.minScan", PARA.minScan );
    }
    if ( PARA.maxScan.is_initialized() ) {
        parameters.add( "parameters.maxScan", PARA.maxScan );
    }

    if ( PARA.minNumberOfStations.is_initialized() ) {
        parameters.add( "parameters.minNumberOfStations", PARA.minNumberOfStations );
    }
    if ( PARA.minRepeat.is_initialized() ) {
        parameters.add( "parameters.minRepeat", PARA.minRepeat );
    }
    if ( PARA.minFlux.is_initialized() ) {
        parameters.add( "parameters.minFlux", PARA.minFlux );
    }
    if ( PARA.maxNumberOfScans.is_initialized() ) {
        parameters.add( "parameters.maxNumberOfScans", PARA.maxNumberOfScans );
    }

    if ( PARA.tryToObserveXTimesEvenlyDistributed.is_initialized() ) {
        parameters.add( "parameters.tryToObserveXTimesEvenlyDistributed", *PARA.tryToObserveXTimesEvenlyDistributed );
        parameters.add( "parameters.tryToObserveXTimesMinRepeat", *PARA.tryToObserveXTimesMinRepeat );
    }
    if ( PARA.fixedScanDuration.is_initialized() ) {
        parameters.add( "parameters.fixedScanDuration", *PARA.fixedScanDuration );
    }
    if ( PARA.tryToFocusIfObservedOnce.is_initialized() ) {
        parameters.add( "parameters.tryToFocusIfObservedOnce", *PARA.tryToFocusIfObservedOnce );
        parameters.add( "parameters.tryToFocusFactor", *PARA.tryToFocusFactor );
        if ( *PARA.tryToFocusOccurrency == TryToFocusOccurrency::once ) {
            parameters.add( "parameters.tryToFocusOccurrency", "once" );
        } else {
            parameters.add( "parameters.tryToFocusOccurrency", "perScan" );
        }
        if ( *PARA.tryToFocusType == TryToFocusType::additive ) {
            parameters.add( "parameters.tryToFocusType", "additive" );
        } else {
            parameters.add( "parameters.tryToFocusType", "multiplicative" );
        }
    }

    for ( const auto &any : PARA.minSNR ) {
        boost::property_tree::ptree minSNR;
        minSNR.add( "minSNR", any.second );
        minSNR.put( "minSNR.<xmlattr>.band", any.first );
        parameters.add_child( "parameters.minSNR", minSNR.get_child( "minSNR" ) );
    }

    if ( !PARA.ignoreStationsString.empty() ) {
        boost::property_tree::ptree ignoreStations;
        for ( const auto &any : PARA.ignoreStationsString ) {
            boost::property_tree::ptree stationName;
            stationName.add( "station", any );
            ignoreStations.add_child( "ignoreStations.station", stationName.get_child( "station" ) );
        }
        parameters.add_child( "parameters.ignoreStations", ignoreStations.get_child( "ignoreStations" ) );
    }

    if ( !PARA.requiredStationsString.empty() ) {
        boost::property_tree::ptree requiredStations;
        for ( const auto &any : PARA.requiredStationsString ) {
            boost::property_tree::ptree stationName;
            stationName.add( "station", any );
            requiredStations.add_child( "requiredStations.station", stationName.get_child( "station" ) );
        }
        parameters.add_child( "parameters.requiredStations", requiredStations.get_child( "requiredStations" ) );
    }

    if ( !PARA.ignoreBaselinesString.empty() ) {
        boost::property_tree::ptree ignoreBaselines;
        for ( const auto &any : PARA.ignoreBaselinesString ) {
            boost::property_tree::ptree baselineName;
            //            string bname = any.first + "-" + any.second;
            baselineName.add( "baseline", any );
            ignoreBaselines.add_child( "ignoreBaselines.baseline", baselineName.get_child( "baseline" ) );
        }
        parameters.add_child( "parameters.ignoreBaselines", ignoreBaselines.get_child( "ignoreBaselines" ) );
    }

    parameters.add( "parameters.<xmlattr>.name", name );
    return parameters;
}


std::pair<string, ParameterSettings::ParametersSources> ParameterSettings::ptree2parameterSource(
    boost::property_tree::ptree ptree ) {
    string parameterName = ptree.get_child( "<xmlattr>.name" ).data();

    ParametersSources para;

    for ( auto &it : ptree ) {
        string paraName = it.first;
        if ( paraName == "<xmlattr>" ) {
            continue;
        } else if ( paraName == "available" ) {
            para.available = it.second.get_value<bool>();
        } else if ( paraName == "availableForFillinmode" ) {
            para.availableForFillinmode = it.second.get_value<bool>();
        } else if ( paraName == "weight" ) {
            para.weight = it.second.get_value<double>();
        } else if ( paraName == "minElevation" ) {
            para.minElevation = it.second.get_value<double>();
        } else if ( paraName == "minSunDistance" ) {
            para.minSunDistance = it.second.get_value<double>();
        } else if ( paraName == "minScan" ) {
            para.minScan = it.second.get_value<unsigned int>();
        } else if ( paraName == "maxScan" ) {
            para.maxScan = it.second.get_value<unsigned int>();
        } else if ( paraName == "minNumberOfStations" ) {
            para.minNumberOfStations = it.second.get_value<unsigned int>();
        } else if ( paraName == "minRepeat" ) {
            para.minRepeat = it.second.get_value<unsigned int>();
        } else if ( paraName == "minFlux" ) {
            para.minFlux = it.second.get_value<double>();
        } else if ( paraName == "tryToObserveXTimesEvenlyDistributed" ) {
            para.tryToObserveXTimesEvenlyDistributed = it.second.get_value<unsigned int>();
        } else if ( paraName == "tryToObserveXTimesMinRepeat" ) {
            para.tryToObserveXTimesMinRepeat = it.second.get_value<unsigned int>();
        } else if ( paraName == "fixedScanDuration" ) {
            para.fixedScanDuration = it.second.get_value<unsigned int>();
        } else if ( paraName == "maxNumberOfScans" ) {
            para.maxNumberOfScans = it.second.get_value<unsigned int>();
        } else if ( paraName == "tryToFocusIfObservedOnce" ) {
            para.tryToFocusIfObservedOnce = it.second.get_value<bool>();
        } else if ( paraName == "tryToFocusFactor" ) {
            para.tryToFocusFactor = it.second.get_value<double>();
        } else if ( paraName == "tryToFocusOccurrency" ) {
            if ( it.second.data() == "once" ) {
                para.tryToFocusOccurrency = TryToFocusOccurrency::once;
            } else {
                para.tryToFocusOccurrency = TryToFocusOccurrency::perScan;
            }
        } else if ( paraName == "tryToFocusType" ) {
            if ( it.second.data() == "additive" ) {
                para.tryToFocusType = TryToFocusType::additive;
            } else {
                para.tryToFocusType = TryToFocusType::multiplicative;
            }
        } else if ( paraName == "minSNR" ) {
            string bandName = it.second.get_child( "<xmlattr>.band" ).data();
            double value = it.second.get_value<double>();
            para.minSNR[bandName] = value;
        } else if ( paraName == "ignoreStations" ) {
            for ( auto &it3 : it.second ) {
                string staName = it3.second.data();
                para.ignoreStationsString.push_back( staName );
            }
        } else if ( paraName == "requiredStations" ) {
            for ( auto &it3 : it.second ) {
                string staName = it3.second.data();
                para.requiredStationsString.push_back( staName );
            }
        } else if ( paraName == "ignoreBaselines" ) {
            for ( auto &it3 : it.second ) {
                string baselineName = it3.second.data();
                para.ignoreBaselinesString.push_back( baselineName );
            }
        } else {
            //            std::cerr << "<source> <parameter>: " << parameterName << ": parameter <" << name
            //                    << "> not understood! (Ignored)\n";
        }
    }
    return {parameterName, para};
}


void ParameterSettings::parameters( const std::string &name, const ParametersBaselines &PARA ) {
    paraBaselines_[name] = PARA;
    const boost::property_tree::ptree &parameters = parameterBaseline2ptree( name, PARA );
    master_.add_child( "VieSchedpp.baseline.parameters.parameter", parameters.get_child( "parameters" ) );
}


boost::property_tree::ptree ParameterSettings::parameterBaseline2ptree( const string &name,
                                                                        const ParametersBaselines &PARA ) {
    boost::property_tree::ptree parameters;
    if ( PARA.ignore.is_initialized() ) {
        parameters.add( "parameters.ignore", PARA.ignore );
    }
    if ( PARA.minScan.is_initialized() ) {
        parameters.add( "parameters.minScan", PARA.minScan );
    }
    if ( PARA.maxScan.is_initialized() ) {
        parameters.add( "parameters.maxScan", PARA.maxScan );
    }
    if ( PARA.weight.is_initialized() ) {
        parameters.add( "parameters.weight", PARA.weight );
    }

    for ( const auto &any : PARA.minSNR ) {
        boost::property_tree::ptree minSNR;
        minSNR.add( "minSNR", any.second );
        minSNR.put( "minSNR.<xmlattr>.band", any.first );
        parameters.add_child( "parameters.minSNR", minSNR.get_child( "minSNR" ) );
    }

    parameters.add( "parameters.<xmlattr>.name", name );

    return parameters;
}


std::pair<string, ParameterSettings::ParametersBaselines> ParameterSettings::ptree2parameterBaseline(
    boost::property_tree::ptree ptree ) {
    string parameterName = ptree.get_child( "<xmlattr>.name" ).data();

    ParametersBaselines para;

    for ( auto &it : ptree ) {
        string paraName = it.first;
        if ( paraName == "<xmlattr>" ) {
            continue;
        } else if ( paraName == "ignore" ) {
            para.ignore = it.second.get_value<bool>();
        } else if ( paraName == "weight" ) {
            para.weight = it.second.get_value<double>();
        } else if ( paraName == "minScan" ) {
            para.minScan = it.second.get_value<unsigned int>();
        } else if ( paraName == "maxScan" ) {
            para.maxScan = it.second.get_value<unsigned int>();
        } else if ( paraName == "minSNR" ) {
            string bandName = it.second.get_child( "<xmlattr>.band" ).data();
            double value = it.second.get_value<double>();
            para.minSNR[bandName] = value;
        } else {
            //            std::cerr << "<baseline> <parameter>: " << parameterName << ": parameter <" << name
            //                 << "> not understood! (Ignored)\n";
        }
    }
    return {parameterName, para};
}


void ParameterSettings::setup( ParameterSettings::Type type, const ParameterSetup &setup ) {
    boost::property_tree::ptree root;
    boost::property_tree::ptree defaultTree = getChildTree( setup );
    auto tmp = defaultTree.get_child_optional( "root.setup" );

    if ( tmp.is_initialized() ) {
        boost::property_tree::ptree copy = *tmp;
        defaultTree.get_child( "root" ).erase( "setup" );
        root.add_child( "setup", defaultTree.get_child( "root" ) );
        for ( const auto &any : copy ) {
            root.add_child( "setup.setup", any.second );
        }
    } else {
        root.add_child( "setup", defaultTree.get_child( "root" ) );
    }

    //    root.add_child("setup", defaultTree.get_child("root"));

    //    if (!setup.getChildren().empty()) {
    //        for (const auto &any:setup.getChildren()) {
    //            boost::property_tree::ptree thisChildTree = getChildTree(any);
    //            root.add_child("setup", thisChildTree.get_child("root"));
    //        }
    //    }

    if ( type == ParameterSettings::Type::station ) {
        master_.add_child( "VieSchedpp.station", root );
    } else if ( type == ParameterSettings::Type::source ) {
        master_.add_child( "VieSchedpp.source", root );
    } else if ( type == ParameterSettings::Type::baseline ) {
        master_.add_child( "VieSchedpp.baseline", root );
    }
}


void ParameterSettings::stationWaitTimes( const std::string &name, unsigned int fieldSystem, unsigned int preob,
                                          unsigned int midob, unsigned int postob ) {
    vector<string> members;
    if ( groupStations_.find( name ) != groupStations_.end() ) {
        members.insert( members.end(), groupStations_[name].begin(), groupStations_[name].end() );
    } else {
        members.push_back( name );
    }

    vector<string> membersAlreadyUsed;
    boost::property_tree::ptree PARA_station = master_.get_child( "VieSchedpp.station" );
    for ( const auto &any : PARA_station ) {
        if ( any.first == "waitTimes" ) {
            if ( name == "__all__" ) {
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( warning )
                    << "double use of station/group " << name << " in cable wrap buffer block -> ignored";
#else
                cout << "[warning] double use of station/group " << name << " in cable wrap buffer block -> ignored";
#endif
                return;
            }
            string memberName = any.second.get<std::string>( "waitTime.<xmlattr>.member" );
            if ( groupStations_.find( memberName ) != groupStations_.end() ) {
                membersAlreadyUsed.insert( membersAlreadyUsed.end(), groupStations_[memberName].begin(),
                                           groupStations_[memberName].end() );
            } else if ( memberName == "__all__" ) {
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( warning )
                    << "double use of station/group " << name << " in wait time block -> ignored";
#else
                cout << "[warning] double use of station/group " << name << " in wait time block -> ignored";
#endif
                return;
            } else {
                membersAlreadyUsed.push_back( memberName );
            }
        }
    }
    for ( const auto &any : members ) {
        if ( find( membersAlreadyUsed.begin(), membersAlreadyUsed.end(), any ) != membersAlreadyUsed.end() ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( warning ) << "double use of station/group " << name << " in wait time block -> ignored";
#else
            cout << "[warning] double use of station/group " << name << " in wait time block -> ignored";
#endif
            return;
        }
    }

    boost::property_tree::ptree wtimes;
    wtimes.add( "waitTime.fieldSystem", fieldSystem );
    wtimes.add( "waitTime.preob", preob );
    wtimes.add( "waitTime.midob", midob );
    wtimes.add( "waitTime.postob", postob );
    wtimes.add( "waitTime.<xmlattr>.member", name );

    master_.add_child( "VieSchedpp.station.waitTimes.waitTime", wtimes.get_child( "waitTime" ) );
}


void ParameterSettings::stationCableWrapBuffer( const std::string &name, double axis1LowOffset, double axis1UpOffset,
                                                double axis2LowOffset, double axis2UpOffset ) {
    vector<string> members;
    if ( groupStations_.find( name ) != groupStations_.end() ) {
        members.insert( members.end(), groupStations_[name].begin(), groupStations_[name].end() );
    } else {
        members.push_back( name );
    }

    vector<string> membersAlreadyUsed;
    boost::property_tree::ptree PARA_station = master_.get_child( "VieSchedpp.station" );
    for ( const auto &any : PARA_station ) {
        if ( any.first == "cableWrapBuffer" ) {
            if ( name == "__all__" ) {
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( warning )
                    << "double use of station/group " << name << " in cable wrap buffer block -> ignored";
#else
                cout << "[warning] double use of station/group " << name << " in cable wrap buffer block -> ignored";
#endif
                return;
            }
            string memberName = any.second.get<std::string>( "waitTime.<xmlattr>.member" );
            if ( groupStations_.find( memberName ) != groupStations_.end() ) {
                membersAlreadyUsed.insert( membersAlreadyUsed.end(), groupStations_[memberName].begin(),
                                           groupStations_[memberName].end() );
            } else if ( memberName == "__all__" ) {
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( warning )
                    << "double use of station/group " << name << " in cable wrap buffer block -> ignored";
#else
                cout << "[warning] double use of station/group " << name << " in cable wrap buffer block -> ignored";
#endif
                return;
            } else {
                membersAlreadyUsed.push_back( memberName );
            }
        }
    }

    for ( const auto &any : members ) {
        if ( find( membersAlreadyUsed.begin(), membersAlreadyUsed.end(), any ) != membersAlreadyUsed.end() ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( warning ) << "double use of station/group " << name
                                         << " in cable wrap buffer block -> ignored";
#else
            cout << "[warning] double use of station/group " << name << " in cable wrap buffer block -> ignored";
#endif
            return;
        }
    }

    boost::property_tree::ptree cable;
    cable.add( "cableWrapBuffer.axis1LowOffset", axis1LowOffset );
    cable.add( "cableWrapBuffer.axis1UpOffset", axis1UpOffset );
    cable.add( "cableWrapBuffer.axis2LowOffset", axis2LowOffset );
    cable.add( "cableWrapBuffer.axis2UpOffset", axis2UpOffset );
    cable.add( "cableWrapBuffer.<xmlattr>.member", name );

    master_.add_child( "VieSchedpp.station.cableWrapBuffers.cableWrapBuffer", cable.get_child( "cableWrapBuffer" ) );
}


boost::property_tree::ptree ParameterSettings::getChildTree( const ParameterSetup &setup ) {
    boost::property_tree::ptree root;

    string startstr = master_.get<string>( "VieSchedpp.general.startTime" );
    auto syear = boost::lexical_cast<unsigned short>( startstr.substr( 0, 4 ) );
    auto smonth = boost::lexical_cast<unsigned short>( startstr.substr( 5, 2 ) );
    auto sday = boost::lexical_cast<unsigned short>( startstr.substr( 8, 2 ) );
    auto shour = boost::lexical_cast<int>( startstr.substr( 11, 2 ) );
    auto sminute = boost::lexical_cast<int>( startstr.substr( 14, 2 ) );
    auto ssecond = boost::lexical_cast<int>( startstr.substr( 17, 2 ) );
    boost::posix_time::ptime start = boost::posix_time::ptime(
        boost::gregorian::date( syear, smonth, sday ), boost::posix_time::time_duration( shour, sminute, ssecond ) );

    string endstr = master_.get<string>( "VieSchedpp.general.endTime" );
    auto eyear = boost::lexical_cast<unsigned short>( endstr.substr( 0, 4 ) );
    auto emonth = boost::lexical_cast<unsigned short>( endstr.substr( 5, 2 ) );
    auto eday = boost::lexical_cast<unsigned short>( endstr.substr( 8, 2 ) );
    auto ehour = boost::lexical_cast<int>( endstr.substr( 11, 2 ) );
    auto eminute = boost::lexical_cast<int>( endstr.substr( 14, 2 ) );
    auto esecond = boost::lexical_cast<int>( endstr.substr( 17, 2 ) );
    boost::posix_time::ptime end = boost::posix_time::ptime(
        boost::gregorian::date( eyear, emonth, eday ), boost::posix_time::time_duration( ehour, eminute, esecond ) );

    int sec = util::duration( start, end );
    auto duration = static_cast<unsigned int>( sec );

    bool ms = true;
    if ( setup.getParameterName() != "multi scheduling" ) {
        ms = false;
        const std::vector<string> &members = setup.getMembers();
        const string &memberName = setup.getMemberName();
        if ( members.size() == 1 && members[0] == memberName ) {
            root.add( "root.member", setup.getMemberName() );
        } else {
            root.add( "root.group", setup.getMemberName() );
        }
        root.add( "root.parameter", setup.getParameterName() );

        unsigned int thisStart = setup.getStart();
        if ( thisStart != 0 ) {
            boost::posix_time::ptime tmp = start + boost::posix_time::seconds( thisStart );
            int smonthtmp = start.date().month();

            string stmp =
                ( boost::format( "%04d.%02d.%02d %02d:%02d:%02d" ) % tmp.date().year() % smonthtmp % tmp.date().day() %
                  tmp.time_of_day().hours() % tmp.time_of_day().minutes() % tmp.time_of_day().seconds() )
                    .str();
            root.add( "root.start", stmp );
        }

        unsigned int thisEnd = setup.getEnd();
        if ( thisEnd < duration ) {
            boost::posix_time::ptime tmp = start + boost::posix_time::seconds( thisEnd );
            int smonthtmp = start.date().month();

            string stmp =
                ( boost::format( "%04d.%02d.%02d %02d:%02d:%02d" ) % tmp.date().year() % smonthtmp % tmp.date().day() %
                  tmp.time_of_day().hours() % tmp.time_of_day().minutes() % tmp.time_of_day().seconds() )
                    .str();
            root.add( "root.end", stmp );
        }

        if ( setup.getTransition() != ParameterSetup::Transition::smooth ) {
            root.add( "root.transition", "hard" );
        }
    }

    if ( !setup.getChildren().empty() ) {
        for ( const auto &any : setup.getChildren() ) {
            boost::property_tree::ptree thisChildTree = getChildTree( any );
            auto tmp = thisChildTree.get_child_optional( "root" );
            if ( tmp.is_initialized() ) {
                root.add_child( "root.setup", *tmp );
            }
        }
    }

    return root;
}


void ParameterSettings::skyCoverage( double influenceDistance, unsigned int influenceInterval,
                                     double maxTwinTelecopeDistance, string interpolationDistance,
                                     string interpolationTime ) {
    boost::property_tree::ptree skyCoverage;
    skyCoverage.add( "skyCoverage.influenceDistance", influenceDistance );
    skyCoverage.add( "skyCoverage.influenceInterval", influenceInterval );
    skyCoverage.add( "skyCoverage.maxTwinTelecopeDistance", maxTwinTelecopeDistance );
    skyCoverage.add( "skyCoverage.interpolationDistance", interpolationDistance );
    skyCoverage.add( "skyCoverage.interpolationTime", interpolationTime );

    master_.add_child( "VieSchedpp.skyCoverage", skyCoverage.get_child( "skyCoverage" ) );
}


void ParameterSettings::weightFactor( double weight_skyCoverage, double weight_numberOfObservations,
                                      double weight_duration, double weight_averageSources,
                                      double weight_averageStations, double weight_averageBaselines,
                                      double weight_idleTime, unsigned int idleTimeInterval, double weightDeclination,
                                      double declinationSlopeStart, double declinationSlopeEnd,
                                      double weightLowElevation, double lowElevationSlopeStart,
                                      double lowElevationSlopeEnd ) {
    boost::property_tree::ptree weightFactor;
    if ( weight_skyCoverage != 0 ) {
        weightFactor.add( "weightFactor.skyCoverage", weight_skyCoverage );
    }
    if ( weight_numberOfObservations != 0 ) {
        weightFactor.add( "weightFactor.numberOfObservations", weight_numberOfObservations );
    }
    if ( weight_duration != 0 ) {
        weightFactor.add( "weightFactor.duration", weight_duration );
    }
    if ( weight_averageSources != 0 ) {
        weightFactor.add( "weightFactor.averageSources", weight_averageSources );
    }
    if ( weight_averageStations != 0 ) {
        weightFactor.add( "weightFactor.averageStations", weight_averageStations );
    }
    if ( weight_averageBaselines != 0 ) {
        weightFactor.add( "weightFactor.averageBaselines", weight_averageBaselines );
    }
    if ( weight_idleTime != 0 ) {
        weightFactor.add( "weightFactor.idleTime", weight_idleTime );
        weightFactor.add( "weightFactor.idleTimeInterval", idleTimeInterval );
    }
    if ( weightDeclination != 0 ) {
        weightFactor.add( "weightFactor.weightDeclination", weightDeclination );
        weightFactor.add( "weightFactor.declinationStartWeight", declinationSlopeStart );
        weightFactor.add( "weightFactor.declinationFullWeight", declinationSlopeEnd );
    }
    if ( weightLowElevation != 0 ) {
        weightFactor.add( "weightFactor.weightLowElevation", weightLowElevation );
        weightFactor.add( "weightFactor.lowElevationStartWeight", lowElevationSlopeStart );
        weightFactor.add( "weightFactor.lowElevationFullWeight", lowElevationSlopeEnd );
    }
    master_.add_child( "VieSchedpp.weightFactor", weightFactor.get_child( "weightFactor" ) );
}


void ParameterSettings::conditions( std::vector<string> members, std::vector<int> minScans,
                                    std::vector<int> minBaselines, bool andForCombination, int maxNumberOfIterations,
                                    int numberOfGentleSourceReductions, int minNumberOfSourcesToReduce,
                                    double percentage ) {
    boost::property_tree::ptree conditions;
    if ( andForCombination ) {
        conditions.add( "optimization.combination", "and" );
    } else {
        conditions.add( "optimization.combination", "or" );
    }
    conditions.add( "optimization.maxNumberOfIterations", maxNumberOfIterations );
    conditions.add( "optimization.numberOfGentleSourceReductions", numberOfGentleSourceReductions );
    conditions.add( "optimization.minNumberOfSourcesToReduce", minNumberOfSourcesToReduce );
    conditions.add( "optimization.percentageGentleSourceReduction", percentage );
    for ( int i = 0; i < members.size(); ++i ) {
        boost::property_tree::ptree condition;
        condition.add( "condition.members", members.at( i ) );
        condition.add( "condition.minScans", minScans.at( i ) );
        condition.add( "condition.minBaselines", minBaselines.at( i ) );
        conditions.add_child( "optimization.condition", condition.get_child( "condition" ) );
    }
    master_.add_child( "VieSchedpp.optimization", conditions.get_child( "optimization" ) );
}


void ParameterSettings::mode( const std::string &skdMode ) {
    boost::property_tree::ptree mode;
    mode.add( "mode.skdMode", skdMode );

    master_.add_child( "VieSchedpp.mode", mode.get_child( "mode" ) );
}


void ParameterSettings::mode( const boost::property_tree::ptree &obsMode ) {
    master_.add_child( "VieSchedpp.mode.custom", obsMode );
}


void ParameterSettings::mode( double sampleRate, unsigned int bits, double efficiencyFactor ) {
    boost::property_tree::ptree mode;
    mode.add( "mode.sampleRate", sampleRate );
    mode.add( "mode.bits", bits );
    mode.add( "mode.efficiencyFactor", efficiencyFactor );

    master_.add_child( "VieSchedpp.mode.simple", mode.get_child( "mode" ) );
}


void ParameterSettings::mode_band( const std::string &name, double wavelength, unsigned int channels ) {
    boost::property_tree::ptree band;
    band.add( "band.wavelength", wavelength );
    band.add( "band.channels", channels );

    band.add( "band.<xmlattr>.name", name );

    master_.add_child( "VieSchedpp.mode.simple.bands.band", band.get_child( "band" ) );
}


void ParameterSettings::mode_bandPolicy( const std::string &name, double minSNR, ObservationModeProperty station,
                                         ObservationModeBackup stationBackup, double stationBackupValue,
                                         ObservationModeProperty source, ObservationModeBackup sourceBackup,
                                         double sourceBackupValue ) {
    boost::property_tree::ptree band;

    band.add( "band.minSNR", minSNR );
    if ( station == ObservationModeProperty::required ) {
        band.add( "band.station.tag", "required" );
    } else if ( station == ObservationModeProperty::optional ) {
        band.add( "band.station.tag", "optional" );
    }
    if ( stationBackup == ObservationModeBackup::maxValueTimes ) {
        band.add( "band.station.backup_maxValueTimes", stationBackupValue );
    } else if ( stationBackup == ObservationModeBackup::minValueTimes ) {
        band.add( "band.station.backup_minValueTimes", stationBackupValue );
    } else if ( stationBackup == ObservationModeBackup::value ) {
        band.add( "band.station.backup_value", stationBackupValue );
    } else if ( stationBackup == ObservationModeBackup::none ) {
    }

    if ( source == ObservationModeProperty::required ) {
        band.add( "band.source.tag", "required" );
    } else if ( source == ObservationModeProperty::optional ) {
        band.add( "band.source.tag", "optional" );
    }

    if ( sourceBackup == ObservationModeBackup::maxValueTimes ) {
        band.add( "band.source.backup_maxValueTimes", sourceBackupValue );
    } else if ( sourceBackup == ObservationModeBackup::minValueTimes ) {
        band.add( "band.source.backup_minValueTimes", sourceBackupValue );
    } else if ( sourceBackup == ObservationModeBackup::value ) {
        band.add( "band.source.backup_value", sourceBackupValue );
    } else if ( sourceBackup == ObservationModeBackup::internalModel ) {
        band.add_child( "band.source.backup_internalModel", boost::property_tree::ptree() );
    } else if ( sourceBackup == ObservationModeBackup::none ) {
    }

    band.add( "band.<xmlattr>.name", name );

    master_.add_child( "VieSchedpp.mode.bandPolicies.bandPolicy", band.get_child( "band" ) );
}


void ParameterSettings::write( const std::string &name ) {
    std::ofstream of;
    of.open( name );
    boost::property_tree::xml_parser::write_xml( of, master_,
                                                 boost::property_tree::xml_writer_make_settings<string>( '\t', 1 ) );
    of.close();
}


void ParameterSettings::multisched( const boost::property_tree::ptree &ms_tree, const std::string &number, int maxn,
                                    const std::string &useSeed, int seed ) {
    master_.add_child( "VieSchedpp.multisched", ms_tree.get_child( "multisched" ) );
    if ( number != "all" ) {
        master_.add( "VieSchedpp.multisched.maxNumber", maxn );
    }
    if ( useSeed != "random" ) {
        master_.add( "VieSchedpp.multisched.seed", seed );
    }
}


void ParameterSettings::multiCore( const string &threads, int nThreadsManual, const string &jobScheduler,
                                   int chunkSize ) {
    boost::property_tree::ptree mc;
    mc.add( "multiCore.threads", threads );
    if ( threads == "manual" ) {
        mc.add( "multiCore.nThreads", nThreadsManual );
    }
    mc.add( "multiCore.jobScheduling", jobScheduler );
    if ( jobScheduler != "auto" ) {
        mc.add( "multiCore.chunkSize", chunkSize );
    }

    master_.add_child( "VieSchedpp.multiCore", mc.get_child( "multiCore" ) );
}


void ParameterSettings::output( const std::string &experimentDescription, const std::string &scheduler,
                                const std::string &correlator, const std::string &notes, bool initializer,
                                bool iteration_log, bool createSummary, bool createNGS,
                                const std::string &NGS_directory, bool createSKD, bool createVex, bool createSnrTable,
                                bool operNotes, bool srcGrp, const std::vector<std::string> &srcGroupsForStatistic,
                                bool createSkyCoverage, const std::vector<Contact> &contacts ) {
    boost::property_tree::ptree output;
    if ( experimentDescription.empty() ) {
        output.add( "output.experimentDescription", "no further description" );
    } else {
        output.add( "output.experimentDescription", experimentDescription );
    }
    if ( scheduler.empty() ) {
        output.add( "output.scheduler", "unknown" );
    } else {
        output.add( "output.scheduler", scheduler );
    }
    if ( correlator.empty() ) {
        output.add( "output.correlator", "unknown" );
    } else {
        output.add( "output.correlator", correlator );
    }
    for ( auto &any : contacts ) {
        addContact( any, output, "output.contacts" );
    }
    if ( !notes.empty() ) {
        output.add( "output.notes", notes );
    }

    output.add( "output.initializer_log", initializer );
    output.add( "output.iteration_log", iteration_log );
    output.add( "output.createSummary", createSummary );
    output.add( "output.createNGS", createNGS );
    if ( !NGS_directory.empty() ) {
        output.add( "output.NGS_directory", NGS_directory );
    }
    output.add( "output.createSKD", createSKD );
    output.add( "output.createVEX", createVex );
    output.add( "output.createSnrTable", createSnrTable );
    output.add( "output.createOperationsNotes", operNotes );
    output.add( "output.createSourceGroupStatistics", srcGrp );
    if ( srcGrp ) {
        boost::property_tree::ptree all_groups;
        for ( const auto &any : srcGroupsForStatistic ) {
            boost::property_tree::ptree tmp;
            tmp.add( "name", any );
            all_groups.add_child( "sourceGroupsForStatistic.name", tmp.get_child( "name" ) );
        }
        if ( !all_groups.empty() ) {
            output.add_child( "output.sourceGroupsForStatistic", all_groups.get_child( "sourceGroupsForStatistic" ) );
        }
    }
    output.add( "output.createSkyCoverage", createSkyCoverage );

    master_.add_child( "VieSchedpp.output", output.get_child( "output" ) );
}

void ParameterSettings::ruleFocusCorners( int cadence ) {
    boost::property_tree::ptree t;
    t.add( "focusCorners.cadence", cadence );

    master_.add_child( "VieSchedpp.focusCorners", t.get_child( "focusCorners" ) );
}

void ParameterSettings::ruleScanSequence( unsigned int cadence, const vector<unsigned int> &modulo,
                                          const vector<string> &member ) {
    boost::property_tree::ptree rules;
    rules.add( "rules.sourceSequence.cadence", cadence );

    unsigned long n = modulo.size();
    for ( int i = 0; i < n; ++i ) {
        boost::property_tree::ptree sourceSelection;

        sourceSelection.add( "sequence.modulo", modulo[i] );
        sourceSelection.add( "sequence.member", member[i] );

        rules.add_child( "rules.sourceSequence.sequence", sourceSelection.get_child( "sequence" ) );
    }

    master_.add_child( "VieSchedpp.rules.sourceSequence", rules.get_child( "rules.sourceSequence" ) );
}

void ParameterSettings::calibratorBlock( const std::vector<CalibratorBlock> &blocks ) {
    if ( blocks.empty() ) {
        return;
    }

    boost::property_tree::ptree rules;
    for ( const auto &any : blocks ) {
        boost::property_tree::ptree tmp;
        tmp.add( "block.startTime", any.getStartTime() );
        tmp.add( "block.scans", any.getNScans() );
        tmp.add( "block.duration", any.getDuration() );
        tmp.add( "block.sources", any.getAllowedSourceGroup() );
        rules.add_child( "calibration.block", tmp.get_child( "block" ) );
    }

    master_.add_child( "VieSchedpp.rules.calibration", rules.get_child( "calibration" ) );
}

void ParameterSettings::ruleCalibratorBlockTime( unsigned int cadence, const std::string &member,
                                                 const std::vector<std::pair<double, double>> &between_elevation,
                                                 unsigned int nMaxScans, unsigned int scanTime ) {
    boost::property_tree::ptree rules;

    rules.add( "calibratorBlock.cadence_seconds", cadence );
    rules.add( "calibratorBlock.member", member );
    rules.add( "calibratorBlock.nMaxScans", nMaxScans );
    if ( scanTime < 0 ) {
        rules.add( "calibratorBlock.fixedScanTime", scanTime );
    }

    rules.add( "calibratorBlock.lowElevation.startWeight", between_elevation.at( 0 ).first );
    rules.add( "calibratorBlock.lowElevation.fullWeight", between_elevation.at( 0 ).second );
    rules.add( "calibratorBlock.highElevation.startWeight", between_elevation.at( 1 ).first );
    rules.add( "calibratorBlock.highElevation.fullWeight", between_elevation.at( 1 ).second );

    //    for(const auto &any:between_elevation){
    //        boost::property_tree::ptree be;
    //        double el1 = any.first;
    //        double el2 = any.second;
    //        if(el1>el2){
    //            swap(el1,el2);
    //        }
    //        be.add("targetElevation.lower_limit",el1);
    //        be.add("targetElevation.upper_limit",el1);
    //        rules.add_child("calibratorBlock.targetElevations",be.get_child("targetElevations"));
    //    }

    master_.add_child( "VieSchedpp.rules.calibratorBlock", rules.get_child( "calibratorBlock" ) );
}


void ParameterSettings::ruleCalibratorBlockNScanSelections(
    unsigned int cadence, const std::string &member, const std::vector<std::pair<double, double>> &between_elevation,
    unsigned int nMaxScans, unsigned int scanTime ) {
    boost::property_tree::ptree rules;

    rules.add( "calibratorBlock.cadence_nScanSelections", cadence );
    rules.add( "calibratorBlock.member", member );
    rules.add( "calibratorBlock.nMaxScans", nMaxScans );
    if ( scanTime < 0 ) {
        rules.add( "calibratorBlock.fixedScanTime", scanTime );
    }

    rules.add( "calibratorBlock.lowElevation.startWeight", between_elevation.at( 0 ).first );
    rules.add( "calibratorBlock.lowElevation.fullWeight", between_elevation.at( 0 ).second );
    rules.add( "calibratorBlock.highElevation.startWeight", between_elevation.at( 1 ).first );
    rules.add( "calibratorBlock.highElevation.fullWeight", between_elevation.at( 1 ).second );

    //    for(const auto &any:between_elevation){
    //        boost::property_tree::ptree be;
    //        double el1 = any.first;
    //        double el2 = any.second;
    //        if(el1>el2){
    //            swap(el1,el2);
    //        }
    //        be.add("targetElevation.lower_limit",el1);
    //        be.add("targetElevation.upper_limit",el1);
    //        rules.add_child("calibratorBlock.targetElevations",be.get_child("targetElevation"));
    //    }

    master_.add_child( "VieSchedpp.rules.calibratorBlock", rules.get_child( "calibratorBlock" ) );
}


void ParameterSettings::highImpactAzEl( const std::vector<string> &members, const std::vector<double> &azs,
                                        const std::vector<double> &els, const std::vector<double> &margins,
                                        int interval, int repeat ) {
    boost::property_tree::ptree hi;
    hi.add( "highImpact.interval", interval );
    hi.add( "highImpact.repeat", repeat );

    for ( int i = 0; i < members.size(); ++i ) {
        boost::property_tree::ptree azEl;

        azEl.add( "targetAzEl.member", members[i] );
        azEl.add( "targetAzEl.az", azs[i] );
        azEl.add( "targetAzEl.el", els[i] );
        azEl.add( "targetAzEl.margin", margins[i] );

        hi.add_child( "highImpact.targetAzEl", azEl.get_child( "targetAzEl" ) );
    }

    master_.add_child( "VieSchedpp.highImpact", hi.get_child( "highImpact" ) );
}


void ParameterSettings::addContact( const ParameterSettings::Contact &contact, boost::property_tree::ptree &tree,
                                    const std::string &node ) {
    if ( !contact.name.empty() ) {
        boost::property_tree::ptree c;
        if ( ~contact.function.empty() ) {
            c.add( "contact.function", contact.function );
        }
        if ( ~contact.name.empty() ) {
            c.add( "contact.name", contact.name );
        }
        if ( ~contact.email.empty() ) {
            c.add( "contact.email", contact.email );
        }
        if ( ~contact.phone.empty() ) {
            c.add( "contact.phone", contact.phone );
        }
        if ( ~contact.affiliation.empty() ) {
            c.add( "contact.affiliation", contact.affiliation );
        }
        tree.add_child( node + ".contact", c.get_child( "contact" ) );
    }
}


VieVS::ParameterSettings::Contact ParameterSettings::readContact( const boost::property_tree::ptree &tree ) {
    Contact contact;
    contact.function = tree.get( ".contact", "" );
    contact.name = tree.get( ".name", "" );
    contact.email = tree.get( ".email", "" );
    contact.phone = tree.get( ".phone", "" );
    contact.affiliation = tree.get( ".affiliation", "" );

    return contact;
}
