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


/**
* @file HighImpactScanDescriptor.h
* @brief class HighImpactScanDescriptor
*
* @author Matthias Schartner
* @date 27.04.2018
*
*/

#ifndef HIGHIMPACTSCANDESCRIPTOR_H
#define HIGHIMPACTSCANDESCRIPTOR_H

#include <vector>

#include "VieVS_Object.h"
#include "Scan.h"
#include "Subcon.h"
#include "Subnetting.h"

namespace VieVS{

    /**
     * @class HighImpactScanDescriptor
     * @brief define high impact scans
     *
     * @author Matthias Schartner
     * @date 27.04.2018
     */
    class HighImpactScanDescriptor: public VieVS_Object {
    public:

        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param interval test interval in seconds
         * @param minTimeBetweenScans minimum time between high impact scans in seconds
         */
        HighImpactScanDescriptor(unsigned int interval, unsigned int minTimeBetweenScans);

        /**
         * @brief add azimuth elevation based high impact scan selection
         * @author Matthias Schartner
         *
         * @param az target azimuth in radians
         * @param el target elevation in radians
         * @param margin azimuth elevation margin in radians
         * @param staids list of stations with high impact descriptions
         */
        void addAzElDescriptor(double az, double el, double margin, const std::vector<unsigned long> &staids);

        /**
         * @brief calc high impact score per station
         * @author Matthias Schartner
         *
         * @param pv target pointing vector
         * @return high impact score per station
         */
        double highImpactScore(const PointingVector &pv) const;

        /**
         * @brief calc high impact score per scan
         * @author Matthias Schartner
         *
         * @param scan target scan
         * @return high impact score per scan
         */
        double highImpactScore(const Scan& scan) const;

        /**
         * @brief get target interval
         * @author Matthias Schartner
         *
         * @return interval in seconds
         */
        unsigned int getInterval() const {
            return interval_;
        }

       /**
        * @brief get minimum time between high impact scans
        * @author Matthias Schartner
        *
        * @return minimum time between high impact scans in seconds
        */
        unsigned int getMinTimeBetweenScans() const {
            return minTimeBetweenScans_;
        }

        /**
         * @brief list of stations with high impact descriptions
         * @author Matthias Schartner
         *
         * @return list of stations
         */
        std::vector<unsigned long> getStationIds() const;

        /**
         * @brief calc possible high impact scans
         * @author Matthias Schartner
         *
         * @param time target time
         * @param network station network
         * @param sources source list
         */
        void possibleHighImpactScans(unsigned int time, const Network &network, const std::vector<Source> &sources);

        /**
         * @brief update high impact scans
         * @author Matthias Schartner
         *
         * @param network station network
         * @param sources source list
         * @param subnetting subnetting information
         */
        void updateHighImpactScans(const Network &network, const std::vector<Source> &sources,
                                   const boost::optional<Subnetting> &subnetting);

        /**
         * @brief get highest impact scans
         * @author Matthias Schartner
         *
         * @param network station network
         * @param sources source list
         * @return list of scans with highest impact score
         */
        std::vector<Scan> highestImpactScans(Network &network, const std::vector<Source> &sources);

        /**
         * @brief check if target scan is valid high impact scans
         * @author Matthias Schartner
         *
         * based on minimum time between high impact scans
         *
         * @param target target scan
         * @param scans already scheduled scans
         * @param source target source
         * @return true if target scan is valid high impact scan, otherwise false
         */
        bool isCorrectHighImpactScan(const Scan &target, const std::vector<Scan> &scans, const Source &source);

        /**
         * @brief checks if there are more possible high impact scans
         * @author Matthias Schartner
         *
         * @return true if there are more possible high impact scans, otherwise false
         */
        bool hasMoreScans(){
            return highImpactScans_.getNumberSingleScans()>0 || highImpactScans_.getNumberSubnettingScans()>0;
        }

        /**
         * @brief update log file for new high impact scans
         * @author Matthias Schartner
         *
         * @param of out file stream
         */
        void updateLogfile(std::ofstream &of);

    private:
        /**
         * @class AzElDescriptor
         * @brief azimuth elevation based high impact scan descriptor
         *
         * @author Matthias Schartner
         * @date 27.04.2018
         */
        class AzElDescriptor;

        static unsigned long nextId; ///< next id for this object type
        unsigned int interval_; ///< test interval in seconds
        unsigned int minTimeBetweenScans_; ///< minimum time between high impact scans in seconds
        std::vector<AzElDescriptor> azElDescritors_; ///< list of azimuth elevation descriptors

        std::vector< std::map<unsigned long, double> > scores_; ///< list of scores by high impact scans
        Subcon highImpactScans_; ///< high impact scan subcon

    };

    class HighImpactScanDescriptor::AzElDescriptor: public VieVS_Object {
    public:
        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param az target azimuth in radians
         * @param el target elevation in radians
         * @param margin target azimuth elevation margin
         * @param staids target station ids
         */
        AzElDescriptor(double az, double el, double margin, std::vector<unsigned long> staids);

        /**
         * @brief calculate high impact score
         * @author Matthias Schartner
         *
         * @param pv target pointing vector
         * @return high impact score
         */
        double highImpactScore(const PointingVector &pv) const;

        /**
         * @brief high impact station ids
         * @author Matthias Schartner
         *
         * @return high impact station ids
         */
        const std::vector<unsigned long> &getStaids() const;

    private:
        static unsigned long nextId; ///< next id for this object type
        double az_; ///< target azimuth in radians
        double el_; ///< target elevation in radians
        double margin_; ///< target azimuth elevation margin
        std::vector<unsigned long> staids_; ///< target station ids
    };
}


#endif //HIGHIMPACTSCANDESCRIPTOR_H
