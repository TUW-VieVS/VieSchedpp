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

#include "Output.h"


using namespace std;
using namespace VieVS;
unsigned long Output::nextId = 0;


Output::Output( Scheduler &sched, std::string path, string fname, int version )
    : VieVS_NamedObject( move( fname ), nextId++ ),
      xml_{sched.xml_},
      network_{std::move( sched.network_ )},
      sources_{std::move( sched.sources_ )},
      scans_{std::move( sched.scans_ )},
      obsModes_{sched.obsModes_},
      path_{std::move( path )},
      multiSchedulingParameters_{std::move( sched.multiSchedulingParameters_ )},
      version_{version} {}


void Output::createAllOutputFiles( std::ofstream &of, const SkdCatalogReader &skdCatalogReader ) {
    for ( auto &sky : network_.refSkyCoverages() ) {
        sky.calculateSkyCoverageScores();
    }

    if ( scans_.empty() ) {
        return;
    }

    writeStatistics( of );

    if ( xml_.get<bool>( "VieSchedpp.output.createSummary", false ) ) {
        writeSkdsum();
    }
    if ( xml_.get<bool>( "VieSchedpp.output.createNGS", false ) ) {
        writeNGS();
    }
    if ( xml_.get<bool>( "VieSchedpp.output.createSKD", false ) ) {
        writeSkd( skdCatalogReader );
    }
    if ( xml_.get<bool>( "VieSchedpp.output.createVEX", false ) ) {
        writeVex();
    }
    if ( xml_.get<bool>( "VieSchedpp.output.createOperationsNotes", false ) ) {
        writeOperationsNotes();
    }
    if ( xml_.get<bool>( "VieSchedpp.output.createSourceGroupStatistics", false ) ) {
        writeSourceStatistics();
    }
    if ( xml_.get<bool>( "VieSchedpp.output.createSnrTable", false ) ) {
        writeSnrTable();
    }
    if ( false ) {
        writeAstFile();
    }
}


void Output::writeVex() {
    string fileName = getName();
    fileName.append( ".vex" );
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing vex file to: " << fileName;
#else
    cout << "[info] writing vex file to: " << fileName;
#endif
    Vex vex( path_ + fileName );
    vex.writeVex( network_, sources_, scans_, obsModes_, xml_ );
}


void Output::writeSkd( const SkdCatalogReader &skdCatalogReader ) {
    string fileName = getName();
    fileName.append( ".skd" );
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing skd file to: " << fileName;
#else
    cout << "[info] writing skd file to: " << fileName;
#endif
    Skd skd( path_ + fileName );
    skd.writeSkd( network_, sources_, scans_, skdCatalogReader, xml_ );
}


void Output::writeOperationsNotes() {
    string fileName = getName();
    fileName.append( ".txt" );
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing operation notes file to: " << fileName;
#else
    cout << "[info] writing operation notes file to: " << fileName;
#endif
    OperationNotes notes( path_ + fileName );
    notes.writeOperationNotes( network_, sources_, scans_, obsModes_, xml_, version_, multiSchedulingParameters_ );
}


void Output::writeSourceStatistics() {
    string fileName = getName();
    fileName.append( "_sourceStatistics.txt" );
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing source statistics file to: " << fileName;
#else
    cout << "[info] writing operation notes file to: " << fileName;
#endif
    SourceStatistics sourceStatistics( path_ + fileName );
    sourceStatistics.writeFile( network_, sources_, scans_, xml_ );
}


void Output::writeSkdsum() {
    string fileName = getName();
    fileName.append( "_skdsum.txt" );
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing skdsum to: " << fileName;
#else
    cout << "[info] writing skdsum to: " << fileName;
#endif
    OperationNotes notes( path_ + fileName );
    notes.writeSkdsum( network_, sources_, scans_ );
}


void Output::writeSnrTable() {
    string fileName = getName();
    fileName.append( ".snr" );
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing SNR table to: " << fileName;
#else
    cout << "[info] writing SNR table to: " << fileName;
#endif
    SNR_table snr( path_ + fileName );
    snr.writeTable( network_, sources_, scans_, obsModes_ );
}


void Output::writeAstFile() {
    string fileName = getName();
    fileName.append( ".ast" );
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing ast file to: " << fileName;
#else
    cout << "[info] writing ast file to: " << fileName;
#endif
    Ast ast( path_ + fileName );
    ast.writeAstFile( network_, sources_, scans_, xml_, obsModes_ );
}


void Output::writeNGS() {
    string fname;
    if ( version_ == 0 ) {
        fname = TimeSystem::time2date( TimeSystem::startTime ).erase( 0, 2 ).append( "VS_N000" );
    } else {
        fname = ( boost::format( "%sVS_v%03d" ) % TimeSystem::time2date( TimeSystem::startTime ).erase( 0, 2 ) %
                  ( version_ ) )
                    .str();
    }
#ifdef VIESCHEDPP_LOG
    BOOST_LOG_TRIVIAL( info ) << "writing empty NGS file to " << fname;
#else
    cout << "[info] writing empty NGS file to " << fname;
#endif

    string NGS_path = xml_.get( "VieSchedpp.output.NGS_directory", "" );

    ofstream of;
    if ( NGS_path.empty() ) {
        of.open( path_ + fname );
    } else {
        if ( NGS_path.back() != '/' && NGS_path.back() != '\\' ) {
            NGS_path.append( "/" );
        }

        of.open( NGS_path + fname );
        if ( !of.good() ) {
            of.open( path_ + fname );
#ifdef VIESCHEDPP_LOG
            BOOST_LOG_TRIVIAL( warning ) << "NGS output path changed to " << path_;
#else
            cout << "[warning] NGS output path changed to " << path_;
#endif
        }
    }

    of << "DATA IN NGS FORMAT FROM VieSched++\n";
    of << "This NGS file is empty and serves as input for simulations \n";
    for ( const auto &sta : network_.getStations() ) {
        string mount = sta.getAntenna().getMount();
        string mountNGS;
        if ( mount == "ALTAZ" ) {
            mountNGS = "AZEL";
        } else if ( mount == "EQUA" ) {
            mountNGS = "EQUA";
        } else if ( mount == "XY_E" ) {
            mountNGS = "X-YE";
        }

        of << boost::format( "%-8s   %14.5f %14.5f %14.5f %s %9.5f\n" ) % sta.getName() % sta.getPosition().getX() %
                  sta.getPosition().getY() % sta.getPosition().getZ() % mountNGS % sta.getAntenna().getOffset();
    }
    of << "$END\n";

    for ( const auto &src : sources_ ) {
        if ( src.getNTotalScans() == 0 ) {
            continue;
        }

        string strRa;
        {
            double ra = src.getRa();
            double h = rad2deg * ra / 15;
            auto m = fmod( h, 1. );
            m *= 60;
            auto s = fmod( m, 1. );
            s *= 60;
            strRa =
                ( boost::format( "%02d %02d   %08.5f " ) % static_cast<int>( h ) % static_cast<int>( m ) % s ).str();
        }

        string strDe;
        {
            double de = src.getDe();
            double d = rad2deg * de;
            auto m = abs( fmod( d, 1. ) );
            m *= 60;
            auto s = fmod( m, 1. );
            s *= 60;
            strDe =
                ( boost::format( "%+03d %02d   %08.5f" ) % static_cast<int>( d ) % static_cast<int>( m ) % s ).str();
        }

        of << boost::format( "%-8s  %s  %s\n" ) % src.getName() % strRa % strDe;
    }
    of << "$END\n";

    double refFreq = 0;
    if ( ObservingMode::bands.find( "X" ) != ObservingMode::bands.end() ) {
        refFreq = util::wavelength2frequency( ObservingMode::wavelengths["X"] );
        of << boost::format( "%20e %19s %s %s\n" ) % refFreq % "" % "GR" % "PH";
        of << "$END\n";
    }

    boost::posix_time::ptime start = TimeSystem::startTime;
    unsigned long counter = 1;

    for ( const auto &any : scans_ ) {
        for ( int i = 0; i < any.getNObs(); ++i ) {
            const Observation &obs = any.getObservation( i );
            string sta1 = network_.getStation( obs.getStaid1() ).getName();
            string sta2 = network_.getStation( obs.getStaid2() ).getName();
            if ( sta1 > sta2 ) {
                swap( sta1, sta2 );
            }
            string src = sources_[obs.getSrcid()].getName();
            unsigned int time = obs.getStartTime();

            boost::posix_time::ptime tmp = TimeSystem::internalTime2PosixTime( time );
            int year = tmp.date().year();
            int month = tmp.date().month();
            int day = tmp.date().day();
            int hour = tmp.time_of_day().hours();
            int minute = tmp.time_of_day().minutes();
            double second = tmp.time_of_day().seconds();

            of << boost::format( "%-8s  %-8s  %-8s %4d %02d %02d %02d %02d  %13.10f            " ) % sta1 % sta2 % src %
                      year % month % day % hour % minute % second;
            of << boost::format( "%6d" ) % counter << "01\n";

            of << "    0000000.00000000    .00000  -000000.0000000000    .00000 0      I   ";
            of << boost::format( "%6d" ) % counter << "02\n";

            of << "    .00000    .00000    .00000    .00000   0.000000000000000        0.  ";
            of << boost::format( "%6d" ) % counter << "03\n";

            of << "       .00   .0       .00   .0       .00   .0       .00   .0            ";
            of << boost::format( "%6d" ) % counter << "04\n";

            of << "   -.00000   -.00000    .00000    .00000    .00000    .00000            ";
            of << boost::format( "%6d" ) % counter << "05\n";

            of << "     0.000    00.000   000.000   000.000    00.000    00.000 0 0        ";
            of << boost::format( "%6d" ) % counter << "06\n";

            of << "        0.0000000000    .00000        -.0000000000    .00000  0         ";
            of << boost::format( "%6d" ) % counter << "08\n";

            of << "          0.00000000    .00000        0.0000000000    .00000 0      I   ";
            of << boost::format( "%6d" ) % counter << "09\n";

            ++counter;
        }
    }

    of.close();
}


void Output::writeStatistics( std::ofstream &of ) {
    string oString;

    auto n_scans = static_cast<int>( scans_.size() );
    int n_standard = 0;
    int n_fillin = 0;
    int n_calibrator = 0;
    int n_single = 0;
    int n_subnetting = 0;
    int n_obs_total = 0;
    vector<unsigned int> nscan_sta( network_.getNSta(), 0 );
    vector<unsigned int> nobs_sta( network_.getNSta(), 0 );
    vector<unsigned int> nobs_bl( network_.getNBls(), 0 );
    vector<unsigned int> nscan_src( sources_.size(), 0 );
    vector<unsigned int> nobs_src( sources_.size(), 0 );

    for ( const auto &any : scans_ ) {
        switch ( any.getType() ) {
            case Scan::ScanType::fillin: {
                ++n_fillin;
                break;
            }
            case Scan::ScanType::calibrator: {
                ++n_calibrator;
                break;
            }
            case Scan::ScanType::standard: {
                ++n_standard;
                break;
            }
            case Scan::ScanType::highImpact: {
                ++n_standard;
                break;
            }
        }
        switch ( any.getScanConstellation() ) {
            case Scan::ScanConstellation::single: {
                ++n_single;
                break;
            }
            case Scan::ScanConstellation::subnetting: {
                ++n_subnetting;
                break;
            }
        }
        auto n_obs = any.getNObs();
        n_obs_total += n_obs;
        for ( int ista = 0; ista < any.getNSta(); ++ista ) {
            const PointingVector &pv = any.getPointingVector( ista );
            unsigned long id = pv.getStaid();
            ++nscan_sta[id];
        }
        for ( int ibl = 0; ibl < any.getNObs(); ++ibl ) {
            const Observation &obs = any.getObservation( ibl );
            ++nobs_sta[obs.getStaid1()];
            ++nobs_sta[obs.getStaid2()];
            ++nobs_bl[network_.getBaseline( obs.getStaid1(), obs.getStaid2() ).getId()];
        }
        unsigned long id = any.getSourceId();
        ++nscan_src[id];
        nobs_src[id] += n_obs;
    }
    int n_src = static_cast<int>( count_if( nscan_src.begin(), nscan_src.end(), []( int i ) { return i > 0; } ) );

    auto totalTime = static_cast<double>( TimeSystem::duration );
    vector<double> obsPer;
    for ( const auto &station : network_.getStations() ) {
        int t = station.getStatistics().totalObservingTime;
        obsPer.push_back( static_cast<double>( t ) / totalTime * 100 );
    }
    double obsMean = accumulate( obsPer.begin(), obsPer.end(), 0.0 ) / ( network_.getNSta() );

    vector<double> preobPer;
    for ( const auto &station : network_.getStations() ) {
        int t = station.getStatistics().totalPreobTime;
        preobPer.push_back( static_cast<double>( t ) / totalTime * 100 );
    }
    double preobMean = accumulate( preobPer.begin(), preobPer.end(), 0.0 ) / ( network_.getNSta() );

    vector<double> slewPer;
    for ( const auto &station : network_.getStations() ) {
        int t = station.getStatistics().totalSlewTime;
        slewPer.push_back( static_cast<double>( t ) / totalTime * 100 );
    }
    double slewMean = accumulate( slewPer.begin(), slewPer.end(), 0.0 ) / ( network_.getNSta() );

    vector<double> idlePer;
    for ( const auto &station : network_.getStations() ) {
        int t = station.getStatistics().totalIdleTime;
        idlePer.push_back( static_cast<double>( t ) / totalTime * 100 );
    }
    double idleMean = accumulate( idlePer.begin(), idlePer.end(), 0.0 ) / ( network_.getNSta() );

    vector<double> fieldPer;
    for ( const auto &station : network_.getStations() ) {
        int t = station.getStatistics().totalFieldSystemTime;
        fieldPer.push_back( static_cast<double>( t ) / totalTime * 100 );
    }
    double fieldMean = accumulate( fieldPer.begin(), fieldPer.end(), 0.0 ) / ( network_.getNSta() );

    vector<double> a13m30;
    vector<double> a25m30;
    vector<double> a37m30;
    vector<double> a13m60;
    vector<double> a25m60;
    vector<double> a37m60;
    for ( const auto &station : network_.getStations() ) {
        unsigned long id = station.getId();
        const auto &map = network_.getStaid2skyCoverageId();
        unsigned long skyCovId = map.at( id );
        const auto &skyCov = network_.getSkyCoverage( skyCovId );
        a13m30.push_back( skyCov.getSkyCoverageScore_a13m30() );
        a25m30.push_back( skyCov.getSkyCoverageScore_a25m30() );
        a37m30.push_back( skyCov.getSkyCoverageScore_a37m30() );
        a13m60.push_back( skyCov.getSkyCoverageScore_a13m60() );
        a25m60.push_back( skyCov.getSkyCoverageScore_a25m60() );
        a37m60.push_back( skyCov.getSkyCoverageScore_a37m60() );
    }
    double a13m30Mean = accumulate( a13m30.begin(), a13m30.end(), 0.0 ) / ( network_.getNSta() );
    double a25m30Mean = accumulate( a25m30.begin(), a25m30.end(), 0.0 ) / ( network_.getNSta() );
    double a37m30Mean = accumulate( a37m30.begin(), a37m30.end(), 0.0 ) / ( network_.getNSta() );
    double a13m60Mean = accumulate( a13m60.begin(), a13m60.end(), 0.0 ) / ( network_.getNSta() );
    double a25m60Mean = accumulate( a25m60.begin(), a25m60.end(), 0.0 ) / ( network_.getNSta() );
    double a37m60Mean = accumulate( a37m60.begin(), a37m60.end(), 0.0 ) / ( network_.getNSta() );

    oString.append( std::to_string( version_ ) ).append( "," );
    oString.append( std::to_string( n_scans ) ).append( "," );
    oString.append( std::to_string( n_single ) ).append( "," );
    oString.append( std::to_string( n_subnetting ) ).append( "," );
    oString.append( std::to_string( n_fillin ) ).append( "," );
    oString.append( std::to_string( n_calibrator ) ).append( "," );
    oString.append( std::to_string( n_obs_total ) ).append( "," );
    oString.append( std::to_string( network_.getNSta() ) ).append( "," );
    oString.append( std::to_string( n_src ) ).append( "," );

    oString.append( std::to_string( obsMean ) ).append( "," );
    oString.append( std::to_string( preobMean ) ).append( "," );
    oString.append( std::to_string( slewMean ) ).append( "," );
    oString.append( std::to_string( idleMean ) ).append( "," );
    oString.append( std::to_string( fieldMean ) ).append( "," );

    oString.append( std::to_string( a13m30Mean ) ).append( "," );
    oString.append( std::to_string( a25m30Mean ) ).append( "," );
    oString.append( std::to_string( a37m30Mean ) ).append( "," );
    oString.append( std::to_string( a13m60Mean ) ).append( "," );
    oString.append( std::to_string( a25m60Mean ) ).append( "," );
    oString.append( std::to_string( a37m60Mean ) ).append( "," );

    oString.append( WeightFactors::statisticsValues() );

    for ( auto any : obsPer ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : preobPer ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : slewPer ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : idlePer ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : fieldPer ) {
        oString.append( std::to_string( any ) ).append( "," );
    }

    for ( auto any : a13m30 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : a25m30 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : a37m30 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : a13m60 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : a25m60 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }
    for ( auto any : a37m60 ) {
        oString.append( std::to_string( any ) ).append( "," );
    }

    for ( int i = 0; i < network_.getNSta(); ++i ) {
        oString.append( std::to_string( nscan_sta[i] ) ).append( "," );
    }
    for ( int i = 0; i < network_.getNSta(); ++i ) {
        oString.append( std::to_string( nobs_sta[i] ) ).append( "," );
    }
    for ( int i = 0; i < network_.getNBls(); ++i ) {
        oString.append( std::to_string( nobs_bl[i] ) ).append( "," );
    }
    for ( int i = 0; i < sources_.size(); ++i ) {
        oString.append( std::to_string( nscan_src[i] ) ).append( "," );
    }
    for ( int i = 0; i < sources_.size(); ++i ) {
        oString.append( std::to_string( nobs_src[i] ) ).append( "," );
    }

    if ( multiSchedulingParameters_.is_initialized() ) {
        oString.append( multiSchedulingParameters_->statisticsOutput() );
    }

#ifdef _OPENMP
#pragma omp critical
#endif
    { of << oString << endl; };
}
