//
// Created by mschartner on 12/6/21.
//

#ifndef VIESCHEDPP_ABSTRACTEQUIPMENT_H
#define VIESCHEDPP_ABSTRACTEQUIPMENT_H

#include <boost/format.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../../Misc/VieVS_Object.h"


namespace VieVS {

class AbstractEquipment : VieVS_Object {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     */
    AbstractEquipment();

    /**
     * @brief get SEFD value for given band, polarization, eleation and azimuth
     *
     * @param band name of band
     * @param el elevation
     * @return SEFD of this band
     */
    virtual double getSEFD( const std::string &band, double el ) const noexcept = 0;

    //    /**
    //     * @brief returns vector of bands for which SEFD information is available
    //     * @author Matthias Schartner
    //     *
    //     * @return vector of band names provided with SEFD information
    //     */
    //    virtual std::vector<std::string> getBands() const noexcept = 0;

    /**
     * @brief returns maximum SEFD of this antenna
     * @author Matthias Schartner
     *
     * @return maximum SEFD of this antenna
     */
    virtual double getMaxSEFD() const noexcept = 0;

    /**
     * @brief creates a short summary of SEFD parameters
     * @author Matthis Schartner
     *
     * @param band band name
     * @return short summary of SEFD parameters
     */
    virtual std::string shortSummary( const std::string &band ) const noexcept = 0;

   private:
    static unsigned long nextId_;  ///< next id for this object type
};
}  // namespace VieVS
#endif  // VIESCHEDPP_ABSTRACTEQUIPMENT_H
