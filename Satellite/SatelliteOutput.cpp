//
// Created by hwolf on 8/1/19.
//

#include "SatelliteOutput.h"

using namespace std;
using namespace VieVS;
unsigned long SatelliteOutput::nextId = 0;


SatelliteOutput::SatelliteOutput( Network &network_, Satellite sat, DateTime start, DateTime end )
    : VieVS_NamedObject( "", nextId++ ), network_{network_}, satellite{sat}, start_date{start}, end_date{end} {}

SatelliteOutput::~SatelliteOutput() {}

void SatelliteOutput::addScans( std::vector<Scan> scanList ) { this->scans_ = std::move( scanList ); }


void SatelliteOutput::printStart() {
    std::cout << ".------------------------------------------------------.\n";
    std::cout << "| Start of Experiment: " << boost::format( "%8s  |\n" ) % this->start_date;
    std::cout << "| End of Experiment:   " << boost::format( "%8s  |\n" ) % this->end_date;
    std::cout << "'------------------------------------------------------'\n";
}


void SatelliteOutput::printStations() {
    std::cout << ".--------------------------------." << std::endl;
    std::cout << "| participating stations:        |" << std::endl;
    std::cout << "|--------------------------------|" << std::endl;
    for ( const auto &any : this->network_.getStations() ) {
        std::cout << boost::format( "| %-8s         |  %2s  |  %2s  |" ) % any.getName() % any.getAlternativeName() %
                         any.getId()
                  << std::endl;
    }
    std::cout << "'--------------------------------'" << std::endl << std::endl;
}

void SatelliteOutput::printPassList( std::vector<std::vector<Satellite::SatPass>> PassList ) {
    std::stringstream ss;
    ss << ".-----------------------------------------------------------------------------------------------------------"
          "------------------.\n";
    ss << "|" << std::setw( 16 ) << "Satellite" << std::setw( 10 ) << "|" << std::setw( 21 ) << "Start [UT]"
       << std::setw( 14 ) << "|" << std::setw( 21 ) << " End [UT]" << std::setw( 14 ) << "|" << std::setw( 14 )
       << " Duration" << std::setw( 6 ) << "|" << std::setw( 7 ) << " Station" << std::setw( 2 ) << "|" << std::endl;

    if ( PassList.empty() ) {
        ss << "|-------------------------------------------------------------------------------------------------------"
              "----------------------|\n";
        ss << "| No passes found!                                                                                      "
              "                      |\n";
    } else {
        for ( const auto &any : PassList ) {
            ss << std::right << std::setprecision( 1 ) << std::fixed;
            ss << "|---------------------------------------------------------------------------------------------------"
                  "--------------------------|"
               << std::endl;
            auto itr = any.begin();
            do {
                string StationName;
                for ( const Station &station : this->network_.getStations() ) {
                    if ( itr->StationID == station.getId() ) {
                        StationName = station.getAlternativeName();
                    }
                }
                ss << boost::format( "| %-22s|  %-8s  |  %-8s  |  %-8s  |    %-3s  |\n" ) % satellite.getName() %
                          itr->start % itr->end % ( itr->end - itr->start ) % StationName;
            } while ( ++itr != any.end() );
        }
    }
    std::cout << ss.str()
              << "'----------------------------------------------------------------------------------------------------"
                 "-------------------------'\n\n";
}

void SatelliteOutput::printScan() {
    for ( unsigned long i = 0; i < this->scans_.size(); i++ ) {
        std::cout << ".------------------------------------------------------------------------------------------------"
                     "----------------------------------------------.\n";
        const auto &thisScan = this->scans_[i];
        thisScan.outputSatScan( i, this->network_, this->satellite );
        std::cout << std::endl;
    }
}


void SatelliteOutput::printOverlaps( std::vector<SatelliteObs> overlaps ) {
    if ( overlaps.begin() == overlaps.end() ) {
        std::cout << ".-------------------------------------------.\n";
        std::cout << "| No overlaps found!                        |\n";
        std::cout << "'-------------------------------------------'\n";
    } else {
        std::cout << "Overlaps:\n";
        std::cout << ".------------------------------------------------------------------------------------------------"
                     "---------------------------------------------.\n";
        std::cout << "|" << std::setw( 16 ) << "Satellite" << std::setw( 10 ) << "|" << std::setw( 21 ) << "Start [UT]"
                  << std::setw( 14 ) << "|" << std::setw( 21 ) << " End [UT]" << std::setw( 14 ) << "|"
                  << std::setw( 14 ) << "Duration" << std::setw( 6 ) << "|" << std::setw( 16 ) << "Stations"
                  << std::setw( 10 ) << "|" << std::endl;
        std::cout << "|------------------------------------------------------------------------------------------------"
                     "---------------------------------------------|\n";

        std::vector<std::string> station_vector;
        std::stringstream sname;
        for ( unsigned long j = 0; j < overlaps.size(); j++ ) {
            sname.str( "" );
            for ( const unsigned long &i : overlaps.at( j ).getStationIDList() ) {
                string StationName;
                for ( const Station &station : this->network_.getStations() ) {
                    if ( i == station.getId() ) {
                        StationName = station.getAlternativeName();
                        sname << StationName << " ";
                        break;
                    }
                }
            }
            station_vector.push_back( sname.str() );
            std::cout << boost::format( "| %-22s|  %-8s  |  %-8s  |  %-8s  |  %-21s  |\n" ) % satellite.getName() %
                             overlaps.at( j ).getStart() % overlaps.at( j ).getEnd() %
                             ( overlaps.at( j ).getEnd() - overlaps.at( j ).getStart() ) % station_vector.at( j );
        }
        std::cout << "'------------------------------------------------------------------------------------------------"
                     "---------------------------------------------'\n\n";
    }
}

void SatelliteOutput::printPV( std::vector<std::vector<std::vector<VieVS::PointingVector>>> pvRes ) {
    for ( unsigned long i = 0; i < pvRes.size(); i++ ) {
        std::cout << "\n.----------------------------------------------------------------------------------------."
                  << std::endl;
        std::cout << boost::format(
                         "| Station:  %-8s                                                                     |\n" ) %
                         this->network_.getStations().at( i ).getName();
        std::cout << "|----------------------------------------------------------------------------------------|"
                  << std::endl;
        std::cout << "|              Time [UT]            |   Az [°]   |   El [°]   |   Dec [°]  |    LHA[°]   |"
                  << std::endl;
        if ( pvRes.at( i ).empty() ) {
            std::cout << "|----------------------------------------------------------------------------------------|"
                      << std::endl;
            std::cout << "| No Passes for this station.                                                           |"
                      << std::endl;
            std::cout << "'----------------------------------------------------------------------------------------'"
                      << std::endl;
            continue;
        } else {
            for ( const auto &pvPass : pvRes.at( i ) ) {
                std::cout
                    << "|----------------------------------------------------------------------------------------|"
                    << std::endl;
                for ( const auto &pv : pvPass ) {
                    DateTime t = this->start_date + TimeSpan( 0, 0, pv.getTime() );
                    std::cout << boost::format( "|  %-29s   |  %8.4f  |  %8.4f  |  %8.4f  |  %9.4f  |\n" ) % t %
                                     ( pv.getAz() * 180 / pi ) % ( pv.getEl() * 180 / pi ) % ( pv.getDc() * 180 / pi ) %
                                     ( pv.getHa() * 180 / pi );
                }
            }
        }
        std::cout << "'----------------------------------------------------------------------------------------'"
                  << std::endl;
    }
}
