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

#include "SNR_table.h"


using namespace VieVS;
using namespace std;

unsigned long SNR_table::nextId = 0;


SNR_table::SNR_table( const std::string &file ) : VieVS_Object( nextId++ ) { of = ofstream( file ); }


void SNR_table::writeTable( const Network &network, const SourceList &sourceList, std::vector<Scan> &scans,
                            const std::shared_ptr<const ObservingMode> &obsModes ) {
    const set<string> &bands = ObservingMode::bands;

    vector<string> stations;
    for ( const auto &any : network.getStations() ) {
        stations.push_back( any.getAlternativeName() );
    }
    obsModes->getMode( 0 )->summary( of, stations );
    of << "------------------------------------------------------------------------------------------------------------"
          "-------------\n";

    of << boost::format( "%-9s  %-8s  %8s  %-14s  %-8s  %6s  %4s  %4s  %7s  %5s  %5s  %7s  %7s  %9s\n" ) % "scan" %
              "baseline" % "SEFD1" % "SEFD2" % "source" % "flux" % "band" % "dur" % "SNR" % "el1" % "el2" % "unaz1" %
              "unaz2" % "scheduled";

    for ( unsigned long iScan = 0; iScan < scans.size(); ++iScan ) {
        Scan &thisScan = scans[iScan];
        vector<unsigned long> staids = thisScan.getStationIds();
        unsigned long nsta = staids.size();
        string scanName = thisScan.getName( iScan, scans );

        const auto &src = sourceList.getSource( thisScan.getSourceId() );

        for ( unsigned long staidx1 = 0; staidx1 < nsta; ++staidx1 ) {
            unsigned long staid1 = staids[staidx1];
            const Station &sta1 = network.getStation( staid1 );

            for ( unsigned long staidx2 = staidx1 + 1; staidx2 < nsta; ++staidx2 ) {
                unsigned long staid2 = staids[staidx2];
                const Station &sta2 = network.getStation( staid2 );

                unsigned int dur = thisScan.getTimes().getObservingDuration( staidx1, staidx2 );
                unsigned int startTime = thisScan.getTimes().getObservingTime( Timestamp::start );

                double date1 = 2400000.5;
                double date2 = TimeSystem::mjdStart + static_cast<double>( startTime ) / 86400.0;
                double gmst = iauGmst82( date1, date2 );

                for ( const auto &band : bands ) {
                    if ( staid1 > staid2 ) {
                        swap( staid1, staid2 );
                    }

                    const auto &dxyz = network.getDxyz( staid1, staid2 );
                    const PointingVector &pv1 =
                        thisScan.getPointingVector( static_cast<int>( staidx1 ), Timestamp::start );
                    double el1 = pv1.getEl();
                    const PointingVector &pv2 =
                        thisScan.getPointingVector( static_cast<int>( staidx2 ), Timestamp::start );
                    double el2 = pv2.getEl();
                    double SEFD_src;
                    if ( src->needsElDistFlux() ) {
                        SEFD_src = src->observedFluxElDist( band, startTime, sta1.getPosition(), sta2.getPosition(),
                                                            el1, el2 );
                    } else if ( src->hasFluxInformation( band ) ) {
                        // calculate observed flux density for each band
                        SEFD_src = src->observedFlux( band, startTime, gmst, dxyz );
                    } else if ( ObservingMode::sourceBackup[band] == ObservingMode::Backup::internalModel ) {
                        // calculate observed flux density based on model
                        double wavelength = ObservingMode::wavelengths[band];
                        SEFD_src = src->observedFlux_model( wavelength, startTime, gmst, dxyz );
                    } else {
                        SEFD_src = 1e-3;
                    }

                    // calculate system equivalent flux density for each station
                    double az1 = pv1.getAz();
                    double SEFD_sta1 = sta1.getEquip().getSEFD( band, el1 );

                    double az2 = pv2.getAz();
                    double SEFD_sta2 = sta2.getEquip().getSEFD( band, el2 );

                    double efficiency = obsModes->getMode( 0 )->efficiency( staid1, staid2 );
                    double recRate = obsModes->getMode( 0 )->recordingRate( staid1, staid2, band );

                    double SNR = efficiency * SEFD_src / sqrt( SEFD_sta1 * SEFD_sta2 ) * sqrt( recRate * dur );

                    unsigned long obsIdx = thisScan.indexOfObservation( staid1, staid2 );
                    string sscheduled;
                    if ( obsIdx != -1 ) {
                        sscheduled = "true";
                        //                        thisScan.refObservation(obsIdx).setSNR(SNR);
                    } else {
                        sscheduled = "false";
                    }

                    string blName = sta1.getAlternativeName() + "-" + sta2.getAlternativeName();

                    of << boost::format(
                              "%-9s  %=8s  %8.2f  %8.2f  %-14s  %6.3f  %=4s  %4d  %7.2f  %5.2f  %5.2f  %7.2f  %7.2f  "
                              "%=9s\n" ) %
                              scanName % blName % SEFD_sta1 % SEFD_sta2 % src->getName() % SEFD_src % band % dur % SNR %
                              ( el1 * rad2deg ) % ( el2 * rad2deg ) % ( pv1.getAz() * rad2deg ) %
                              ( pv2.getAz() * rad2deg ) % sscheduled;
                }
            }
        }
    }
}
