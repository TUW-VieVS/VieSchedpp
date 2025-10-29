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

/*
 * File:   Initializer.cpp
 * Author: mschartn
 *
 * Created on June 28, 2017, 12:38 PM
 */

#include "Initializer.h"


using namespace std;
using namespace VieVS;
unsigned long Initializer::nextId = 0;


Initializer::Initializer()
    : VieVS_Object( nextId++ ){

      };


Initializer::Initializer( const std::string &path ) : VieVS_Object( nextId++ ) {
    ifstream is( path );

#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "constructing initializer for:" << path;
#endif

    boost::property_tree::read_xml( is, xml_, boost::property_tree::xml_parser::trim_whitespace );
    double maxDistCorrestpondingTelescopes = xml_.get( "VieSchedpp.skyCoverage.maxTwinTelecopeDistance", 0.0 );
    network_.setMaxDistBetweenCorrespondingTelescopes( maxDistCorrestpondingTelescopes );
}


Initializer::Initializer( const boost::property_tree::ptree &xml ) : VieVS_Object( nextId++ ), xml_{ xml } {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "constructing initializer " << this->getId();
#endif

    double maxDistCorrestpondingTelescopes = xml_.get( "VieSchedpp.skyCoverage.maxTwinTelecopeDistance", 0.0 );
    network_.setMaxDistBetweenCorrespondingTelescopes( maxDistCorrestpondingTelescopes );
}


void Initializer::precalcSubnettingSrcIds() noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "calculating subnetting source combinations";
#endif
    const auto &sources = sourceList_.getQuasars();
    unsigned long nquasars = sources.size();
    unsigned long nsrc = sourceList_.getNSrc();
    vector<vector<unsigned long>> subnettingSrcIds( nsrc );
    for ( int i = 0; i < nquasars; ++i ) {
        for ( int j = i + 1; j < nquasars; ++j ) {
            double tmp = sin( sources[i]->getDe() ) * sin( sources[j]->getDe() ) +
                         cos( sources[i]->getDe() ) * cos( sources[j]->getDe() ) *
                             cos( sources[i]->getRa() - sources[j]->getRa() );
            double dist = acos( tmp );

            if ( dist > parameters_.subnettingMinAngle ) {
#ifdef VIESCHEDPP_LOG
                if ( Flags::logTrace )
                    BOOST_LOG_TRIVIAL( trace ) << "possible subnetting between source " << i << " and " << j;
#endif
                subnettingSrcIds.at( i ).push_back( j );
            }
        }
    }

    preCalculated_.subnettingSrcIds = subnettingSrcIds;
}


void Initializer::createStations( const SkdCatalogReader &reader, ofstream &of ) noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "creating stations";
#endif

    const map<string, vector<string>> &antennaCatalog = reader.getAntennaCatalog();
    const map<string, vector<string>> &positionCatalog = reader.getPositionCatalog();
    const map<string, vector<string>> &equipCatalog = reader.getEquipCatalog();
    const map<string, vector<string>> &maskCatalog = reader.getMaskCatalog();

    of << "Create Stations:\n";
    unsigned long nant;
    int counter = 0;

    const vector<string> &selectedStations = reader.getStaNames();

    if ( !selectedStations.empty() ) {
        nant = selectedStations.size();
    } else {
        nant = antennaCatalog.size();
    }

    int created = 0;
    // loop through all antennas in antennaCatalog
    for ( auto &any : antennaCatalog ) {
        counter++;
        // get antenna name
        string name = any.first;

#ifdef VIESCHEDPP_LOG
        if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "try to create station " << name;
#endif

        // check if station is in PARA.selectedStations or if PARA.selectedStations is not empty
        if ( !selectedStations.empty() &&
             ( find( selectedStations.begin(), selectedStations.end(), name ) == selectedStations.end() ) ) {
            continue;
        }

        // look if vector in antenna.cat is long enough. Otherwise not all information is available!
        if ( any.second.size() < 15 ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( error ) << "station " << name << " antenna.cat: not enough elements in catalog";
#else
            cout << "[error] station " << name << " antenna.cat: not enough elements in catalog";
#endif

            of << "*** ERROR: " << any.first << ": antenna.cat to small ***\n";
            continue;
        }

        // convert all IDs to upper case for case insensitivity
        const string &id_PO = reader.positionKey( name );
        const string &id_EQ = reader.equipKey( name );
        const string &id_MS = reader.maskKey( name );
        const string &tlc = reader.getTwoLetterCode().at( name );

        // check if corresponding position and equip CATALOG exists.
        if ( positionCatalog.find( id_PO ) == positionCatalog.end() ) {
            of << "*** ERROR: creating station " << name << ": position CATALOG not found ***\n";
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( error ) << "station " << name << " position.cat: not found";
#else
            cout << "[error] station " << name << " position.cat: not found";
#endif
            continue;
        }
        if ( equipCatalog.find( id_EQ ) == equipCatalog.end() ) {
            of << "*** ERROR: creating station " << name << ": equip CATALOG not found ***\n";
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( error ) << "station " << name << " equip.cat: not found";
#else
            cout << "[error] station " << name << " equip.cat: not found";
#endif
            continue;
        }

        // convert all items from antenna.cat
        string type = any.second[2];
        double offset, rate1, con1, axis1_low, axis1_up, rate2, con2, axis2_low, axis2_up, diam;
        try {
            offset = boost::lexical_cast<double>( any.second.at( 3 ) );
            rate1 = boost::lexical_cast<double>( any.second.at( 4 ) );
            con1 = boost::lexical_cast<double>( any.second.at( 5 ) );
            axis1_low = boost::lexical_cast<double>( any.second.at( 6 ) );
            axis1_up = boost::lexical_cast<double>( any.second.at( 7 ) );
            rate2 = boost::lexical_cast<double>( any.second.at( 8 ) );
            con2 = boost::lexical_cast<double>( any.second.at( 9 ) );
            axis2_low = boost::lexical_cast<double>( any.second.at( 10 ) );
            axis2_up = boost::lexical_cast<double>( any.second.at( 11 ) );
            diam = boost::lexical_cast<double>( any.second.at( 12 ) );
        } catch ( const std::exception &e ) {
            of << "*** ERROR: creating station " << name << ": " << e.what() << " ***\n";
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( error ) << "station " << name << " antenna.cat: cannot cast text to number";
#else
            cout << "[error] station " << name << " antenna.cat: cannot cast text to number";
#endif
            continue;
        }
        double acc1 = 0;
        double acc2 = 0;
        try {
            if ( any.second.size() == 18 ) {
                acc1 = boost::lexical_cast<double>( any.second.at( 16 ) );
                acc2 = boost::lexical_cast<double>( any.second.at( 17 ) );
            }
        } catch ( const std::exception &e ) {
            of << "*** WARNING: creating station " << name << " cannot cast acceleration to number \n";
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( warning ) << "station " << name << " antenna.cat: cannot cast acceleration to number";
#else
            cout << "[warning] station " << name << " antenna.cat: cannot cast acceleration to number";
#endif
        }


        // check if position.cat is long enough. Otherwise not all information is available.
        vector<string> po_cat = positionCatalog.at( id_PO );
        if ( po_cat.size() < 5 ) {
            of << "*** ERROR: creating station " << name << ": " << any.first << ": positon.cat to small ***\n";
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( error ) << "station " << name << " position.cat: not enough elements in catalog";
#else
            cout << "[error] station " << name << " position.cat: not enough elements in catalog";
#endif
            continue;
        }

        // convert all items from position.cat
        double x, y, z;
        try {
            x = boost::lexical_cast<double>( po_cat.at( 2 ) );
            y = boost::lexical_cast<double>( po_cat.at( 3 ) );
            z = boost::lexical_cast<double>( po_cat.at( 4 ) );
        } catch ( const std::exception &e ) {
            of << "*** ERROR: creating station " << name << ": " << e.what() << " ***\n";
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( error ) << "station " << name << " position.cat: cannot cast text to number";
#else
            cout << "[error] station " << name << " position.cat: cannot cast text to number";
#endif
            continue;
        }

        // check if equip.cat is long enough. Otherwise not all information is available.
        vector<string> eq_cat = equipCatalog.at( id_EQ );
        if ( eq_cat.size() < 9 ) {
            of << "*** ERROR: creating station " << name << ": " << any.first << ": equip.cat to small ***\n";
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( error ) << "station " << name << " equip.cat: not enough elements in catalog";
#else
            cout << "[error] station " << name << " equip.cat: not enough elements in catalog";
#endif
            //            continue;
        }

        const auto &bands = ObservingMode::bands;
        int nbands = bands.size();

        vector<int> band_idxs;
        vector<string> band_names;
        for ( int i = 5; i < eq_cat.size() - 2; i += 2 ) {
            string t = eq_cat[i];
            // if SEFD of "t" is already found --> skip
            if ( find( band_names.begin(), band_names.end(), t ) != band_names.end() ) {
                continue;
            }

            // check if "t" is not a number...
            bool isnumber;
            try {
                double dummy = boost::lexical_cast<double>( t );
                isnumber = true;
            } catch ( boost::bad_lexical_cast const &e ) {
                isnumber = false;
            }
            if ( !isnumber ) {
                band_idxs.push_back( i );
                band_names.push_back( t );
            }
        }

        unordered_map<std::string, double> SEFDs;
        // convert all items from equip.cat
        unordered_map<std::string, double> SEFD_found;
        for ( int i_band = 0; i_band < band_idxs.size(); ++i_band ) {
            int band_idx = band_idxs[i_band];
            string band_name = band_names[i_band];
            try {
                SEFD_found[band_name] = boost::lexical_cast<double>( eq_cat.at( band_idx + 1 ) );
            } catch ( const std::exception &e ) {
                of << "*** ERROR: creating station " << name << ": " << e.what() << "\n";
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( error ) << "station " << name << " equip.cat: cannot cast text to number";
#else
                cout << "[error] station " << name << " equip.cat: cannot cast text to number";
#endif
                //                continue;
            }
        }
        bool everythingOkWithBands = true;
        for ( const auto &bandName : ObservingMode::bands ) {
            if ( SEFD_found.find( bandName ) != SEFD_found.end() ) {
                SEFDs[bandName] = SEFD_found[bandName];
            } else {
                if ( ObservingMode::stationProperty[bandName] == ObservingMode::Property::required ) {
                    everythingOkWithBands = false;
                } else if ( ObservingMode::stationBackup[bandName] == ObservingMode::Backup::value ) {
                    SEFDs[bandName] = ObservingMode::stationBackupValue[bandName];
                }
            }
        }
        if ( !everythingOkWithBands ) {
            of << "*** WARNING: creating station " << name
               << ": required SEFD information missing in sked catalogs!;\n";
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( warning ) << "station " << name << " required SEFD information missing in sked catalogs";
#else
            cout << "[warning] station " << name << " required SEFD information missing";
#endif
        }

        if ( SEFDs.size() != ObservingMode::bands.size() ) {
            if ( SEFDs.empty() ) {
                of << "*** WARNING: creating station " << name
                   << ": no SEFD information found to calculate backup value!;\n";
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( warning )
                    << "station " << name << " no SEFD information found to calculate backup value";
#else
                cout << "[warning] station " << name << " no SEFD information found to calculate backup value";
#endif
            }
            double max = 0;
            double min = std::numeric_limits<double>::max();
            for ( const auto &any2 : SEFDs ) {
                if ( any2.second < min ) {
                    min = any2.second;
                }
                if ( any2.second > max ) {
                    max = any2.second;
                }
            }
            for ( const auto &bandName : ObservingMode::bands ) {
                if ( SEFDs.find( bandName ) == SEFDs.end() ) {
                    if ( ObservingMode::stationBackup[bandName] == ObservingMode::Backup::minValueTimes ) {
                        SEFDs[bandName] = min * ObservingMode::stationBackupValue[bandName];
                    }
                    if ( ObservingMode::stationBackup[bandName] == ObservingMode::Backup::maxValueTimes ) {
                        SEFDs[bandName] = max * ObservingMode::stationBackupValue[bandName];
                    }
                }
            }
        }

        bool elSEFD = false;
        unordered_map<std::string, double> SEFD_y;
        unordered_map<std::string, double> SEFD_c0;
        unordered_map<std::string, double> SEFD_c1;
        int nBands = ObservingMode::bands.size();

        if ( eq_cat.size() >= 16 ) {
            for ( int idx = 5 + 2 * nBands; idx < eq_cat.size(); idx += 4 ) {
                elSEFD = true;
                if ( bands.find( eq_cat[idx] ) != bands.end() ) {
                    try {
                        string band = eq_cat[idx];
                        auto elSEFD_y = boost::lexical_cast<double>( eq_cat.at( idx + 1 ) );
                        auto elSEFD_c0 = boost::lexical_cast<double>( eq_cat.at( idx + 2 ) );
                        auto elSEFD_c1 = boost::lexical_cast<double>( eq_cat.at( idx + 3 ) );

                        SEFD_y[band] = elSEFD_y;
                        SEFD_c0[band] = elSEFD_c0;
                        SEFD_c1[band] = elSEFD_c1;

                    } catch ( const std::exception &e ) {
                        of << "*** ERROR: creating station " << name
                           << ": elevation dependent SEFD value not understood and therefore ignored!!;\n";
#ifdef VIESCHEDPP_LOG
                        BOOST_LOG_TRIVIAL( warning )
                            << "station " << name
                            << " elevation dependent SEFD value not understood and therefore ignored";
#else
                        cout << "[warning] station " << name
                             << " elevation dependent SEFD value not understood and therefore ignored\n";
#endif
                        elSEFD = false;
                    }
                }
            }

            //            if ( bands.find( eq_cat[9] ) != bands.end() ) {
            //                elSEFD = true;
            //                try {
            //                    string band = eq_cat[9];
            //                    auto elSEFD_y = boost::lexical_cast<double>( eq_cat.at( 10 ) );
            //                    auto elSEFD_c0 = boost::lexical_cast<double>( eq_cat.at( 11 ) );
            //                    auto elSEFD_c1 = boost::lexical_cast<double>( eq_cat.at( 12 ) );
            //
            //                    SEFD_y[band] = elSEFD_y;
            //                    SEFD_c0[band] = elSEFD_c0;
            //                    SEFD_c1[band] = elSEFD_c1;
            //
            //                } catch ( const std::exception &e ) {
            //                    of << "*** ERROR: creating station " << name
            //                       << ": elevation dependent SEFD value not understood and therefore ignored!!;\n";
            //#ifdef VIESCHEDPP_LOG
            //                    BOOST_LOG_TRIVIAL( warning )
            //                        << "station " << name << " elevation dependent SEFD value not understood and
            //                        therefore ignored";
            //#else
            //                    cout << "[warning] station " << name
            //                         << " elevation dependent SEFD value not understood and therefore ignored";
            //#endif
            //                    elSEFD = false;
            //                }
            //            }
            //            if ( bands.find( eq_cat[13] ) != bands.end() ) {
            //                try {
            //                    string band = eq_cat[13];
            //                    auto elSEFD_y = boost::lexical_cast<double>( eq_cat.at( 14 ) );
            //                    auto elSEFD_c0 = boost::lexical_cast<double>( eq_cat.at( 15 ) );
            //                    auto elSEFD_c1 = boost::lexical_cast<double>( eq_cat.at( 16 ) );
            //
            //                    SEFD_y[band] = elSEFD_y;
            //                    SEFD_c0[band] = elSEFD_c0;
            //                    SEFD_c1[band] = elSEFD_c1;
            //
            //                } catch ( const std::exception &e ) {
            //                    of << "*** ERROR: creating station " << name
            //                       << ": elevation dependent SEFD value not understood and therefore ignored!!;\n";
            //#ifdef VIESCHEDPP_LOG
            //                    BOOST_LOG_TRIVIAL( warning )
            //                        << "station " << name << " elevation dependent SEFD value not understood and
            //                        therefore ignored";
            //#else
            //                    cout << "[warning] station " << name
            //                         << " elevation dependent SEFD value not understood and therefore ignored";
            //#endif
            //                    elSEFD = false;
            //                }
            //            }
        }

        // check if an horizontal mask exists
        vector<double> hmask;
        if ( maskCatalog.find( id_MS ) != maskCatalog.end() ) {
            vector<string> mask_cat = maskCatalog.at( id_MS );

            // loop through all element and convert them
            for ( size_t i = 3; i < mask_cat.size(); ++i ) {
                try {
                    hmask.push_back( boost::lexical_cast<double>( mask_cat.at( i ) ) );
                } catch ( const std::exception &e ) {
                    of << "*** ERROR: creating station " << name << ": mask catalog entry " << mask_cat.at( i )
                       << " not understood \n";
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( error ) << "station " << name << " mask.cat: cannot cast text to number";
#else
                    cout << "[error] station " << name << " mask.cat: cannot cast text to number";
#endif
                    continue;
                }
            }
        } else {
            if ( id_MS != "--" ) {
                of << "*** WARNING: creating station " << name << ": mask CATALOG not found ***\n";
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( warning ) << "station " << name << " mask.cat: not found";
#else
                cout << "[warning] station " << name << " mask.cat: not found";
#endif
            }
        }

        std::vector<double> hmask_az;
        std::vector<double> hmask_el;
        for ( unsigned long i = 0; i < hmask.size(); ++i ) {
            if ( i % 2 == 0 ) {
                hmask_az.push_back( hmask.at( i ) * deg2rad );
            } else {
                hmask_el.push_back( hmask.at( i ) * deg2rad );
            }
        }

        if ( !hmask.empty() && hmask_az.back() != twopi ) {
            hmask_az.push_back( twopi );
            hmask_el.push_back( *hmask_el.end() );
        }

        shared_ptr<AbstractAntenna> antenna;
        shared_ptr<AbstractCableWrap> cableWrap;
        if ( name == "GGAO12M" ) {
            antenna = make_shared<Antenna_GGAO>( offset, diam, rate1, con1, rate2, con2 );
            cableWrap = make_shared<CableWrap_AzEl>( axis1_low, axis1_up, axis2_low, axis2_up );
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( info ) << "special slew model for GGAO12M to avoid slewing through radar mask";
#endif
        } else if ( name == "ONSA13SW" || name == "ONSA13NE" ) {
            antenna = make_shared<Antenna_ONSALA_VGOS>( offset, diam, rate1, con1, rate2, con2 );
            cableWrap = make_shared<CableWrap_AzEl>( -90.0, 450.0, axis2_low, axis2_up );
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( info ) << "special slew model for " << name
                                      << " with slower slew rate close to cable wrap limits";
            BOOST_LOG_TRIVIAL( info ) << "force cable wrap of " << name << " to -90:450";
#endif
        } else if ( acc1 > 0 && acc2 > 0 ) {
            rate1 /= 60;
            rate2 /= 60;
            antenna = make_shared<Antenna_AzEl_acceleration>( offset, diam, rate1, acc1, acc1, con1, rate2, acc2, acc2,
                                                              con2 );
            cableWrap = make_shared<CableWrap_AzEl>( axis1_low, axis1_up, axis2_low, axis2_up );
        } else if ( ( name.length() >= 4 && name.substr( name.length() - 4 ) == "VLBA" ) || name == "PIETOWN" ) {
            rate1 /= 60;
            rate2 /= 60;
            antenna = make_shared<Antenna_AzEl_acceleration>( offset, diam, rate1, 0.750, 0.750, con1, rate2, 0.250,
                                                              0.250, con2 );
            cableWrap = make_shared<CableWrap_AzEl>( axis1_low, axis1_up, axis2_low, axis2_up );
        } else if ( type == "AZEL" ) {
            antenna = make_shared<Antenna_AzEl>( offset, diam, rate1, con1, rate2, con2 );
            cableWrap = make_shared<CableWrap_AzEl>( axis1_low, axis1_up, axis2_low, axis2_up );
        } else if ( type == "HADC" ) {
            antenna = make_shared<Antenna_HaDc>( offset, diam, rate1, con1, rate2, con2 );
            cableWrap = make_shared<CableWrap_HaDc>( axis1_low, axis1_up, axis2_low, axis2_up );
        } else if ( type == "XYEW" ) {
            antenna = make_shared<Antenna_XYew>( offset, diam, rate1, con1, rate2, con2 );
            cableWrap = make_shared<CableWrap_XYew>( axis1_low, axis1_up, axis2_low, axis2_up );
        }

        shared_ptr<AbstractEquipment> equipment = nullptr;
        boost::optional<boost::property_tree::ptree &> sefdAdjustment_tree =
            xml_.get_child_optional( "VieSchedpp.station.sefdAdjustment" );
        if ( sefdAdjustment_tree.is_initialized() ) {
            for ( const auto &node : *sefdAdjustment_tree ) {
                if ( node.first == "fixed" && node.second.get<std::string>( "<xmlattr>.member" ) == name ) {
                    unordered_map<std::string, double> fixed_SEFDs;
                    string summary = "";
                    for ( const auto &any2 : node.second ) {
                        if ( any2.first != "band" ) {
                            continue;
                        }
                        string band = any2.second.get<std::string>( "<xmlattr>.name" );
                        auto val = any2.second.get_value<double>();
                        fixed_SEFDs[band] = val;
                        summary = summary.append( ( boost::format( "%s %.0f " ) % band % val ).str() );
                    }
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( info ) << name << " replace SEFD values: " << summary;
#endif
                    equipment = make_shared<Equipment_constant>( fixed_SEFDs );
                }
                if ( node.first == "factor" && node.second.get<std::string>( "<xmlattr>.member" ) == name ) {
                    string summary = "";
                    for ( const auto &any2 : node.second ) {
                        if ( any2.first != "band" ) {
                            continue;
                        }
                        string band = any2.second.get<std::string>( "<xmlattr>.name" );
                        auto val = any2.second.get_value<double>();
                        SEFDs[band] *= val;
                        summary = summary.append( ( boost::format( "%s %.0f " ) % band % val ).str() );
                    }
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( info ) << name << " replace SEFD values: " << summary;
#endif
                }
            }
        }
        if ( equipment == nullptr ) {
            if ( elSEFD ) {
                equipment = make_shared<Equipment_elModel>( SEFDs, SEFD_y, SEFD_c0, SEFD_c1 );
            } else {
                equipment = make_shared<Equipment_constant>( SEFDs );
            }
        }

        auto position = make_shared<Position>( x, y, z, name, "sked_catalog" );

        shared_ptr<AbstractHorizonMask> horizonMask;
        if ( !hmask_az.empty() && hmask_az.size() == hmask_el.size() ) {
            horizonMask = make_shared<HorizonMask_line>( hmask_az, hmask_el );
        } else if ( !hmask_az.empty() ) {
            horizonMask = make_shared<HorizonMask_step>( hmask_az, hmask_el );
        }

        string stp_dir = xml_.get<string>( "VieSchedpp.catalogs.stp_dir", "" );
        if ( !stp_dir.empty() ) {
            string fname = stp_dir;
            if ( stp_dir.find( '/' ) != string::npos ) {
                if ( *stp_dir.end() == '/' ) {
                    fname.append( boost::to_lower_copy( name ) ).append( ".stp" );
                } else {
                    fname.append( "/" ).append( boost::to_lower_copy( name ) ).append( ".stp" );
                }
            } else {
                if ( *stp_dir.end() == '\\' ) {
                    fname.append( boost::to_lower_copy( name ) ).append( ".stp" );
                } else {
                    fname.append( "\\" ).append( boost::to_lower_copy( name ) ).append( ".stp" );
                }
            }
            StpParser stp( fname );
            if ( stp.exists() ) {
                stp.parse();
                if ( stp.hasValidAntenna() ) {
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( info ) << name << " replace slew model with .stp model";
#endif
                    antenna = stp.getAntenna();
                }
                if ( stp.hasValidCableWrap() ) {
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( info ) << name << " replace cable wrap model with .stp model";
#endif
                    cableWrap = stp.getCableWrap();
                }
                if ( stp.hasValidEquipment() ) {
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( info ) << name << " replace SEFD model with .stp model";
#endif
                    equipment = stp.getEquip();
                }
                if ( stp.hasValidPosition() ) {
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( info ) << name << " replace position with .stp entries";
#endif
                    position = stp.getPosition();
                }
                if ( stp.hasValidHorizonMask() ) {
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( info ) << name << " replace horizon mask with .stp model";
#endif
                    horizonMask = stp.getHorizionMask();
                }
            }
        }

        network_.addStation(
            Station( name, tlc, antenna, cableWrap, position, equipment, horizonMask, sourceList_.getNSrc() ) );

        string occupation_code = po_cat.at( 5 );
        string record_transport_type = eq_cat.at( eq_cat.size() - 1 );
        string electronics_rack_type = eq_cat.at( eq_cat.size() - 2 );
        string recording_system_ID = any.second.at( 14 );

        network_.refStation( created ).addAdditionalParameters( occupation_code, record_transport_type,
                                                                electronics_rack_type, recording_system_ID );
        created++;

        of << boost::format( "  %-8s (%s) added\n" ) % name % tlc;
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug )
            BOOST_LOG_TRIVIAL( debug ) << "station " << name << " successfully created "
                                       << network_.getStation( name ).printId();
#endif
    }
    of << "Finished! " << created << " of " << nant << " stations created\n";
    of << "          " << network_.getBaselines().size() << " of " << nant * ( nant - 1 ) / 2
       << " baselines created\n\n";
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "successfully created " << created << " of " << nant << " stations";
    BOOST_LOG_TRIVIAL( info ) << "successfully created " << network_.getBaselines().size() << " of "
                              << nant * ( nant - 1 ) / 2 << " baselines";
#else
    cout << "[info] successfully created " << created << " of " << nant << " stations";
    cout << "[info] successfully created " << network_.getBaselines().size() << " of " << nant * ( nant - 1 ) / 2
         << " baselines";
#endif
    network_.stationSummary( of );
}


void Initializer::createSources( const SkdCatalogReader &reader, std::ofstream &of ) noexcept {
    const map<string, vector<string>> &sourceCatalog = reader.getSourceCatalog();
    const map<string, vector<string>> &fluxCatalog = reader.getFluxCatalog();

    int counter = 0;
    unsigned long nsrc = sourceCatalog.size();
    int created = 0;
    of << "Create Sources:\n";
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "creating sources";
#endif

    vector<string> src_created;
    vector<string> src_ignored;
    vector<string> src_failed;

    bool selectSources = false;
    vector<string> sel_sources;
    const auto &ptree_useSources = xml_.get_child_optional( "VieSchedpp.general.onlyUseListedSources" );
    if ( ptree_useSources.is_initialized() ) {
        selectSources = true;
        auto it = ptree_useSources->begin();
        while ( it != ptree_useSources->end() ) {
            auto item = it->second.data();
            sel_sources.push_back( item );
            ++it;
        }
    }

    vector<string> ignore_sources;
    const auto &ptree_ignoreSources = xml_.get_child_optional( "VieSchedpp.general.ignoreListedSources" );
    if ( ptree_ignoreSources.is_initialized() ) {
        auto it = ptree_ignoreSources->begin();
        while ( it != ptree_ignoreSources->end() ) {
            auto item = it->second.data();
            ignore_sources.push_back( item );
            ++it;
        }
    }

    bool fluxNecessary = all_of( ObservingMode::sourceProperty.begin(), ObservingMode::sourceProperty.end(),
                                 []( auto item ) { return item.second == ObservingMode::Property::required; } );

    for ( auto any : sourceCatalog ) {
        counter++;
        string name = any.first;
#ifdef VIESCHEDPP_LOG
        if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "try to create source " << name;
#endif

        if ( any.second.size() < 8 ) {
            of << "*** ERROR: " << any.first << ": source.cat to small ***\n";
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( warning ) << "source " << name << " source.cat: not enough elements in catalog";
#else
            cout << "[warning] source " << name << " source.cat: not enough elements in catalog";
#endif
            src_failed.push_back( name );
            continue;
        }
        string commonname = any.second.at( 1 );
        if ( commonname == "$" ) {
            commonname = "";
        }

        if ( selectSources ) {
            if ( find( sel_sources.begin(), sel_sources.end(), name ) == sel_sources.end() &&
                 find( sel_sources.begin(), sel_sources.end(), commonname ) == sel_sources.end() ) {
                src_ignored.push_back( name );
#ifdef VIESCHEDPP_LOG
                if ( Flags::logTrace )
                    BOOST_LOG_TRIVIAL( trace ) << "source " << name << " not in list of allowed sources";
#endif
                continue;
            }
        } else {
            if ( find( ignore_sources.begin(), ignore_sources.end(), name ) != ignore_sources.end() ||
                 find( ignore_sources.begin(), ignore_sources.end(), commonname ) != ignore_sources.end() ) {
                src_ignored.push_back( name );
#ifdef VIESCHEDPP_LOG
                if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "source " << name << " in list of ignored sources";
#endif
                continue;
            }
        }

        double ra_h, ra_m, ra_s, de_deg, de_m, de_s;
        char sign = any.second.at( 5 ).at( 0 );
        try {
            ra_h = boost::lexical_cast<double>( any.second.at( 2 ) );
            ra_m = boost::lexical_cast<double>( any.second.at( 3 ) );
            ra_s = boost::lexical_cast<double>( any.second.at( 4 ) );
            de_deg = boost::lexical_cast<double>( any.second.at( 5 ) );
            de_m = boost::lexical_cast<double>( any.second.at( 6 ) );
            de_s = boost::lexical_cast<double>( any.second.at( 7 ) );
        } catch ( const std::exception &e ) {
            src_failed.push_back( name );
            of << "*** ERROR: reading right ascension and declination for " << name << " ***\n";
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( warning ) << "source " << name << " source.cat: cannot cast text to number";
#else
            cout << "[warning] source " << name << " source.cat: cannot cast text to number";
#endif
            continue;
        }
        double ra = 15 * ( ra_h + ra_m / 60 + ra_s / 3600 );
        double de = ( abs( de_deg ) + de_m / 60 + de_s / 3600 );
        if ( sign == '-' ) {
            de = -1 * de;
        }

        auto flux = generateFluxObject( name, commonname, fluxCatalog, fluxNecessary, of );
        bool allFluxes = true;
        for(const auto &band: ObservingMode::bands){
            if (flux.empty()){
                allFluxes = false;
            }
            if (flux.find(band) == flux.end() && ObservingMode::sourceBackup[band] != ObservingMode::Backup::internalModel){
                allFluxes = false;
            }
        }

        if ( allFluxes ) {
            string name1, name2;
            if ( commonname.empty() ) {
                name1 = name;
                name2 = "";
            } else {
                name1 = commonname;
                name2 = name;
            }
            int nflux = flux.size();
            const auto &it = find( any.second.begin(), any.second.end(), "jet_angle" );
            if(it != any.second.end() ){
                try {
                    auto jet_angle = boost::lexical_cast<double>(*(it+1));
                    auto jet_angle_std = boost::lexical_cast<double>(*(it+2));

                    auto src = make_shared<Quasar>( name1, name2, ra, de, flux, jet_angle, jet_angle_std );
                    sourceList_.addQuasar( src );

                } catch ( const std::exception &e ) {
                    auto src = make_shared<Quasar>( name1, name2, ra, de, flux );
                    sourceList_.addQuasar( src );
                }
            } else {
                auto src = make_shared<Quasar>( name1, name2, ra, de, flux );
                sourceList_.addQuasar( src );
            }
            created++;
            src_created.push_back( name );
            of << boost::format( "successfully added: %s with %d flux entries\n" ) % name % nflux;

#ifdef VIESCHEDPP_LOG
            if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "quasar " << name << " successfully created ";
#endif
        } else {
            of << boost::format( "failed to add: %s \n" ) % name;
            src_failed.push_back( name );
        }
    }
    of << "Finished! " << created << " of " << nsrc << " sources created\n\n";
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "successfully created " << created << " of " << nsrc << " sources";
#else
    cout << "[info] successfully created " << created << " of " << nsrc << " sources";
#endif

    util::outputObjectList( "created sources", src_created, of );
    util::outputObjectList( "ignored sources", src_ignored, of );
    util::outputObjectList( "failed to create source", src_failed, of );
}

void Initializer::createSatellites( const SkdCatalogReader &reader, ofstream &of ) noexcept {
    int counter = 0;
    int created = 0;
    int count_tle = 0;
    of << "Create Satellites:\n";
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "creating satellites";
#endif

    bool fluxNecessary = all_of( ObservingMode::sourceProperty.begin(), ObservingMode::sourceProperty.end(),
                                 []( auto item ) { return item.second == ObservingMode::Property::required; } );

    vector<string> satellites;
    vector<string> src_created;
    vector<string> src_ignored;
    vector<string> src_fluxInformationNotFound;
    vector<string> src_failed;
    const map<string, vector<string>> &fluxCatalog = reader.getFluxCatalog();

    const auto &sat_xml_list_o = xml_.get_child_optional( "VieSchedpp.general.satellites" );
    if ( sat_xml_list_o.is_initialized() ) {
        const auto &sat_xml = *sat_xml_list_o;
        for ( const auto &any : sat_xml ) {
            string name = any.second.data();
            util::simplify_inline( name );
            name = boost::replace_all_copy( name, " ", "_" );
            satellites.push_back( name );
        }
    } else {
        return;
    }

    const auto &sat_xml_o = xml_.get_optional<string>( "VieSchedpp.catalogs.satellite" );
    if ( sat_xml_o.is_initialized() ) {
        const auto &sat_xml = *sat_xml_o;
        ifstream fid( sat_xml );
        string line;

        if ( !fid.is_open() ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( fatal ) << "unable to open " << sat_xml << " file";
#else
            cout << "[fatal] unable to open " << sat_xml << " file\n";
#endif
            terminate();
        } else {
            string header;
            string line1;
            string line2;
            int flag = 0;
            while ( getline( fid, line ) ) {
                line = boost::trim_copy( line );
                switch ( flag ) {
                    case 0: {
                        header = line;
                        ++flag;
                        break;
                    }
                    case 1: {
                        line1 = line;
                        ++flag;
                        break;
                    }
                    case 2: {
                        line2 = line;
                        ++flag;
                        break;
                    }
                    default: {
                        break;
                    }
                }

                if ( flag == 3 ) {
                    flag = 0;
                    string processedName = header;
                    util::simplify_inline( processedName );
                    processedName = boost::replace_all_copy( processedName, " ", "_" );

                    if ( find( satellites.begin(), satellites.end(), processedName ) == satellites.end() ) {
                        of << "ignoring satellite " << header << endl;
                        src_ignored.push_back( header );
                        continue;
                    }
                    ++count_tle;
                    // check if satellite was already generated
                    bool existed = false;
                    for ( auto &any : sourceList_.refSatellites()){
                        if ( any->hasName( header ) ){
                            any->addpSGP4Data(header, line1, line2);
                            existed = true;
                        }
                    }
                    if (existed){
                        continue;
                    }
                    ++counter;

                    bool foundName = fluxCatalog.find( header ) != fluxCatalog.end();

                    if ( !foundName && fluxNecessary ) {
                        src_fluxInformationNotFound.push_back( header );
#ifdef VIESCHEDPP_LOG
                        BOOST_LOG_TRIVIAL( warning ) << "satellite " << header << " flux.cat: source not found";
#else
                        cout << "[warning] satellite " << header << " flux.cat: source not found\n";
#endif
                        continue;
                    }
                    string commonname;
                    //                    auto flux = generateFluxObject( header, commonname, fluxCatalog,
                    //                    fluxNecessary, of );
                    unordered_map<string, unique_ptr<AbstractFlux>> flux;
                    for ( string band : ObservingMode::bands ) {
                        double wavelength = ObservingMode::wavelengths[band];
                        double satellite_flux = xml_.get( "VieSchedpp.satellite.satellite_flux", 10. );
                        double satellite_db_loss = xml_.get( "VieSchedpp.satellite.satellite_db_loss", 10. );
                        flux[band] = make_unique<Flux_satellite>( wavelength, satellite_flux, satellite_db_loss );
                    }
                    if ( flux.size() == ObservingMode::bands.size() ) {
                        auto src = make_shared<Satellite>( header, line1, line2, flux );
                        sourceList_.addSatellite( src );
                        created++;
                        src_created.push_back( header );
#ifdef VIESCHEDPP_LOG
                        if ( Flags::logDebug )
                            BOOST_LOG_TRIVIAL( debug ) << "satellite " << header << " successfully created ";
#endif
                    } else {
                        src_failed.push_back( header );
                    }
                }
            }
            of << "Finished! " << created << " of " << counter << " satellites created ("<< count_tle<<" TLE entries)\n\n";
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( info ) << "successfully created " << created << " of " << counter << " satellites ("
                                      << count_tle<<" TLE entries)";
#else
            cout << "[info] successfully created " << created << " of " << counter << " satellites";
#endif

            util::outputObjectList( "created satellites", src_created, of );
            util::outputObjectList( "ignored satellites", src_ignored, of );
            util::outputObjectList( "failed to create satellites", src_failed, of );
        }

    } else {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( error ) << "satellite TLE file not defined";
#else
        cout << "[error] satellite TLE file not defined\n";
#endif
        return;
    }
}

void Initializer::createSatellitesToAvoid( ofstream &of ) noexcept {
    int counter = 0;
    int created = 0;
    int count_tle = 0;

    vector<string> satellites;
    vector<string> src_created;
    vector<string> src_failed;
    map<string, double> deltaTimes;
    boost::posix_time::ptime sessionMid = TimeSystem::internalTime2PosixTime( TimeSystem::duration / 2 );

    const auto &sat_xml_o = xml_.get_optional<string>( "VieSchedpp.catalogs.satellite_avoid" );
    if ( !sat_xml_o.is_initialized() ) {
        return;
    }
    if ( sat_xml_o.is_initialized() ) {
        AvoidSatellites::extraMargin = xml_.get( "VieSchedpp.satelliteAvoidance.extraMargin", 0.2 ) * deg2rad;
        AvoidSatellites::orbitError = xml_.get( "VieSchedpp.satelliteAvoidance.orbitError", 2000 );
        AvoidSatellites::orbitErrorPerDay = xml_.get( "VieSchedpp.satelliteAvoidance.orbitErrorPerDay", 2000 );
        AvoidSatellites::frequency = xml_.get( "VieSchedpp.satelliteAvoidance.checkFrequency", 30 );
        AvoidSatellites::minElevation = xml_.get( "VieSchedpp.satelliteAvoidance.minElevation", 20 ) * deg2rad;

        const auto &sat_xml = *sat_xml_o;
        if ( sat_xml.empty() ) {
            return;
        }
        of << "Create satellites to be avoided:\n";
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "Create satellites to be avoided";
#endif


        ifstream fid( sat_xml );
        string line;
        if ( !fid.is_open() ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( fatal ) << "unable to open " << sat_xml << " file";
#else
            cout << "[fatal] unable to open " << sat_xml << " file\n";
#endif
            terminate();
        } else {
            string header;
            string line1;
            string line2;
            int flag = 0;
            while ( getline( fid, line ) ) {
                line = boost::trim_copy( line );
                if ( line.empty() ) {
                    continue;
                }
                switch ( flag ) {
                    case 0: {
                        header = line;
                        ++flag;
                        break;
                    }
                    case 1: {
                        line1 = line;
                        ++flag;
                        break;
                    }
                    case 2: {
                        line2 = line;
                        ++flag;
                        break;
                    }
                    default: {
                        break;
                    }
                }

                if ( flag == 3 ) {
                    try {
                        flag = 0;
                        string processedName = header;
                        util::simplify_inline( processedName );
                        processedName = boost::replace_all_copy( processedName, " ", "_" );

                        ++count_tle;
                        // check reference time
                        double dt =
                            abs( ( Satellite::extractReferenceEpoch( line1 ) - sessionMid ).total_seconds() / 86400.0 );
                        if ( deltaTimes.find( header ) == deltaTimes.end() ) {
                            deltaTimes[header] = dt;
                        } else {
                            double current = deltaTimes[header];
                            if ( dt < current ) {
                                deltaTimes[header] = dt;
                            }
                        }

                        // check if satellite was already generated
                        bool existed = false;
                        for ( auto &any : AvoidSatellites::satellitesToAvoid ) {
                            if ( any->hasName( header ) ) {
                                any->addpSGP4Data( header, line1, line2 );
                                existed = true;
                            }
                        }
                        if ( existed ) {
                            continue;
                        }
                        ++counter;

                        std::unordered_map<std::string, std::unique_ptr<AbstractFlux>> src_flux;
                        for ( const auto &band : ObservingMode::bands ) {
                            src_flux[band] = make_unique<Flux_constant>( ObservingMode::wavelengths[band], 0 );
                        }
                        auto src = make_shared<Satellite>( header, line1, line2, src_flux );
                        AvoidSatellites::satellitesToAvoid.push_back( src );
                        ++created;
                        src_created.push_back( header );
#ifdef VIESCHEDPP_LOG
                        if ( Flags::logDebug )
                            BOOST_LOG_TRIVIAL( debug ) << "satellite " << header << " successfully created ";
#endif
                    } catch ( ... ) {
                        string processedName = header;
                        util::simplify_inline( processedName );
                        src_failed.push_back( processedName );
                    }
                }
            }
            of << "Finished! " << created << " of " << counter << " satellites created that should be avoided("
               << count_tle << " TLE entries)\n\n";
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( info ) << "successfully created " << created << " of " << counter
                                      << " satellites to be avoided (" << count_tle << " TLE entries)";
#else
            cout << "[info] successfully created " << created << " of " << counter << " satellites";
#endif

            //            util::outputObjectList( "created satellites to be avoided", src_created, of );
            util::outputObjectList( "failed to create satellites to be avoided", src_failed, of );
        }

        for ( const auto &any : deltaTimes ) {
            double dt = any.second;
            string name = any.first;
            if ( dt > 14 && AvoidSatellites::orbitErrorPerDay > 0 ) {
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( warning )
                    << boost::format( "satellite orbit data for %s possibly outdated (%.1f days)" ) % name % dt;
#else
                cout << boost::format( "satellite orbit data for %s possibly outdated (%.1f days)" ) % name % dt;
#endif
            }
        }

    } else {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( error ) << "satellite TLE file not defined";
#else
        cout << "[error] satellite TLE file not defined\n";
#endif
        return;
    }
}

void Initializer::createSpacecrafts( const SkdCatalogReader &reader, ofstream &of ) noexcept {

    string path_to_files = xml_.get<string>( "VieSchedpp.catalogs.spacecraft_dir", "" );
    if ( path_to_files.empty() ) {
        return;
    }
    vector<string> stations;
    boost::property_tree::ptree stations_ptree = xml_.get_child( "VieSchedpp.general.stations" );
    auto it = stations_ptree.begin();;
    while ( it != stations_ptree.end() ) {
        auto item = it->second.data();
        stations.push_back( item );
        ++it;
    }

    of << "Create Spacecrafts:\n";
    vector<string> spacecrafts;
    const auto &spacecraft_xml_list_o = xml_.get_child_optional( "VieSchedpp.general.spacecrafts" );
    if ( spacecraft_xml_list_o.is_initialized() ) {
        const auto &spacecraft_xml = *spacecraft_xml_list_o;
        for ( const auto &any : spacecraft_xml ) {
            string name = any.second.data();
            util::simplify_inline( name );
            name = boost::replace_all_copy( name, " ", "_" );
            spacecrafts.push_back( name );
        }
    } else {
        return;
    }

    vector<string> spacecraftsCreated;
    vector<string> spacecraftsFailed;

    for (const auto &spacecraft : spacecrafts) {

        std::unordered_map<std::string, std::unique_ptr<AbstractFlux>> src_flux;
        for (const auto &band : ObservingMode::bands) {
            src_flux[band] = std::make_unique<Flux_constant>(ObservingMode::wavelengths[band], 1);
        }

        std::unordered_map<std::string, std::vector<std::tuple<unsigned int,double,double,double>>> EphemerisMap;

        bool failed = false;  // track if any station failed
        for (const auto &station : stations) {
            auto ephemOpt = Spacecraft::extractEphemerisData(path_to_files, spacecraft, station);

            if (ephemOpt) {
                EphemerisMap[station] = *ephemOpt;
            } else {
                failed = true;  // mark failure
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL(warning)
                    << boost::format("Failed to load ephemeris for spacecraft '%s' at station '%s'")
                       % spacecraft % station;
#else
                std::cout << boost::format("Failed to load ephemeris for spacecraft '%s' at station '%s'")
                             % spacecraft % station << std::endl;
#endif
            }
        }

        if (failed) {
            spacecraftsFailed.push_back(spacecraft);  // add to failed list
        } else {
            auto src = std::make_shared<Spacecraft>(spacecraft, src_flux, EphemerisMap);
            sourceList_.addSpacecraft(src);
            spacecraftsCreated.push_back(spacecraft);
        }
    }



    // --------------------- TEST START -----------------------
    std::cout << "--------------------- TEST START -----------------------" << std::endl;

    // Loop through all spacecrafts in the list
    for (unsigned long i = 0; i < sourceList_.getNSpacecrafts(); ++i) {
        auto sc = sourceList_.getSpacecraft(i);
        std::cout << "Spacecraft[" << i << "] Name: " << sc->getName()
                  << " ID: " << sc->getId() << "\n";

        // Test interpolation for each station
        for (size_t j = 0; j < stations.size(); ++j) {
            auto RaDe = sc->calcRaDe2(150, stations[j]);
            std::cout << "Interpolation at t = 150, station " << stations[j]
                      << ": ra: " << RaDe.first << ", dec: " << RaDe.second << std::endl;
            sc->printEphemSample(sc->getName(), stations[j], 5);
        }
    }

    // Check counts
    std::cout << "Total sources: " << sourceList_.getNSrc() << "\n";
    std::cout << "Spacecrafts: " << sourceList_.getNSpacecrafts() << "\n";

    // Check vector
    for (unsigned long id = 0; id < sourceList_.getNSrc(); ++id) {
        auto src = sourceList_.getSource(id);
        if (sourceList_.isSpacecraft(id)) {
            std::cout << "Vector contains spacecraft ID: " << id
                      << " Name: " << src->getName() << "\n";
        }
    }

    std::cout << "--------------------- TEST END -----------------------" << std::endl;
    // --------------------- TEST END -----------------------





    of << "Finished! " << spacecraftsCreated.size() << " of " << spacecrafts.size() << " spacecrafts created\n\n";
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "successfully created " << spacecraftsCreated.size() << " of " << spacecrafts.size() << " spacecrafts";
#else
    cout << "[info] successfully created " << spacecraftsCreated.size() << " of " << spacecrafts.size() << " spacecrafts";
#endif

    util::outputObjectList( "created spacecrafts", spacecraftsCreated, of );
    util::outputObjectList( "failed to create spacecrafts", spacecraftsFailed, of );
}


void Initializer::initializeGeneral( ofstream &of ) noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "initialize general";
#endif
    try {
        string startString = xml_.get<string>( "VieSchedpp.general.startTime" );
        boost::posix_time::ptime startTime = TimeSystem::string2ptime( startString );
        of << "start time: " << TimeSystem::time2string( startTime ) << "\n";
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug )
            BOOST_LOG_TRIVIAL( debug ) << "session start time: " << TimeSystem::time2string( startTime );
#endif
        int sec_ = startTime.time_of_day().total_seconds();
        double mjdStart = startTime.date().modjulian_day() + sec_ / 86400.0;

        string endString = xml_.get<string>( "VieSchedpp.general.endTime" );
        boost::posix_time::ptime endTime = TimeSystem::string2ptime( endString );
        of << "end time:   " << TimeSystem::time2string( endTime ) << "\n";
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "session end time: " << TimeSystem::time2string( endTime );
#endif

        int sec = util::duration( startTime, endTime );
        if ( sec < 0 ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( error ) << "duration is less than zero seconds";
#else
            cout << "[error] duration is less than zero seconds";
#endif
        }
        auto duration = static_cast<unsigned int>( sec );
        of << "duration: " << duration << " [s]\n";
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "session duration: " << duration << "[s]";
#endif

        TimeSystem::mjdStart = mjdStart;
        TimeSystem::startTime = startTime;
        TimeSystem::endTime = endTime;
        TimeSystem::duration = duration;
        TimeSystem::startSgp4 = TimeSystem::internalTime2sgpt4Time( 0 );

        vector<string> sel_stations;
        boost::property_tree::ptree stations = xml_.get_child( "VieSchedpp.general.stations" );
        auto it = stations.begin();
        while ( it != stations.end() ) {
            auto item = it->second.data();
            sel_stations.push_back( item );
            ++it;
        }
        parameters_.selectedStations = sel_stations;

        parameters_.subnetting = xml_.get<bool>( "VieSchedpp.general.subnetting" );
        parameters_.subnettingMinAngle = xml_.get<double>( "VieSchedpp.general.subnettingMinAngle", 120. ) * deg2rad;
        boost::optional<double> subnettingMinNStaPercent =
            xml_.get_optional<double>( "VieSchedpp.general.subnettingMinNStaPercent" );
        boost::optional<double> subnettingMinNStaAllBut =
            xml_.get_optional<double>( "VieSchedpp.general.subnettingMinNStaAllBut" );
        if ( subnettingMinNStaPercent.is_initialized() ) {
            parameters_.subnettingMinNStaPercent = subnettingMinNStaPercent.get() / 100.;
            parameters_.subnettingMinNStaPercent_otherwiseAllBut = true;
        }
        if ( subnettingMinNStaAllBut.is_initialized() ) {
            parameters_.subnettingMinNStaAllBut = subnettingMinNStaAllBut.get();
            parameters_.subnettingMinNStaPercent_otherwiseAllBut = false;
        }

        parameters_.fillinmodeDuringScanSelection =
            xml_.get( "VieSchedpp.general.fillinmodeDuringScanSelection", false );
        parameters_.fillinmodeInfluenceOnSchedule =
            xml_.get( "VieSchedpp.general.fillinmodeInfluenceOnSchedule", false );
        parameters_.fillinmodeAPosteriori = xml_.get( "VieSchedpp.general.fillinmodeAPosteriori", false );

        if ( xml_.get_optional<int>( "VieSchedpp.general.fillinmodeAPosteriori_minNumberOfStations" )
                 .is_initialized() ) {
            parameters_.fillinmodeAPosteriori_minSites =
                xml_.get_optional<int>( "VieSchedpp.general.fillinmodeAPosteriori_minNumberOfStations" );
        }
        if ( xml_.get_optional<int>( "VieSchedpp.general.fillinmodeAPosteriori_minNumberOfSites" ).is_initialized() ) {
            parameters_.fillinmodeAPosteriori_minSites =
                xml_.get_optional<int>( "VieSchedpp.general.fillinmodeAPosteriori_minNumberOfSites" );
        }
        parameters_.fillinmodeAPosteriori_minRepeat =
            xml_.get_optional<int>( "VieSchedpp.general.fillinmodeAPosteriori_minRepeat" );

        parameters_.idleToObservingTime = xml_.get( "VieSchedpp.general.idleToObservingTime", false );
        parameters_.idleToObservingTimeGroup = xml_.get( "VieSchedpp.general.idleToObservingTimeGroup", "__all__" );

        std::string anchor = xml_.get<std::string>( "VieSchedpp.general.scanAlignment", "start" );
        if ( anchor == "start" ) {
            ScanTimes::setAlignmentAnchor( ScanTimes::AlignmentAnchor::start );
        } else if ( anchor == "end" ) {
            ScanTimes::setAlignmentAnchor( ScanTimes::AlignmentAnchor::end );
        } else if ( anchor == "individual" ) {
            ScanTimes::setAlignmentAnchor( ScanTimes::AlignmentAnchor::individual );
        } else {
            of << "ERROR: cannot read scan alignment type:" << anchor << endl;
        }
        parameters_.doNotObserveSourcesWithinMinRepeat =
            xml_.get( "VieSchedpp.general.doNotObserveSourcesWithinMinRepeat", true );
        parameters_.ignoreSuccessiveScansSameSrc =
            xml_.get( "VieSchedpp.general.ignore_successive_scans_same_source", true );

    } catch ( const boost::property_tree::ptree_error &e ) {
        of << "ERROR: reading VieSchedpp.xml file!" << endl;
    }

    of << "\n";
}


void Initializer::initializeStations() noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "initialize stations";
#endif

    // get station tree
    const auto &PARA_station_o = xml_.get_child_optional( "VieSchedpp.station" );
    if ( PARA_station_o.is_initialized() ) {
        const auto &PARA_station = PARA_station_o.get();

        // get all defined baseline group
        staGroups_ = readGroups( PARA_station, MemberType::station );

        // get all defined parameters
        unordered_map<std::string, ParameterSettings::ParametersStations> parameters;
        const auto &para_tree = PARA_station.get_child( "parameters" );
        for ( auto &it : para_tree ) {
            string name = it.first;
            if ( name == "parameter" ) {
                string parameterName = it.second.get_child( "<xmlattr>.name" ).data();
#ifdef VIESCHEDPP_LOG
                if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "read station parameter " << parameterName;
#endif

                ParameterSettings::ParametersStations PARA;
                auto PARA_ = ParameterSettings::ptree2parameterStation( it.second );
                PARA = PARA_.second;
                parameters[parameterName] = PARA;
            }
        }

        // change ignore source name to id
        for ( auto &any : parameters ) {
            auto &iss = any.second.ignoreSourcesString;
            for ( const auto &iss_n : iss ) {
                for ( const auto &src : sourceList_.getSources() ) {
                    if ( src->hasName( iss_n ) ) {
                        any.second.ignoreSources.push_back( src->getId() );
                        break;
                    }
                }
            }
        }

        // define backup parameter
        Station::Parameters parentPARA( "backup" );
#ifdef VIESCHEDPP_LOG
        if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "create backup station parameters";
#endif

        // store events for each station
        vector<vector<Station::Event>> events( network_.getNSta() );

        // set observation mode band names
        for ( const auto &any : ObservingMode::bands ) {
            const string &name = any;
            parentPARA.minSNR[name] = ObservingMode::minSNR[name];
        }

        // create default events at start and end
        for ( int staid = 0; staid < network_.getNSta(); ++staid ) {
            parentPARA.totalRecordingRate = obsModes_->getMode( 0 )->recordingRate( staid );
            Station::Event newEvent_start( 0, false, parentPARA );
            events[staid].push_back( newEvent_start );

            Station::Event newEvent_end( TimeSystem::duration, true, parentPARA );
            events[staid].push_back( newEvent_end );
        }

        // add setup for all stations
        for ( auto &it : PARA_station ) {
            string name = it.first;
            if ( name == "setup" ) {
                stationSetup( events, it.second, parameters, staGroups_, parentPARA );
            }
        }

        // set to start state
        for ( unsigned long ista = 0; ista < network_.getNSta(); ++ista ) {
            Station &thisStation = network_.refStation( ista );
#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "set events for station " << thisStation.getName();
#endif

            PointingVector pV( ista, 0 );
            pV.setAz( ( thisStation.getCableWrap().getNLow() + thisStation.getCableWrap().getNUp() ) / 2 );
            pV.setEl( 0 );
            pV.setTime( 0 );

            thisStation.setCurrentPointingVector( pV );
            thisStation.setEVENTS( events[ista] );

            bool hardBreak = false;
            thisStation.checkForNewEvent( 0, hardBreak );

            thisStation.referencePARA().firstScan = true;
        }

        // set cable wrap buffers
        auto cableBuffer_tree = PARA_station.get_child( "cableWrapBuffers" );
        vector<string> cableInitialized;
        for ( auto &it : cableBuffer_tree ) {
            string name = it.first;
            if ( name == "cableWrapBuffer" ) {
                vector<string> cableNow;
                string memberName = it.second.get_child( "<xmlattr>.member" ).data();
                if ( staGroups_.find( memberName ) != staGroups_.end() ) {
                    cableNow.insert( cableNow.end(), staGroups_[memberName].begin(), staGroups_[memberName].end() );
                } else if ( memberName == "__all__" ) {
                    for ( const auto &sta : network_.getStations() ) {
                        cableNow.push_back( sta.getName() );
                    }
                } else {
                    cableNow.push_back( memberName );
                }

                bool errorFlagWaitTime = false;
                for ( const auto &any : cableNow ) {
                    if ( find( cableInitialized.begin(), cableInitialized.end(), any ) != cableInitialized.end() ) {
#ifdef VIESCHEDPP_LOG
                        BOOST_LOG_TRIVIAL( error )
                            << "double use of station/group " << name << " in cable wrap buffer block -> ignored";
#else
                        cout << "[error] double use of station/group " << name
                             << " in cable wrap buffer block -> ignored";
#endif
                        errorFlagWaitTime = true;
                    }
                }
                if ( errorFlagWaitTime ) {
                    continue;
                }

                for ( const auto &any : cableNow ) {
                    for ( auto &sta : network_.refStations() ) {
                        if ( sta.hasName( any ) ) {
#ifdef VIESCHEDPP_LOG
                            if ( Flags::logTrace )
                                BOOST_LOG_TRIVIAL( trace ) << "set cable wrap buffers for  " << sta.getName();
#endif

                            auto axis1Low = it.second.get<double>( "axis1LowOffset" );
                            auto axis1Up = it.second.get<double>( "axis1UpOffset" );
                            auto axis2Low = it.second.get<double>( "axis2LowOffset" );
                            auto axis2Up = it.second.get<double>( "axis2UpOffset" );
                            sta.referenceCableWrap().setMinimumOffsets( axis1Low, axis1Up, axis2Low, axis2Up );
                            break;
                        }
                    }
                }
                cableInitialized.insert( cableInitialized.end(), cableNow.begin(), cableNow.end() );
            }
        }

    } else {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( fatal ) << "cannot read <station> block in VieSchedpp.xml file";
        terminate();
#else
        cout << "cannot read <station> block in VieSchedpp.xml file";
#endif
    }
}


void Initializer::precalcAzElStations() noexcept {
    for ( auto &sta : network_.refStations() ) {
        for ( const auto &source : sourceList_.getQuasars() ) {
            int step = 600;
            PointingVector npv( sta.getId(), source->getId() );
            for ( unsigned int t = 0; t < TimeSystem::duration + 1800; t += step ) {
                npv.setTime( t );
                sta.calcAzEl_rigorous( source, npv );
            }
        }
        for ( const auto &source : sourceList_.getSatellites() ) {
//            ofstream o("/scratch/programming/tmp/"+sta.getName()+".csv");
            int step = 60;
            PointingVector npv( sta.getId(), source->getId() );
            for ( unsigned int t = 0; t < TimeSystem::duration + 1800; t += step ) {
                npv.setTime( t );
                sta.calcAzEl_rigorous( source, npv );
//                o << boost::format("%f,%f,%f\n")% (TimeSystem::mjdStart+t/86400) % (npv.getAz()*rad2deg) % (npv.getEl()*rad2deg);
            }
        }
    }
}


void Initializer::stationSetup( vector<vector<Station::Event>> &events, const boost::property_tree::ptree &tree,
                                const unordered_map<std::string, ParameterSettings::ParametersStations> &parameters,
                                const unordered_map<std::string, std::vector<std::string>> &groups,
                                const Station::Parameters &parentPARA ) noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "station setup";
#endif
    vector<string> members;
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace )
        BOOST_LOG_TRIVIAL( trace ) << "creating new station parameter " << tree.get<string>( "parameter" );
#endif
    Station::Parameters combinedPARA = Station::Parameters( tree.get<string>( "parameter" ) );
    combinedPARA.setParameters( parentPARA );
    unsigned int start = 0;
    unsigned int end = TimeSystem::duration;
    bool smoothTransition = true;

    for ( auto &it : tree ) {
        string paraName = it.first;
        if ( paraName == "member" || paraName == "group" ) {
            string tmp = it.second.data();
            if ( tmp == "__all__"){
                for ( const auto &any : network_.getStations() ) {
                    members.push_back( any.getName() );
                }
            }else{
                if ( groups.find( tmp ) != groups.end() ){
                    members = groups.at( tmp );
                }else{
                    members.push_back( tmp );
                }
            }
        } else if ( paraName == "parameter" ) {
            string tmp = it.second.data();

            ParameterSettings::ParametersStations newPARA = parameters.at( tmp );
            if ( newPARA.available.is_initialized() ) {
                combinedPARA.available = *newPARA.available;
            }
            if ( newPARA.availableForFillinmode.is_initialized() ) {
                combinedPARA.availableForFillinmode = *newPARA.availableForFillinmode;
            }

            if ( newPARA.tagalong.is_initialized() ) {
                combinedPARA.tagalong = *newPARA.tagalong;
            }
            //            if (newPARA.firstScan.is_initialized()) {
            //                combinedPARA.firstScan = *newPARA.firstScan;
            //            }

            if ( newPARA.weight.is_initialized() ) {
                combinedPARA.weight = *newPARA.weight;
            }

            if ( newPARA.minScan.is_initialized() ) {
                combinedPARA.minScan = *newPARA.minScan;
            }
            if ( newPARA.maxScan.is_initialized() ) {
                combinedPARA.maxScan = *newPARA.maxScan;
            }
            if ( newPARA.minSlewtime.is_initialized() ) {
                combinedPARA.minSlewtime = *newPARA.minSlewtime;
            }
            if ( newPARA.maxSlewtime.is_initialized() ) {
                combinedPARA.maxSlewtime = *newPARA.maxSlewtime;
            }
            if ( newPARA.maxSlewDistance.is_initialized() ) {
                combinedPARA.maxSlewDistance = *newPARA.maxSlewDistance * deg2rad;
            }
            if ( newPARA.minSlewDistance.is_initialized() ) {
                combinedPARA.minSlewDistance = *newPARA.minSlewDistance * deg2rad;
            }
            if ( newPARA.maxWait.is_initialized() ) {
                combinedPARA.maxWait = *newPARA.maxWait;
            }
            if ( newPARA.minElevation.is_initialized() ) {
                combinedPARA.minElevation = *newPARA.minElevation * deg2rad;
            }
            if ( newPARA.maxNumberOfScans.is_initialized() ) {
                combinedPARA.maxNumberOfScans = *newPARA.maxNumberOfScans;
            }
            if ( newPARA.maxNumberOfScansDist.is_initialized() ) {
                combinedPARA.maxNumberOfScansDist = *newPARA.maxNumberOfScansDist;
            }
            if ( newPARA.maxTotalObsTime.is_initialized() ) {
                combinedPARA.maxTotalObsTime = *newPARA.maxTotalObsTime;
            }

            if ( newPARA.dataWriteRate.is_initialized() ) {
                combinedPARA.dataWriteRate = *newPARA.dataWriteRate * 1e6;
            }
            if ( newPARA.preob.is_initialized() ) {
                combinedPARA.preob = *newPARA.preob;
            }
            if ( newPARA.midob.is_initialized() ) {
                combinedPARA.midob = *newPARA.midob;
            }
            if ( newPARA.systemDelay.is_initialized() ) {
                combinedPARA.systemDelay = *newPARA.systemDelay;
            }

            if ( !newPARA.minSNR.empty() ) {
                for ( const auto &any : newPARA.minSNR ) {
                    string name = any.first;
                    double value = any.second;
                    combinedPARA.minSNR[name] = value;
                }
            }
            if ( !newPARA.ignoreSources.empty() ) {
                combinedPARA.ignoreSources = newPARA.ignoreSources;
            }

        } else if ( paraName == "start" ) {
            string t = it.second.get_value<string>();
            boost::posix_time::ptime thisStartTime = TimeSystem::string2ptime( t );
            start = TimeSystem::posixTime2InternalTime( thisStartTime );
        } else if ( paraName == "end" ) {
            string t = it.second.get_value<string>();
            boost::posix_time::ptime thisEndTime = TimeSystem::string2ptime( t );
            end = TimeSystem::posixTime2InternalTime( thisEndTime );
        } else if ( paraName == "transition" ) {
            string tmp = it.second.data();
            if ( tmp == "hard" ) {
                smoothTransition = false;
            } else if ( tmp == "smooth" ) {
                smoothTransition = true;
            } else {
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( warning ) << "unknown transition type in <station><setup> block -> set to 'smooth'";
#else
                cout << "[warning] unknown transition type in <station><setup> block -> set to 'soft'";
#endif
            }
        }
    }

#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << boost::format( "  station setup '%s' (%d members) parameter '%s' \n" ) %
                                     tree.get<string>( "member" ) % members.size() % tree.get<string>( "parameter" );
#else
    cout << boost::format( "[info]   station setup %s (%d members) parameter %s \n" ) % tree.get<string>( "member" ) %
                members.size() % tree.get<string>( "parameter" );
#endif

    vector<string> staNames;
    for ( const auto &any : network_.getStations() ) {
        staNames.push_back( any.getName() );
    }

    for ( const auto &any : members ) {
        auto it = find( staNames.begin(), staNames.end(), any );
        if ( it == staNames.end() ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( warning ) << "station " << any << " not found but defined in setup -> ignoring setup";
#else
            cout << "[warning] station " << any << " not found but defined in setup -> ignoring setup";
#endif
            continue;
        }

        long id = distance( staNames.begin(), it );
        auto &thisEvents = events[id];
        combinedPARA.totalRecordingRate = thisEvents[0].PARA.totalRecordingRate;

        Station::Event newEvent_start( start, smoothTransition, combinedPARA );

        for ( auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit ) {
            if ( iit->time > newEvent_start.time ) {
                thisEvents.insert( iit, newEvent_start );
#ifdef VIESCHEDPP_LOG
                if ( Flags::logTrace )
                    BOOST_LOG_TRIVIAL( trace ) << "insert event for station " << network_.getStation( id ).getName();
#endif
                break;
            }
        }

        Station::Event newEvent_end( end, true, parentPARA );
        for ( auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit ) {
            if ( iit->time >= newEvent_end.time ) {
                thisEvents.insert( iit, newEvent_end );
#ifdef VIESCHEDPP_LOG
                if ( Flags::logTrace )
                    BOOST_LOG_TRIVIAL( trace ) << "insert event for station " << network_.getStation( id ).getName();
#endif
                break;
            }
        }
    }

    for ( auto &it : tree ) {
        string paraName = it.first;
        if ( paraName == "setup" ) {
            stationSetup( events, it.second, parameters, groups, combinedPARA );
        }
    }
}


void Initializer::initializeSources( MemberType type ) noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "initialize sources";
#endif

    // get source tree
    string path;
    unsigned long nSrc = 0;

    switch ( type ) {
        case MemberType::source: {
            path = "VieSchedpp.source";
            nSrc = sourceList_.getNQuasars();
            break;
        }
        case MemberType::satellite: {
            path = "VieSchedpp.satellite";
            nSrc = sourceList_.getNSatellites();
            break;
        }
        case MemberType::spacecraft: {
            path = "VieSchedpp.spacecraft";
            break;
        }
        default: {
            return;
        }
    }

    const auto &PARA_source_o = xml_.get_child_optional( path );
    if ( PARA_source_o.is_initialized() ) {
        const auto &PARA_source = PARA_source_o.get();

        // get all defined source groups
        const auto &groups = readGroups( PARA_source, type );
        switch ( type ) {
            case MemberType::source: {
                srcGroups_ = groups;
                break;
            }
            case MemberType::satellite: {
                satGroups_ = groups;
                break;
            }
            case MemberType::spacecraft: {
                spacecraftGroups_ = groups;
                break;
            }
            default: {
                return;
            }
        }

        // get all defined parameters
        const auto &para_tree = PARA_source.get_child( "parameters" );
        unordered_map<std::string, ParameterSettings::ParametersSources> parameters;
        for ( auto &it : para_tree ) {
            string name = it.first;
            if ( name == "parameter" ) {
                string parameterName = it.second.get_child( "<xmlattr>.name" ).data();
#ifdef VIESCHEDPP_LOG
                if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "read source parameter " << parameterName;
#endif

                ParameterSettings::ParametersSources PARA;

                auto PARA_ = ParameterSettings::ptree2parameterSource( it.second );
                PARA = PARA_.second;

                parameters[parameterName] = PARA;
            }
        }
        // change names in parameters to ids
        for ( auto &any : parameters ) {
            // get ignore station ids
            auto &ignoreStationsString = any.second.ignoreStationsString;
            for ( const auto &ignoreStationName : ignoreStationsString ) {
                unsigned long id = network_.getStation( ignoreStationName ).getId();
                any.second.ignoreStations.push_back( id );
            }

            // get required station ids
            auto &requiredStationsString = any.second.requiredStationsString;
            for ( const auto &requiredStationName : requiredStationsString ) {
                unsigned long id = network_.getStation( requiredStationName ).getId();
                any.second.requiredStations.push_back( id );
            }

            // get ignore baseline ids
            auto &ignoreBaselineString = any.second.ignoreBaselinesString;
            for ( const auto &ignoreBaselineName : ignoreBaselineString ) {
                unsigned long id = network_.getBaseline( ignoreBaselineName ).getId();
                any.second.ignoreBaselines.push_back( id );
            }
        }

        // define backup parameter
        AbstractSource::Parameters parentPARA( "backup" );
#ifdef VIESCHEDPP_LOG
        if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "create backup source parameters";
#endif

        // set observation mode band names
        for ( const auto &any : ObservingMode::bands ) {
            const string &name = any;
            parentPARA.minSNR[name] = ObservingMode::minSNR[name];
        }

        // store events for each source
        vector<vector<AbstractSource::Event>> events( nSrc );

        // create default events at start and end
        for ( int i = 0; i < nSrc; ++i ) {
            AbstractSource::Event newEvent_start( 0, false, parentPARA );
            events[i].push_back( newEvent_start );
            AbstractSource::Event newEvent_end( TimeSystem::duration, true, parentPARA );
            events[i].push_back( newEvent_end );
        }

        // add setup for all sources
        for ( auto &it : PARA_source ) {
            string name = it.first;
            if ( name == "setup" ) {
                sourceSetup( events, it.second, parameters, groups, parentPARA, type );
            }
        }

        // set events for all sources
        for ( int i = 0; i < nSrc; ++i ) {
            switch ( type ) {
                case MemberType::source: {
                    const auto &src = sourceList_.refQuasar( i );
                    src->setEVENTS( events[i] );
                    break;
                }
                case MemberType::satellite: {
                    const auto &src = sourceList_.refSatellite( i );
                    src->setEVENTS( events[i] );
                    break;
                }
                case MemberType::spacecraft: {
                }
            }
        }

        // set to start event
        bool hardBreak = false;
        for ( int i = 0; i < nSrc; ++i ) {
            switch ( type ) {
                case MemberType::source: {
                    const auto &src = sourceList_.refQuasar( i );
                    src->checkForNewEvent( 0, hardBreak );
                    break;
                }
                case MemberType::satellite: {
                    const auto &src = sourceList_.refSatellite( i );
                    src->checkForNewEvent( 0, hardBreak );
                    break;
                }
                case MemberType::spacecraft: {
                }
            }
        }
    }
}


void Initializer::sourceSetup( vector<vector<AbstractSource::Event>> &events, const boost::property_tree::ptree &tree,
                               const unordered_map<std::string, ParameterSettings::ParametersSources> &parameters,
                               const unordered_map<std::string, std::vector<std::string>> &groups,
                               const AbstractSource::Parameters &parentPARA, MemberType type ) noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "source setup";
#endif
    vector<string> members;
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace )
        BOOST_LOG_TRIVIAL( trace ) << "creating new source parameter " << tree.get<string>( "parameter" );
#endif
    AbstractSource::Parameters combinedPARA = AbstractSource::Parameters( tree.get<string>( "parameter" ) );
    combinedPARA.setParameters( parentPARA );
    unsigned int start = 0;
    unsigned int end = TimeSystem::duration;
    bool smoothTransition = true;

    for ( auto &it : tree ) {
        string paraName = it.first;
        if ( paraName == "member" || paraName == "group") {
            string tmp = it.second.data();
            if ( tmp == "__all__"){
                switch ( type ) {
                    case MemberType::source: {
                        for ( const auto &any : sourceList_.getQuasars() ) {
                            members.push_back( any->getName() );
                        }
                        break;
                    }
                    case MemberType::satellite: {
                        for ( const auto &any : sourceList_.getSatellites() ) {
                            members.push_back( any->getName() );
                        }
                        break;
                    }
                    case MemberType::spacecraft: {
                        break;
                    }
                }
            }else{
                if ( groups.find( tmp ) != groups.end() ){
                    members = groups.at( tmp );
                }else{
                    members.push_back( tmp );
                }
            }
        } else if ( paraName == "parameter" ) {
            const string &tmp = it.second.data();

            ParameterSettings::ParametersSources newPARA = parameters.at( tmp );
            if ( newPARA.available.is_initialized() ) {
                combinedPARA.available = *newPARA.available;
            }
            if ( newPARA.availableForFillinmode.is_initialized() ) {
                combinedPARA.availableForFillinmode = *newPARA.availableForFillinmode;
            }

            if ( newPARA.weight.is_initialized() ) {
                combinedPARA.weight = *newPARA.weight;
            }

            if ( newPARA.minNumberOfSites.is_initialized() ) {
                int n = *newPARA.minNumberOfSites;
                if ( n > Network::nSites() ) {
                    n = Network::nSites();
                }
                combinedPARA.minNumberOfSites = n;
            }
            if ( newPARA.minFlux.is_initialized() ) {
                combinedPARA.minFlux = *newPARA.minFlux;
            }
            if ( newPARA.minRepeat.is_initialized() ) {
                combinedPARA.minRepeat = *newPARA.minRepeat;
            }
            if ( newPARA.minScan.is_initialized() ) {
                combinedPARA.minScan = *newPARA.minScan;
            }
            if ( newPARA.maxScan.is_initialized() ) {
                combinedPARA.maxScan = *newPARA.maxScan;
            }
            if ( newPARA.maxNumberOfScans.is_initialized() ) {
                combinedPARA.maxNumberOfScans = *newPARA.maxNumberOfScans;
            }
            if ( newPARA.jetAngleFactor.is_initialized() ) {
                combinedPARA.jetAngleFactor = *newPARA.jetAngleFactor;
            }
            if ( newPARA.jetAngleBuffer.is_initialized() ) {
                combinedPARA.jetAngleBuffer = *newPARA.jetAngleBuffer;
            }
            if ( newPARA.forceSameObservingDuration.is_initialized() ) {
                combinedPARA.forceSameObservingDuration = *newPARA.forceSameObservingDuration;
            }
            if ( newPARA.tryToFocusIfObservedOnce.is_initialized() ) {
                combinedPARA.tryToFocusIfObservedOnce = *newPARA.tryToFocusIfObservedOnce;
                combinedPARA.tryToFocusFactor = *newPARA.tryToFocusFactor;
                if ( *newPARA.tryToFocusOccurrency == ParameterSettings::TryToFocusOccurrency::once ) {
                    combinedPARA.tryToFocusOccurrency = AbstractSource::TryToFocusOccurrency::once;
                } else {
                    combinedPARA.tryToFocusOccurrency = AbstractSource::TryToFocusOccurrency::perScan;
                }
                if ( *newPARA.tryToFocusType == ParameterSettings::TryToFocusType::additive ) {
                    combinedPARA.tryToFocusType = AbstractSource::TryToFocusType::additive;
                } else {
                    combinedPARA.tryToFocusType = AbstractSource::TryToFocusType::multiplicative;
                }
            }
            if ( newPARA.tryToObserveXTimesEvenlyDistributed.is_initialized() ) {
                combinedPARA.tryToObserveXTimesEvenlyDistributed = *newPARA.tryToObserveXTimesEvenlyDistributed;
                combinedPARA.tryToObserveXTimesMinRepeat = *newPARA.tryToObserveXTimesMinRepeat;
            }
            if ( newPARA.fixedScanDuration.is_initialized() ) {
                combinedPARA.fixedScanDuration = *newPARA.fixedScanDuration;
            }
            if ( newPARA.minElevation.is_initialized() ) {
                combinedPARA.minElevation = *newPARA.minElevation * deg2rad;
            }
            if ( newPARA.minSunDistance.is_initialized() ) {
                combinedPARA.minSunDistance = *newPARA.minSunDistance * deg2rad;
            }

            if ( !newPARA.ignoreStations.empty() ) {
                combinedPARA.ignoreStations = newPARA.ignoreStations;
            }
            if ( !newPARA.requiredStations.empty() ) {
                combinedPARA.requiredStations = newPARA.requiredStations;
            }
            if ( !newPARA.ignoreBaselines.empty() ) {
                combinedPARA.ignoreBaselines = newPARA.ignoreBaselines;
            }

            if ( !newPARA.minSNR.empty() ) {
                for ( const auto &any : newPARA.minSNR ) {
                    string name = any.first;
                    double value = any.second;
                    combinedPARA.minSNR[name] = value;
                }
            }

        } else if ( paraName == "start" ) {
            string t = it.second.get_value<string>();
            boost::posix_time::ptime thisStartTime = TimeSystem::string2ptime( t );
            start = TimeSystem::posixTime2InternalTime( thisStartTime );
        } else if ( paraName == "end" ) {
            string t = it.second.get_value<string>();
            boost::posix_time::ptime thisEndTime = TimeSystem::string2ptime( t );
            end = TimeSystem::posixTime2InternalTime( thisEndTime );
        } else if ( paraName == "transition" ) {
            string tmp = it.second.data();
            if ( tmp == "hard" ) {
                smoothTransition = false;
            } else if ( tmp == "smooth" ) {
                smoothTransition = true;
            } else {
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( warning ) << "unknown transition type in <source><setup> block -> set to 'smooth'";
#else
                cout << "[warning] unknown transition type in <source><setup> block -> set to 'soft'";
#endif
            }
        }
    }


#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << boost::format( "  source setup '%s' (%d members) parameter '%s' \n" ) %
                                     tree.get<string>( "member" ) % members.size() % tree.get<string>( "parameter" );
#else
    cout << boost::format( "[info]   source setup %s (%d members) parameter %s \n" ) % tree.get<string>( "member" ) %
                members.size() % tree.get<string>( "parameter" );
#endif


    vector<string> srcNames;
    vector<string> srcNames2;
    switch ( type ) {
        case MemberType::source: {
            for ( const auto &any : sourceList_.getQuasars() ) {
                srcNames.push_back( any->getName() );
                srcNames2.push_back( any->getAlternativeName() );
            }
            break;
        }
        case MemberType::satellite: {
            for ( const auto &any : sourceList_.getSatellites() ) {
                srcNames.push_back( any->getName() );
                srcNames2.push_back( any->getAlternativeName() );
            }
            break;
        }
        case MemberType::spacecraft: {
            break;
        }
    }

    bool tryToFocusIfObservedOnceBackup = combinedPARA.tryToFocusIfObservedOnce;
    unsigned int maxNumberOfScansBackup = combinedPARA.maxNumberOfScans;
    unsigned int minRepeatBackup = combinedPARA.minRepeat;

    for ( const auto &any : members ) {
        long id;
        auto it = find( srcNames.begin(), srcNames.end(), any );
        if ( it == srcNames.end() ) {
            auto it2 = find( srcNames2.begin(), srcNames2.end(), any );
            if ( it2 == srcNames2.end()) {
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( warning ) << "source " << any << " not found but defined in setup -> ignoring setup";
#else
                cout << "[warning] source " << any << " not found but defined in setup -> ignoring setup";
#endif
                continue;
            } else {
                id = distance( srcNames2.begin(), it2 );
            }
        }else{
            id = distance( srcNames.begin(), it );
        }

        if ( combinedPARA.tryToObserveXTimesEvenlyDistributed.is_initialized() &&
             *combinedPARA.tryToObserveXTimesEvenlyDistributed ) {
//            combinedPARA.maxNumberOfScans = *combinedPARA.tryToObserveXTimesEvenlyDistributed;

            unsigned int minutes = 0;
            switch ( type ) {
                case MemberType::source: {
                    const auto &thisSource = sourceList_.getQuasar( id );
                    minutes = minutesVisible( thisSource, combinedPARA, start, end );
                    break;
                }
                case MemberType::satellite: {
                    const auto &thisSource = sourceList_.getSatellite( id );
                    minutes = minutesVisible( thisSource, combinedPARA, start, end );
                    break;
                }
                case MemberType::spacecraft: {
                    break;
                }
            }

            unsigned int minRepeat = ( 60 * minutes ) / ( *combinedPARA.tryToObserveXTimesEvenlyDistributed );
            unsigned int minRepeatOther = *combinedPARA.tryToObserveXTimesMinRepeat;
            if ( minRepeat < minRepeatOther ) {
                minRepeat = minRepeatOther;
            }
            combinedPARA.minRepeat = minRepeat;
        }
        auto &thisEvents = events[id];

        AbstractSource::Event newEvent_start( start, smoothTransition, combinedPARA );

        for ( auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit ) {
            if ( iit->time > newEvent_start.time ) {
                thisEvents.insert( iit, newEvent_start );
                break;
            }
        }

        AbstractSource::Event newEvent_end( end, true, parentPARA );
        for ( auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit ) {
            if ( iit->time >= newEvent_end.time ) {
                thisEvents.insert( iit, newEvent_end );
                break;
            }
        }

        combinedPARA.tryToFocusIfObservedOnce = tryToFocusIfObservedOnceBackup;
        combinedPARA.maxNumberOfScans = maxNumberOfScansBackup;
        combinedPARA.minRepeat = minRepeatBackup;
    }

    for ( auto &it : tree ) {
        string paraName = it.first;
        if ( paraName == "setup" ) {
            sourceSetup( events, it.second, parameters, groups, combinedPARA, type );
        }
    }
}


void Initializer::initializeBaselines() noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "initialize baselines";
#endif
    // get baseline tree
    const auto &PARA_baseline_o = xml_.get_child_optional( "VieSchedpp.baseline" );
    if ( PARA_baseline_o.is_initialized() ) {
        const auto &PARA_baseline = PARA_baseline_o.get();

        // get all defined baseline groups
        blGroups_ = readGroups( PARA_baseline, MemberType::baseline );

        // get all defined parameters
        const auto &para_tree = PARA_baseline.get_child( "parameters" );
        unordered_map<std::string, ParameterSettings::ParametersBaselines> parameters;
        for ( auto &it : para_tree ) {
            string name = it.first;
            if ( name == "parameter" ) {
                string parameterName = it.second.get_child( "<xmlattr>.name" ).data();

                ParameterSettings::ParametersBaselines PARA;

                auto PARA_ = ParameterSettings::ptree2parameterBaseline( it.second );
                PARA = PARA_.second;

                parameters[parameterName] = PARA;
            }
        }

        // define backup parameter
        Baseline::Parameters parentPARA( "backup" );
#ifdef VIESCHEDPP_LOG
        if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "create backup baseline parameters";
#endif
        // set observation mode band names
        for ( const auto &any : ObservingMode::bands ) {
            const string &name = any;
            parentPARA.minSNR[name] = ObservingMode::minSNR[name];
        }

        // store events for each baseline
        vector<vector<Baseline::Event>> events( network_.getNBls() );

        // create default events at start and end
        for ( int i = 0; i < network_.getNBls(); ++i ) {
            Baseline::Event newEvent_start( 0, false, parentPARA );
            events[i].push_back( newEvent_start );
            Baseline::Event newEvent_end( TimeSystem::duration, true, parentPARA );
            events[i].push_back( newEvent_end );
        }

        // add setup for all baselines
        for ( auto &it : PARA_baseline ) {
            string name = it.first;
            if ( name == "setup" ) {
                baselineSetup( events, it.second, parameters, blGroups_, parentPARA );
            }
        }
        // set events for all baselines
        for ( unsigned int i = 0; i < network_.getNBls(); ++i ) {
            Baseline &bl = network_.refBaseline( i );
            bl.setEVENTS( events[i] );
#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "set events for baseline " << bl.getName();
#endif
        }

        // set to start event
        for ( auto &thisBl : network_.refBaselines() ) {
            bool dummy = false;
            thisBl.checkForNewEvent( 0, dummy );
        }

    } else {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( fatal ) << "cannot read <baseline> block in VieSchedpp.xml file";
        terminate();
#else
        cout << "cannot read <baseline> block in VieSchedpp.xml file";
#endif
    }
}


void Initializer::baselineSetup( vector<vector<Baseline::Event>> &events, const boost::property_tree::ptree &tree,
                                 const unordered_map<std::string, ParameterSettings::ParametersBaselines> &parameters,
                                 const unordered_map<std::string, std::vector<std::string>> &groups,
                                 const Baseline::Parameters &parentPARA ) noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "baseline setup";
#endif
    vector<string> members;
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace )
        BOOST_LOG_TRIVIAL( trace ) << "creating new baseline parameter " << tree.get<string>( "parameter" );
#endif
    Baseline::Parameters combinedPARA = Baseline::Parameters( tree.get<string>( "parameter" ) );
    combinedPARA.setParameters( parentPARA );
    unsigned int start = 0;
    unsigned int end = TimeSystem::duration;
    bool smoothTransition = true;

    for ( auto &it : tree ) {
        string paraName = it.first;
        if ( paraName == "member" || paraName == "group" ) {
            string tmp = it.second.data();
            if ( tmp == "__all__"){
                for ( const auto &any : network_.getBaselines() ) {
                    members.push_back( any.getName() );
                }
            }else{
                if ( groups.find( tmp ) != groups.end() ){
                    members = groups.at( tmp );
                }else{
                    members.push_back( tmp );
                }
            }
        } else if ( paraName == "parameter" ) {
            const string &tmp = it.second.data();

            ParameterSettings::ParametersBaselines newPARA = parameters.at( tmp );
            if ( newPARA.ignore.is_initialized() ) {
                combinedPARA.ignore = *newPARA.ignore;
            }
            if ( newPARA.weight.is_initialized() ) {
                combinedPARA.weight = *newPARA.weight;
            }
            if ( newPARA.minScan.is_initialized() ) {
                combinedPARA.minScan = *newPARA.minScan;
            }
            if ( newPARA.maxScan.is_initialized() ) {
                combinedPARA.maxScan = *newPARA.maxScan;
            }
            if ( !newPARA.minSNR.empty() ) {
                for ( const auto &any : newPARA.minSNR ) {
                    string name = any.first;
                    double value = any.second;
                    combinedPARA.minSNR[name] = value;
                }
            }

        } else if ( paraName == "start" ) {
            string t = it.second.get_value<string>();
            boost::posix_time::ptime thisStartTime = TimeSystem::string2ptime( t );
            start = TimeSystem::posixTime2InternalTime( thisStartTime );
        } else if ( paraName == "end" ) {
            string t = it.second.get_value<string>();
            boost::posix_time::ptime thisEndTime = TimeSystem::string2ptime( t );
            end = TimeSystem::posixTime2InternalTime( thisEndTime );
        } else if ( paraName == "transition" ) {
            string tmp = it.second.data();
            if ( tmp == "hard" ) {
                smoothTransition = false;
            } else if ( tmp == "smooth" ) {
                smoothTransition = true;
            } else {
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( warning ) << "unknown transition type in <baseline><setup> block -> set to 'smooth'";
#else
                cout << "[warning] unknown transition type in <baseline><setup> block -> set to 'soft'";
#endif
            }
        }
    }
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << boost::format( "  baseline setup '%s' (%d members) parameter '%s' \n" ) %
                                     tree.get<string>( "member" ) % members.size() % tree.get<string>( "parameter" );
#else
    cout << boost::format( "[info]   baseline setup %s (%d members) parameter %s \n" ) % tree.get<string>( "member" ) %
                members.size() % tree.get<string>( "parameter" );
#endif

    vector<string> blNames;
    for ( const auto &any : network_.getBaselines() ) {
        blNames.push_back( any.getName() );
    }

    for ( auto &any : members ) {
        auto it = find( blNames.begin(), blNames.end(), any );
        if ( it == blNames.end() ) {
            any = any.substr( 3, 2 ) + "-" + any.substr( 0, 2 );
            it = find( blNames.begin(), blNames.end(), any );
        }
        if ( it == blNames.end() ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( warning ) << "baseline " << any << " not found but defined in setup -> ignoring setup";
#else
            cout << "[warning] baseline " << any << " not found but defined in setup -> ignoring setup";
#endif
            continue;
        }
        long id = distance( blNames.begin(), it );
        auto &thisEvents = events[id];

        Baseline::Event newEvent_start( start, smoothTransition, combinedPARA );

        for ( auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit ) {
            if ( iit->time > newEvent_start.time ) {
                thisEvents.insert( iit, newEvent_start );
#ifdef VIESCHEDPP_LOG
                if ( Flags::logTrace )
                    BOOST_LOG_TRIVIAL( trace ) << "insert event for baseline " << network_.getBaseline( id ).getName();
#endif
                break;
            }
        }

        Baseline::Event newEvent_end( end, true, parentPARA );
        for ( auto iit = thisEvents.begin(); iit < thisEvents.end(); ++iit ) {
            if ( iit->time >= newEvent_end.time ) {
                thisEvents.insert( iit, newEvent_end );
#ifdef VIESCHEDPP_LOG
                if ( Flags::logTrace )
                    BOOST_LOG_TRIVIAL( trace ) << "insert event for baseline " << network_.getBaseline( id ).getName();
#endif
                break;
            }
        }
    }

    for ( auto &it : tree ) {
        string paraName = it.first;
        if ( paraName == "setup" ) {
            baselineSetup( events, it.second, parameters, groups, combinedPARA );
        }
    }
}


void Initializer::initializeAstronomicalParameteres() noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "initialize astronomical parameters";
#endif
    // earth velocity
    double date1 = 2400000.5;
    double date2 = TimeSystem::mjdStart + static_cast<double>( TimeSystem::duration ) / 2 / 86400;
    double pvh[2][3];
    double pvb[2][3];
    iauEpv00( date1, date2, pvh, pvb );
    double aud2ms = DAU / 86400.0;
    double vearth[3] = { aud2ms * pvb[1][0], aud2ms * pvb[1][1], aud2ms * pvb[1][2] };
    AstronomicalParameters::earth_velocity = { vearth[0], vearth[1], vearth[2] };

    // earth nutation
    vector<unsigned int> nut_t;
    vector<double> nut_x;
    vector<double> nut_y;
    vector<double> nut_s;

    date2 = TimeSystem::mjdStart;

    unsigned int counter = 0;
    unsigned int frequency = 3600;
    unsigned int refTime;

    do {
        refTime = counter * frequency;
        date2 = TimeSystem::mjdStart + static_cast<double>( refTime ) / 86400;

        double x, y, s;
        iauXys06a( date1, date2, &x, &y, &s );
        nut_t.push_back( refTime );
        nut_x.push_back( x );
        nut_y.push_back( y );
        nut_s.push_back( s );
        ++counter;
    } while ( refTime < TimeSystem::duration + 3600 );

    AstronomicalParameters::earth_nutX = nut_x;
    AstronomicalParameters::earth_nutY = nut_y;
    AstronomicalParameters::earth_nutS = nut_s;
    AstronomicalParameters::earth_nutTime = nut_t;

    counter = 0;
    do {
        refTime = counter * frequency;
        // sunPosition
        double mjd = TimeSystem::mjdStart + static_cast<double>( refTime ) / 86400.0;
        // NUMBER OF DAYS SINCE J2000.0
        double days = mjd - 51544.5;
        // MEAN SOLAR LONGITUDE
        double slon = 280.460 + 0.9856474 * days;
        slon = fmod( slon, 360 );
        if ( slon < 1.0e-3 ) {
            slon = slon + 360.0;
        }
        // MEAN ANOMALY OF THE SUN
        double sanom = 357.528 + 0.9856003 * days;
        sanom = sanom * pi / 180.0;
        sanom = fmod( sanom, 2 * pi );
        if ( sanom < 1.0e-3 ) {
            sanom = sanom + 2 * pi;
        }
        // ECLIPTIC LONGITUDE AND OBLIQUITY OF THE ECLIPTIC
        double ecllon = slon + 1.915 * sin( sanom ) + 0.020 * sin( 2 * sanom );
        ecllon = ecllon * pi / 180.0;
        ecllon = fmod( ecllon, 2 * pi );
        double quad = ecllon / ( 0.5 * pi );
        double iquad = floor( 1 + quad );
        double obliq = 23.439 - 0.0000004 * days;
        obliq = obliq * pi / 180.0;
        // RIGHT ASCENSION AND DECLINATION
        // (RA IS IN SAME QUADRANT AS ECLIPTIC LONGITUDE)
        double sunra = atan( cos( obliq ) * tan( ecllon ) );
        if ( iquad == 2 ) {
            sunra = sunra + pi;
        } else if ( iquad == 3 ) {
            sunra = sunra + pi;
        } else if ( iquad == 4 ) {
            sunra = sunra + 2 * pi;
        }
        double sunde = asin( sin( obliq ) * sin( ecllon ) );
        AstronomicalParameters::sun_ra.push_back( sunra );
        AstronomicalParameters::sun_dec.push_back( sunde );
        AstronomicalParameters::sun_time.push_back( refTime );
        ++counter;
    } while ( refTime < TimeSystem::duration + 3600 );
}


void Initializer::initializeWeightFactors() noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "initialize weight factors";
#endif
    WeightFactors::weightSkyCoverage = xml_.get<double>( "VieSchedpp.weightFactor.skyCoverage", 0 );
    WeightFactors::weightNumberOfObservations = xml_.get<double>( "VieSchedpp.weightFactor.numberOfObservations", 0 );
    WeightFactors::weightDuration = xml_.get<double>( "VieSchedpp.weightFactor.duration", 0 );

    WeightFactors::weightAverageSources = xml_.get<double>( "VieSchedpp.weightFactor.averageSources", 0 );
    WeightFactors::weightAverageStations = xml_.get<double>( "VieSchedpp.weightFactor.averageStations", 0 );
    WeightFactors::weightAverageBaselines = xml_.get<double>( "VieSchedpp.weightFactor.averageBaselines", 0 );

    WeightFactors::weightIdleTime = xml_.get<double>( "VieSchedpp.weightFactor.idleTime", 0 );
    WeightFactors::idleTimeInterval = xml_.get<unsigned int>( "VieSchedpp.weightFactor.idleTimeInterval", 0 );

    WeightFactors::weightClosures = xml_.get<double>( "VieSchedpp.weightFactor.closures", 0 );
    WeightFactors::maxClosures = xml_.get<unsigned int>( "VieSchedpp.weightFactor.closures_max", 0 );

    WeightFactors::weightDeclination = xml_.get<double>( "VieSchedpp.weightFactor.weightDeclination", 0 );
    WeightFactors::declinationStartWeight =
        xml_.get<double>( "VieSchedpp.weightFactor.declinationStartWeight", 0 ) * deg2rad;
    WeightFactors::declinationFullWeight =
        xml_.get<double>( "VieSchedpp.weightFactor.declinationFullWeight", 0 ) * deg2rad;

    WeightFactors::weightLowElevation = xml_.get<double>( "VieSchedpp.weightFactor.weightLowElevation", 0 );
    WeightFactors::lowElevationStartWeight =
        xml_.get<double>( "VieSchedpp.weightFactor.lowElevationStartWeight", 30 ) * deg2rad;
    WeightFactors::lowElevationFullWeight =
        xml_.get<double>( "VieSchedpp.weightFactor.lowElevationFullWeight", 20 ) * deg2rad;
}


void Initializer::initializeSkyCoverages() noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "initialize sky coverage";
#endif
    const auto &tree = xml_.get_child_optional( "VieSchedpp.skyCoverage" );

    map<string, string> sta2id;
    if ( tree.is_initialized() && tree->get_child_optional( "individual" ).is_initialized() ) {
        const auto &indtree = tree->get_child( "individual" );
        for ( const auto &any : indtree ) {
            string id = any.second.get( "<xmlattr>.ID", "" );
            for ( const auto &any2 : any.second.get_child( "stations" ) ) {
                string name = any2.second.data();
                sta2id[name] = id;
            }
            double dist = any.second.get( "influence_distance", 30.0 );
            double time = any.second.get( "influence_time", 3600.0 );
            auto distType = SkyCoverage::str2interpolation( any.second.get( "distance_function", "cosine" ) );
            auto timeType = SkyCoverage::str2interpolation( any.second.get( "time_function", "cosine" ) );

            network_.addSkyCoverage( id, dist, time, distType, timeType );
        }
        network_.connectSkyCoverageWithStation( sta2id );
    } else if ( tree.is_initialized() ) {
        double distance = tree->get( "maxTwinTelecopeDistance", 0.0 );
        double dist = tree->get( "influenceDistance", 30.0 );
        double time = tree->get( "influenceInterval", 3600.0 );
        SkyCoverage::Interpolation distType =
            SkyCoverage::str2interpolation( tree->get( "interpolationDistance", "cosine" ) );
        SkyCoverage::Interpolation timeType =
            SkyCoverage::str2interpolation( tree->get( "interpolationTime", "cosine" ) );

        network_.addSkyCoverages( distance, dist, time, distType, timeType );
        //        for(auto &ref : network_.refSkyCoverages()) {
        //            ref.setInfluenceDistance( dist );
        //            ref.setInfluenceTime( time );
        //            ref.setInterpolationDistance( distType );
        //            ref.setInterpolationTime( timeType );
        //        }
    } else {
        double distance = 0.0;
        double dist = 30.0;
        double time = 3600.0;
        SkyCoverage::Interpolation distType = SkyCoverage::Interpolation::cosine;
        SkyCoverage::Interpolation timeType = SkyCoverage::Interpolation::cosine;

        network_.addSkyCoverages( distance, dist, time, distType, timeType );
    }
}

void Initializer::initializeSites() noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "initialize sites";
#endif
    const auto &tree = xml_.get_child_optional( "VieSchedpp.sites" );

    map<unsigned long, int> sta2id;
    map<string, int> siteId2intId;
    int nextid = 0;
    if ( tree.is_initialized() ) {
        for ( const auto &any : *tree ) {
            if ( any.first != "site" ) {
                continue;
            }
            string id = any.second.get( "<xmlattr>.ID", "" );
            for ( const auto &any2 : any.second ) {
                if ( any2.first != "station" ) {
                    continue;
                }
                string name = any2.second.data();
                unsigned long staid = network_.getStation( name ).getId();
                int intid;
                if ( siteId2intId.find( id ) != siteId2intId.end() ) {
                    intid = siteId2intId[id];
                } else {
                    intid = nextid;
                    siteId2intId[id] = intid;
                    ++nextid;
                }
                sta2id[staid] = intid;
            }
        }
        Network::addSites( sta2id );
    } else {
        for ( const auto &any : network_.getStations() ) {
            sta2id[any.getId()] = nextid;
            ++nextid;
        }
        Network::addSites( sta2id );
    }
}

//    SkyCoverage::maxInfluenceDistance = xml_.get<double>( "VieSchedpp.skyCoverage.influenceDistance", 30 ) * deg2rad;
//    SkyCoverage::maxInfluenceTime = xml_.get<double>( "VieSchedpp.skyCoverage.influenceInterval", 3600 );
//
//    string interpolationDistance = xml_.get<string>( "VieSchedpp.skyCoverage.interpolationDistance", "linear" );
//    if ( interpolationDistance == "constant" ) {
//        SkyCoverage::interpolationDistance = SkyCoverage::Interpolation::constant;
//    } else if ( interpolationDistance == "linear" ) {
//        SkyCoverage::interpolationDistance = SkyCoverage::Interpolation::linear;
//    } else if ( interpolationDistance == "cosine" ) {
//        SkyCoverage::interpolationDistance = SkyCoverage::Interpolation::cosine;
//    }
//
//    string interpolationTime = xml_.get<string>( "VieSchedpp.skyCoverage.interpolationTime", "linear" );
//    if ( interpolationTime == "constant" ) {
//        SkyCoverage::interpolationTime = SkyCoverage::Interpolation::constant;
//    } else if ( interpolationTime == "linear" ) {
//        SkyCoverage::interpolationTime = SkyCoverage::Interpolation::linear;
//    } else if ( interpolationTime == "cosine" ) {
//        SkyCoverage::interpolationTime = SkyCoverage::Interpolation::cosine;
//    }
//


void Initializer::initializeObservingMode( const SkdCatalogReader &skdCatalogs, ofstream &of ) noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "initialize observing mode";
#endif
    auto PARA_mode = xml_.get_child( "VieSchedpp.mode" );
    for ( const auto &it : PARA_mode ) {
        if ( it.first == "skdMode" ) {
#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "skd observing mode found";
#endif

            obsModes_ = std::make_shared<ObservingMode>();
            obsModes_->readFromSkedCatalogs( skdCatalogs );

            ObservingMode::type = ObservingMode::Type::sked;

            unordered_map<string, ObservingMode::Property> stationProperty;
            unordered_map<string, ObservingMode::Backup> stationBackup;
            unordered_map<string, double> stationBackupValue;

            unordered_map<string, ObservingMode::Property> sourceProperty;
            unordered_map<string, ObservingMode::Backup> sourceBackup;
            unordered_map<string, double> sourceBackupValue;

            for ( const auto &any : ObservingMode::bands ) {
                stationProperty[any] = ObservingMode::Property::required;
                sourceProperty[any] = ObservingMode::Property::required;
                stationBackup[any] = ObservingMode::Backup::none;
                sourceBackup[any] = ObservingMode::Backup::none;
                stationBackupValue[any] = 0;
                sourceBackupValue[any] = 0;
            }

            ObservingMode::stationProperty = stationProperty;
            ObservingMode::stationBackup = stationBackup;
            ObservingMode::stationBackupValue = stationBackupValue;

            ObservingMode::sourceProperty = sourceProperty;
            ObservingMode::sourceBackup = sourceBackup;
            ObservingMode::sourceBackupValue = sourceBackupValue;

        } else if ( it.first == "simple" ) {
            ObservingMode::type = ObservingMode::Type::simple;

            auto samplerate = it.second.get<double>( "sampleRate" );
            auto bits = it.second.get<unsigned int>( "bits" );

            std::unordered_map<string, unsigned int> band2channel;
            std::unordered_map<string, double> band2wavelength;

            for ( const auto &band : it.second.get_child( "bands" ) ) {
                auto wavelength = band.second.get<double>( "wavelength" );
                auto channels = band.second.get<unsigned int>( "channels" );

                string name = band.second.get<string>( "<xmlattr>.name" );

                band2channel[name] = channels;
                band2wavelength[name] = wavelength;
            }
            double efficiencyFactor = it.second.get<double>( "efficiencyFactor", -1.0 );

            obsModes_ = std::make_shared<ObservingMode>();
            obsModes_->simpleMode( util::getNumberOfStations( xml_ ), samplerate, bits, band2channel, band2wavelength,
                                   efficiencyFactor );

#ifdef VIESCHEDPP_LOG
            if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "sample rate set to " << it.second.get_value<double>();
            if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "bits set to " << it.second.get_value<unsigned int>();
            for ( const auto &any : band2channel ) {
                if ( Flags::logTrace )
                    BOOST_LOG_TRIVIAL( trace ) << "band " << any.first << " mean wavelenght "
                                               << band2wavelength[any.first] << " [MHz] channels " << any.second;
            }
#endif

        } else if ( it.first == "custom" ) {
            ObservingMode::type = ObservingMode::Type::custom;

            ObservingMode om( it.second, skdCatalogs.getStaNames() );
            obsModes_ = std::make_shared<ObservingMode>( it.second, skdCatalogs.getStaNames() );

        } else if ( it.first == "bandPolicies" ) {
            for ( const auto &it_bandPolicies : it.second ) {
                ObservingMode::Property station_property = ObservingMode::Property::required;
                ObservingMode::Backup station_backup = ObservingMode::Backup::none;
                double station_backupValue = 0;
                ObservingMode::Property source_property = ObservingMode::Property::required;
                ObservingMode::Backup source_backup = ObservingMode::Backup::none;
                double source_backupValue = 0;
                double minSNR = 0;
                string name;

                for ( const auto &it_bandPolicy : it_bandPolicies.second ) {
                    if ( it_bandPolicy.first == "<xmlattr>" ) {
                        name = it_bandPolicy.second.get_child( "name" ).data();
#ifdef VIESCHEDPP_LOG
                        if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "setting band policy for '" << name << "'";
#endif

                    } else if ( it_bandPolicy.first == "minSNR" ) {
                        minSNR = it_bandPolicy.second.get_value<double>();
                    } else if ( it_bandPolicy.first == "station" ) {
                        for ( const auto &it_band_station : it_bandPolicy.second ) {
                            string thisName = it_band_station.first;
                            if ( thisName == "tag" ) {
                                if ( it_band_station.second.get_value<std::string>() == "required" ) {
                                    station_property = ObservingMode::Property::required;
                                } else if ( it_band_station.second.get_value<std::string>() == "optional" ) {
                                    station_property = ObservingMode::Property::optional;
                                }
                            } else if ( thisName == "backup_maxValueTimes" ) {
                                station_backup = ObservingMode::Backup::maxValueTimes;
                                station_backupValue = it_band_station.second.get_value<double>();

                            } else if ( thisName == "backup_minValueTimes" ) {
                                station_backup = ObservingMode::Backup::minValueTimes;
                                station_backupValue = it_band_station.second.get_value<double>();

                            } else if ( thisName == "backup_value" ) {
                                station_backup = ObservingMode::Backup::value;
                                station_backupValue = it_band_station.second.get_value<double>();
                            }
                        }
                    } else if ( it_bandPolicy.first == "source" ) {
                        for ( const auto &it_band_source : it_bandPolicy.second ) {
                            string thisName = it_band_source.first;
                            if ( thisName == "tag" ) {
                                if ( it_band_source.second.get_value<std::string>() == "required" ) {
                                    source_property = ObservingMode::Property::required;
                                } else if ( it_band_source.second.get_value<std::string>() == "optional" ) {
                                    source_property = ObservingMode::Property::optional;
                                }
                            } else if ( thisName == "backup_internalModel" ) {
                                source_backup = ObservingMode::Backup::internalModel;

                            } else if ( thisName == "backup_maxValueTimes" ) {
                                source_backup = ObservingMode::Backup::maxValueTimes;
                                source_backupValue = it_band_source.second.get_value<double>();

                            } else if ( thisName == "backup_minValueTimes" ) {
                                source_backup = ObservingMode::Backup::minValueTimes;
                                source_backupValue = it_band_source.second.get_value<double>();

                            } else if ( thisName == "backup_value" ) {
                                source_backup = ObservingMode::Backup::value;
                                source_backupValue = it_band_source.second.get_value<double>();
                            }
                        }
                    }
                }
                ObservingMode::minSNR[name] = minSNR;

                ObservingMode::stationProperty[name] = station_property;
                ObservingMode::stationBackup[name] = station_backup;
                ObservingMode::stationBackupValue[name] = station_backupValue;

                ObservingMode::sourceProperty[name] = source_property;
                ObservingMode::sourceBackup[name] = source_backup;
                ObservingMode::sourceBackupValue[name] = source_backupValue;
            }
        }
    }

#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << boost::format( "observing mode: %s" ) % obsModes_->getMode( 0 )->getName();
#else
    cout << boost::format( "[info] observing mode: %s" ) % obsModes_->getMode( 0 )->getName();
#endif

    of << "\n";
}


void Initializer::initializeObservingMode( unsigned long nsta, double samplerate, unsigned int bits,
                                           const std::unordered_map<std::string, unsigned int> &band2channel,
                                           const std::unordered_map<std::string, double> &band2wavelength,
                                           double efficiencyFactor ) noexcept {
    ObservingMode::type = ObservingMode::Type::simple;
    obsModes_ = std::make_shared<ObservingMode>();
    obsModes_->simpleMode( nsta, samplerate, bits, band2channel, band2wavelength );
}


unordered_map<string, vector<string>> Initializer::readGroups( boost::property_tree::ptree root,
                                                               MemberType type ) noexcept {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "create groups";
#endif
    unordered_map<std::string, std::vector<std::string>> groups;
    auto groupTree = root.get_child_optional( "groups" );
    if ( groupTree.is_initialized() ) {
        for ( auto &it : *groupTree ) {
            string name = it.first;
            if ( name == "group" ) {
                string groupName = it.second.get_child( "<xmlattr>.name" ).data();
#ifdef VIESCHEDPP_LOG
                if ( Flags::logTrace ) BOOST_LOG_TRIVIAL( trace ) << "create new group " << groupName;
#endif
                std::vector<std::string> members;
                for ( auto &it2 : it.second ) {
                    if ( it2.first == "member" ) {
                        members.push_back( it2.second.data() );
                    }
                }
                groups[groupName] = members;
            }
        }
    }

    switch ( type ) {
        case MemberType::source: {
            std::vector<std::string> members;
            for ( int i = 0; i < sourceList_.getNQuasars(); ++i ) {
                const auto &src = sourceList_.getQuasar( i );
                members.push_back( src->getName() );
            }
            groups["__all__"] = members;
            groups["__AGNs__"] = members;
            break;
        }
        case MemberType::satellite: {
            std::vector<std::string> members;
            for ( int i = 0; i < sourceList_.getNSatellites(); ++i ) {
                const auto &src = sourceList_.getSatellite( i );
                members.push_back( src->getName() );
            }
            groups["__all__"] = members;
            groups["__satellites__"] = members;
            break;
        }
        case MemberType::spacecraft: {
            break;
        }
        case MemberType::station: {
            std::vector<std::string> members;
            for ( const auto &any : network_.getStations() ) {
                members.push_back( any.getName() );
            }
            groups["__all__"] = members;
            groups["__spacecrafts__"] = members;
            break;
        }
        case MemberType::baseline: {
            std::vector<std::string> members;
            for ( const auto &any : network_.getBaselines() ) {
                members.push_back( any.getName() );
            }
            groups["__all__"] = members;
            break;
        }
    }

    return groups;
}


void Initializer::applyMultiSchedParameters(const VieVS::MultiScheduling::Parameters &parameters, int version) {
    //    parameters.output(bodyLog);
#ifdef VIESCHEDPP_LOG
    if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "apply multi scheduling parameters";
#endif
    version_ = version;

    //    Initializer copyOfInit(*this);
    multiSchedulingParameters_ = parameters;

    unsigned long nsta = network_.getNSta();

    // GENERAL
    if ( parameters.start.is_initialized() ) {
        //        boost::posix_time::ptime startTime = *parameters.start;
        //        int sec_ = startTime.time_of_day().total_seconds();
        //        double mjdStart = startTime.date().modjulian_day() + sec_ / 86400;
        //
        //        boost::posix_time::ptime endTime = startTime + boost::posix_time::seconds(
        //                static_cast<long>(TimeSystem::duration));
        //
        //        TimeSystem::mjdStart = mjdStart;
        //        TimeSystem::startTime = startTime;
        //        TimeSystem::endTime = endTime;
    }
    if ( parameters.subnetting.is_initialized() ) {
        parameters_.subnetting = *parameters.subnetting;
    }
    if ( parameters.subnetting_minSourceAngle.is_initialized() ) {
        parameters_.subnettingMinAngle = *parameters.subnetting_minSourceAngle;
    }
    if ( parameters.subnetting_minParticipatingStations.is_initialized() ) {
        //        parameters_.subnettingMinNSta = *parameters.subnetting_minParticipatingStations;
    }
    if ( parameters.fillinmode_duringScanSelection.is_initialized() ) {
        parameters_.fillinmodeDuringScanSelection = *parameters.fillinmode_duringScanSelection;
    }
    if ( parameters.fillinmode_influenceOnScanSelection.is_initialized() ) {
        parameters_.fillinmodeDuringScanSelection = *parameters.fillinmode_influenceOnScanSelection;
    }
    if ( parameters.fillinmode_aPosteriori.is_initialized() ) {
        parameters_.fillinmodeAPosteriori = *parameters.fillinmode_aPosteriori;
    }
    if ( parameters.focusCornerSwitchCadence.is_initialized() ) {
        FocusCorners::flag = true;
        unsigned int interval = *parameters.focusCornerSwitchCadence;
        FocusCorners::interval = interval;
    }

    // WEIGHT FACTORS
    if ( parameters.weightSkyCoverage.is_initialized() ) {
        WeightFactors::weightSkyCoverage = *parameters.weightSkyCoverage;
    }
    if ( parameters.weightNumberOfObservations.is_initialized() ) {
        WeightFactors::weightNumberOfObservations = *parameters.weightNumberOfObservations;
    }
    if ( parameters.weightDuration.is_initialized() ) {
        WeightFactors::weightDuration = *parameters.weightDuration;
    }
    if ( parameters.weightAverageSources.is_initialized() ) {
        WeightFactors::weightAverageSources = *parameters.weightAverageSources;
    }
    if ( parameters.weightAverageStations.is_initialized() ) {
        WeightFactors::weightAverageStations = *parameters.weightAverageStations;
    }
    if ( parameters.weightAverageBaselines.is_initialized() ) {
        WeightFactors::weightAverageBaselines = *parameters.weightAverageBaselines;
    }
    if ( parameters.weightIdleTime.is_initialized() ) {
        WeightFactors::weightIdleTime = *parameters.weightIdleTime;
    }
    if ( parameters.weightIdleTime_interval.is_initialized() ) {
        WeightFactors::idleTimeInterval = static_cast<unsigned int>( *parameters.weightIdleTime_interval );
    }
    if ( parameters.weightClosures.is_initialized() ) {
        WeightFactors::weightClosures = *parameters.weightClosures;
    }
    if ( parameters.weightMaxClosures.is_initialized() ) {
        WeightFactors::maxClosures = static_cast<unsigned int>( *parameters.weightMaxClosures );
    }
    if ( parameters.weightLowDeclination.is_initialized() ) {
        WeightFactors::weightDeclination = *parameters.weightLowDeclination;
    }
    if ( parameters.weightLowDeclination_begin.is_initialized() ) {
        WeightFactors::declinationStartWeight = *parameters.weightLowDeclination_begin * deg2rad;
    }
    if ( parameters.weightLowDeclination_full.is_initialized() ) {
        WeightFactors::declinationFullWeight = *parameters.weightLowDeclination_full * deg2rad;
    }
    if ( parameters.weightLowElevation.is_initialized() ) {
        WeightFactors::weightLowElevation = *parameters.weightLowElevation;
    }
    if ( parameters.weightLowElevation_begin.is_initialized() ) {
        WeightFactors::lowElevationStartWeight = *parameters.weightLowElevation_begin * deg2rad;
    }
    if ( parameters.weightLowElevation_full.is_initialized() ) {
        WeightFactors::lowElevationFullWeight = *parameters.weightLowElevation_full * deg2rad;
    }

    // SKY COVERAGE
    if ( !parameters.skyCoverageInfluenceDistance.empty() ) {
        for ( const auto &any : parameters.skyCoverageInfluenceDistance ) {
            string name = any.first;
            for ( auto &sky : network_.refSkyCoverages() ) {
                if ( sky.hasName( name ) ) {
                    sky.setInfluenceDistance( any.second );
                }
            }
        }
    }
    if ( !parameters.skyCoverageInfluenceTime.empty() ) {
        for ( const auto &any : parameters.skyCoverageInfluenceTime ) {
            string name = any.first;
            for ( auto &sky : network_.refSkyCoverages() ) {
                if ( sky.hasName( name ) ) {
                    sky.setInfluenceTime( any.second );
                }
            }
        }
    }

    // STATION
    if ( !parameters.stationWeight.empty() ) {
        for ( const auto &any : parameters.stationWeight ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, network_.getStations() );
            for ( auto id : ids ) {
                for ( auto &ev : network_.refStation( id ).refParaForMultiScheduling() ) {
                    ev.PARA.weight = any.second;
                }
            }
        }
    }
    if ( !parameters.stationMinSlewtime.empty() ) {
        for ( const auto &any : parameters.stationMinSlewtime ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, network_.getStations() );
            for ( auto id : ids ) {
                for ( auto & ev : network_.refStation( id ).refParaForMultiScheduling() ) {
                    ev.PARA.minSlewtime = any.second;
                }
            }
        }
    }
    if ( !parameters.stationMaxSlewtime.empty() ) {
        for ( const auto &any : parameters.stationMaxSlewtime ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, network_.getStations() );
            for ( auto id : ids ) {
                for ( auto & ev : network_.refStation( id ).refParaForMultiScheduling() ) {
                    ev.PARA.maxSlewtime = any.second;
                }
            }
        }
    }
    if ( !parameters.stationMaxSlewDistance.empty() ) {
        for ( const auto &any : parameters.stationMaxSlewDistance ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, network_.getStations() );
            for ( auto id : ids ) {
                for ( auto & ev : network_.refStation( id ).refParaForMultiScheduling() ) {
                    ev.PARA.maxSlewDistance = any.second;
                }
            }
        }
    }
    if ( !parameters.stationMinSlewDistance.empty() ) {
        for ( const auto &any : parameters.stationMinSlewDistance ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, network_.getStations() );
            for ( auto id : ids ) {
                for ( auto & ev : network_.refStation( id ).refParaForMultiScheduling() ) {
                    ev.PARA.minSlewDistance = any.second;
                }
            }
        }
    }
    if ( !parameters.stationMaxWait.empty() ) {
        for ( const auto &any : parameters.stationMaxWait ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, network_.getStations() );
            for ( auto id : ids ) {
                for ( auto & ev : network_.refStation( id ).refParaForMultiScheduling() ) {
                    ev.PARA.maxWait = any.second;
                }
            }
        }
    }
    if ( !parameters.stationMinElevation.empty() ) {
        for ( const auto &any : parameters.stationMinElevation ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, network_.getStations() );
            for ( auto id : ids ) {
                for ( auto & ev : network_.refStation( id ).refParaForMultiScheduling() ) {
                    ev.PARA.minElevation = any.second;
                }
            }
        }
    }
    if ( !parameters.stationMaxNumberOfScans.empty() ) {
        for ( const auto &any : parameters.stationMaxNumberOfScans ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, network_.getStations() );
            for ( auto id : ids ) {
                for ( auto &ev : network_.refStation( id ).refParaForMultiScheduling() ) {
                    ev.PARA.maxNumberOfScans = any.second;
                }
            }
        }
    }
    if ( !parameters.stationMaxNumberOfScansDist.empty() ) {
        for ( const auto &any : parameters.stationMaxNumberOfScansDist ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, network_.getStations() );
            for ( auto id : ids ) {
                auto &evs = network_.refStation( id ).refParaForMultiScheduling();
                for ( int i = 1; i < evs.size() - 1; ++i ) {
                    evs[i].PARA.maxNumberOfScansDist = any.second;
                }
            }
        }
    }
    if ( !parameters.stationMaxScan.empty() ) {
        for ( const auto &any : parameters.stationMaxScan ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, network_.getStations() );
            for ( auto id : ids ) {
                for ( auto &ev : network_.refStation( id ).refParaForMultiScheduling() ) {
                    ev.PARA.maxScan = any.second;
                }
            }
        }
    }
    if ( !parameters.stationMinScan.empty() ) {
        for ( const auto &any : parameters.stationMinScan ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, network_.getStations() );
            for ( auto id : ids ) {
                for ( auto & ev : network_.refStation( id ).refParaForMultiScheduling() ) {
                    ev.PARA.minScan = any.second;
                }
            }
        }
    }

    // SOURCE
    if ( !parameters.sourceWeight.empty() ) {
        for ( const auto &any : parameters.sourceWeight ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, sourceList_ );
            for ( auto id : ids ) {
                for ( auto &ev : sourceList_.refSource( id )->refParaForMultiScheduling() ) {
                    ev.PARA.weight = any.second;
                }
            }
        }
    }
    if ( !parameters.sourceMinNumberOfSites.empty() ) {
        for ( const auto &any : parameters.sourceMinNumberOfSites ) {
            string name = any.first;
            int n = any.second;
            if ( n > Network::nSites() ) {
                n = Network::nSites();
            }
            vector<unsigned long> ids = getMembers( name, sourceList_ );
            for ( auto id : ids ) {
                for ( auto &ev : sourceList_.refSource( id )->refParaForMultiScheduling() ) {
                    ev.PARA.minNumberOfSites = n;
                }
            }
        }
    }
    if ( !parameters.sourceMinFlux.empty() ) {
        for ( const auto &any : parameters.sourceMinFlux ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, sourceList_ );
            for ( auto id : ids ) {
                for ( auto & ev : sourceList_.refSource( id )->refParaForMultiScheduling() ) {
                    ev.PARA.minFlux = any.second;
                }
            }
        }
    }
    if ( !parameters.sourceMaxNumberOfScans.empty() ) {
        for ( const auto &any : parameters.sourceMaxNumberOfScans ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, sourceList_ );
            for ( auto id : ids ) {
                for ( auto & ev : sourceList_.refSource( id )->refParaForMultiScheduling() ) {
                    ev.PARA.maxNumberOfScans = any.second;
                }
            }
        }
    }
    if ( !parameters.sourceMinElevation.empty() ) {
        for ( const auto &any : parameters.sourceMinElevation ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, sourceList_ );
            for ( auto id : ids ) {
                for ( auto & ev : sourceList_.refSource( id )->refParaForMultiScheduling() ) {
                    ev.PARA.minElevation = any.second;
                }
            }
        }
    }
    if ( !parameters.sourceMinSunDistance.empty() ) {
        for ( const auto &any : parameters.sourceMinSunDistance ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, sourceList_ );
            for ( auto id : ids ) {
                for ( auto & ev : sourceList_.refSource( id )->refParaForMultiScheduling() ) {
                    ev.PARA.minSunDistance = any.second;
                }
            }
        }
    }
    if ( !parameters.sourceMaxScan.empty() ) {
        for ( const auto &any : parameters.sourceMaxScan ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, sourceList_ );
            for ( auto id : ids ) {
                for ( auto & ev : sourceList_.refSource( id )->refParaForMultiScheduling() ) {
                    ev.PARA.maxScan = any.second;
                }
            }
        }
    }
    if ( !parameters.sourceMinScan.empty() ) {
        for ( const auto &any : parameters.sourceMinScan ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, sourceList_ );
            for ( auto id : ids ) {
                for ( auto & ev : sourceList_.refSource( id )->refParaForMultiScheduling() ) {
                    ev.PARA.minScan = any.second;
                }
            }
        }
    }
    if ( !parameters.sourceMinRepeat.empty() ) {
        for ( const auto &any : parameters.sourceMinRepeat ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, sourceList_ );
            for ( auto id : ids ) {
                for ( auto & ev : sourceList_.refSource( id )->refParaForMultiScheduling() ) {
                    ev.PARA.minRepeat = any.second;
                }
            }
        }
    }

    // BASELINES
    if ( !parameters.baselineWeight.empty() ) {
        for ( const auto &any : parameters.baselineWeight ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, network_.getBaselines() );
            for ( auto id : ids ) {
                for ( auto & ev : network_.refBaseline( id ).refParaForMultiScheduling() ) {
                    ev.PARA.weight = any.second;
                }
            }
        }
    }
    if ( !parameters.baselineMinScan.empty() ) {
        for ( const auto &any : parameters.baselineMinScan ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, network_.getBaselines() );
            for ( auto id : ids ) {
                for ( auto & ev : network_.refBaseline( id ).refParaForMultiScheduling() ) {
                    ev.PARA.minScan = any.second;
                }
            }
        }
    }
    if ( !parameters.baselineMaxScan.empty() ) {
        for ( const auto &any : parameters.baselineMaxScan ) {
            string name = any.first;
            vector<unsigned long> ids = getMembers( name, network_.getBaselines() );
            for ( auto id : ids ) {
                for ( auto & ev : network_.refBaseline( id ).refParaForMultiScheduling() ) {
                    ev.PARA.maxScan = any.second;
                }
            }
        }
    }
}


vector<MultiScheduling::Parameters> Initializer::readMultiSched( std::ostream &out ) {
    vector<MultiScheduling::Parameters> para;

    MultiScheduling ms( staGroups_, srcGroups_, blGroups_ );
    std::vector<MultiScheduling::Parameters> ms_para;
    boost::optional<boost::property_tree::ptree &> mstree_o = xml_.get_child_optional( "VieSchedpp.multisched" );
    if ( mstree_o.is_initialized() ) {
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "read multi scheduling parameters";
#endif
        boost::property_tree::ptree mstree = *mstree_o;

        MultiScheduling::setConstants( network_.getNSta(), sourceList_.getNQuasars() );

        unsigned int maxNumber = mstree.get( "maxNumber", numeric_limits<unsigned int>::max() );
        unsigned int seed = mstree.get(
            "seed", static_cast<unsigned int>( chrono::system_clock::now().time_since_epoch().count() ) % 2147483647 );
        MultiScheduling::setSeed( seed );

        for ( const auto &any : mstree ) {
            std::string name = any.first;
            if ( name == "maxNumber" || name == "seed" || name == "version" || name == "version_offset" ||
                 name == "genetic" ) {
                continue;
            }
            if ( name == "pick_random" ) {
                MultiScheduling::pick_random_values( any.second.get_value<bool>() );
                continue;
            }
            if ( name == "general_subnetting" || name == "general_fillin-mode_during_scan_selection" ||
                 name == "general_fillin-mode_influence_on_scan_selection" ||
                 name == "general_fillin-mode_a_posteriori" ) {
                ms.addParameters( name );
                continue;
            }

            vector<double> values;
            string member = any.second.get( "<xmlattr>.member", "" );
            for ( const auto &any2 : any.second ) {
                std::string name2 = any2.first;
                if ( name2 == "value" ) {
                    values.push_back( any2.second.get_value<double>() );
                }
            }
            if ( member.empty() ) {
                ms.addParameters( name, values );
            } else {
                ms.addParameters( name, member, values );
            }
        }

        ms_para = ms.createMultiScheduleParameters( maxNumber );

        if ( ms_para.size() != maxNumber ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( info ) << "multi scheduling found ... creating " << ms_para.size() << " schedules!";
#else
            cout << "[info] multi scheduling found ... creating " << ms_para.size() << " schedules!\n";
#endif
            out << "multi scheduling found ... creating " << ms_para.size() << " schedules!\n";
        } else {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( info ) << "multi scheduling found ... creating " << ms_para.size()
                                      << " schedules using this seed: " << seed << "!";
#else
            cout << "[info] multi scheduling found ... creating " << ms_para.size()
                 << " schedules using this seed: " << seed << "!\n";
#endif
            out << "multi scheduling found ... creating " << ms_para.size() << " schedules using this seed: " << seed
                << "!\n";
        }

        // only calculate single version from multi scheduling
        auto version = xml_.get_optional<int>( "VieSchedpp.multisched.version" );
        if ( version.is_initialized() && *version - 1 < ms_para.size() ) {
            ms_para = std::vector<MultiScheduling::Parameters>{ ms_para.at( *version - 1 ) };
        }
    }

    return ms_para;
}

void Initializer::initializeFocusCornersAlgorithm() noexcept {
    boost::optional<boost::property_tree::ptree &> t = xml_.get_child_optional( "VieSchedpp.focusCorners" );
    if ( t.is_initialized() ) {
        unsigned int cadence = xml_.get( "VieSchedpp.focusCorners.cadence", 900u );
        int nscans = xml_.get( "VieSchedpp.focusCorners.nscans", 1 );
        FocusCorners::flag = true;
        FocusCorners::interval = cadence;
        FocusCorners::nscans = nscans;
    } else {
        FocusCorners::flag = false;
        FocusCorners::nextStart = numeric_limits<unsigned int>::max();
    }
}

void Initializer::initializeSourceSequence() noexcept {
    boost::optional<boost::property_tree::ptree &> sq = xml_.get_child_optional( "VieSchedpp.rules.sourceSequence" );
    if ( sq.is_initialized() ) {
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "initialize source sequence";
#endif
        const auto & PARA_source = xml_.get_child( "VieSchedpp.source" );
        unordered_map<std::string, std::vector<std::string>> groups_src = readGroups( PARA_source, MemberType::source );

        const auto & PARA_sat_o = xml_.get_child_optional( "VieSchedpp.satellite" );
        unordered_map<std::string, std::vector<std::string>> groups_sat;
        if (PARA_sat_o.is_initialized()){
            groups_sat = readGroups( *PARA_sat_o, MemberType::satellite );
        }

        const auto & PARA_space_o = xml_.get_child_optional( "VieSchedpp.spacecraft" );
        unordered_map<std::string, std::vector<std::string>> groups_space;
        if (PARA_space_o.is_initialized()){
            groups_space = readGroups( *PARA_space_o, MemberType::spacecraft );
        }

        Scan::scanSequence_flag = true;
        int c = 0;
        for ( const auto &any : *sq ) {
            if ( any.first == "sequence" ) {
                string member = any.second.get_value<string>();
                bool all = false;
                vector<string> targetSources;

                if ( member == "__all__" ) {
                    all = true;
                } else if ( groups_src.find( member ) != groups_src.end() ) {
                    targetSources = groups_src[member];
                } else if ( groups_sat.find( member ) != groups_sat.end() ) {
                    targetSources = groups_sat[member];
                } else if ( groups_space.find( member ) != groups_space.end() ) {
                    targetSources = groups_space[member];
                } else if ( member == "__AGNs__" ) {
                    for ( const auto &source : sourceList_.getQuasars() ) {
                        const string &name = source->getName();
                        targetSources.push_back( name );
                    }
                } else if ( member == "__satellites__" ) {
                    for ( const auto &source : sourceList_.getSatellites() ) {
                        const string &name = source->getName();
                        targetSources.push_back( name );
                    }
                } else if ( member == "__spacecrafts__" ) {
                    //                            for ( const auto &source : sourceList_.getSpacecrafts() ) {
                    //                                const string &name = source->getName();
                    //                                targetSources.push_back( name );
                    //                            }
                } else {
                    targetSources.push_back( member );
                }
                vector<unsigned long> targetIds;

                for ( const auto &source : sourceList_.getSources() ) {
                    const string &name = source->getName();
                    if ( find( targetSources.begin(), targetSources.end(), name ) != targetSources.end() ) {
                        targetIds.push_back( source->getId() );
                    } else {
                        const string &name2 = source->getAlternativeName();
                        if ( find( targetSources.begin(), targetSources.end(), name2 ) != targetSources.end() ) {
                            targetIds.push_back( source->getId() );
                        }
                    }
                }
                if ( !all ) {
                    Scan::scanSequence_target[c] = targetIds;
                }
                ++c;
            }
        }
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( info ) << "custom scan sequence initialized";
#else
        cout << "[info] custom scan sequence initialized";
#endif

        for ( int i = 0; i < Scan::scanSequence_target.size(); ++i ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( info ) << boost::format( "  modulo %2d: %3d sources" ) % i %
                                             Scan::scanSequence_target[i].size();
#else
            cout << "[info] " << boost::format( "  modulo %2d: %3d sources" ) % i % Scan::scanSequence_target[i].size();
#endif
        }
    }
}

void Initializer::initializeCalibrationBlocks() {
    const auto &tree = xml_.get_child_optional( "VieSchedpp.rules.calibration" );


    if ( tree.is_initialized() ) {
        CalibratorBlock::subnetting = tree->get( "subnetting", false );
        CalibratorBlock::stationFlag = vector<int>( network_.getNSta(), 0 );
        CalibratorBlock::tryToIncludeAllStationFlag = tree->get( "tryToIncludeAllStations", false );
        CalibratorBlock::tryToIncludeAllStations_factor = tree->get( "tryToIncludeAllStations_factor", 3.0 );
        CalibratorBlock::numberOfObservations_factor = tree->get( "numberOfObservations_factor", 5.0 );
        CalibratorBlock::numberOfObservations_offset = tree->get( "numberOfObservations_offset", 0.0 );
        CalibratorBlock::averageStations_factor = tree->get( "averageStations_factor", 100.0 );
        CalibratorBlock::averageStations_offset = tree->get( "averageStations_offset", 1.0 );
        CalibratorBlock::duration_factor = tree->get( "duration_factor", 0.2 );
        CalibratorBlock::duration_offset = tree->get( "duration_offset", 1.0 );
        CalibratorBlock::averageBaseline_factor = tree->get( "averageBaseline_factor", 0 );
        CalibratorBlock::averageBaseline_offset = tree->get( "averageBaseline_offset", 1.0 );
        for ( const auto &any : *tree ) {
            if ( any.first == "block" ) {
                unsigned int time = any.second.get( "startTime", TimeSystem::duration );
                unsigned int scans = any.second.get( "scans", 2 );
                unsigned int duration = any.second.get( "duration", 300 );
                unsigned int overlap = any.second.get( "overlap", 2 );
                bool rigorosOverlap = any.second.get( "rigorosOverlap", false );
                string sourceGroup = any.second.get( "sources", "__all__" );
                vector<string> allowedSources;
                if ( srcGroups_.find( sourceGroup ) != srcGroups_.end() ) {
                    allowedSources = srcGroups_[sourceGroup];
                } else {
                    allowedSources.push_back( sourceGroup );
                }

                calib_.emplace_back( time, scans, duration, allowedSources, overlap, rigorosOverlap );
            }
            if ( any.first == "intent" ) {
                CalibratorBlock::intent_ = any.second.get_value<string>();
            }
        }

        const auto &dpara = xml_.get_child_optional( "VieSchedpp.rules.calibration.diffParallacticAngle" );
        if ( dpara.is_initialized() ) {
            DifferentialParallacticAngleBlock::nscans = dpara->get( "nscans", 0 );
            DifferentialParallacticAngleBlock::duration = dpara->get( "duration", 300 );
            DifferentialParallacticAngleBlock::distanceScaling = dpara->get( "distanceScaling", 2.0 );
            DifferentialParallacticAngleBlock::cadence = dpara->get( "investigationCadence", 600 );
            DifferentialParallacticAngleBlock::intent_ = dpara->get( "intent", "" );

            auto angles = dpara->get_child_optional( "angles" );
            if ( angles.is_initialized() ) {
                int counter = 0;
                for ( const auto &any : *angles ) {
                    if ( any.first == "angle" ) {
                        auto v = any.second.get_value<double>();
                        DifferentialParallacticAngleBlock::angles.push_back( v * deg2rad );
                        ++counter;
                    }
                }
                // fill missing values with 45 degrees
                for ( int i = counter; i < DifferentialParallacticAngleBlock::nscans; ++i ) {
                    DifferentialParallacticAngleBlock::angles.push_back( 45.0 * deg2rad );
                }
            } else {
                // fill empty angles with 45 degrees for backwards comperability
                for ( int i = 0; i < DifferentialParallacticAngleBlock::nscans; ++i ) {
                    DifferentialParallacticAngleBlock::angles.push_back( 45.0 * deg2rad );
                }
            }

            string sourceName = dpara->get( "sources", "__all__" );
            vector<unsigned long> srcids = getMembers( sourceName, sourceList_ );
            DifferentialParallacticAngleBlock::allowedSources = srcids;

            string baselineName = dpara->get( "baselines", "__all__" );
            vector<unsigned long> blids = getMembers( sourceName, network_.getBaselines() );
            DifferentialParallacticAngleBlock::allowedBaseline = blids;
        }

        const auto &para = xml_.get_child_optional( "VieSchedpp.rules.calibration.parallacticAngleChange" );
        if ( para.is_initialized() ) {
            ParallacticAngleBlock::nscans = para->get( "nscans", 0 );
            ParallacticAngleBlock::duration = para->get( "duration", 300 );
            ParallacticAngleBlock::distanceScaling = para->get( "distanceScaling", 10.0 );
            ParallacticAngleBlock::cadence = para->get( "investigationCadence", 600 );
            ParallacticAngleBlock::intent_ = para->get( "intent", "" );

            string sourceName = para->get( "sources", "__all__" );
            vector<unsigned long> srcids = getMembers( sourceName, sourceList_ );
            ParallacticAngleBlock::allowedSources = srcids;

            string baselineName = para->get( "stations", "__all__" );
            vector<unsigned long> staids = getMembers( sourceName, network_.getStations() );
            ParallacticAngleBlock::allowedStations = staids;
        }
    }
}

void Initializer::initializeAstrometricCalibrationBlocks( std::ofstream &of ) {
    boost::optional<boost::property_tree::ptree &> cb = xml_.get_child_optional( "VieSchedpp.rules.calibratorBlock" );
    if ( cb.is_initialized() ) {
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "initialize calibration block";
#endif
        boost::property_tree::ptree PARA_source = xml_.get_child( "VieSchedpp.source" );
        unordered_map<std::string, std::vector<std::string>> groups = readGroups( PARA_source, MemberType::source );

        AstrometricCalibratorBlock::scheduleCalibrationBlocks = true;

        of << "Calibration Block found!\n";

        for ( const auto &any : *cb ) {
            if ( any.first == "cadence_nScanSelections" ) {
                AstrometricCalibratorBlock::cadenceUnit = AstrometricCalibratorBlock::CadenceUnit::scans;
                AstrometricCalibratorBlock::cadence = any.second.get_value<unsigned int>();
                AstrometricCalibratorBlock::nextBlock = AstrometricCalibratorBlock::cadence;
                of << "  calibration block every " << AstrometricCalibratorBlock::cadence << " scan selections\n";
            } else if ( any.first == "cadence_seconds" ) {
                AstrometricCalibratorBlock::cadenceUnit = AstrometricCalibratorBlock::CadenceUnit::seconds;
                AstrometricCalibratorBlock::cadence = any.second.get_value<unsigned int>();
                AstrometricCalibratorBlock::nextBlock = AstrometricCalibratorBlock::cadence;
                of << "  calibration block every " << AstrometricCalibratorBlock::cadence << " seconds\n";
            } else if ( any.first == "member" ) {
                string member = any.second.get_value<string>();
                vector<string> targetSources;

                if ( groups.find( member ) != groups.end() ) {
                    targetSources = groups[member];
                } else if ( member == "__all__" ) {
                    for ( int i = 0; i < sourceList_.getNSrc(); ++i ) {
                        const auto &src = sourceList_.getSource( i );
                        const string &name = src->getName();
                        targetSources.push_back( name );
                    }
                } else {
                    targetSources.push_back( member );
                }

                of << "  allowed calibrator sources: \n    ";
                vector<unsigned long> targetIds;
                int c = 0;
                for ( const auto &source : sourceList_.getSources() ) {
                    const string &name = source->getName();
                    if ( find( targetSources.begin(), targetSources.end(), name ) != targetSources.end() ) {
                        if ( c == 9 ) {
                            of << "\n    ";
                            c = 0;
                        }
                        of << boost::format( "%-8s " ) % name;
                        targetIds.push_back( source->getId() );
                        ++c;
                    }
                }
                of << "\n";
                AstrometricCalibratorBlock::calibratorSourceIds = std::move( targetIds );
            } else if ( any.first == "nMaxScans" ) {
                AstrometricCalibratorBlock::nmaxScans = any.second.get_value<unsigned int>();
                of << "  maximum number of calibration block scans: " << AstrometricCalibratorBlock::nmaxScans << "\n";

            } else if ( any.first == "fixedScanTime" ) {
                AstrometricCalibratorBlock::targetScanLengthType =
                    AstrometricCalibratorBlock::TargetScanLengthType::seconds;
                AstrometricCalibratorBlock::scanLength = any.second.get_value<unsigned int>();

                of << "  fixed scan length for calibrator scans: " << AstrometricCalibratorBlock::scanLength
                   << " seconds\n";

            } else if ( any.first == "lowElevation" ) {
                AstrometricCalibratorBlock::lowElevationStartWeight = any.second.get<double>( "startWeight" ) * deg2rad;
                AstrometricCalibratorBlock::lowElevationFullWeight = any.second.get<double>( "fullWeight" ) * deg2rad;
            } else if ( any.first == "highElevation" ) {
                AstrometricCalibratorBlock::highElevationStartWeight =
                    any.second.get<double>( "startWeight" ) * deg2rad;
                AstrometricCalibratorBlock::highElevationFullWeight = any.second.get<double>( "fullWeight" ) * deg2rad;
            }
        }
    }
}


unsigned int Initializer::minutesVisible( std::shared_ptr<const AbstractSource> source,
                                          const AbstractSource::Parameters &parameters, unsigned int start,
                                          unsigned int end ) {
#ifdef VIESCHEDPP_LOG
    if ( Flags::logTrace )
        BOOST_LOG_TRIVIAL( trace ) << "calculate possible observing time for source " << source->getName();
#endif
    unsigned int minutes = 0;
    unsigned int minVisible = parameters.minNumberOfSites;

    vector<unsigned long> reqSta = parameters.requiredStations;
    vector<unsigned long> ignSta = parameters.ignoreStations;

    for ( unsigned int t = start; t < end; t += 60 ) {
        vector<unsigned long> staids;

        bool requiredStationNotVisible = false;
        for ( unsigned long staid = 0; staid < network_.getNSta(); ++staid ) {
            Station &thisSta = network_.refStation( staid );
            bool dummy = false;
            thisSta.checkForNewEvent( t, dummy );

            if ( find( ignSta.begin(), ignSta.end(), staid ) != ignSta.end() ) {
                continue;
            }
            if ( !thisSta.getPARA().available || thisSta.getPARA().tagalong ) {
                continue;
            }

            PointingVector p( staid, source->getId() );
            p.setTime( t );

            thisSta.calcAzEl_simple( source, p );

            // check if source is up from station
            bool flag = thisSta.isVisible( p, source->getPARA().minElevation );
            if ( flag ) {
                staids.push_back( staid );
            } else {
                if ( find( reqSta.begin(), reqSta.end(), staid ) != reqSta.end() ) {
                    requiredStationNotVisible = true;
                    break;
                }
            }
        }
        if ( requiredStationNotVisible ) {
            continue;
        }
        if ( Network::stationIdsToNSites( staids ) >= minVisible ) {
            ++minutes;
        }
    }

    for ( auto &any : network_.refStations() ) {
        PointingVector pv( any.getId(), numeric_limits<unsigned long>::max() );
        pv.setTime( 0 );
        pv.setAz( ( any.getCableWrap().getNLow() + any.getCableWrap().getNUp() ) / 2 );
        pv.setEl( 0 );
        any.setCurrentPointingVector( pv );
        any.setNextEvent( 0 );
        bool dummy = false;
        any.checkForNewEvent( 0, dummy );
    }
    return minutes;
}


void Initializer::statisticsLogHeader( ofstream &of, const std::vector<VieVS::MultiScheduling::Parameters> &ms ) {
    of << "version,n_scans,n_single_source_scans,n_subnetting_scans,n_fillin-mode_scans,n_calibrator_scans,n_"
          "observations,n_stations,n_sources,time_average_observation,time_average_preob,time_average_slew,time_"
          "average_idle,time_average_field_system,sky-coverage_average_13_areas_30_min,sky-coverage_average_25_areas_"
          "30_min,sky-coverage_average_37_areas_30_min,sky-coverage_average_13_areas_60_min,sky-coverage_average_25_"
          "areas_60_min,sky-coverage_average_37_areas_60_min,";

    of << WeightFactors::statisticsHeader();

    for ( const auto &any : network_.getStations() ) {
        of << "time_" << any.getName() << "_observation,";
    }
    for ( const auto &any : network_.getStations() ) {
        of << "time_" << any.getName() << "_preob,";
    }
    for ( const auto &any : network_.getStations() ) {
        of << "time_" << any.getName() << "_slew,";
    }
    for ( const auto &any : network_.getStations() ) {
        of << "time_" << any.getName() << "_idle,";
    }
    for ( const auto &any : network_.getStations() ) {
        of << "time_" << any.getName() << "_field_system,";
    }

    for ( const auto &any : network_.getStations() ) {
        of << "sky-coverage_" << any.getName() << "_13_areas_30_min,";
    }
    for ( const auto &any : network_.getStations() ) {
        of << "sky-coverage_" << any.getName() << "_25_areas_30_min,";
    }
    for ( const auto &any : network_.getStations() ) {
        of << "sky-coverage_" << any.getName() << "_37_areas_30_min,";
    }
    for ( const auto &any : network_.getStations() ) {
        of << "sky-coverage_" << any.getName() << "_13_areas_60_min,";
    }
    for ( const auto &any : network_.getStations() ) {
        of << "sky-coverage_" << any.getName() << "_25_areas_60_min,";
    }
    for ( const auto &any : network_.getStations() ) {
        of << "sky-coverage_" << any.getName() << "_37_areas_60_min,";
    }

    for ( const auto &any : network_.getStations() ) {
        of << "n_sta_scans_" << any.getName() << ",";
    }
    for ( const auto &any : network_.getStations() ) {
        of << "n_sta_obs_" << any.getName() << ",";
    }
    for ( const auto &any : network_.getBaselines() ) {
        of << "n_bl_obs_" << any.getName() << ",";
    }
    for ( int i = 0; i < sourceList_.getNSrc(); ++i ) {
        std::shared_ptr<const AbstractSource> source = sourceList_.getSource( i );
        of << "n_src_scans_" << source->getName() << ",";
    }
    for ( int i = 0; i < sourceList_.getNSrc(); ++i ) {
        std::shared_ptr<const AbstractSource> source = sourceList_.getSource( i );
        of << "n_src_obs_" << source->getName() << ",";
    }
    for ( int i = 0; i < sourceList_.getNSrc(); ++i ) {
        std::shared_ptr<const AbstractSource> source = sourceList_.getSource( i );
        of << "n_src_closure_phases_" << source->getName() << ",";
    }
    for ( int i = 0; i < sourceList_.getNSrc(); ++i ) {
        std::shared_ptr<const AbstractSource> source = sourceList_.getSource( i );
        of << "n_src_closures_" << source->getName() << ",";
    }
    if ( !ms.empty() ) {
        ms[0].statisticsHeaderOutput( of );
    }

    for ( int i = 2; i < network_.getNSta()+1; ++i){
        of <<  boost::format("%d-station_scans,") %i;
    }



    if ( xml_.get_child_optional( "VieSchedpp.simulator" ).is_initialized() ) {
        of << "sim_mean_formal_error_n_sim,";

        of << "sim_mean_formal_error_dUT1_[mus],";
        of << "sim_mean_formal_error_x_pol_[muas],";
        of << "sim_mean_formal_error_y_pol_[muas],";
        of << "sim_mean_formal_error_x_nut_[muas],";
        of << "sim_mean_formal_error_y_nut_[muas],";
        of << "sim_mean_formal_error_scale_[ppb],";

        of << "sim_mean_formal_error_average_3d_station_coord._[mm],";
        for ( const auto &sta : network_.getStations() ) {
            of << "sim_mean_formal_error_" << sta.getName() << ",";
        }
        of << "sim_mean_formal_error_average_2d_source_coord._[mas],";
        for ( const auto &src : sourceList_.getQuasars() ) {
            of << "sim_mean_formal_error_" << src->getName() << ",";
        }

        of << "sim_repeatability_n_sim,";

        of << "sim_repeatability_dUT1_[mus],";
        of << "sim_repeatability_x_pol_[muas],";
        of << "sim_repeatability_y_pol_[muas],";
        of << "sim_repeatability_x_nut_[muas],";
        of << "sim_repeatability_y_nut_[muas],";
        of << "sim_repeatability_scale_[ppb],";

        of << "sim_repeatability_average_3d_station_coord._[mm],";
        for ( const auto &sta : network_.getStations() ) {
            of << "sim_repeatability_" << sta.getName() << ",";
        }
        of << "sim_repeatability_average_2d_source_coord._[mas],";
        for ( const auto &src : sourceList_.getQuasars() ) {
            of << "sim_repeatability_" << src->getName() << ",";
        }
    }

    of << endl;
}


void Initializer::initializeOptimization( std::ofstream &of ) {
    boost::optional<boost::property_tree::ptree &> ctree = xml_.get_child_optional( "VieSchedpp.optimization" );
    if ( ctree.is_initialized() ) {
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "initialize optimization";
#endif

        boost::property_tree::ptree PARA_agn = xml_.get_child( "VieSchedpp.source" );
        unordered_map<std::string, std::vector<std::string>> groups_agn = readGroups( PARA_agn, MemberType::source );

        const auto &PARA_sat = xml_.get_child_optional( "VieSchedpp.satellite" );
        unordered_map<std::string, std::vector<std::string>> groups_sat;
        if (PARA_sat.is_initialized()){
            groups_sat = readGroups( *PARA_sat, MemberType::satellite );
        }

        const auto &PARA_space = xml_.get_child_optional( "VieSchedpp.spacecraft" );
        unordered_map<std::string, std::vector<std::string>> groups_space;
        if (PARA_space.is_initialized()){
            groups_space = readGroups( *PARA_space, MemberType::spacecraft );
        }

        for ( const auto &any : *ctree ) {
            if ( any.first == "combination" ) {
                bool tmp = any.second.get_value<string>() == "and";
                parameters_.andAsConditionCombination = tmp;
            } else if ( any.first == "maxNumberOfIterations" ) {
                parameters_.maxNumberOfIterations = any.second.get_value<unsigned int>();
            } else if ( any.first == "numberOfGentleSourceReductions" ) {
                parameters_.numberOfGentleSourceReductions_1 = any.second.get_value<unsigned int>();
                parameters_.numberOfGentleSourceReductions_2 = any.second.get_value<unsigned int>();
            } else if ( any.first == "numberOfGentleSourceReductions_1" ) {
                parameters_.numberOfGentleSourceReductions_1 = any.second.get_value<unsigned int>();
            } else if ( any.first == "numberOfGentleSourceReductions_2" ) {
                parameters_.numberOfGentleSourceReductions_2 = any.second.get_value<unsigned int>();
            } else if ( any.first == "minNumberOfSourcesToReduce" ) {
                parameters_.minNumberOfSourcesToReduce = any.second.get_value<unsigned int>();
            } else if ( any.first == "percentageGentleSourceReduction" ) {
                parameters_.reduceFactor_1 = any.second.get_value<double>() / 100;
                parameters_.reduceFactor_2 = any.second.get_value<double>() / 100;
            } else if ( any.first == "percentageGentleSourceReduction_1" ) {
                parameters_.reduceFactor_1 = any.second.get_value<double>() / 100;
            } else if ( any.first == "percentageGentleSourceReduction_2" ) {
                parameters_.reduceFactor_2 = any.second.get_value<double>() / 100;
            } else if ( any.first == "condition" ) {
                string member = any.second.get<string>( "members" );
                auto scans = any.second.get<unsigned int>( "minScans" );
                auto bls = any.second.get<unsigned int>( "minBaselines" );

                if ( member == "__all__" ) {
                    for ( auto &source : sourceList_.refSources() ) {
                        source->referenceCondition().minNumScans = scans;
                        source->referenceCondition().minNumObs = bls;
                    }
                } else if ( groups_agn.find( member ) != groups_agn.end() ) {
                    const vector<string> &group = groups_agn.at( member );
                    for ( auto &source : sourceList_.refQuasars() ) {
                        if ( find( group.begin(), group.end(), source->getName() ) != group.end() ||
                             find( group.begin(), group.end(), source->getAlternativeName() ) != group.end() ) {
                            source->referenceCondition().minNumScans = scans;
                            source->referenceCondition().minNumObs = bls;
                        }
                    }
                } else if ( groups_sat.find( member ) != groups_sat.end() ) {
                    const vector<string> &group = groups_sat.at( member );
                    for ( auto &source : sourceList_.refSatellites() ) {
                        if ( find( group.begin(), group.end(), source->getName() ) != group.end() ||
                             find( group.begin(), group.end(), source->getAlternativeName() ) != group.end() ) {
                            source->referenceCondition().minNumScans = scans;
                            source->referenceCondition().minNumObs = bls;
                        }
                    }
                } else if ( groups_space.find( member ) != groups_space.end() ) {
                    const vector<string> &group = groups_space.at( member );
                    for ( auto &source : sourceList_.refSatellites() ) {
                        if ( find( group.begin(), group.end(), source->getName() ) != group.end() ||
                             find( group.begin(), group.end(), source->getAlternativeName() ) != group.end() ) {
                            source->referenceCondition().minNumScans = scans;
                            source->referenceCondition().minNumObs = bls;
                        }
                    }
                } else {
                    for ( int i = 0; i < sourceList_.getNSrc(); ++i ) {
                        const auto &source = sourceList_.refSource( i );
                        if ( source->hasName( member ) ) {
                            source->referenceCondition().minNumScans = scans;
                            source->referenceCondition().minNumObs = bls;
                        }
                    }
                }
            }
        }
    }
}


void Initializer::initializeHighImpactScanDescriptor( std::ofstream &of ) {
    boost::optional<boost::property_tree::ptree &> ctree = xml_.get_child_optional( "VieSchedpp.highImpact" );
    if ( ctree.is_initialized() ) {
#ifdef VIESCHEDPP_LOG
        if ( Flags::logDebug ) BOOST_LOG_TRIVIAL( debug ) << "initialize high impact scan descriptor";
#endif

        of << "High impact block found!\n";
        unsigned int interval = ctree->get( "interval", 60 );
        unsigned int repeat = ctree->get( "repeat", 300 );
        of << "    high impact check interval: " << interval << "\n";
        of << "    high impact repeat        : " << repeat << "\n";

        boost::property_tree::ptree PARA_station = xml_.get_child( "VieSchedpp.station" );
        unordered_map<std::string, std::vector<std::string>> groups = readGroups( PARA_station, MemberType::station );

        HighImpactScanDescriptor himp = HighImpactScanDescriptor( interval, repeat );
        for ( const auto &any : *ctree ) {
            if ( any.first == "targetAzEl" ) {
                auto member = any.second.get<string>( "member" );
                vector<unsigned long> staids;

                if ( groups.find( member ) != groups.end() ) {
                    const vector<string> &group = groups.at( member );
                    for ( auto &station : network_.getStations() ) {
                        if ( find( group.begin(), group.end(), station.getName() ) != group.end() ) {
                            staids.push_back( station.getId() );
                        }
                    }
                } else {
                    for ( auto &station : network_.getStations() ) {
                        if ( station.hasName( member ) ) {
                            staids.push_back( station.getId() );
                        }
                    }
                }

                auto az = any.second.get<double>( "az" );
                auto el = any.second.get<double>( "el" );
                auto margin = any.second.get<double>( "margin" );

                of << "    target az: " << az << " el: " << el << " margin: " << margin << " station: ";
                for ( unsigned long i : staids ) {
                    of << network_.getStation( i ).getName() << " ";
                }
                of << "\n";

                himp.addAzElDescriptor( az * deg2rad, el * deg2rad, margin * deg2rad, staids );
            }
        }
        himp_ = himp;
    }
}


std::vector<unsigned long> Initializer::getMembers( const std::string &name, const std::vector<Station> &stations ) {
    vector<unsigned long> ids;

    // add all ids if name == "__all__"
    if ( name == "__all__" ) {
        ids.resize( stations.size() );
        iota( ids.begin(), ids.end(), 0 );

        // add all ids from group if name is equal to a group name
    } else if ( staGroups_.find( name ) != staGroups_.end() ) {
        const auto &members = staGroups_.at( name );
        for ( const auto &thisTarget : members ) {
            for ( const auto &thisObject : stations ) {
                if ( thisObject.hasName( thisTarget ) ) {
                    ids.push_back( thisObject.getId() );
                    break;
                }
            }
        }

        // add single id instead
    } else {
        for ( const auto &any : stations ) {
            if ( any.hasName( name ) ) {
                ids.push_back( any.getId() );
            }
        }
    }

    return ids;
}


std::vector<unsigned long> Initializer::getMembers( const std::string &name, const std::vector<Baseline> &baselines ) {
    vector<unsigned long> ids;

    // add all ids if name == "__all__"
    if ( name == "__all__" ) {
        ids.resize( baselines.size() );
        iota( ids.begin(), ids.end(), 0 );

        // add all ids from group if name is equal to a group name
    } else if ( blGroups_.find( name ) != blGroups_.end() ) {
        const auto &members = blGroups_.at( name );
        for ( const auto &thisTarget : members ) {
            for ( const auto &thisObject : baselines ) {
                if ( thisObject.hasName( thisTarget ) ) {
                    ids.push_back( thisObject.getId() );
                    break;
                }
            }
        }

        // add single id instead
    } else {
        for ( const auto &any : baselines ) {
            if ( any.hasName( name ) ) {
                ids.push_back( any.getId() );
            }
        }
    }

    return ids;
}


std::vector<unsigned long> Initializer::getMembers( const std::string &name, const SourceList &sourceList ) {
    vector<unsigned long> ids;

    // add all ids if name == "__all__"
    if ( name == "__all__" ) {
        ids.resize( sourceList.getNSrc() );
        iota( ids.begin(), ids.end(), 0 );

        // add all ids from group if name is equal to a group name
    } else if ( srcGroups_.find( name ) != srcGroups_.end() ) {
        const auto &members = srcGroups_.at( name );
        for ( const auto &thisTarget : members ) {
            for ( const auto &thisObject : sourceList.getSources() ) {
                if ( thisObject->hasName( thisTarget ) ) {
                    ids.push_back( thisObject->getId() );
                    break;
                }
            }
        }

        // add single id instead
    } else {
        for ( const auto &any : sourceList.getSources() ) {
            if ( any->hasName( name ) ) {
                ids.push_back( any->getId() );
            }
        }
    }

    return ids;
}


void Initializer::connectObservingMode( std::ofstream &of ) noexcept {
    vector<string> staNames;
    for ( const auto &any : network_.getStations() ) {
        staNames.push_back( any.getAlternativeName() );
    }
    obsModes_->setStationNames( staNames );
    obsModes_->summary( of );
}


unordered_map<string, unique_ptr<AbstractFlux>> Initializer::generateFluxObject(
    const string &name, const string &commonname, const map<std::string, vector<string>> &fluxCatalog,
    bool fluxNecessary, std::ofstream &of ) {
    unordered_map<string, unique_ptr<AbstractFlux>> flux;
    double flcon2{ pi / ( 3600.0 * 180.0 * 1000.0 ) };

    // check if there is a entry in the flux catalog
    bool foundName = fluxCatalog.find( name ) != fluxCatalog.end();
    bool foundCommName = ( !commonname.empty() && fluxCatalog.find( commonname ) != fluxCatalog.end() );

    if ( !foundName && !foundCommName && fluxNecessary ) {
        of << "source " << name << " flux catalog entry is required but was not found!\n";
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( warning ) << "source " << name << " flux.cat: source not found";
#else
        cout << "[warning] source " << name << " flux.cat: source not found";
#endif
        return flux;
    }

    // get flux entry from catalog
    vector<string> flux_cat;
    if ( foundName ) {
        flux_cat = fluxCatalog.at( name );
    } else if ( foundCommName ) {
        flux_cat = fluxCatalog.at( commonname );
    }

    vector<vector<string>> flux_split;
    for ( auto &i : flux_cat ) {
        vector<string> splitVector;
        boost::split( splitVector, i, boost::is_space(), boost::token_compress_on );
        if ( splitVector.size() > 3 ) {
            flux_split.push_back( splitVector );
        }
    }

    // parse flux catalog entry
    vector<int> alreadyConsidered;
    int cflux = 0;
    while ( cflux < flux_split.size() ) {
        if ( find( alreadyConsidered.begin(), alreadyConsidered.end(), cflux ) != alreadyConsidered.end() ) {
            ++cflux;
            continue;
        }
        vector<string> parameters;
        alreadyConsidered.push_back( cflux );

        string thisBand = flux_split[cflux][1];
        string thisType = flux_split[cflux][2];
        if ( thisType == "M" ) {
            bool flagAdd = false;
            if ( flux_split[cflux].size() == 5 ) {
                flux_split[cflux].emplace_back( "0" );
                flagAdd = true;
            }
            if ( flux_split[cflux].size() == 4 ) {
                flux_split[cflux].emplace_back( "0" );
                flux_split[cflux].emplace_back( "0" );
                flagAdd = true;
            }
            if ( flagAdd ) {
                of << "*** WARNING: Flux of type M lacks elements! zeros added!\n";
#ifdef VIESCHEDPP_LOG
                BOOST_LOG_TRIVIAL( warning )
                    << "source " << name << " flux.cat: lacking element in M format, zeros added";
#else
                cout << "[warning] source " << name << " flux.cat: lacking element in M format, zeros added";
#endif
            }
        }

        parameters.insert( parameters.end(), flux_split[cflux].begin() + 3, flux_split[cflux].end() );

        for ( int i = cflux + 1; i < flux_split.size(); ++i ) {
            if ( flux_split[i][1] == thisBand ) {
                if ( flux_split[i][2] == thisType ) {
                    parameters.insert( parameters.end(), flux_split[i].begin() + 3, flux_split[i].end() );
                    alreadyConsidered.push_back( i );
                } else {
                    of << "*** ERROR: Source:" << name << "Flux: " << thisBand
                       << " You can not mix B and M flux information for one band!;\n";
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( warning )
                        << "source " << name << " flux.cat: both B and M flux information found";
#else
                    cout << "[warning] source " << name << " flux.cat: both B and M flux information found";
#endif
                }
            }
        }
        unique_ptr<AbstractFlux> srcFlux;
        bool errorWhileReadingFlux = false;

        if ( thisType == "M" ) {
            std::vector<double> tflux;
            std::vector<double> tmajorAxis;
            std::vector<double> taxialRatio;
            std::vector<double> tpositionAngle;

            unsigned long npara = parameters.size();

            unsigned long nmodels = npara / 6;
            for ( unsigned int i = 0; i < nmodels; ++i ) {
                try {
                    tflux.push_back( boost::lexical_cast<double>( parameters.at( i * 6 + 0 ) ) );
                    tmajorAxis.push_back( boost::lexical_cast<double>( parameters.at( i * 6 + 1 ) ) * flcon2 );
                    taxialRatio.push_back( boost::lexical_cast<double>( parameters.at( i * 6 + 2 ) ) );
                    tpositionAngle.push_back( boost::lexical_cast<double>( parameters.at( i * 6 + 3 ) ) * deg2rad );
                } catch ( const std::exception &e ) {
                    errorWhileReadingFlux = true;
                    of << "*** ERROR: " << parameters[0] << " " << parameters[1] << " " << e.what()
                       << " reading flux information;\n";
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( warning ) << "source " << name << " flux.cat: cannot cast text to number";
#else
                    cout << "[warning] source " << name << " flux.cat: cannot cast text to number";
#endif
                    break;
                }
            }

            if ( !errorWhileReadingFlux ) {
                srcFlux = make_unique<Flux_M>( ObservingMode::wavelengths[thisBand], tflux, tmajorAxis, taxialRatio,
                                               tpositionAngle );
            }
        } else {
            std::vector<double> knots;   ///< baseline length of flux information (type B)
            std::vector<double> values;  ///< corresponding flux information for baseline length (type B)

            unsigned long npara = parameters.size();
            for ( int i = 0; i < npara; ++i ) {
                try {
                    double value = boost::lexical_cast<double>( parameters[i] );
                    if ( i % 2 == 0 ) {
                        knots.push_back( value );
                    } else {
                        if ( value == 0 ) {
                            value = 0.001;
                        }
                        values.push_back( value );
                    }
                } catch ( const std::exception &e ) {
                    errorWhileReadingFlux = true;
                    of << "*** ERROR: reading flux information; \n";
#ifdef VIESCHEDPP_LOG
                    BOOST_LOG_TRIVIAL( warning ) << "source " << name << " flux.cat: cannot cast text to number";
#else
                    cout << "[warning] source " << name << " flux.cat: cannot cast text to number";
#endif
                    return flux;
                }
            }

            if ( !errorWhileReadingFlux ) {
                double wavelength = ObservingMode::wavelengths[thisBand];
                if ( values.size() == 1 ) {
                    srcFlux = make_unique<Flux_constant>( wavelength, values[0] );
                } else {
                    srcFlux = make_unique<Flux_B>( wavelength, std::move( knots ), std::move( values ) );
                }
            }
        }

        if ( !errorWhileReadingFlux ) {
            flux[thisBand] = move( srcFlux );
            ++cflux;
        }
    }

    bool fluxBandInfoOk = true;
    for ( const auto &bandName : ObservingMode::bands ) {
        if ( flux.find( bandName ) == flux.end() ) {
            if ( ObservingMode::sourceProperty[bandName] == ObservingMode::Property::required ) {
                fluxBandInfoOk = false;
                break;
            }
            if ( ObservingMode::sourceBackup[bandName] == ObservingMode::Backup::value ) {
                flux[bandName] = make_unique<Flux_B>( ObservingMode::wavelengths[bandName], vector<double>{ 0, 13000 },
                                                      vector<double>{ ObservingMode::sourceBackupValue[bandName] } );
            }
        }
    }
    if ( !fluxBandInfoOk ) {
        of << "*** WARNING: source " << name << " required flux information missing!;\n";
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( warning ) << "source " << name << " required flux information missing";
#else
        cout << "[warning] source " << name << " required flux information missing";
#endif
    }

    if ( flux.size() != ObservingMode::bands.size() ) {
        if ( flux.empty() ) {
            of << "*** ERROR: source " << name << " no flux information found to calculate backup value!;\n";
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( warning ) << "source " << name << " no flux information found to calculate backup value";
#else
            cout << "[warning] source " << name << " no flux information found to calculate backup value";
#endif
            return flux;
        }
        double max = 0;
        double min = std::numeric_limits<double>::max();
        for ( const auto &any2 : flux ) {
            // TODO use something like .getMinimumFlux instead of .getMaximumFlux
            if ( any2.second->getMaximumFlux() < min ) {
                min = any2.second->getMaximumFlux();
            }
            if ( any2.second->getMaximumFlux() > max ) {
                max = any2.second->getMaximumFlux();
            }
        }
        for ( const auto &bandName : ObservingMode::bands ) {
            if ( flux.find( bandName ) == flux.end() ) {
                if ( ObservingMode::stationBackup[bandName] == ObservingMode::Backup::minValueTimes ) {
                    flux[bandName] =
                        make_unique<Flux_B>( ObservingMode::wavelengths[bandName], vector<double>{ 0, 13000 },
                                             vector<double>{ min * ObservingMode::stationBackupValue[bandName] } );
                }
                if ( ObservingMode::stationBackup[bandName] == ObservingMode::Backup::maxValueTimes ) {
                    flux[bandName] =
                        make_unique<Flux_B>( ObservingMode::wavelengths[bandName], vector<double>{ 0, 13000 },
                                             vector<double>{ max * ObservingMode::stationBackupValue[bandName] } );
                }
            }
        }
    }

    return flux;
}
void Initializer::initializeSatellitesToAvoid() {
    if ( !AvoidSatellites::satellitesToAvoid.empty() ) {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( info ) << "calculate lookup table for satellites to avoid";
#else
        cout << "[info] precalc lookup table for satellites to avoid\n";
#endif
    } else {
        return;
    }

    try {
        AvoidSatellites::initialize( network_ );
    } catch ( ... ) {
#ifdef VIESCHEDPP_LOG
        BOOST_LOG_TRIVIAL( error ) << "calculate lookup table for satellites to avoid";
#else
        cout << "[error] calculate lookup table for satellites to avoid\n";
#endif
        throw;
    }

    unsigned long total = 0;
    for ( const auto &any : AvoidSatellites::visible_ ) {
        for ( const auto &sat : any.second ) {
            total += sat.second.size();
        }
    }
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "total of " << total << " periods with satellite passings";
#else
    cout << "total of " << total << " periods with satellite passings\n";
#endif
}
