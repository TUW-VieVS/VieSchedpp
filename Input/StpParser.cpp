//
// Created by mschartner on 12/3/21.
//

#include "StpParser.h"

using namespace VieVS;
using namespace std;
unsigned long StpParser::nextId = 0;

void StpParser::parse() {
    std::ifstream file( fname_ );
    // Find the positions of the last '/' and the last '.'
    size_t lastSlash = fname_.find_last_of('/');
    size_t lastDot   = fname_.find_last_of('.');

    std::string staname;
    if (lastDot != std::string::npos && lastDot > lastSlash)
        staname = fname_.substr(lastSlash + 1, lastDot - lastSlash - 1);
    else
        staname = fname_.substr(lastSlash + 1); // fallback if there's no dot

    // antenna parameters
    double slew_az = numeric_limits<double>::quiet_NaN();
    double slew_el = numeric_limits<double>::quiet_NaN();
    double accl_az = numeric_limits<double>::quiet_NaN();
    double accl_el = numeric_limits<double>::quiet_NaN();
    double dece_az = numeric_limits<double>::quiet_NaN();
    double dece_el = numeric_limits<double>::quiet_NaN();
    double tsettle_az = numeric_limits<double>::quiet_NaN();
    double tsettle_el = numeric_limits<double>::quiet_NaN();

    // cable wrap parameters
    double el_min = numeric_limits<double>::quiet_NaN();
    double el_max = numeric_limits<double>::quiet_NaN();
    double az_min = numeric_limits<double>::quiet_NaN();
    double az_cmin = numeric_limits<double>::quiet_NaN();
    double az_cmax = numeric_limits<double>::quiet_NaN();
    double az_max = numeric_limits<double>::quiet_NaN();

    // horizon mask parameters
    vector<double> hor_az;
    vector<double> hor_el;

    if ( file.is_open() ) {
        std::string line;
        while ( std::getline( file, line ) ) {
            boost::trim( line );
            if ( line[0] == '#' ) {
                continue;
            }
            vector<string> splitVector;
            boost::split( splitVector, line, boost::is_space(), boost::token_compress_on );
            if ( splitVector[0] == "LAST_UPDATE:" ) {
                version_ = splitVector[3];
            }
            if ( splitVector[0] == "COORD:" ) {
                auto x = boost::lexical_cast<double>( splitVector[3] );
                auto y = boost::lexical_cast<double>( splitVector[4] );
                auto z = boost::lexical_cast<double>( splitVector[5] );
                position_ = make_shared<Position>( x, y, z, staname, "stp_file" );
            }
            if ( splitVector[0] == "MOUNT:" ) {
                mount_ = splitVector[3];
            }

            // antenna parameters
            if ( splitVector[0] == "SLEW_AZ:" ) {
                slew_az = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "SLEW_EL:" ) {
                slew_el = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "ACCL_AZ:" ) {
                accl_az = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "ACCL_EL:" ) {
                accl_el = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "DECE_AZ:" ) {
                dece_az = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "DECE_EL:" ) {
                dece_el = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "TSETTLE_AZ:" ) {
                tsettle_az = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "TSETTLE_EL:" ) {
                tsettle_el = boost::lexical_cast<double>( splitVector[3] );
            }

            // cable wrap parameters
            if ( splitVector[0] == "EL_MIN:" ) {
                el_min = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "EL_MAX:" ) {
                el_max = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "AZ_RANGE:" ) {
                az_min = boost::lexical_cast<double>( splitVector[3] );
                az_cmin = boost::lexical_cast<double>( splitVector[4] );
                az_cmax = boost::lexical_cast<double>( splitVector[5] );
                az_max = boost::lexical_cast<double>( splitVector[6] );
            }

            // horizon mask parameters
            if ( splitVector[0] == "HOR_AZIM:" ) {
                for ( int i = 3; i < splitVector.size(); ++i ) {
                    const auto& v = splitVector[i];
                    hor_az.push_back( boost::lexical_cast<double>( v ) * deg2rad );
                }
            }
            if ( splitVector[0] == "HOR_ELEV:" ) {
                for ( int i = 3; i < splitVector.size(); ++i ) {
                    const auto& v = splitVector[i];
                    hor_el.push_back( boost::lexical_cast<double>( v ) * deg2rad );
                }
            }


            // general parameters
            if ( splitVector[0] == "PREOB:" ) {
                preob_ = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "POSTOB:" ) {
                postob_ = boost::lexical_cast<double>( splitVector[3] );
            }
            if ( splitVector[0] == "RECORDER:" ) {
                recorder_ = splitVector[3];
            }
            if ( splitVector[0] == "TSYS_DATES:" ) {
                Tsys tsys = parse_tsys( file, line );
                tsyss_.push_back( tsys );
            }
            if ( splitVector[0] == "GAIN_DATES:" ) {
                Gain gain = parse_gain( file, line );
                gains_.push_back( gain );
            }
        }
        file.close();

        if ( isnan( dece_az ) && !isnan( accl_az ) ) {
            dece_az = accl_az;
        }
        if ( isnan( dece_el ) && !isnan( accl_el ) ) {
            dece_el = accl_el;
        }

        calcEquip();

        if ( mount_ == "ALTAZ" && !isnan( slew_az ) && !isnan( accl_az ) && !isnan( dece_az ) && !isnan( tsettle_az ) &&
             !isnan( slew_el ) && !isnan( accl_el ) && !isnan( dece_el ) && !isnan( tsettle_el ) ) {
            antenna_ = make_shared<Antenna_AzEl_acceleration>( 0, 0, slew_az, accl_az, dece_az, tsettle_az, slew_el,
                                                               accl_el, dece_el, tsettle_el );
        } else if ( mount_ == "AZEL" && !isnan( slew_az ) && isnan( accl_az ) && isnan( dece_az ) &&
                    !isnan( tsettle_az ) && !isnan( slew_el ) && isnan( accl_el ) && isnan( dece_el ) &&
                    !isnan( tsettle_el ) ) {
            antenna_ = make_shared<Antenna_AzEl>( 0, 0, slew_az / 60, tsettle_az, slew_el / 60, tsettle_el );
        }

        if ( mount_ == "ALTAZ" && !isnan( az_min ) && !isnan( az_cmin ) && !isnan( az_cmax ) && !isnan( az_max ) &&
             !isnan( el_min ) && !isnan( el_max ) ) {
            cableWrap_ = make_shared<CableWrap_AzEl>( az_min, az_cmin, az_cmax, az_max, el_min, el_max );
        }

        if ( !hor_az.empty() && hor_az.size() == hor_el.size() ) {
            mask_ = make_shared<HorizonMask_step>( hor_az, hor_el );
        }
    }
}

StpParser::Tsys StpParser::parse_tsys( std::ifstream &file, const string &header ) {
    Tsys tsys;
    vector<string> splitVector;
    boost::split( splitVector, header, boost::is_space(), boost::token_compress_on );

    tsys.start = TimeSystem::string2ptime( splitVector[3] );
    tsys.end = TimeSystem::string2ptime( splitVector[4] );

    string line;
    string band;

    while ( getline( file, line ) ) {
        boost::trim( line );
        if ( line[0] == '#' ) {
            break;
        }
        vector<string> splitVector;
        boost::split( splitVector, line, boost::is_space(), boost::token_compress_on );

        if ( splitVector[0] == "TSYS_ELEVS:" ) {
            vector<double> elevs;
            for ( int i = 3; i < splitVector.size(); ++i ) {
                const auto &v = splitVector[i];
                elevs.push_back( boost::lexical_cast<double>( v ) * deg2rad );
            }
            tsys.elevations = elevs;
        }
        if ( splitVector[0] == "TSYS_FREQS:" ) {
            band = "";
            double start = util::freqency2wavelenth( boost::lexical_cast<double>( splitVector[3] ) * 1e9 );
            double end = util::freqency2wavelenth( boost::lexical_cast<double>( splitVector[4] ) * 1e9 );
            for ( const auto &any : ObservingMode::wavelengths ) {
                const string &wf_band = any.first;
                double wl = any.second;
                if ( wl <= start && wl >= end ) {
                    band = wf_band;
                    break;
                }
            }
        }
        if ( splitVector[0] == "TSYS_POLVALS:" ) {
            if ( band.empty() ) {
                continue;
            }
            string polstr = splitVector[3];
            If::Polarization pol = If::polarizationFromString( polstr );
            vector<double> polvals;
            for ( int i = 4; i < splitVector.size(); ++i ) {
                const auto &v = splitVector[i];
                polvals.push_back( boost::lexical_cast<double>( v ) );
            }
            //            pair<If::Polarization, vector<double>> tmp(pol, polvals);
            //            std::pair<std::string, std::pair<If::Polarization, std::vector<double>>> tmp2(band, tmp);
            tsys.polvals.emplace_back( make_tuple( band, pol, polvals ) );
        }
    }

    return tsys;
}
StpParser::Gain StpParser::parse_gain( std::ifstream &file, const string &header ) {
    Gain gain;
    vector<string> splitVector;
    boost::split( splitVector, header, boost::is_space(), boost::token_compress_on );

    gain.start = TimeSystem::string2ptime( splitVector[3] );
    gain.end = TimeSystem::string2ptime( splitVector[4] );

    string line;
    string band;

    while ( getline( file, line ) ) {
        boost::trim( line );
        if ( line[0] == '#' ) {
            break;
        }
        vector<string> splitVector;
        boost::split( splitVector, line, boost::is_space(), boost::token_compress_on );

        if ( splitVector[0] == "GAIN_ELEVS:" ) {
            vector<double> elevs;
            for ( int i = 3; i < splitVector.size(); ++i ) {
                const auto &v = splitVector[i];
                elevs.push_back( boost::lexical_cast<double>( v ) * deg2rad );
            }
            gain.elevations = elevs;
        }
        if ( splitVector[0] == "GAIN_FREQS:" ) {
            band = "";
            double start = util::freqency2wavelenth( boost::lexical_cast<double>( splitVector[3] ) * 1e9 );
            double end = util::freqency2wavelenth( boost::lexical_cast<double>( splitVector[4] ) * 1e9 );
            for ( const auto &any : ObservingMode::wavelengths ) {
                const string &wf_band = any.first;
                double wl = any.second;
                if ( wl <= start && wl >= end ) {
                    band = wf_band;
                    break;
                }
            }
        }
        if ( splitVector[0] == "GAIN_POLVALS:" ) {
            if ( band.empty() ) {
                continue;
            }
            string polstr = splitVector[3];
            If::Polarization pol = If::polarizationFromString( polstr );
            vector<double> polvals;
            for ( int i = 4; i < splitVector.size(); ++i ) {
                const auto &v = splitVector[i];
                polvals.push_back( boost::lexical_cast<double>( v ) );
            }
            //            pair<If::Polarization, vector<double>> tmp(pol, polvals);
            //            std::pair<std::string, std::pair<If::Polarization, std::vector<double>>> tmp2(band, tmp);
            gain.polvals.emplace_back( make_tuple( band, pol, polvals ) );
        }
    }

    return gain;
}


void StpParser::calcEquip() {
    unordered_map<string, vector<double>> el_;    ///< elevation angle
    unordered_map<string, vector<double>> SEFD_;  ///< corresponding SEFD value

    for ( const auto &band : ObservingMode::bands ) {
        auto tmp1 = extract_tsys( band );
        auto tmp2 = extract_gain( band );
        if ( !tmp1.is_initialized() || !tmp2.is_initialized() ) {
            continue;
        }
        const auto &tsys = tmp1->first;
        const auto &tsys_angle = tmp1->second;

        const auto &gain = tmp2->first;
        const auto &gain_angle = tmp2->second;

        set<double> angles;
        angles.insert( tsys_angle.begin(), tsys_angle.end() );
        angles.insert( gain_angle.begin(), gain_angle.end() );

        vector<double> vangles;
        vector<double> sefds;
        for ( double x : angles ) {
            double t_tsys = interp1( tsys_angle, tsys, x );
            double t_gain = interp1( gain_angle, gain, x );
            double sefd = t_tsys / t_gain;
            vangles.push_back( x );
            sefds.push_back( sefd );
        }
        el_[band] = vangles;
        SEFD_[band] = sefds;
    }

    if ( el_.size() == ObservingMode::bands.size() && SEFD_.size() == ObservingMode::bands.size() ) {
        equip_ = make_shared<Equipment_elTable>( el_, SEFD_ );
    }
}


boost::optional<std::pair<std::vector<double>, std::vector<double>>> StpParser::extract_tsys( const string &band ) {
    boost::optional<Tsys> latest = boost::none;

    // loop over all tsys entries for this station
    for ( const auto &tsys : tsyss_ ) {
        // see if this tsys entry contains the required band
        bool found = false;
        for ( const auto &any : tsys.polvals ) {
            if ( get<0>( any ) == band ) {
                found = true;
                break;
            }
        }

        // if band is not included, skip this tsys entry
        if ( !found ) {
            continue;
        }

        // check if this is the first tsys entry or if it is a more up-to-date version
        if ( !latest.is_initialized() || tsys.end > latest->end ) {
            latest = tsys;
        }
    }
    // now we should have latest tsys entries or none
    // skip band in case no tsys entries are available
    if ( !latest.is_initialized() ) {
        return boost::none;
    }

    // if tsys entry is available, loop entries and extract the ones for this band
    std::vector<std::tuple<std::string, If::Polarization, std::vector<double>>> entries;
    for ( const auto &any : latest->polvals ) {
        if ( get<0>( any ) == band ) {
            entries.push_back( any );
        }
    }

    // average values
    vector<double> vals( get<2>( entries[0] ).size(), 0.0 );
    for ( const auto &any : entries ) {
        const auto &val = get<2>( any );
        for ( int i = 0; i < val.size(); ++i ) {
            vals[i] += val[i];
        }
    }
    vector<double> tsys_vals( vals.size(), 0.0 );
    for ( int i = 0; i < tsys_vals.size(); ++i ) {
        tsys_vals[i] = vals[i] / entries.size();
    }
    vector<double> angles = latest->elevations;

    return make_pair( tsys_vals, angles );
}

boost::optional<std::pair<std::vector<double>, std::vector<double>>> StpParser::extract_gain( const string &band ) {
    boost::optional<Gain> latest = boost::none;

    // loop over all tsys entries for this station
    for ( const auto &gain : gains_ ) {
        // see if this gain entry contains the required band
        bool found = false;
        for ( const auto &any : gain.polvals ) {
            if ( get<0>( any ) == band ) {
                found = true;
                break;
            }
        }

        // if band is not included, skip this gain entry
        if ( !found ) {
            continue;
        }

        // check if this is the first gain entry or if it is a more up-to-date version
        if ( !latest.is_initialized() || gain.end > latest->end ) {
            latest = gain;
        }
    }
    // now we should have latest gain entries or none
    // skip band in case no gain entries are available
    if ( !latest.is_initialized() ) {
        return boost::none;
    }

    // if tsys entry is available, loop entries and extract the ones for this band
    std::vector<std::tuple<std::string, If::Polarization, std::vector<double>>> entries;
    for ( const auto &any : latest->polvals ) {
        if ( get<0>( any ) == band ) {
            entries.push_back( any );
        }
    }

    // average values
    vector<double> vals( get<2>( entries[0] ).size(), 0.0 );
    for ( const auto &any : entries ) {
        const auto &val = get<2>( any );
        for ( int i = 0; i < val.size(); ++i ) {
            vals[i] += val[i];
        }
    }
    vector<double> gain_vals( vals.size(), 0.0 );
    for ( int i = 0; i < gain_vals.size(); ++i ) {
        gain_vals[i] = vals[i] / entries.size();
    }
    vector<double> angles = latest->elevations;

    return make_pair( gain_vals, angles );
}

double StpParser::interp1( const std::vector<double> &x, const std::vector<double> &y, double x_ ) {
    if ( x_ <= x[0] ) {
        return y[0];
    }
    if ( x_ >= x.back() ) {
        return y.back();
    }

    unsigned int idx = 1;
    while ( x_ >= x[idx] ) {
        ++idx;
    }
    double dy = y[idx] - y[idx - 1];
    double dx = ( x_ - x[idx - 1] ) / ( x[idx] - x[idx - 1] );
    double y_ = y[idx - 1] + dy * dx;

    return y_;
}
