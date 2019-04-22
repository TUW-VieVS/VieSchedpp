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

#include "MultiScheduling.h"


using namespace std;
using namespace VieVS;

unsigned long MultiScheduling::nextId = 0;


MultiScheduling::MultiScheduling( std::unordered_map<std::string, std::vector<std::string>> sta_group,
                                  std::unordered_map<std::string, std::vector<std::string>> src_group,
                                  std::unordered_map<std::string, std::vector<std::string>> bls_group )
    : VieVS_Object( nextId++ ),
      stationGroups_{move( sta_group )},
      sourceGroups_{move( src_group )},
      baselineGroups_{move( bls_group )} {}


void MultiScheduling::addParameters( const std::string &name ) { singleArgumentLogical.push_back( name ); }


void MultiScheduling::addParameters( const std::string &name, const std::vector<double> &values ) {
    singleArgumentNumeric.emplace_back( name, values );
}


void MultiScheduling::addParameters( const std::string &name, const std::string &member,
                                     const std::vector<double> &values ) {
    doubleArgumentNumeric.emplace_back( name, make_pair( member, values ) );
}


std::vector<MultiScheduling::Parameters> MultiScheduling::createMultiScheduleParameters( unsigned int maxNr,
                                                                                         unsigned int seed ) {
    std::vector<unsigned int> counter;

    // count start times
    if ( !start_.empty() ) {
        counter.push_back( static_cast<unsigned int &&>( start_.size() ) );
    }

    // count all logical single argument parameters
    for ( const auto &tmp : singleArgumentLogical ) {
        counter.push_back( 2 );
    }

    // create map with all weight factors
    map<string, vector<double>> weightFactors = {
        {"weight_factor_sky_coverage", vector<double>{WeightFactors::weightSkyCoverage}},
        {"weight_factor_number_of_observations", vector<double>{WeightFactors::weightNumberOfObservations}},
        {"weight_factor_duration", vector<double>{WeightFactors::weightDuration}},
        {"weight_factor_average_sources", vector<double>{WeightFactors::weightAverageSources}},
        {"weight_factor_average_stations", vector<double>{WeightFactors::weightAverageStations}},
        {"weight_factor_average_baselines", vector<double>{WeightFactors::weightAverageBaselines}},
        {"weight_factor_idle_time", vector<double>{WeightFactors::weightIdleTime}},
        {"weight_factor_low_declination", vector<double>{WeightFactors::weightDeclination}},
        {"weight_factor_low_elevation", vector<double>{WeightFactors::weightLowElevation}}};

    // check if a weight factor is changed during multi scheduling
    bool weigthFactorFound = false;
    for ( const auto &any : singleArgumentNumeric ) {
        const string &name = any.first;
        const vector<double> &value = any.second;

        if ( weightFactors.find( name ) != weightFactors.end() ) {
            weightFactors[name] = value;
            weigthFactorFound = true;
        }
    }

    // normalize all weight factors
    vector<vector<double>> weightFactorValues;
    if ( weigthFactorFound ) {
        for ( double wsky : weightFactors["weight_factor_sky_coverage"] ) {
            for ( double wobs : weightFactors["weight_factor_number_of_observations"] ) {
                for ( double wdur : weightFactors["weight_factor_duration"] ) {
                    for ( double wasrc : weightFactors["weight_factor_average_sources"] ) {
                        for ( double wasta : weightFactors["weight_factor_average_stations"] ) {
                            for ( double wabls : weightFactors["weight_factor_average_baselines"] ) {
                                for ( double widle : weightFactors["weight_factor_idle_time"] ) {
                                    for ( double wdec : weightFactors["weight_factor_low_declination"] ) {
                                        for ( double wel : weightFactors["weight_factor_low_elevation"] ) {
                                            double sum =
                                                wsky + wobs + wdur + wasrc + wasta + wabls + widle + wdec + wel;

                                            if ( sum == 0 ) {
                                                continue;
                                            }

                                            vector<double> wf{wsky / sum,  wobs / sum,  wdur / sum,
                                                              wasrc / sum, wasta / sum, wabls / sum,
                                                              widle / sum, wdec / sum,  wel / sum};
                                            weightFactorValues.push_back( std::move( wf ) );
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // remove duplicated weight factors
    int i1 = 0;
    while ( i1 < weightFactorValues.size() ) {
        const vector<double> &v1 = weightFactorValues[i1];
        int i2 = i1 + 1;

        while ( i2 < weightFactorValues.size() ) {
            const vector<double> &v2 = weightFactorValues[i2];
            int equal = 0;
            for ( int i3 = 0; i3 < v1.size(); ++i3 ) {
                if ( abs( v1[i3] - v2[i3] ) < 1e-10 ) {
                    ++equal;
                }
            }
            if ( equal == v1.size() ) {
                weightFactorValues.erase( next( weightFactorValues.begin(), i2 ) );
            } else {
                ++i2;
            }
        }
        ++i1;
    }

    // count weight factors
    if ( !weightFactorValues.empty() ) {
        counter.push_back( static_cast<unsigned int &&>( weightFactorValues.size() ) );
    }

    // count single argument parameters with values
    for ( const auto &any : singleArgumentNumeric ) {
        const string &name = any.first;
        if ( weightFactors.find( name ) != weightFactors.end() ) {
            continue;
        }
        counter.push_back( static_cast<unsigned int &&>( any.second.size() ) );
    }

    // count double argument parameters with values
    for ( const auto &any : doubleArgumentNumeric ) {
        counter.push_back( static_cast<unsigned int &&>( any.second.second.size() ) );
    }

    // total number of multi scheduing parameters
    unsigned int n_total = 1;
    for ( auto &i : counter ) {
        n_total *= i;
    }

    Parameters thisPARA;
    if ( n_total > 9999 ) {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( warning ) << "too many multi scheduling parameters! (" << n_total
                                     << ") -> multi scheduling ignored";
#else
        cout << "[warning] too many multi scheduling parameters! (" << n_total << ") -> multi scheduling ignored";
#endif
        return std::vector<Parameters>{};
    }

    std::vector<Parameters> allPARA( n_total, thisPARA );

    unsigned long n_before = 1;

    if ( !start_.empty() ) {
        unsigned long n_this = start_.size();
        unsigned long n_block = n_before * n_this;
        unsigned long n_items = n_total / n_block;
        unsigned int c = 0;
        for ( int i_block = 0; i_block < n_block; ++i_block ) {
            auto thisValue = start_[i_block % n_this];

            for ( int i_item = 0; i_item < n_items; ++i_item ) {
                allPARA[c].start = thisValue;
                ++c;
            }
        }
        n_before = n_block;
    }

    // add weight factors
    if ( !weightFactorValues.empty() ) {
        unsigned long n_this = weightFactorValues.size();
        unsigned long n_block = n_before * n_this;
        unsigned long n_items = n_total / n_block;
        unsigned int c = 0;
        for ( int i_block = 0; i_block < n_block; ++i_block ) {
            auto thisValue = weightFactorValues[i_block % n_this];

            for ( int i_item = 0; i_item < n_items; ++i_item ) {
                for ( const auto &any : singleArgumentNumeric ) {
                    if ( any.first == "weight_factor_sky_coverage" ) {
                        allPARA[c].weightSkyCoverage = thisValue[0];
                        break;
                    }
                }
                for ( const auto &any : singleArgumentNumeric ) {
                    if ( any.first == "weight_factor_number_of_observations" ) {
                        allPARA[c].weightNumberOfObservations = thisValue[1];
                        break;
                    }
                }
                for ( const auto &any : singleArgumentNumeric ) {
                    if ( any.first == "weight_factor_duration" ) {
                        allPARA[c].weightDuration = thisValue[2];
                        break;
                    }
                }
                for ( const auto &any : singleArgumentNumeric ) {
                    if ( any.first == "weight_factor_average_sources" ) {
                        allPARA[c].weightAverageSources = thisValue[3];
                        break;
                    }
                }
                for ( const auto &any : singleArgumentNumeric ) {
                    if ( any.first == "weight_factor_average_stations" ) {
                        allPARA[c].weightAverageStations = thisValue[4];
                        break;
                    }
                }
                for ( const auto &any : singleArgumentNumeric ) {
                    if ( any.first == "weight_factor_average_baselines" ) {
                        allPARA[c].weightAverageBaselines = thisValue[5];
                        break;
                    }
                }
                for ( const auto &any : singleArgumentNumeric ) {
                    if ( any.first == "weight_factor_idle_time" ) {
                        allPARA[c].weightIdleTime = thisValue[6];
                        break;
                    }
                }
                for ( const auto &any : singleArgumentNumeric ) {
                    if ( any.first == "weight_factor_low_declination" ) {
                        allPARA[c].weightLowDeclination = thisValue[7];
                        break;
                    }
                }
                for ( const auto &any : singleArgumentNumeric ) {
                    if ( any.first == "weight_factor_low_elevation" ) {
                        allPARA[c].weightLowElevation = thisValue[8];
                        break;
                    }
                }

                ++c;
            }
        }
        n_before = n_block;
    }

    // add logical single argument parameters
    for ( const auto &name : singleArgumentLogical ) {
        addParameter( allPARA, n_before, name );
    }

    // add single argument parameters with values - ignore weight factors
    for ( const auto &any : singleArgumentNumeric ) {
        const string &name = any.first;
        const vector<double> &values = any.second;

        // ignore weight factors
        if ( weightFactors.find( name ) == weightFactors.end() ) {
            addParameter( allPARA, n_before, name, values );
        }
    }

    // add double argument parameters
    for ( const auto &any : doubleArgumentNumeric ) {
        const string &name = any.first;
        const string &member = any.second.first;
        const vector<double> &values = any.second.second;

        // ignore weight factors
        if ( weightFactors.find( name ) == weightFactors.end() ) {
            addParameter( allPARA, n_before, name, member, values );
        }
    }

    // schuffle parameters randomly (using seed)
    if ( allPARA.size() > maxNr ) {
        std::shuffle( allPARA.begin(), allPARA.end(), std::default_random_engine( seed ) );
        allPARA.resize( maxNr );
    }

    // return all multi scheduling parameters
    return allPARA;
}


void MultiScheduling::addParameter( vector<MultiScheduling::Parameters> &allPara, unsigned long &n_before,
                                    const std::string &name ) {
    unsigned long n_total = allPara.size();
    unsigned long n_block = n_before * 2;
    unsigned long n_items = n_total / n_block;
    unsigned int c = 0;
    for ( int i_block = 0; i_block < n_block; ++i_block ) {
        bool thisValue = i_block % 2 == 0;

        for ( int i_item = 0; i_item < n_items; ++i_item ) {
            if ( name == "general_subnetting" ) {
                allPara[c].subnetting = thisValue;
            } else if ( name == "general_fillinmode_during_scan_selection" ) {
                allPara[c].fillinmode_duringScanSelection = thisValue;
            } else if ( name == "general_fillinmode_influence_on_scan_selection" ) {
                allPara[c].fillinmode_influenceOnScanSelection = thisValue;
            } else if ( name == "general_fillinmode_a_posteriori" ) {
                allPara[c].fillinmode_aPosteriori = thisValue;
            }
            ++c;
        }
    }
    n_before = n_block;
}


void MultiScheduling::addParameter( vector<MultiScheduling::Parameters> &allPara, unsigned long &n_before,
                                    const std::string &name, const std::vector<double> &values ) {
    unsigned long n_total = allPara.size();
    unsigned long n_this = values.size();
    unsigned long n_block = n_before * n_this;
    unsigned long n_items = n_total / n_block;
    unsigned int c = 0;
    for ( int i_block = 0; i_block < n_block; ++i_block ) {
        auto thisValue = values[i_block % n_this];

        for ( int i_item = 0; i_item < n_items; ++i_item ) {
            if ( name == "general_subnetting_min_source_angle" ) {
                allPara[c].subnetting_minSourceAngle = thisValue;

            } else if ( name == "general_subnetting_min_participating_stations" ) {
                allPara[c].subnetting_minParticipatingStations = thisValue;

            } else if ( name == "weight_factor_idle_time" ) {
                allPara[c].weightIdleTime = thisValue;

            } else if ( name == "weight_factor_idle_time_interval" ) {
                allPara[c].weightIdleTime_interval = thisValue;

            } else if ( name == "weight_factor_low_declination_begin" ) {
                allPara[c].weightLowDeclination_begin = thisValue;

            } else if ( name == "weight_factor_low_declination_full" ) {
                allPara[c].weightLowDeclination_full = thisValue;

            } else if ( name == "weight_factor_low_elevation_begin" ) {
                allPara[c].weightLowElevation_begin = thisValue;

            } else if ( name == "weight_factor_low_elevation_full" ) {
                allPara[c].weightLowElevation_full = thisValue;

            } else if ( name == "weight_factor_influence_distance" ) {
                allPara[c].skyCoverageInfluenceDistance = thisValue;

            } else if ( name == "weight_factor_influence_time" ) {
                allPara[c].skyCoverageInfluenceTime = thisValue;
            }
            ++c;
        }
    }
    n_before = n_block;
}


void MultiScheduling::addParameter( vector<MultiScheduling::Parameters> &allPara, unsigned long &n_before,
                                    const std::string &name, const std::string &member,
                                    const std::vector<double> &values ) {
    unsigned long n_total = allPara.size();
    unsigned long n_this = values.size();
    unsigned long n_block = n_before * n_this;
    unsigned long n_items = n_total / n_block;
    unsigned int c = 0;
    for ( int i_block = 0; i_block < n_block; ++i_block ) {
        auto thisValue = values[i_block % n_this];

        for ( int i_item = 0; i_item < n_items; ++i_item ) {
            if ( name == "station_weight" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        allPara[c].stationWeight[thisId] = thisValue;
                    }
                } else {
                    allPara[c].stationWeight[member] = thisValue;
                }

            } else if ( name == "station_max_slew_time" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        allPara[c].stationMaxSlewtime[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].stationMaxSlewtime[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "station_max_slew_time" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        allPara[c].stationMinSlewDistance[thisId] = thisValue;
                    }
                } else {
                    allPara[c].stationMinSlewDistance[member] = thisValue;
                }

            } else if ( name == "station_max_slew_distance" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        allPara[c].stationMaxSlewDistance[thisId] = thisValue;
                    }
                } else {
                    allPara[c].stationMaxSlewDistance[member] = thisValue;
                }

            } else if ( name == "station_max_wait_time" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        allPara[c].stationMaxWait[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].stationMaxWait[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "station_min_elevation" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        allPara[c].stationMinElevation[thisId] = thisValue;
                    }
                } else {
                    allPara[c].stationMinElevation[member] = thisValue;
                }

            } else if ( name == "station_max_number_of_scans" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        allPara[c].stationMaxNumberOfScans[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].stationMaxNumberOfScans[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "station_max_scan_time" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        allPara[c].stationMaxScan[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].stationMaxScan[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "station_min_scan_time" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        allPara[c].stationMinScan[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].stationMinScan[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "source_weight" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        allPara[c].sourceWeight[thisId] = thisValue;
                    }
                } else {
                    allPara[c].sourceWeight[member] = thisValue;
                }

            } else if ( name == "source_min_number_of_stations" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        allPara[c].sourceMinNumberOfStations[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].sourceMinNumberOfStations[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "source_min_flux" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        allPara[c].sourceMinFlux[thisId] = thisValue;
                    }
                } else {
                    allPara[c].sourceMinFlux[member] = thisValue;
                }

            } else if ( name == "source_max_number_of_scans" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        allPara[c].sourceMaxNumberOfScans[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].sourceMaxNumberOfScans[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "source_min_elevation" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        allPara[c].sourceMinElevation[thisId] = thisValue;
                    }
                } else {
                    allPara[c].sourceMinElevation[member] = thisValue;
                }

            } else if ( name == "source_min_sun_distance" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        allPara[c].sourceMinSunDistance[thisId] = thisValue;
                    }
                } else {
                    allPara[c].sourceMinSunDistance[member] = thisValue;
                }

            } else if ( name == "source_max_scan_time" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        allPara[c].sourceMaxScan[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].sourceMaxScan[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "source_min_scan_time" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        allPara[c].sourceMinScan[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].sourceMinScan[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "source_min_repeat_time" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        allPara[c].sourceMinRepeat[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].sourceMinRepeat[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "baseline_weight" ) {
                if ( baselineGroups_.find( member ) != baselineGroups_.end() ) {
                    for ( const auto &thisId : baselineGroups_[member] ) {
                        allPara[c].baselineWeight[thisId] = thisValue;
                    }
                } else {
                    allPara[c].baselineWeight[member] = thisValue;
                }

            } else if ( name == "baseline_max_scan_time" ) {
                if ( baselineGroups_.find( member ) != baselineGroups_.end() ) {
                    for ( const auto &thisId : baselineGroups_[member] ) {
                        allPara[c].baselineMaxScan[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].baselineMaxScan[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "baseline_min_scan_time" ) {
                if ( baselineGroups_.find( member ) != baselineGroups_.end() ) {
                    for ( const auto &thisId : baselineGroups_[member] ) {
                        allPara[c].baselineMinScan[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].baselineMinScan[member] = static_cast<unsigned int>( lround( thisValue ) );
                }
            }
            ++c;
        }
    }
    n_before = n_block;
}


boost::property_tree::ptree MultiScheduling::createPropertyTree() const {
    boost::property_tree::ptree pt;

    if ( !start_.empty() ) {
        boost::property_tree::ptree pt_tmp;
        for ( const auto &any : start_ ) {
            boost::property_tree::ptree value;
            int month = any.date().month();
            std::string dateStr =
                ( boost::format( "%04d.%02d.%02d %02d:%02d:%02d" ) % any.date().year() % month % any.date().day() %
                  any.time_of_day().hours() % any.time_of_day().minutes() % any.time_of_day().seconds() )
                    .str();
            value.add( "start.value", dateStr );
            pt_tmp.add_child( "start.value", value.get_child( "start.value" ) );
        }
        pt.add_child( "multisched.start", pt_tmp.get_child( "start" ) );
    }

    string path = string( "multisched." );
    for ( const auto &any : singleArgumentLogical ) {
        pt.add( path + any, "" );
    }

    for ( const auto &any : singleArgumentNumeric ) {
        const string &name = any.first;
        const vector<double> &values = any.second;

        boost::property_tree::ptree pt_tmp;
        for ( const auto &v : values ) {
            boost::property_tree::ptree value;
            value.add( name + ".value", v );
            pt_tmp.add_child( name + ".value", value.get_child( name + ".value" ) );
        }
        pt.add_child( path + name, pt_tmp.get_child( name ) );
    }

    for ( const auto &any : doubleArgumentNumeric ) {
        const string &name = any.first;
        const string &member = any.second.first;
        const vector<double> &values = any.second.second;

        boost::property_tree::ptree pt_tmp;
        for ( const auto &v : values ) {
            boost::property_tree::ptree value;
            value.add( name + ".value", v );
            pt_tmp.add_child( name + ".value", value.get_child( name + ".value" ) );
        }
        pt_tmp.add( name + ".<xmlattr>.member", member );

        pt.add_child( path + name, pt_tmp.get_child( name ) );
    }

    return pt;
}
