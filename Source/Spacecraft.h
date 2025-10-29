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
    void printEphemSample(const std::string &spacecraft, const std::string &station, std::size_t n = 5, std::ostream &os = std::cout) const;

    Spacecraft( const std::string &name, std::unordered_map<std::string, std::unique_ptr<AbstractFlux>> &src_flux,
        std::unordered_map<std::string, std::vector<std::tuple<unsigned int,double,double,double>>> const &EphemerisMap );


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
    std::pair<double, double> calcRaDe2( unsigned int time, const std::string &stationID ) const;


    std::tuple<double, double, double, double> calcRaDeDistTime(
        unsigned int time, const std::shared_ptr<const Position> &sta_pos ) const noexcept override;

    std::string getNameTime( unsigned int t ) const {
        return ( boost::format( "%s<=>%s" ) % getName() % TimeSystem::time2string_doy_minus( t ) ).str();
    }

    static boost::optional<std::vector<std::tuple<unsigned int,double,double,double>>>
    extractEphemerisData(const std::string &folder,
                         const std::string &name,
                         const std::string &station);
private:
    static unsigned long nextId;   ///< next id for this object type


    std::unordered_map<std::string, std::vector<unsigned int>> time_list_; ///< per station
    std::unordered_map<std::string, std::vector<double>> ra_list_;         ///< per station
    std::unordered_map<std::string, std::vector<double>> dec_list_;        ///< per station
    std::unordered_map<std::string, std::vector<double>> dist_list_;       ///< per station
};
}

#endif  // VIESCHEDPP_SPACECRAFT_H
