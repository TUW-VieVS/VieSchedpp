//
// Created by hwolf on 05.02.2019.
//

#include "SatelliteMain.h"

using namespace std;
using namespace VieVS;

SatelliteMain::SatelliteMain( const VieVS::Network &network ) { network_ = network; }

void SatelliteMain::run() {
    ScanTimes::setAlignmentAnchor( ScanTimes::AlignmentAnchor::individual );
    std::vector<VieVS::Station> stations_ = this->network_.getStations();

    // read in satellites
    vector<Satellite> satellites_ = Satellite::readSatelliteFile( "../../CATALOGS/satellites.txt" );
    std::cout << "Choose Satellite to observe  (press number in brackets): " << std::endl;
    for ( int k = 0; k < satellites_.size(); k++ ) {
        std::cout << "[" << k << "]: " << satellites_[k].getHeader() << std::endl;
    }
    int sat;  //=7;
    cin >> sat;

    // generate start Time and End Time
    DateTime start_date( TimeSystem::startTime.date().year(), TimeSystem::startTime.date().month(),
                         TimeSystem::startTime.date().day(), TimeSystem::startTime.time_of_day().hours(),
                         TimeSystem::startTime.time_of_day().minutes(), TimeSystem::startTime.time_of_day().seconds() );
    DateTime end_date( start_date.AddSeconds( TimeSystem::duration ) );

    // create Satellite Output
    SatelliteOutput SatOut = SatelliteOutput( network_, satellites_[sat], start_date, end_date );
    SatOut.printStart();
    SatOut.printStations();

    // calculate the passes for each Station
    std::vector<std::vector<Satellite::SatPass>> PassList_ =
        satellites_[sat].GeneratePassList( network_, start_date, end_date, 60 );
    SatOut.printPassList( PassList_ );

    // PassList to Overlaps
    std::vector<SatelliteObs> overlaps = SatelliteObs::passList2Overlap( PassList_ );
    SatOut.printOverlaps( overlaps );

    // Overlaps to PointingVector
    std::vector<std::vector<std::vector<VieVS::PointingVector>>> pvRes =
        SatelliteObs::PassList2pv( PassList_, satellites_[sat], this->network_, start_date );
    SatOut.printPV( pvRes );

    // SatelliteObs::createOutput(PassList_, overlaps, network_);

    // create Scan List
    vector<VieVS::Scan> scanList =
        SatelliteObs::createScanList( PassList_, this->network_, satellites_[sat], start_date );
    SatOut.addScans( scanList );
    SatOut.printScan();
}