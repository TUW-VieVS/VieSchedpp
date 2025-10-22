//
// Created by mschartner on 10/22/25.
//

#include "Spacecraft.h"


using namespace std;
using namespace VieVS;
namespace fs = std::filesystem;

Spacecraft::Spacecraft(const std::string &name, std::unordered_map<std::string, std::unique_ptr<AbstractFlux>> &src_flux )
    :AbstractSource( name, name, src_flux ){
    // TODO: expand constructor to pass lists of ra/dec/time/dist/station and store them appropriately in private members

}

std::pair<double, double> Spacecraft::calcRaDe( unsigned int time,
                                                const std::shared_ptr<const Position> &sta_pos ) const {
    const string &station = sta_pos->getName();

    // TODO: implement interpolation of ra/dec from stored lists
    double ra = 0;
    double de = 0;
    return {ra, de};
}

std::tuple<double, double, double, double> Spacecraft::calcRaDeDistTime(
    unsigned int time, const std::shared_ptr<const Position> &sta_pos ) const noexcept {
    // TODO: implement interpolation of ra/dec/dist from stored lists - time is not relevant
    const string &station = sta_pos->getName();

    double ra = 0;
    double de = 0;
    double dist = 0;

    return {ra, de, dist, time};
}

void Spacecraft::extractEphemerisData( const std::string &folder, const std::string &name,
                                       const std::string &station ) {
     boost::posix_time::ptime session_start = TimeSystem::startTime;
     boost::posix_time::ptime session_end = TimeSystem::endTime;

    // TODO: Loop over files in folder, read ephemeris data for spacecraft 'name' as seen from 'station'
    // ensure that all relevant data is there and return it (potentially use boost::optional as return to handle missing data)

    if (!fs::exists(folder)) return;
    if (!fs::is_directory(folder)) return;

    for (const auto& entry : fs::directory_iterator(folder)) {
        if (!entry.is_regular_file()) continue;

        const auto& filepath = entry.path();
        std::string filename = filepath.filename().string();
    }
}

std::pair<std::pair<double, double>, std::vector<double>> Spacecraft::getSourceInCrs(
    unsigned int time, const std::shared_ptr<const Position> &sta_pos ) const {

    auto srcRaDe = getRaDe( time, sta_pos );
    double cosDe = cos( srcRaDe.second );

    return { srcRaDe, { cosDe * cos( srcRaDe.first ), cosDe * sin( srcRaDe.first ), sin( srcRaDe.second ) } };
}



void Spacecraft::toVex( std::ofstream &of ) const {
    string eol = ";\n";
    of << "    def " << getName() << eol;
    of << "        source_type = spacecraft" << eol;
    of << "    enddef;\n";
}

void Spacecraft::toVex( std::ofstream &of, const std::vector<unsigned int> &times,
                        const std::shared_ptr<const Position> &sta_pos ) const {
    string eol = ";\n";
    for ( unsigned int t : times ) {
        string name = getNameTime( t );
        auto rade = getRaDe( t, sta_pos );

        of << "    def " << name << eol;
        of << "        source_type = star" << eol;
        of << "        source_name = " << name << eol;
        of << "        ra = " << getRaString( rade.first ) << eol;
        of << "        dec = " << getDeString( rade.second ) << eol;
        of << "        ref_coord_frame = J2000" << eol;
        of << "        ra_rate = 0 asec/yr" << eol;
        of << "        dec_rate = 0 asec/yr" << eol;
        of << "    enddef;\n";
    }
}
void Spacecraft::toNgsHeader( std::ofstream &of ) const {
    string name = getName();
    of << "spacecraft " << getName() << "\n";
}

