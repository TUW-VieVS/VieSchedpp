//
// Created by mschartner on 12/3/21.
//

#ifndef VIESCHEDPP_STPPARSER_H
#define VIESCHEDPP_STPPARSER_H

#include "../Misc/VieVS_Object.h"
#include "../Misc/util.h"
#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#endif
#include <utility>

#include "../Misc/TimeSystem.h"
#include "../ObservingMode/ObservingMode.h"
#include "../Station/Antenna/AbstractAntenna.h"
#include "../Station/Antenna/Antenna_AzEl.h"
#include "../Station/Antenna/Antenna_AzEl_acceleration.h"
#include "../Station/CableWrap/AbstractCableWrap.h"
#include "../Station/CableWrap/CableWrap_AzEl.h"
#include "../Station/Equip/AbstractEquipment.h"
#include "../Station/Equip/Equipment_elTable.h"
#include "../Station/HorizonMask/AbstractHorizonMask.h"
#include "../Station/HorizonMask/HorizonMask_step.h"
#include "../Station/Position.h"


namespace VieVS {

/**
 * @class SkdCatalogReader
 * @brief sked catalog reader class
 *
 * @author Matthias Schartner
 * @date 07.12.2017
 */

class StpParser : public VieVS_Object {
   public:
    explicit StpParser( std::string fname ) : VieVS_Object( nextId++ ), fname_{ std::move( fname ) } {};

    bool exists() {
        std::ifstream f( fname_ );
        return f.good();
    }

    void parse();

    bool hasValidAntenna() { return antenna_ != nullptr; }

    bool hasValidCableWrap() { return cableWrap_ != nullptr; }

    bool hasValidPosition() { return position_ != nullptr; }

    bool hasValidEquipment() { return equip_ != nullptr; }

    bool hasValidHorizonMask() { return mask_ != nullptr; }

    std::shared_ptr<AbstractAntenna> getAntenna() { return antenna_; }

    std::shared_ptr<AbstractCableWrap> getCableWrap() { return cableWrap_; }

    std::shared_ptr<Position> getPosition() { return position_; }

    std::shared_ptr<AbstractEquipment> getEquip() { return equip_; }

    std::shared_ptr<AbstractHorizonMask> getHorizionMask() { return mask_; }

   private:
    static unsigned long nextId;  ///< next id for this object type


    std::string fname_;
    std::string version_;
    std::string mount_;
    double preob_;
    double postob_;
    std::string recorder_;

    std::shared_ptr<AbstractAntenna> antenna_ = nullptr;      ///< station antenna
    std::shared_ptr<AbstractCableWrap> cableWrap_ = nullptr;  ///< station cable wrap
    std::shared_ptr<Position> position_ = nullptr;            ///< station position
    std::shared_ptr<AbstractEquipment> equip_ = nullptr;      ///< station equipment
    std::shared_ptr<AbstractHorizonMask> mask_ = nullptr;     ///< station horizon mask

    struct Tsys {
        boost::posix_time::ptime start;
        boost::posix_time::ptime end;
        std::vector<double> elevations;
        std::vector<std::tuple<std::string, If::Polarization, std::vector<double>>> polvals;
    };
    static Tsys parse_tsys( std::ifstream &f, const std::string &l );

    struct Gain {
        boost::posix_time::ptime start;
        boost::posix_time::ptime end;
        std::vector<double> elevations;
        std::vector<std::tuple<std::string, If::Polarization, std::vector<double>>> polvals;
    };
    static Gain parse_gain( std::ifstream &f, const std::string &l );

    std::vector<Tsys> tsyss_;
    std::vector<Gain> gains_;

    void calcEquip();

    boost::optional<std::pair<std::vector<double>, std::vector<double>>> extract_tsys( const std::string &band );
    boost::optional<std::pair<std::vector<double>, std::vector<double>>> extract_gain( const std::string &band );

    static double interp1( const std::vector<double> &x, const std::vector<double> &y, double x_ );
};
}  // namespace VieVS

#endif  // VIESCHEDPP_STPPARSER_H
