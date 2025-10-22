//
// Created by mschartner on 10/22/25.
//

#ifndef VIESCHEDPP_SPACECRAFT_H
#define VIESCHEDPP_SPACECRAFT_H
#include <string>
#include <memory>
#include <utility>

#include "../SGP4/CoordTopocentric.h"
#include "../Station/Network.h"
#include "../Station/Station.h"
#include "AbstractSource.h"

namespace VieVS {
class Spacecraft : public AbstractSource {
public:

    Spacecraft( const std::string &name, std::unordered_map<std::string, std::unique_ptr<AbstractFlux>> &src_flux ); //todo...  pass lists of ra/dec/time/dist/station...


    std::pair<std::pair<double, double>, std::vector<double>> getSourceInCrs(
        unsigned int time, const std::shared_ptr<const Position> &sta_pos ) const override;

    std::pair<double, double> getRaDe( unsigned int time,
                                       const std::shared_ptr<const Position> &sta_pos ) const noexcept override {
        return calcRaDe( time, sta_pos );
    }

    void toVex( std::ofstream &of ) const override;

    void toVex( std::ofstream &of, const std::vector<unsigned int> &times,
                const std::shared_ptr<const Position> &sta_pos ) const override;

    void toNgsHeader( std::ofstream &of ) const override;

    std::pair<double, double> calcRaDe( unsigned int time, const std::shared_ptr<const Position> &sta_pos ) const;

    std::tuple<double, double, double, double> calcRaDeDistTime(
        unsigned int time, const std::shared_ptr<const Position> &sta_pos ) const noexcept override;

    std::string getNameTime( unsigned int t ) const {
        return ( boost::format( "%s<=>%s" ) % getName() % TimeSystem::time2string_doy_minus( t ) ).str();
    }

    static void extractEphemerisData( const std::string& folder, const std::string& name, const std::string& station );

private:
    static unsigned long nextId;   ///< next id for this object type

    std::vector<unsigned int> time_list_; ///< list of times corresponding to ra/dec
    std::vector<double> ra_list_;   ///< list of right ascensions at given times
    std::vector<double> dec_list_;  ///< list of declinations at given times
    std::vector<double> dist_list_; ///< list of distances at given times
};
}

#endif  // VIESCHEDPP_SPACECRAFT_H
