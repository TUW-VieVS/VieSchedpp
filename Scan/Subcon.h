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
 * @file Subcon.h
 * @brief class Subcon
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */

#ifndef SUBCON_H
#define SUBCON_H


#include <boost/optional.hpp>
#include <limits>
#include <numeric>
#include <queue>
#include <utility>
#include <vector>

#include "../Misc/StationEndposition.h"
#include "../Misc/Subnetting.h"
#include "../Source/Source.h"
#include "../Station/Network.h"
#include "Scan.h"


namespace VieVS {
/**
 * @class Subcon
 * @brief representation of a VLBI subcon
 *
 * @author Matthias Schartner
 * @date 29.06.2017
 */
class Subcon : public VieVS_Object {
   public:
    /**
     * @brief empty default constructor
     * @author Matthias Schartner
     */
    Subcon();


    /**
     * @brief add a single source scan to subcon
     * @author Matthias Schartner
     *
     * @param scan scan which should be added
     */
    void addScan( Scan &&scan ) noexcept;


    /**
     * @brief removes a scan from the subcon
     * @author Matthias Schartner
     *
     * The index counts first through all single source scans and continues with all subnetting scans. If the index
     * is larger then the number of single scans subnetting scans will be removed.
     *
     * @param idx index of scan which should be removed
     */
    void removeScan( unsigned long idx ) noexcept;


    /**
     * @brief getter for number of possible single source scans
     * @author Matthias Schartner
     *
     * @return number of possible single source scans
     */
    unsigned long getNumberSingleScans() const noexcept { return nSingleScans_; }


    /**
     * @brief getter for number of possible subnetting scans
     * @author Matthias Schartner
     *
     * @return number of possible subnetting scans
     */
    unsigned long getNumberSubnettingScans() const noexcept { return nSubnettingScans_; }


    /**
     * @brief getter for a single source scan
     * @author Matthias Schartner
     *
     * @param idx index
     * @return single source scan at this index
     */
    Scan takeSingleSourceScan( unsigned long idx ) noexcept {
        Scan tmp = std::move( singleScans_[idx] );
        singleScans_.erase( singleScans_.begin() + idx );
        --nSingleScans_;
        return std::move( tmp );
    }


    /**
     * @brief getter for subnettin scan
     * @author Matthias Schartner
     *
     * @param idx index
     * @return subnetting scan at this index
     */
    std::pair<Scan, Scan> takeSubnettingScans( unsigned long idx ) noexcept {
        std::pair<Scan, Scan> tmp = std::move( subnettingScans_[idx] );
        subnettingScans_.erase( subnettingScans_.begin() + idx );
        --nSubnettingScans_;
        return std::move( tmp );
    }


    /**
     * @brief calculates the earliest possible start time for all single source scans in this subcon
     * @author Matthias Schartner
     *
     * @param network station network
     * @param sources list of all sources
     * @param endposition required endposition
     */
    void calcStartTimes( const Network &network, const std::vector<Source> &sources,
                         const boost::optional<StationEndposition> &endposition = boost::none ) noexcept;


    /**
     * @brief constructs all baselines for all single source scans in this subcon
     * @author Matthias Schartner
     *
     * @param network station network
     * @param sources list of all sources
     */
    void constructAllBaselines( const Network &network, const std::vector<Source> &sources ) noexcept;


    /**
     * @brief updates all azimuths and elevations of all pointing vectors for each single source scan in this subcon
     * @author Matthias Schartner
     *
     * @param network station network
     * @param sources list of all sources
     */
    void updateAzEl( const Network &network, const std::vector<Source> &sources ) noexcept;


    /**
     * @brief calculates all baseline scan duration for all single source scans in this subcon
     * @author Matthias Schartner
     *
     * @param network station network
     * @param sources list of all sources
     * @param mode observing mode
     */
    void calcAllBaselineDurations( const Network &network, const std::vector<Source> &sources,
                                   const std::shared_ptr<const Mode> &mode ) noexcept;


    /**
     * @brief calculates all scan duration of all single source scans in this subcon
     * @author Matthias Schartner
     *
     * @param network station network
     * @param sources list of all sources
     * @param endposition required endposition
     */
    void calcAllScanDurations( const Network &network, const std::vector<Source> &sources,
                               const boost::optional<StationEndposition> &endposition = boost::none ) noexcept;


    /**
     * @brief create all subnetting scans from possible single source scans
     * @author Matthias Schartner
     *
     * @param subnetting subnetting parameters
     * @param network station network
     * @param sources list of all sources
     */
    void createSubnettingScans( const std::shared_ptr<Subnetting> &subnetting, const Network &network,
                                const std::vector<Source> &sources ) noexcept;


    /**
     * @brief generate scores for all single source and subnetting scans
     * @author Matthias Schartner
     *
     * @param network station network
     * @param sources list of all sources
     */
    void generateScore( const Network &network, const std::vector<Source> &sources ) noexcept;


    /**
     * @brief generate score for all scans during calibrator block
     * @author Matthias Schartner
     *
     * @param lowElevatrionScore low elevation scores
     * @param highElevationScore high elevation score
     * @param network station network
     * @param sources list of all sources
     */
    void generateScore( const std::vector<double> &lowElevatrionScore, const std::vector<double> &highElevationScore,
                        const Network &network, const std::vector<Source> &sources );


    /**
     * @brief
     * @author Matthias Schartner
     *
     * @param network station network
     * @param sources list of all sources
     * @param hiscores high impact scores
     * @param interval high impact search interval index
     */
    void generateScore( const Network &network, const std::vector<Source> &sources,
                        const std::vector<std::map<unsigned long, double>> &hiscores, unsigned int interval );


    /**
     * @brief check if there is enough time to reach required endposition for all scans
     * @author Matthias Schartner
     *
     * @param network station network
     * @param sources list of all sources
     * @param endposition required endposition
     */
    void checkIfEnoughTimeToReachEndposition( const Network &network, const std::vector<Source> &sources,
                                              const boost::optional<StationEndposition> &endposition = boost::none );


    /**
     * @brief get minimum and maximum time required for a possible scan
     * @author Matthias Schartner
     */
    void minMaxTime() noexcept;


    /**
     * @brief rigorousely updates the best scans until the best one is found
     * @author Matthias Schartner
     *
     * in case a subnetting scan combination has highest score these two scans are returned, otherwise only a single
     * scan is returned
     *
     * @param network station network
     * @param sources list of all sources
     * @param mode observing mode
     * @param endposition required endposition
     * @return scan(s) with highest score
     */
    std::vector<Scan> selectBest( Network &network, const std::vector<Source> &sources,
                                  const std::shared_ptr<const Mode> &mode,
                                  const boost::optional<StationEndposition> &endposition = boost::none ) noexcept;


    /**
     * @brief rigorousely updates the best scans until the best one is found during calibrator block
     * @author Matthias Schartner
     *
     * in case a subnetting scan combination has highest score these two scans are returned, otherwise only a single
     * scan is returned
     *
     * @param network station network
     * @param sources list of all sources
     * @param mode observing mode
     * @param prevLowElevationScores low elevation scores
     * @param prevHighElevationScores high elevation scores
     * @param endposition required endposition
     * @return scan(s) with highest score
     */
    std::vector<Scan> selectBest( Network &network, const std::vector<Source> &sources,
                                  const std::shared_ptr<const Mode> &mode,
                                  const std::vector<double> &prevLowElevationScores,
                                  const std::vector<double> &prevHighElevationScores,
                                  const boost::optional<StationEndposition> &endposition = boost::none ) noexcept;


    /**
     * @brief clear all subnetting scans
     * @author Matthias Schartner
     */
    void clearSubnettingScans();


    /**
     * @brief calc calibration scan duration (deprecated)
     * @author Matthias Schartner
     *
     * @param stations list of stations
     * @param sources list of sources
     */
    void calcCalibratorScanDuration( const std::vector<Station> &stations, const std::vector<Source> &sources );


    /**
     * @brief change scan type of all scans
     * @author Matthias Schartner
     *
     * @param type new scan type
     */
    void changeType( Scan::ScanType type );


    /**
     * @brief create possible visible scan to a source
     * @author Matthias Schartner
     *
     * @param currentTime current start time
     * @param type scan type
     * @param network station network
     * @param thisSource target source
     * @param observedSources list of priviously observed sources
     */
    void visibleScan( unsigned int currentTime, Scan::ScanType type, const Network &network, const Source &thisSource,
                      std::set<unsigned long> observedSources = std::set<unsigned long>() );


   private:
    static unsigned long nextId;  ///< next id for this object type

    unsigned long nSingleScans_;     ///< number of single source scans
    std::vector<Scan> singleScans_;  ///< all single source scans

    unsigned long nSubnettingScans_;                      ///< number of subnetting scans
    std::vector<std::pair<Scan, Scan>> subnettingScans_;  ///< all subnetting scans

    unsigned int minRequiredTime_;  ///< minimum time required for a scan
    unsigned int maxRequiredTime_;  ///< maximum time required for a scan
    std::vector<double> astas_;     ///< average station score for each station
    std::vector<double> asrcs_;     ///< average source score for each source
    std::vector<double> abls_;      ///< average baseline score for each baseline
    std::vector<double> idle_;      ///< extra score for long idle time

    /**
     * @brief precalculate all necessary parameters to generate scores
     * @author Matthias Schartner
     *
     * @param network station network
     * @param sources list of all sources
     */
    void precalcScore( const Network &network, const std::vector<Source> &sources ) noexcept;


    /**
     * @brief pre calculate station average number of observation score
     * @author Matthias Schartner
     *
     * @param stations list of all stations
     */
    void prepareAverageScore( const std::vector<Station> &stations ) noexcept;


    /**
     * @brief pre calculate baseline average number of observation score
     * @author Matthias Schartner
     *
     * @param baselines list of all baselines
     */
    void prepareAverageScore( const std::vector<Baseline> &baselines ) noexcept;


    /**
     * @brief pre calculate source average number of observation score
     * @author Matthias Schartner
     *
     * @param sources list of all sources
     */
    void prepareAverageScore( const std::vector<Source> &sources ) noexcept;


    /**
     * @brief pre calculate extra score after long idle times
     * @author Matthias Scharnter
     *
     * @param stations list of all stations
     */
    void prepareIdleTimeScore( const std::vector<Station> &stations ) noexcept;


    /**
     * @brief pre calculate baseline average number of observation score
     * @author Matthias Schartner
     *
     * @param nobs nuber of observation per object
     * @return average number of observation score
     */
    std::vector<double> prepareAverageScore_base( const std::vector<unsigned long> &nobs ) noexcept;
};
}  // namespace VieVS
#endif /* SUBCON_H */
