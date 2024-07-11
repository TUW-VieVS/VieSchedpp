//
// Created by matth on 27.10.2018.
//

#include "ObservingMode.h"


using namespace VieVS;
using namespace std;

unsigned long VieVS::ObservingMode::nextId = 0;

VieVS::ObservingMode::Type VieVS::ObservingMode::type = VieVS::ObservingMode::Type::simple;

std::set<std::string> VieVS::ObservingMode::bands;
std::unordered_map<std::string, double> VieVS::ObservingMode::wavelengths{
    { "S", util::freqency2wavelenth( 2291 * 1e6 ) }, { "X", util::freqency2wavelenth( 8593 * 1e6 ) } };

std::unordered_map<std::string, double> VieVS::ObservingMode::minSNR;  ///< backup min SNR

std::unordered_map<std::string, VieVS::ObservingMode::Property>
    VieVS::ObservingMode::stationProperty;  ///< is band required or optional for station
std::unordered_map<std::string, VieVS::ObservingMode::Backup>
    VieVS::ObservingMode::stationBackup;                                           ///< backup version for station
std::unordered_map<std::string, double> VieVS::ObservingMode::stationBackupValue;  ///< backup value for station

std::unordered_map<std::string, VieVS::ObservingMode::Property>
    VieVS::ObservingMode::sourceProperty;  ///< is band required or optional for source
std::unordered_map<std::string, VieVS::ObservingMode::Backup>
    VieVS::ObservingMode::sourceBackup;                                           ///< backup version for source
std::unordered_map<std::string, double> VieVS::ObservingMode::sourceBackupValue;  ///< backup value for source

ObservingMode::ObservingMode() : VieVS_Object( nextId++ ) {}


ObservingMode::ObservingMode( const boost::property_tree::ptree &tree, const std::vector<std::string> &staNames )
    : VieVS_Object( nextId++ ), stationNames_{ staNames } {
    for ( const auto &any : tree ) {
        if ( any.first == "FREQ" ) {
            addBlock( make_shared<Freq>( any.second ) );
        } else if ( any.first == "BBC" ) {
            addBlock( make_shared<Bbc>( any.second ) );
        } else if ( any.first == "IF" ) {
            addBlock( make_shared<If>( any.second ) );
        } else if ( any.first == "TRACKS" ) {
            addBlock( make_shared<Track>( any.second ) );
        } else if ( any.first == "track_frame_format" ) {
            addBlock( any.second.get<string>( "<xmlattr>.name" ) );
        }
    }

    for ( const auto &any : tree ) {
        if ( any.first == "MODE" ) {
            const boost::property_tree::ptree &m = any.second;
            const string name = m.get<string>( "<xmlattr>.name" );
            shared_ptr<Mode> mode = make_shared<Mode>( name, stationNames_.size() );

            vector<vector<unsigned long>> freqIds( freqs_.size() );
            vector<vector<unsigned long>> bbcIds( bbcs_.size() );
            vector<vector<unsigned long>> ifIds( ifs_.size() );
            vector<vector<unsigned long>> tracksIds( tracks_.size() );
            vector<vector<unsigned long>> trackFrameFormatIds( trackFrameFormats_.size() );

            for ( const auto &t : m ) {
                if ( t.first == "FREQ" ) {
                    const string n = t.second.get<string>( "<xmlattr>.name" );
                    for ( int i = 0; i < freqs_.size(); ++i ) {
                        if ( freqs_[i]->hasName( n ) ) {
                            freqIds[i] = getStationIds( t.second );
                        }
                    }
                }
            }
            for ( const auto &t : m ) {
                if ( t.first == "BBC" ) {
                    const string n = t.second.get<string>( "<xmlattr>.name" );
                    for ( int i = 0; i < bbcs_.size(); ++i ) {
                        if ( bbcs_[i]->hasName( n ) ) {
                            bbcIds[i] = getStationIds( t.second );
                        }
                    }
                }
            }
            for ( const auto &t : m ) {
                if ( t.first == "IF" ) {
                    const string n = t.second.get<string>( "<xmlattr>.name" );
                    for ( int i = 0; i < ifs_.size(); ++i ) {
                        if ( ifs_[i]->hasName( n ) ) {
                            ifIds[i] = getStationIds( t.second );
                        }
                    }
                }
            }
            for ( const auto &t : m ) {
                if ( t.first == "TRACKS" ) {
                    const string n = t.second.get<string>( "<xmlattr>.name" );
                    for ( int i = 0; i < tracks_.size(); ++i ) {
                        if ( tracks_[i]->hasName( n ) ) {
                            tracksIds[i] = getStationIds( t.second );
                        }
                    }
                }
            }
            for ( const auto &t : m ) {
                if ( t.first == "track_frame_formats" ) {
                    const string n = t.second.get<string>( "<xmlattr>.name" );
                    for ( int i = 0; i < trackFrameFormats_.size(); ++i ) {
                        if ( *trackFrameFormats_[i] == n ) {
                            trackFrameFormatIds[i] = getStationIds( t.second );
                        }
                    }
                }
            }

            for ( int i = 0; i < freqs_.size(); ++i ) {
                mode->addBlock( freqs_[i], freqIds[i] );
            }
            for ( int i = 0; i < bbcs_.size(); ++i ) {
                mode->addBlock( bbcs_[i], bbcIds[i] );
            }
            for ( int i = 0; i < ifs_.size(); ++i ) {
                mode->addBlock( ifs_[i], ifIds[i] );
            }
            for ( int i = 0; i < tracks_.size(); ++i ) {
                mode->addBlock( tracks_[i], tracksIds[i] );
            }
            for ( int i = 0; i < trackFrameFormats_.size(); ++i ) {
                mode->addBlock( trackFrameFormats_[i], trackFrameFormatIds[i] );
            }

            mode->calcRecordingRates();
            modes_.push_back( mode );
        }
    }
    calcMeanFrequencies();
}


std::vector<unsigned long> ObservingMode::getStationIds( const boost::property_tree::ptree &tree ) {
    vector<unsigned long> v;
    for ( const auto &any : tree ) {
        if ( any.first == "station" ) {
            for ( unsigned long i = 0; i < stationNames_.size(); ++i ) {
                if ( any.second.get_value<string>() == stationNames_[i] ) {
                    v.push_back( i );
                    break;
                }
            }
        }
    }
    return v;
}


void ObservingMode::readFromSkedCatalogs( const SkdCatalogReader &skd ) {
    auto mode = make_shared<Mode>( skd.getModeName(), skd.getStaNames().size() );

    const auto &channelNr2Bbc = readSkdTracks( mode, skd );
    const auto &channelNr2Bbc_vdif =  trackorder_vdif ( channelNr2Bbc );

    readSkdFreq( mode, skd, channelNr2Bbc );
    readSkdFreq( mode, skd, channelNr2Bbc_vdif, true );

    readSkdIf( mode, skd );
    readSkdBbc( mode, skd );
    readSkdTrackFrameFormat( mode, skd );
    mode->calcRecordingRates();
    mode->vdif_stations( skd );

    addMode( mode );
    calcMeanFrequencies();
}


void ObservingMode::simpleMode( unsigned long nsta, double samplerate, unsigned int bits,
                                const std::unordered_map<std::string, unsigned int> &band2channel,
                                const std::unordered_map<std::string, double> &band2wavelength,
                                double efficiencyFactor ) {
    auto mode = make_shared<Mode>( "type", nsta );

    for ( const auto &any : band2channel ) {
        bands.insert( any.first );
        double recRate = samplerate * bits * any.second * 1e6;
        mode->setRecordingRates( any.first, recRate );

        // Thompson, Moran, Swenson, 3rd Edition
        // https://www.researchgate.net/publication/234450511_Interferometry_and_Synthesis_in_Radio_Astronomy
        if ( bits == 1 ) {
            mode->setEfficiencyFactor( 0.637 * 0.97 );
        } else {
            mode->setEfficiencyFactor( 0.860 * 0.97 );
        }
        if ( efficiencyFactor != -1 ) {
            mode->setEfficiencyFactor( efficiencyFactor );
        }
    }

    mode->setBands( bands );
    addMode( mode );
    for ( const auto &any : band2wavelength ) {
        wavelengths[any.first] = any.second;
    }
}


boost::property_tree::ptree ObservingMode::toPropertytree() const {
    boost::property_tree::ptree p;
    for ( const auto &any : freqs_ ) {
        p.add_child( "FREQ", any->toPropertytree() );
    }
    for ( const auto &any : bbcs_ ) {
        p.add_child( "BBC", any->toPropertytree() );
    }
    for ( const auto &any : ifs_ ) {
        p.add_child( "IF", any->toPropertytree() );
    }
    for ( const auto &any : tracks_ ) {
        p.add_child( "TRACKS", any->toPropertytree() );
    }
    for ( const auto &any : trackFrameFormats_ ) {
        boost::property_tree::ptree t;
        t.add( "<xmlattr>.name", *any );
        p.add_child( "track_frame_format", t );
    }
    for ( const auto &any : modes_ ) {
        p.add_child( "MODE", any->toPropertytree( stationNames_ ) );
    }
    return p;
}


void ObservingMode::readSkdFreq( const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd,
                                 const std::map<int, pair<int, Freq::Net_sideband>> &channelNr2Bbc,
                                 bool vdif ) {
    const auto &staNames = skd.getStaNames();

    string freqName;
    if ( vdif ){
        freqName = skd.getFreqName() + "_VDIF";
        if ( channelNr2Bbc.size() != 16){
            return;
        }
    } else {
        freqName = skd.getFreqName();
    }
    vector<unsigned long> freqIds = vector<unsigned long>( staNames.size() );
    std::iota( freqIds.begin(), freqIds.end(), 0 );
    auto thisFreq = make_shared<Freq>( freqName );

    thisFreq->setSampleRate( skd.getSampleRate() );
    const double bandwidth = skd.getBandWidth();

    const auto &channelNumber2band = skd.getChannelNumber2band();
    const auto &channelNumber2Bbc = skd.getChannelNumber2BBC();
    const auto &channelNumber2skyFreq = skd.getChannelNumber2skyFreq();

    for ( const auto &any : channelNr2Bbc ) {
        int chNr = any.first;
        int bbcNr = any.second.first;
        Freq::Net_sideband sideband = any.second.second;
        string chStr = ( boost::format( "&CH%02d" ) % chNr ).str();
        string bbcStr = ( boost::format( "&BBC%02d" ) % bbcNr ).str();
        auto skyFreq = boost::lexical_cast<double>( channelNumber2skyFreq.at( bbcNr ) );
        thisFreq->addChannel( channelNumber2band.at( bbcNr ), skyFreq, sideband, bandwidth, chStr, bbcStr, "&U_cal" );
    }

    // add freq to mode
    addBlock( thisFreq );
    mode->addBlock( thisFreq, freqIds );
}

std::map<int, std::pair<int, Freq::Net_sideband>> ObservingMode::trackorder_vdif (
    std::map<int, std::pair<int, Freq::Net_sideband>> tracks ){

    std::map<int, pair<int, Freq::Net_sideband>> channelNr2Bbc;
    vector<int> old2new{-1,1,3,4,5,6,7,8,9,2,10,11,12,13,14,15,16};

    for(int channel = 1; channel<=tracks.size(); ++channel){
        int old_channel = old2new[channel];

        int new_channel = tracks[old_channel].first;
        Freq::Net_sideband new_sideband = tracks[old_channel].second;
        channelNr2Bbc[channel] = { new_channel, new_sideband };
    }
    return channelNr2Bbc;
}


std::map<int, pair<int, Freq::Net_sideband>> ObservingMode::readSkdTracks( const std::shared_ptr<Mode> &mode,
                                                                           const SkdCatalogReader &skd ) {
    const auto &staNames = skd.getStaNames();
    std::map<int, pair<int, Freq::Net_sideband>> channelNr2Bbc;

    // create tracks object
    const auto &staName2tracksMap = skd.getStaName2tracksMap();
    const auto &tracksIds = skd.getTracksIds();
    const auto &tracksId2fanout = skd.getTracksId2fanoutMap();
    const auto &channelNumber2tracksMap = skd.getTracksId2channelNumber2tracksMap();
    const auto &tracksId2bits = skd.getTracksId2bits();

    for ( const auto &tracksId : tracksIds ) {
        int chn = 1;
        int bbcNr = 1;

        // get corresponding station ids
        vector<unsigned long> ids;
        for ( unsigned long i = 0; i < staNames.size(); ++i ) {
            const auto &thisStation = staNames[i];
            const auto &it = staName2tracksMap.find( thisStation );
            if ( it != staName2tracksMap.end() && it->second == tracksId ) {
                ids.push_back( i );
            }
        }
        auto track = make_shared<Track>( tracksId );

        if ( tracksId2fanout.at( tracksId ) == 1 ) {
            // 1:1 fanout
            for ( const auto &any : channelNumber2tracksMap.at( tracksId ) ) {
                const string &t = any.second;
                string tracks = t.substr(0, t.find(' '));
                tracks = tracks.substr(2, tracks.size() - 3);
                vector<string> splitVector;
                boost::split( splitVector, tracks, boost::is_any_of( "," ), boost::token_compress_off );
                if ( splitVector.size() <= 2 ) {
                    // 1 bit
                    for ( int ich = 0; ich < splitVector.size(); ++ich ) {
                        const auto &ch = splitVector[ich];
                        if ( ch.empty() ) {
                            continue;
                        }
                        string chStr = ( boost::format( "&CH%02d" ) % chn ).str();
                        auto nr = boost::lexical_cast<int>( ch );
                        track->addFanout( "A", chStr, Track::Bitstream::sign, 1, nr + 3 );
                        Freq::Net_sideband nsb = ich == 0 ? Freq::Net_sideband::U : Freq::Net_sideband::L;
                        channelNr2Bbc[chn] = { bbcNr, nsb };
                        ++chn;
                    }
                } else if ( splitVector.size() > 2 ) {
                    for ( int ich = 0; ich < splitVector.size() / 2; ++ich ) {
                        const auto &ch = splitVector[ich];
                        if ( ch.empty() ) {
                            continue;
                        }

                        string chStr = ( boost::format( "&CH%02d" ) % chn ).str();
                        auto nr = boost::lexical_cast<int>( ch );
                        track->addFanout( "A", chStr, Track::Bitstream::sign, 1, nr + 3 );
                        nr = boost::lexical_cast<int>( splitVector.at( ich + 2 ) );
                        track->addFanout( "A", chStr, Track::Bitstream::mag, 1, nr + 3 );
                        Freq::Net_sideband nsb = ich == 0 ? Freq::Net_sideband::U : Freq::Net_sideband::L;
                        channelNr2Bbc[chn] = { bbcNr, nsb };
                        ++chn;
                    }
                }
                bbcNr++;
            }
        } else if ( tracksId2fanout.at( tracksId ) == 2 ) {
            // 1:2 fanout
            for ( const auto &any : channelNumber2tracksMap.at( tracksId ) ) {
                const string &t = any.second;
                string tracks = t.substr(0, t.find(' '));

                unsigned long idx1 = tracks.find('(');
                unsigned long idx2 = tracks.find(')');
                tracks = tracks.substr(idx1 + 1, idx2 - idx1 - 1);
                vector<string> splitVector;
                boost::split( splitVector, tracks, boost::is_any_of( "," ), boost::token_compress_off );
                if ( splitVector.size() <= 2 ) {
                    // 1 bit
                    for ( int ich = 0; ich < splitVector.size(); ++ich ) {
                        const auto &ch = splitVector[ich];
                        if ( ch.empty() ) {
                            continue;
                        }
                        string chStr = ( boost::format( "&CH%02d" ) % chn ).str();
                        auto nr = boost::lexical_cast<int>( ch );
                        track->addFanout( "A", chStr, Track::Bitstream::sign, 1, nr + 3, nr + 5 );
                        Freq::Net_sideband nsb = ich == 0 ? Freq::Net_sideband::U : Freq::Net_sideband::L;
                        channelNr2Bbc[chn] = { bbcNr, nsb };
                        ++chn;
                    }
                } else if ( splitVector.size() > 2 ) {
                    // 2 bits
                    for ( int ich = 0; ich < splitVector.size() / 2; ++ich ) {
                        const auto &ch = splitVector[ich];
                        if ( ch.empty() ) {
                            continue;
                        }
                        string chStr = ( boost::format( "&CH%02d" ) % chn ).str();
                        auto nr = boost::lexical_cast<int>( ch );
                        track->addFanout( "A", chStr, Track::Bitstream::sign, 1, nr + 3, nr + 5 );
                        nr = boost::lexical_cast<int>( splitVector.at( ich + 2 ) );
                        track->addFanout( "A", chStr, Track::Bitstream::mag, 1, nr + 3, nr + 5 );
                        Freq::Net_sideband nsb = ich == 0 ? Freq::Net_sideband::U : Freq::Net_sideband::L;
                        channelNr2Bbc[chn] = { bbcNr, nsb };
                        ++chn;
                    }
                }
                bbcNr++;
            }
        }

        // add track to mode
        addBlock( track );
        mode->addBlock( track, ids );
    }

    return channelNr2Bbc;
}


void ObservingMode::readSkdIf( const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd ) {
    const auto &staNames = skd.getStaNames();
    const auto &staName2loifId = skd.getStaName2loifId();
    const auto &loifId2loifInfo = skd.getLoifId2loifInfo();

    for ( const auto &any : loifId2loifInfo ) {
        string loifId = any.first;
        vector<string> ifs = any.second;
        string lastId;

        // get corresponding station ids
        vector<unsigned long> ids;
        for ( unsigned long i = 0; i < staNames.size(); ++i ) {
            const auto &thisStation = staNames[i];
            const auto it = staName2loifId.find( thisStation );
            if ( it != staName2loifId.end() && it->second == loifId ) {
                ids.push_back( i );
            }
        }
        auto thisIf = make_shared<If>( loifId );

        vector<string> alreadyDefined;
        for ( const auto &anyIf : ifs ) {
            vector<string> splitVector;
            boost::split( splitVector, anyIf, boost::is_space(), boost::token_compress_on );
            if ( lastId == splitVector.at( 2 ) ) {
                continue;
            }
            string if_id_link = "&IF_" + splitVector.at( 2 );
            if ( find( alreadyDefined.begin(), alreadyDefined.end(), if_id_link ) != alreadyDefined.end() ) {
                continue;
            } else {
                alreadyDefined.push_back( if_id_link );
            }
            const string &physicalName = splitVector.at( 2 );
            If::Polarization polarization = If::Polarization::R;
            auto total_IO = boost::lexical_cast<double>( splitVector.at( 4 ) );
            If::Net_sidband net_sidband = If::Net_sidband::U;
            if ( splitVector.at( 5 ) == "D" ) {
                net_sidband = If::Net_sidband::D;
            } else if ( splitVector.at( 5 ) == "L" ) {
                net_sidband = If::Net_sidband::L;
            }

            thisIf->addIf( if_id_link, physicalName, polarization, total_IO, net_sidband, 1, 0 );

            lastId = if_id_link;
        }

        // add IF to Mode
        addBlock( thisIf );
        mode->addBlock( thisIf, ids );
    }
}


void ObservingMode::readSkdBbc( const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd ) {
    const auto &staNames = skd.getStaNames();
    const auto &staName2loifId = skd.getStaName2loifId();
    const auto &loifId2loifInfo = skd.getLoifId2loifInfo();

    for ( const auto &any : loifId2loifInfo ) {
        string bbcId = any.first;
        vector<string> ifs = any.second;

        // get corresponding station ids
        vector<unsigned long> ids;
        for ( unsigned long i = 0; i < staNames.size(); ++i ) {
            const auto &thisStation = staNames[i];

            const auto it = staName2loifId.find( thisStation );
            if ( it != staName2loifId.end() && it->second == bbcId ) {
                ids.push_back( i );
            }
        }
        auto bbc = make_shared<Bbc>( bbcId );

        for ( int i = 0; i < ifs.size(); ++i ) {
            const string &anyIf = ifs.at( i );
            vector<string> splitVector;
            boost::split( splitVector, anyIf, boost::is_space(), boost::token_compress_on );
            string bbc_assign_id = ( boost::format( "&BBC%02d" ) % ( i + 1 ) ).str();
            string if_id_link = "&IF_" + splitVector.at( 2 );

            bbc->addBbc( bbc_assign_id, static_cast<unsigned int>( i + 1 ), if_id_link );
        }

        // add BBC to Mode
        addBlock( bbc );
        mode->addBlock( bbc, ids );
    }
}


void ObservingMode::readSkdTrackFrameFormat( const std::shared_ptr<Mode> &mode, const SkdCatalogReader &skd ) {
    const auto &staNames = skd.getStaNames();

    const map<string, vector<string>> &cat = skd.getEquipCatalog();
    const map<string, vector<string>> &acat = skd.getAntennaCatalog();
    vector<string> recorders;

    for ( const auto &staName : staNames ) {
        const string &id_EQ = skd.equipKey( staName );

        if ( cat.find( id_EQ ) == cat.end() ) {
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( fatal ) << "equip catalog not found (" << id_EQ << ")";
#else
            cout << "[fatal] equip catalog not found (" << id_EQ << ")";
#endif
        }
        const vector<string> &eq = cat.at( id_EQ );
        string recorder = eq.at( eq.size() - 1 );
        boost::algorithm::to_lower(recorder);
        if ( recorder == "mark5b" || recorder == "k5" ) {
            recorder = "Mark5B";
        } else if ( recorder == "mark5a" ) {
            recorder = "MARK5A";
        } else if ( recorder == "flexbuff" ) {
            recorder = "VDIF/8032/2";
        } else {
            recorder = "Mark4";
        }
        recorders.push_back( recorder );
    }

    vector<string> uniqueRecorders = recorders;
    sort( uniqueRecorders.begin(), uniqueRecorders.end() );
    uniqueRecorders.erase( unique( uniqueRecorders.begin(), uniqueRecorders.end() ), uniqueRecorders.end() );

    for ( const auto &thisRecorder : uniqueRecorders ) {
        // get corresponding station ids
        vector<unsigned long> ids;
        for ( unsigned long i = 0; i < recorders.size(); ++i ) {
            const auto &thisStation = staNames[i];
            if ( thisRecorder == recorders[i] ) {
                ids.push_back( i );
            }
        }

        addBlock( thisRecorder );
        mode->addBlock( make_shared<std::string>( thisRecorder ), ids );
    }
}


void ObservingMode::toVexModeBlock( std::ofstream &of ) const {
    for ( const auto &any : modes_ ) {
        any->toVexModeDefiniton( of, stationNames_ );
    }
}


void ObservingMode::toVexFreqBlock( std::ofstream &of ) const {
    for ( const auto &any : freqs_ ) {
        string c = "* ";
        for ( const auto &mode : modes_ ) {
            c.append( mode.get()->getName() );
            const auto &o_all = mode->getAllStationsWithBlock( any );
            if ( o_all.is_initialized() ) {
                for ( auto i : *o_all ) {
                    c.append( " : " ).append( stationNames_.at( i ) );
                }
            }
            c.append( "; " );
        }

        any->toVexFreqDefinition( of, c );
    }
}


void ObservingMode::toVexBbcBlock( std::ofstream &of ) const {
    for ( const auto &any : bbcs_ ) {
        string c = "* ";
        for ( const auto &mode : modes_ ) {
            c.append( mode.get()->getName() );
            const auto &o_all = mode->getAllStationsWithBlock( any );
            if ( o_all.is_initialized() ) {
                for ( auto i : *o_all ) {
                    c.append( " : " ).append( stationNames_.at( i ) );
                }
            }
            c.append( "; " );
        }

        any->toVexBbcDefinition( of, c );
    }
}


void ObservingMode::toVexIfBlock( std::ofstream &of ) const {
    for ( const auto &any : ifs_ ) {
        string c;
        for ( const auto &mode : modes_ ) {
            c.append( mode.get()->getName() );
            const auto &o_all = mode->getAllStationsWithBlock( any );
            if ( o_all.is_initialized() ) {
                for ( auto i : *o_all ) {
                    c.append( " : " ).append( stationNames_.at( i ) );
                }
            }
            c.append( "; " );
        }

        any->toVecIfDefinition( of, c );
        of << "*\n";
    }
}


void ObservingMode::toVexTracksBlock( std::ofstream &of ) const {
    for ( const auto &any : tracks_ ) {
        string c = "* ";
        for ( const auto &mode : modes_ ) {
            c.append( mode.get()->getName() );
            const auto &o_all = mode->getAllStationsWithBlock( any );
            if ( o_all.is_initialized() ) {
                for ( auto i : *o_all ) {
                    c.append( " : " ).append( stationNames_.at( i ) );
                }
            }
            c.append( "; " );
        }
        any->toVexTracksDefinition( of, c );
    }
    toTrackFrameFormatDefinitions( of );
}


void ObservingMode::toTrackFrameFormatDefinitions( std::ofstream &of ) const {
    for ( const auto &any : trackFrameFormats_ ) {
        string c = "* ";
        for ( const auto &mode : modes_ ) {
            c.append( mode->getName() );
            const auto &o_all = mode->getAllStationsWithBlock( any );
            if ( o_all.is_initialized() ) {
                for ( auto i : *o_all ) {
                    c.append( " : " ).append( stationNames_.at( i ) );
                }
            }
            c.append( "; " );
        }

        of << "    def " << *any << "_format;    " << c << "\n";
        of << "        track_frame_format = " << *any << ";\n";
        of << "    enddef;\n";
    }
}


void ObservingMode::summary( std::ofstream &of ) const {
    if ( ObservingMode::type != ObservingMode::Type::simple ) {
        of << "Summary of observing mode(s):\n";
        for ( const auto &band : bands ) {
            double meanFrequency = 0;
            auto it = wavelengths.find( band );
            if ( it == wavelengths.end() ) {
                continue;
            } else {
                meanFrequency = util::wavelength2frequency( it->second ) * 1e-6;
            }
            of << boost::format( "    band %2s mean frequency %9.3f [MHz]\n" ) % band % meanFrequency;
        }
        of << "\n";

        for ( const auto &any : modes_ ) {
            any->summary( of, stationNames_ );
        }

    } else {
        of << "Simple observing mode used!\n";
        for ( const auto &any : bands ) {
            double rec = getMode( 0 )->recordingRate( 0, 1, any ) * 1e-6;
            double eff = getMode( 0 )->efficiency( 0, 1 );

            of << "    band: " << any << "\n";
            of << boost::format( "        all stations: recording rate %7.2f [Mbit/s] with efficiency factor %.4f\n" ) %
                      rec % eff;
        }
    }
}


void ObservingMode::operationNotesSummary( std::ofstream &of ) const {
    if ( ObservingMode::type != ObservingMode::Type::simple ) {
        of << "Recording mode:\n";
        for ( const auto &any : modes_ ) {
            any->operationNotesSummary( of, stationNames_ );
        }
    } else {
        of << "Simple observing mode used!\n";
    }
}


void ObservingMode::calcMeanFrequencies() {
    for ( const auto &band : bands ) {
        vector<double> frequencies;
        for ( const auto &freq : freqs_ ) {
            auto tmp = freq->getFrequencies( band );
            frequencies.insert( frequencies.end(), tmp.begin(), tmp.end() );
        }

        double meanFrequency = std::accumulate( frequencies.begin(), frequencies.end(), 0.0 ) / frequencies.size();
        double meanWavelength = util::freqency2wavelenth( meanFrequency * 1e6 );
        wavelengths[band] = meanWavelength;
    }
}


void ObservingMode::addDummyBands( const std::map<std::string, std::vector<double>> &band ) {
    for ( const auto &any : band ) {
        bands.insert( any.first );
        double mfreq = accumulate( any.second.begin(), any.second.end(), 0.0 ) / any.second.size();
        wavelengths[any.first] = mfreq;
    }
}
