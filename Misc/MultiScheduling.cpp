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
bool MultiScheduling::pick_random = false;
unsigned long MultiScheduling::nsta_ = 0;
unsigned long MultiScheduling::nsrc_ = 0;

std::default_random_engine MultiScheduling::random_engine_ = std::default_random_engine();


MultiScheduling::MultiScheduling( std::unordered_map<std::string, std::vector<std::string>> sta_group,
                                  std::unordered_map<std::string, std::vector<std::string>> src_group,
                                  std::unordered_map<std::string, std::vector<std::string>> bls_group )
    : VieVS_Object( nextId++ ),
      stationGroups_{ move( sta_group ) },
      sourceGroups_{ move( src_group ) },
      baselineGroups_{ move( bls_group ) } {}


void MultiScheduling::addParameters( const std::string &name ) { singleArgumentLogical.push_back( name ); }


void MultiScheduling::addParameters( const std::string &name, const std::vector<double> &values ) {
    singleArgumentNumeric.emplace_back( name, values );
}


void MultiScheduling::addParameters( const std::string &name, const std::string &member,
                                     const std::vector<double> &values ) {
    doubleArgumentNumeric.emplace_back( name, make_pair( member, values ) );
}


std::vector<MultiScheduling::Parameters> MultiScheduling::createMultiScheduleParameters( unsigned int maxNr ) {
    if ( pick_random ) {
        return createMultiScheduleParameters_random( maxNr );
    } else {
        return createMultiScheduleParameters_gridwise( maxNr );
    }
}


std::vector<MultiScheduling::Parameters> MultiScheduling::createMultiScheduleParameters_random( unsigned int maxNr ) {
    Parameters thisPARA;
    std::vector<Parameters> allPARA( maxNr, thisPARA );

    unsigned long dummy = 1;
    // add logical single argument parameters
    for ( const auto &name : singleArgumentLogical ) {
        addParameter( allPARA, dummy, name, true );
    }

    // add single argument parameters with values - ignore weight factors
    for ( const auto &any : singleArgumentNumeric ) {
        const string &name = any.first;
        const vector<double> &values = any.second;
        addParameter( allPARA, dummy, name, values, true );
    }

    // add double argument parameters
    for ( const auto &any : doubleArgumentNumeric ) {
        const string &name = any.first;
        const string &member = any.second.first;
        const vector<double> &values = any.second.second;
        addParameter( allPARA, dummy, name, member, values, true );
    }

    for ( auto &para : allPARA ) {
        para.normalizeWeightFactors();
        para.normalizeWeights( nsta_, nsrc_ );
    }
    return allPARA;
}


std::vector<MultiScheduling::Parameters> MultiScheduling::createMultiScheduleParameters_gridwise( unsigned int maxNr ) {
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
        { "weight_factor_sky-coverage", vector<double>{ WeightFactors::weightSkyCoverage } },
        { "weight_factor_number_of_observations", vector<double>{ WeightFactors::weightNumberOfObservations } },
        { "weight_factor_duration", vector<double>{ WeightFactors::weightDuration } },
        { "weight_factor_average_sources", vector<double>{ WeightFactors::weightAverageSources } },
        { "weight_factor_average_stations", vector<double>{ WeightFactors::weightAverageStations } },
        { "weight_factor_average_baselines", vector<double>{ WeightFactors::weightAverageBaselines } },
        { "weight_factor_idle_time", vector<double>{ WeightFactors::weightIdleTime } },
        { "weight_factor_low_declination", vector<double>{ WeightFactors::weightDeclination } },
        { "weight_factor_low_elevation", vector<double>{ WeightFactors::weightLowElevation } } };

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
    vector<double> scaleFactors;
    vector<vector<double>> weightFactorValues;
    if ( weigthFactorFound ) {
        for ( double wsky : weightFactors["weight_factor_sky-coverage"] ) {
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

                                            vector<double> wf{ wsky / sum,  wobs / sum,  wdur / sum,
                                                               wasrc / sum, wasta / sum, wabls / sum,
                                                               widle / sum, wdec / sum,  wel / sum };
                                            weightFactorValues.push_back( std::move( wf ) );
                                            scaleFactors.push_back( sum );
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
                scaleFactors.erase( next( scaleFactors.begin(), i2 ) );
            } else {
                ++i2;
            }
        }
        ++i1;
    }

    for ( int i = 0; i < weightFactorValues.size(); ++i ) {
        double scaleFactor = scaleFactors[i];
        for ( auto &v : weightFactorValues[i] ) {
            v *= scaleFactor;
        }
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
                    if ( any.first == "weight_factor_sky-coverage" ) {
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
        std::shuffle( allPARA.begin(), allPARA.end(), random_engine_ );
        allPARA.resize( maxNr );
    }

    // return all multi scheduling parameters
    return allPARA;
}


void MultiScheduling::addParameter( vector<MultiScheduling::Parameters> &allPara, unsigned long &n_before,
                                    const std::string &name, bool pick_random ) {
    unsigned long n_total = allPara.size();
    unsigned long n_block = n_before * 2;
    unsigned long n_items = n_total / n_block;
    auto gen_bool = uniform_int_distribution<>( 0, 1 );
    if ( pick_random ) {
        n_block = n_total;
        n_items = 1;
    }
    unsigned int c = 0;
    for ( int i_block = 0; i_block < n_block; ++i_block ) {
        bool thisValue;
        if ( pick_random ) {
            thisValue = gen_bool( random_engine_ );
        } else {
            thisValue = i_block % 2 == 0;
        }


        for ( int i_item = 0; i_item < n_items; ++i_item ) {
            if ( name == "general_subnetting" ) {
                allPara[c].subnetting = thisValue;
            } else if ( name == "general_fillin-mode_during_scan_selection" ) {
                allPara[c].fillinmode_duringScanSelection = thisValue;
            } else if ( name == "general_fillin-mode_influence_on_scan_selection" ) {
                allPara[c].fillinmode_influenceOnScanSelection = thisValue;
            } else if ( name == "general_fillin-mode_a_posteriori" ) {
                allPara[c].fillinmode_aPosteriori = thisValue;
            }
            ++c;
        }
    }
    if ( !pick_random ) {
        n_before = n_block;
    }
}


void MultiScheduling::addParameter( vector<MultiScheduling::Parameters> &allPara, unsigned long &n_before,
                                    const std::string &name, const std::vector<double> &values, bool pick_random ) {
    unsigned long n_total = allPara.size();
    unsigned long n_this = values.size();
    unsigned long n_block = n_before * n_this;
    unsigned long n_items = n_total / n_block;
    auto gen_uniform = uniform_real_distribution<>( *min_element( values.begin(), values.end() ),
                                                    *max_element( values.begin(), values.end() ) );
    auto gen_norm = normal_distribution<double>( 0.0, 1.0 );
    if ( pick_random ) {
        n_block = n_total;
        n_items = 1;
    }
    unsigned int c = 0;
    for ( int i_block = 0; i_block < n_block; ++i_block ) {
        double thisValue;
        if ( pick_random ) {
            thisValue = gen_uniform( random_engine_ );
        } else {
            thisValue = values[i_block % n_this];
        }

        for ( int i_item = 0; i_item < n_items; ++i_item ) {
            if ( pick_random ) {
                double randomValue = abs( gen_norm( random_engine_ ) );
                int ci = 0;
                while ( ci < 10 && ( randomValue < *min_element( values.begin(), values.end() ) ||
                                     randomValue > *max_element( values.begin(), values.end() ) ) ) {
                    randomValue = abs( gen_norm( random_engine_ ) );
                    ++ci;
                }

                if ( name == "weight_factor_sky-coverage" ) {
                    allPara[c].weightSkyCoverage = randomValue;
                } else if ( name == "weight_factor_number_of_observations" ) {
                    allPara[c].weightNumberOfObservations = randomValue;
                } else if ( name == "weight_factor_duration" ) {
                    allPara[c].weightDuration = randomValue;
                } else if ( name == "weight_factor_average_sources" ) {
                    allPara[c].weightAverageSources = randomValue;
                } else if ( name == "weight_factor_average_stations" ) {
                    allPara[c].weightAverageStations = randomValue;
                } else if ( name == "weight_factor_average_baselines" ) {
                    allPara[c].weightAverageBaselines = randomValue;
                } else if ( name == "weight_factor_idle_time" ) {
                    allPara[c].weightIdleTime = randomValue;
                } else if ( name == "weight_factor_low_declination" ) {
                    allPara[c].weightLowDeclination = randomValue;
                } else if ( name == "weight_factor_low_elevation" ) {
                    allPara[c].weightLowElevation = randomValue;
                }
            }

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

            } else if ( name == "sky-coverage_influence_distance" ) {
                allPara[c].skyCoverageInfluenceDistance = thisValue;

            } else if ( name == "sky-coverage_influence_time" ) {
                allPara[c].skyCoverageInfluenceTime = thisValue;

            } else if ( name == "general_focus_corner_switch_cadence" ) {
                allPara[c].focusCornerSwitchCadence = thisValue;
            }
            ++c;
        }
    }
    if ( !pick_random ) {
        n_before = n_block;
    }
}


void MultiScheduling::addParameter( vector<MultiScheduling::Parameters> &allPara, unsigned long &n_before,
                                    const std::string &name, const std::string &member,
                                    const std::vector<double> &values, bool pick_random ) {
    unsigned long n_total = allPara.size();
    unsigned long n_this = values.size();
    unsigned long n_block = n_before * n_this;
    unsigned long n_items = n_total / n_block;
    auto gen_double = uniform_real_distribution<>( *min_element( values.begin(), values.end() ),
                                                   *max_element( values.begin(), values.end() ) );
    if ( pick_random ) {
        n_block = n_total;
        n_items = 1;
    }
    unsigned int c = 0;
    for ( int i_block = 0; i_block < n_block; ++i_block ) {
        double thisValue;
        if ( pick_random ) {
            thisValue = gen_double( random_engine_ );
        } else {
            thisValue = values[i_block % n_this];
        }

        for ( int i_item = 0; i_item < n_items; ++i_item ) {
            if ( name == "station_weight" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].stationWeight[thisId] = thisValue;
                    }
                } else {
                    allPara[c].stationWeight[member] = thisValue;
                }

            } else if ( name == "station_min_slew_time" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].stationMinSlewtime[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].stationMinSlewtime[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "station_max_slew_time" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].stationMaxSlewtime[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].stationMaxSlewtime[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "station_min_slew_distance" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].stationMinSlewDistance[thisId] = thisValue;
                    }
                } else {
                    allPara[c].stationMinSlewDistance[member] = thisValue;
                }

            } else if ( name == "station_max_slew_distance" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].stationMaxSlewDistance[thisId] = thisValue;
                    }
                } else {
                    allPara[c].stationMaxSlewDistance[member] = thisValue;
                }

            } else if ( name == "station_max_wait_time" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].stationMaxWait[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].stationMaxWait[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "station_min_elevation" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].stationMinElevation[thisId] = thisValue;
                    }
                } else {
                    allPara[c].stationMinElevation[member] = thisValue;
                }

            } else if ( name == "station_max_number_of_scans" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].stationMaxNumberOfScans[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].stationMaxNumberOfScans[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "station_max_scan_time" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].stationMaxScan[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].stationMaxScan[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "station_min_scan_time" ) {
                if ( stationGroups_.find( member ) != stationGroups_.end() ) {
                    for ( const auto &thisId : stationGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].stationMinScan[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].stationMinScan[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "source_weight" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].sourceWeight[thisId] = thisValue;
                    }
                } else {
                    allPara[c].sourceWeight[member] = thisValue;
                }

            } else if ( name == "source_min_number_of_stations" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].sourceMinNumberOfStations[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].sourceMinNumberOfStations[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "source_min_flux" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].sourceMinFlux[thisId] = thisValue;
                    }
                } else {
                    allPara[c].sourceMinFlux[member] = thisValue;
                }

            } else if ( name == "source_max_number_of_scans" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].sourceMaxNumberOfScans[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].sourceMaxNumberOfScans[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "source_min_elevation" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].sourceMinElevation[thisId] = thisValue;
                    }
                } else {
                    allPara[c].sourceMinElevation[member] = thisValue;
                }

            } else if ( name == "source_min_sun_distance" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].sourceMinSunDistance[thisId] = thisValue;
                    }
                } else {
                    allPara[c].sourceMinSunDistance[member] = thisValue;
                }

            } else if ( name == "source_max_scan_time" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].sourceMaxScan[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].sourceMaxScan[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "source_min_scan_time" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].sourceMinScan[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].sourceMinScan[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "source_min_repeat_time" ) {
                if ( sourceGroups_.find( member ) != sourceGroups_.end() ) {
                    for ( const auto &thisId : sourceGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].sourceMinRepeat[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].sourceMinRepeat[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "baseline_weight" ) {
                if ( baselineGroups_.find( member ) != baselineGroups_.end() ) {
                    for ( const auto &thisId : baselineGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].baselineWeight[thisId] = thisValue;
                    }
                } else {
                    allPara[c].baselineWeight[member] = thisValue;
                }

            } else if ( name == "baseline_max_scan_time" ) {
                if ( baselineGroups_.find( member ) != baselineGroups_.end() ) {
                    for ( const auto &thisId : baselineGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].baselineMaxScan[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].baselineMaxScan[member] = static_cast<unsigned int>( lround( thisValue ) );
                }

            } else if ( name == "baseline_min_scan_time" ) {
                if ( baselineGroups_.find( member ) != baselineGroups_.end() ) {
                    for ( const auto &thisId : baselineGroups_[member] ) {
                        if ( pick_random ) {
                            thisValue = gen_double( random_engine_ );
                        }
                        allPara[c].baselineMinScan[thisId] = static_cast<unsigned int>( lround( thisValue ) );
                    }
                } else {
                    allPara[c].baselineMinScan[member] = static_cast<unsigned int>( lround( thisValue ) );
                }
            }
            ++c;
        }
    }
    if ( !pick_random ) {
        n_before = n_block;
    }
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

vector<MultiScheduling::Parameters> MultiScheduling::evolution_step( int gen,
                                                                     const vector<MultiScheduling::Parameters> &old_pop,
                                                                     const std::map<int, double> &scores,
                                                                     const boost::property_tree::ptree &tree ) {
    vector<MultiScheduling::Parameters> new_pop;
    int n = tree.get( "VieSchedpp.multisched.genetic.population_size", 32 );
    double best_f = tree.get( "VieSchedpp.multisched.genetic.select_best_percent", 20.0 ) / 100;
    double random_f = tree.get( "VieSchedpp.multisched.genetic.select_random_percent", 5.0 ) / 100;
    double mutation = tree.get( "VieSchedpp.multisched.genetic.mutation_acceleration", 0.5 );
    double minMutation = tree.get( "VieSchedpp.multisched.genetic.min_mutation_percent", 10.0 ) / 100;
    int n_parents = tree.get( "VieSchedpp.multisched.genetic.parents_for_crossover", 2 );

    vector<double> scores_vec;
    for ( const auto &any : scores ) {
        scores_vec.push_back( any.second );
    }
    vector<tuple<Parameters, double, int>> tmp;
    int counter = 0;
    for ( unsigned long i = gen == 0 ? 0 : old_pop.size() - n; i < old_pop.size(); ++i ) {
        tmp.emplace_back( old_pop[i], scores_vec[i], i );
    }
    sort( tmp.begin(), tmp.end(),
          []( const tuple<Parameters, double, int> &a, const tuple<Parameters, double, int> &b ) {
              return get<1>( a ) < get<1>( b );
          } );

    long best_n = lround( n * best_f );
    long random_n = lround( n * random_f );
    if ( best_n == 0 && random_n == 0 ) {
        best_n = 1;
    }
    vector<Parameters> parents;
    parents.reserve( best_n + random_n );

    // pick best populations
    for ( long i = tmp.size() - 1; i >= static_cast<long>(tmp.size()) - best_n ; --i ) {
        if(i < tmp.size() - 1 && get<1>( tmp[i] ) < 1e-3){
            break;
        }
        parents.push_back( get<0>( tmp[i] ) );
        double d = *get<0>(tmp[i]).weightSkyCoverage;

#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( info ) << boost::format(
                                         "add multi-scheduling parameter %d as parent [best] (score: %.4f)" ) %
                                         ( get<2>( tmp[i] ) + 1 ) % get<1>( tmp[i] );
#else
        cout << boost::format( "[info] add multi-scheduling parameter %d as parent (best - score: %.4f)" ) %
                    get<2>( tmp[i] ) % get<1>( tmp[i] );
#endif
    }
    tmp.erase( tmp.end() - best_n, tmp.end() );

    // randomly pick elements from remaining population
    shuffle( tmp.begin(), tmp.end(), mt19937( random_device{}() ) );
    long i_rand = 0; // count number of random selections
    long c_rand = 0; // count number of random selection attempts
    while ( i_rand < random_n && c_rand < 3*random_n && c_rand < tmp.size()) {
        if(get<1>( tmp[c_rand] ) < 1e-3){
            ++c_rand;
            continue;
        }
        parents.push_back( get<0>( tmp[c_rand] ) );
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( info ) << boost::format(
                                         "add multi-scheduling parameter %d as parent [random] (score: %.4f)" ) %
                ( get<2>( tmp[c_rand] ) +1 ) % get<1>( tmp[c_rand] );
#else
        cout << boost::format( "[info] add multi-scheduling parameter %d as parent (random - score: %.4f)" ) %
                    get<2>( tmp[c_rand] ) % get<1>( tmp[c_rand] );
#endif
        ++c_rand;
        ++i_rand;
    }

    // get parents and make children
    auto gen_int = std::uniform_int_distribution<>( 0, parents.size() - 1 );
    for ( int i = 0; i < n; ++i ) {
        vector<Parameters> ps;
        vector<int> p_idx;
        for ( int ip = 0; ip < n_parents; ++ip ) {
            for ( int c = 0; c < 5; ++c ) {
                int idx = gen_int( random_engine_ );
                if ( find( p_idx.begin(), p_idx.end(), idx ) == p_idx.end() ) {
                    ps.push_back( parents[idx] ), p_idx.push_back( idx );
                    break;
                }
            }
        }
        new_pop.emplace_back( ps, mutation, minMutation );
    }

    for ( auto &any : new_pop ) {
        any.normalizeWeightFactors();
        any.normalizeWeights( nsta_, nsrc_ );
    }

    return new_pop;
}


MultiScheduling::Parameters::Parameters( const std::vector<Parameters> &v, double mutation, double minMutation ) {
    auto gen_bool = std::uniform_int_distribution<>( 0, 1 );
    auto gen_double = std::normal_distribution<double>( 0.0, 1.0 );


    auto f_bool = [&gen_bool]( const std::vector<boost::optional<bool>> &v ) {
        boost::optional<bool> r = boost::none;
        if ( !v.empty() && v.at( 0 ).is_initialized() ) {
            for ( const auto &any : v ) {
                if ( r == boost::none ) {
                    r = any;
                } else {
                    if ( r != any ) {
                        r = gen_bool( random_engine_ );
                        return r;
                    }
                }
            }
        }
        return r;
    };

    auto f_double = [&gen_double, mutation, minMutation]( const std::vector<boost::optional<double>> &v ) {
        boost::optional<double> r = boost::none;
        if ( !v.empty() && v.at( 0 ).is_initialized() ) {
            vector<double> vv;
            for ( const auto &any : v ) {
                vv.push_back( *any );
            }
            double min = *min_element( vv.begin(), vv.end() );
            double max = *max_element( vv.begin(), vv.end() );
            double mean = accumulate( vv.begin(), vv.end(), 0.0 ) / vv.size();
            double delta = abs( max - min );
            double delta_min = mean * minMutation;
            if ( delta < delta_min ) {
                delta = delta_min;
            }
            double n_val = mean + mutation * delta * gen_double( random_engine_ );
            r = n_val;
            if ( *r < 0 ) {
                r = 0;
            }
        }
        return r;
    };

    auto f_map_double = [&gen_double, mutation, minMutation]( const vector<map<std::string, double>> &v ) {
        map<std::string, double> r;
        map<std::string, double> min;
        map<std::string, double> max;

        for ( const auto &item : v ) {
            for ( const auto &any : item ) {
                string name = any.first;
                double val = any.second;
                if ( r.find( name ) == r.end() ) {
                    r[name] = val;
                    min[name] = val;
                    max[name] = val;
                } else {
                    r[name] += val;
                    if ( val < min[name] ) {
                        min[name] = val;
                    }
                    if ( val > max[name] ) {
                        max[name] = val;
                    }
                }
            }
        }

        for ( auto &any : r ) {
            string name = any.first;
            double mean = any.second / v.size();
            double delta = max[name] - min[name];
            double delta_min = mean * minMutation;
            if ( delta < delta_min ) {
                delta = delta_min;
            }
            double val = mean + mutation * delta * gen_double( random_engine_ );
            if ( val < 0 ) {
                val = 0;
            }
            any.second = val;
        }
        return r;
    };

    auto f_map_uint = [&gen_double, mutation, minMutation]( const vector<map<std::string, unsigned int>> &v ) {
        map<std::string, unsigned int> r;
        map<std::string, unsigned int> min;
        map<std::string, unsigned int> max;

        for ( const auto &item : v ) {
            for ( const auto &any : item ) {
                string name = any.first;
                unsigned int val = any.second;
                if ( r.find( name ) == r.end() ) {
                    r[name] = val;
                    min[name] = val;
                    max[name] = val;
                } else {
                    r[name] += val;
                    if ( val < min[name] ) {
                        min[name] = val;
                    }
                    if ( val > max[name] ) {
                        max[name] = val;
                    }
                }
            }
        }

        for ( auto &any : r ) {
            string name = any.first;
            long mean = lround( static_cast<double>( any.second ) / v.size() );
            int delta = max[name] - min[name];
            int delta_min = lround( mean * minMutation );
            if ( delta < delta_min ) {
                delta = delta_min;
            }
            double val = lround( mean + mutation * delta * gen_double( random_engine_ ) );
            if ( val < 0 ) {
                val = 0;
            }
            any.second = val;
        }
        return r;
    };

    // boolean single value
    subnetting = f_bool( collect<boost::optional<bool>>( &Parameters::subnetting, v ) );
    fillinmode_duringScanSelection = f_bool( collect<boost::optional<bool>>( &Parameters::subnetting, v ) );
    fillinmode_influenceOnScanSelection = f_bool( collect<boost::optional<bool>>( &Parameters::subnetting, v ) );
    fillinmode_aPosteriori = f_bool( collect<boost::optional<bool>>( &Parameters::subnetting, v ) );


    // double single value
    focusCornerSwitchCadence = f_double( collect<boost::optional<double>>( &Parameters::focusCornerSwitchCadence, v ) );
    subnetting_minSourceAngle =
        f_double( collect<boost::optional<double>>( &Parameters::subnetting_minSourceAngle, v ) );
    subnetting_minParticipatingStations =
        f_double( collect<boost::optional<double>>( &Parameters::subnetting_minParticipatingStations, v ) );
    weightSkyCoverage = f_double( collect<boost::optional<double>>( &Parameters::weightSkyCoverage, v ) );
    weightNumberOfObservations =
        f_double( collect<boost::optional<double>>( &Parameters::weightNumberOfObservations, v ) );
    weightDuration = f_double( collect<boost::optional<double>>( &Parameters::weightDuration, v ) );
    weightAverageSources = f_double( collect<boost::optional<double>>( &Parameters::weightAverageSources, v ) );
    weightAverageStations = f_double( collect<boost::optional<double>>( &Parameters::weightAverageStations, v ) );
    weightAverageBaselines = f_double( collect<boost::optional<double>>( &Parameters::weightAverageBaselines, v ) );
    weightIdleTime = f_double( collect<boost::optional<double>>( &Parameters::weightIdleTime, v ) );
    weightIdleTime_interval = f_double( collect<boost::optional<double>>( &Parameters::weightIdleTime_interval, v ) );
    weightLowDeclination = f_double( collect<boost::optional<double>>( &Parameters::weightLowDeclination, v ) );
    weightLowDeclination_begin =
        f_double( collect<boost::optional<double>>( &Parameters::weightLowDeclination_begin, v ) );
    weightLowDeclination_full =
        f_double( collect<boost::optional<double>>( &Parameters::weightLowDeclination_full, v ) );
    weightLowElevation = f_double( collect<boost::optional<double>>( &Parameters::weightLowElevation, v ) );
    weightLowElevation_begin = f_double( collect<boost::optional<double>>( &Parameters::weightLowElevation_begin, v ) );
    weightLowElevation_full = f_double( collect<boost::optional<double>>( &Parameters::weightLowElevation_full, v ) );
    skyCoverageInfluenceDistance =
        f_double( collect<boost::optional<double>>( &Parameters::skyCoverageInfluenceDistance, v ) );
    skyCoverageInfluenceTime = f_double( collect<boost::optional<double>>( &Parameters::skyCoverageInfluenceTime, v ) );

    // maps
    stationWeight = f_map_double( collect<map<string, double>>( &Parameters::stationWeight, v ) );
    stationMinSlewtime = f_map_uint( collect<map<string, unsigned int>>( &Parameters::stationMinSlewtime, v ) );
    stationMaxSlewtime = f_map_uint( collect<map<string, unsigned int>>( &Parameters::stationMaxSlewtime, v ) );
    stationMinSlewDistance = f_map_double( collect<map<string, double>>( &Parameters::stationMinSlewDistance, v ) );
    stationMaxSlewDistance = f_map_double( collect<map<string, double>>( &Parameters::stationMaxSlewDistance, v ) );
    stationMaxWait = f_map_uint( collect<map<string, unsigned int>>( &Parameters::stationMaxWait, v ) );
    stationMinElevation = f_map_double( collect<map<string, double>>( &Parameters::stationMinElevation, v ) );
    stationMaxNumberOfScans =
        f_map_uint( collect<map<string, unsigned int>>( &Parameters::stationMaxNumberOfScans, v ) );
    stationMaxScan = f_map_uint( collect<map<string, unsigned int>>( &Parameters::stationMaxScan, v ) );
    stationMinScan = f_map_uint( collect<map<string, unsigned int>>( &Parameters::stationMinScan, v ) );

    sourceWeight = f_map_double( collect<map<string, double>>( &Parameters::sourceWeight, v ) );
    sourceMinNumberOfStations =
        f_map_uint( collect<map<string, unsigned int>>( &Parameters::sourceMinNumberOfStations, v ) );
    sourceMinFlux = f_map_double( collect<map<string, double>>( &Parameters::sourceMinFlux, v ) );
    sourceMaxNumberOfScans = f_map_uint( collect<map<string, unsigned int>>( &Parameters::sourceMaxNumberOfScans, v ) );
    sourceMinElevation = f_map_double( collect<map<string, double>>( &Parameters::sourceMinElevation, v ) );
    sourceMinSunDistance = f_map_double( collect<map<string, double>>( &Parameters::sourceMinSunDistance, v ) );
    sourceMaxScan = f_map_uint( collect<map<string, unsigned int>>( &Parameters::sourceMaxScan, v ) );
    sourceMinScan = f_map_uint( collect<map<string, unsigned int>>( &Parameters::sourceMinScan, v ) );
    sourceMinRepeat = f_map_uint( collect<map<string, unsigned int>>( &Parameters::sourceMinRepeat, v ) );

    baselineWeight = f_map_double( collect<map<string, double>>( &Parameters::baselineWeight, v ) );
    baselineMaxScan = f_map_uint( collect<map<string, unsigned int>>( &Parameters::baselineMaxScan, v ) );
    baselineMinScan = f_map_uint( collect<map<string, unsigned int>>( &Parameters::baselineMinScan, v ) );
}

void MultiScheduling::Parameters::normalizeWeightFactors() {
    double sum = 0;

    if ( weightSkyCoverage.is_initialized() ) {
        sum += *weightSkyCoverage;
    } else {
        sum += WeightFactors::weightSkyCoverage;
    }

    if ( weightNumberOfObservations.is_initialized() ) {
        sum += *weightNumberOfObservations;
    } else {
        sum += WeightFactors::weightNumberOfObservations;
    }

    if ( weightDuration.is_initialized() ) {
        sum += *weightDuration;
    } else {
        sum += WeightFactors::weightDuration;
    }

    if ( weightAverageSources.is_initialized() ) {
        sum += *weightAverageSources;
    } else {
        sum += WeightFactors::weightAverageSources;
    }

    if ( weightAverageStations.is_initialized() ) {
        sum += *weightAverageStations;
    } else {
        sum += WeightFactors::weightAverageStations;
    }

    if ( weightAverageBaselines.is_initialized() ) {
        sum += *weightAverageBaselines;
    } else {
        sum += WeightFactors::weightAverageBaselines;
    }

    if ( weightIdleTime.is_initialized() ) {
        sum += *weightIdleTime;
    } else {
        sum += WeightFactors::weightIdleTime;
    }

    if ( weightLowDeclination.is_initialized() ) {
        sum += *weightLowDeclination;
    } else {
        sum += WeightFactors::weightDeclination;
    }

    if ( weightLowElevation.is_initialized() ) {
        sum += *weightLowElevation;
    } else {
        sum += WeightFactors::weightLowElevation;
    }

    if ( sum > 0 && weightSkyCoverage.is_initialized() ) {
        *weightSkyCoverage /= sum;
    }
    if ( sum > 0 && weightNumberOfObservations.is_initialized() ) {
        *weightNumberOfObservations /= sum;
    }
    if ( sum > 0 && weightDuration.is_initialized() ) {
        *weightDuration /= sum;
    }
    if ( sum > 0 && weightAverageSources.is_initialized() ) {
        *weightAverageSources /= sum;
    }
    if ( sum > 0 && weightAverageStations.is_initialized() ) {
        *weightAverageStations /= sum;
    }
    if ( sum > 0 && weightAverageBaselines.is_initialized() ) {
        *weightAverageBaselines /= sum;
    }
    if ( sum > 0 && weightIdleTime.is_initialized() ) {
        *weightIdleTime /= sum;
    }
    if ( sum > 0 && weightLowDeclination.is_initialized() ) {
        *weightLowDeclination /= sum;
    }
    if ( sum > 0 && weightLowElevation.is_initialized() ) {
        *weightLowElevation /= sum;
    }
}


void MultiScheduling::Parameters::normalizeWeights( unsigned long nsta, unsigned long nsrc ) {
    unsigned long nbl = ( nsta * ( nsta - 1 ) ) / 2;

    auto f = []( map<string, double> &p ) {
        double sum = 0;
        double n = p.size();
        for ( const auto &any : p ) {
            sum += any.second;
        }
        for ( auto &any : p ) {
            any.second = any.second / sum * n;
        }
    };

    if ( stationWeight.size() == nsta ) {
        f( stationWeight );
    }
    if ( baselineWeight.size() == nbl ) {
        f( baselineWeight );
    }
    if ( sourceWeight.size() == nsrc ) {
        f( sourceWeight );
    }
}
