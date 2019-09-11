//
// Created by hwolf on 05.02.2019.
//

#include "SatelliteMain.h"
#include <stdfix.h>

using namespace std;
using namespace VieVS;

SatelliteMain::SatelliteMain( const VieVS::Network &network ) { network_ = network; }

void SatelliteMain::run() {
    ScanTimes::setAlignmentAnchor( ScanTimes::AlignmentAnchor::individual );

    // read in satellites
    vector<Satellite> satellites = Satellite::readSatelliteFile( "../../CATALOGS/satellites.txt" );
    std::cout << "Choose Satellite to observe  (press number in brackets): " << std::endl;
    for ( int k = 0; k < satellites.size(); k++ ) {
        std::cout << "[" << k << "]: " << satellites[k].getHeader() << std::endl;
    }
    int sat;  //=7;
    cin >> sat;

    // generate start Time and End Time
    DateTime start_date( TimeSystem::startTime.date().year(), TimeSystem::startTime.date().month(),
                         TimeSystem::startTime.date().day(), TimeSystem::startTime.time_of_day().hours(),
                         TimeSystem::startTime.time_of_day().minutes(), TimeSystem::startTime.time_of_day().seconds() );
    DateTime end_date( start_date.AddSeconds( TimeSystem::duration ) );

    // create Satellite Output
    SatelliteOutput satOut = SatelliteOutput( network_, satellites[sat], start_date, end_date );
    satOut.printStart();
    satOut.printStations();

    // calculate the passes for each Station
    std::vector<std::vector<Satellite::SatPass>> passList =
        satellites[sat].generatePassList( network_, start_date, end_date, 60 );
    satOut.printPassList( passList );

    // PassList to Overlaps
    std::vector<SatelliteObs> overlaps = SatelliteObs::passList2Overlap( passList );
    satOut.printOverlaps( overlaps );

    // Overlaps to PointingVector
    std::vector<std::vector<std::vector<VieVS::PointingVector>>> pvRes =
        SatelliteObs::PassList2pv( passList, satellites[sat], this->network_, start_date );
    satOut.printPV( pvRes );

    // SatelliteObs::createOutput(PassList_, overlaps, network_);

    // create Scan List
    vector<VieVS::Scan> scanList =
        SatelliteObs::createScanList( passList, network_, satellites[sat], start_date );
    satOut.addScans( scanList );
    satOut.printScan();
}

void SatelliteMain::initialize( const std::string &pathAntenna, const std::string &pathEquip,
                 const std::string &pathPosition, const std::string &pathMask,
                 boost::posix_time::ptime startTime, boost::posix_time::ptime endTime,
                 const std::vector<std::string> &stations){

    // initialize times
    int sec_ = startTime.time_of_day().total_seconds();
    double mjdStart = startTime.date().modjulian_day() + sec_ / 86400.0;

    int sec = util::duration( startTime, endTime );
    auto duration = static_cast<unsigned int>( sec );

    TimeSystem::mjdStart = mjdStart;
    TimeSystem::startTime = startTime;
    TimeSystem::endTime = endTime;
    TimeSystem::duration = duration;

    startDate_ = DateTime( TimeSystem::startTime.date().year(), TimeSystem::startTime.date().month(),
                         TimeSystem::startTime.date().day(), TimeSystem::startTime.time_of_day().hours(),
                         TimeSystem::startTime.time_of_day().minutes(), TimeSystem::startTime.time_of_day().seconds() );
    endDate_ = DateTime( startDate_.AddSeconds( TimeSystem::duration ) );


    // initialize Network
    SkdCatalogReader skdCatalogs;
    skdCatalogs.setStationNames(stations);
    skdCatalogs.setCatalogFilePathes(pathAntenna, pathEquip, "", "", "", "", pathMask, "", pathPosition, "", "", "", "");
    skdCatalogs.initializeStationCatalogs();

    string outPath = "satelliteSchedulingLog.txt";
    ofstream of( outPath );

    LookupTable::initialize();
    Initializer init;

    std::unordered_map<std::string, unsigned int> band2channel{{"X", 10}, {"S", 6}};
    std::unordered_map<std::string, double> band2wavelength{{"L", 0.3},      {"S", 0.131},   {"C", 0.06},
                                                            {"X", 0.0349},   {"Ku", 0.0231}, {"K", 0.0134},
                                                            {"Ka", 0.01000}, {"E", 0.005},   {"W", 0.00375}};
    init.initializeObservingMode( stations.size(), 32, 2, band2channel, band2wavelength );

    init.initializeAstronomicalParameteres();
    init.createStations( skdCatalogs, of );

    network_ = init.network_;

    // set scan alignment anchor
    ScanTimes::setAlignmentAnchor( ScanTimes::AlignmentAnchor::individual );

}


vector<Satellite> SatelliteMain::readSatelliteFile( const std::string &pathToTLE ) const{
    return  Satellite::readSatelliteFile( pathToTLE );
}


vector<VieVS::Scan> SatelliteMain::generateScanList ( const vector<Satellite> &satellites ) const{

    //[satellite][station][pass]
    std::vector<std::vector<std::vector<Satellite::SatPass>>> passLists;
    for(const auto & thisSat : satellites){
        passLists.push_back(thisSat.generatePassList(network_, startDate_, endDate_, 60));
    }


    vector<VieVS::Scan> list;
    for( int i=0; i< satellites.size(); ++i){
        auto scanList = SatelliteObs::createScanList( passLists[i], network_, satellites[i], startDate_ );
        list.insert(list.end(), scanList.begin(), scanList.end());
    }

    return list;
}
