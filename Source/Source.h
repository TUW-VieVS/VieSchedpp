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
 * @file Source.h
 * @brief class Source
 *
 * @author Matthias Schartner
 * @date 28.06.2017
 */

#ifndef SOURCE_H
#define SOURCE_H
#include <fstream>
#include <iostream>

#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <cmath>
#include <memory>
#include <unordered_map>
#include <utility>

#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#endif

#include "../Misc/AstronomicalParameters.h"
#include "../Misc/Constants.h"
#include "../Misc/Flags.h"
#include "../Misc/TimeSystem.h"
#include "../Misc/VieVS_NamedObject.h"
#include "Flux/AbstractFlux.h"

namespace VieVS {
/**
 * @class Source
 * @brief representation of a quasar which can be scheduled
 *
 * First all source objects must be created, usually via the VLBI_initializer::createSourcesFromCatalogs()
 * Afterwards sources need to be initialized via VLBI_initializer::initializeSources()
 *
 * @author Matthias Schartner
 * @date 28.06.2017
 */
class Source : public VieVS_NamedObject {
   public:
    /**
     * @brief source parameters
     * @author Matthias Schartner
     */
    enum class TryToFocusOccurrency { once, perScan };

    /**
     * @brief try to focus type
     * @author Matthias Schartner
     */
    enum class TryToFocusType {
        additive,       ///< additive type
        multiplicative  ///< multiplicative type
    };

    /**
     * @brief source parameters
     * @author Matthias Schartner
     */
    class Parameters : public VieVS_NamedObject {
       private:
        static unsigned long nextId;  ///< next id for this object type
       public:
        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param name parameter name
         */
        explicit Parameters( const std::string &name ) : VieVS_NamedObject( name, nextId++ ) {}

        /**
         * @brief copy parameters from other
         * @author Matthias Schartner
         *
         * @param other source parameters
         */
        void setParameters( const Parameters &other );

        bool available = true;               ///< flag is source is available
        bool globalAvailable = true;         ///< flag if source is available
        bool availableForFillinmode = true;  ///< flag if source is available for fillin mode

        double weight = 1;  ///< multiplicative factor of score for scans to this source

        std::unordered_map<std::string, double> minSNR;  ///< minimum required signal to noise ration for each band

        unsigned int minNumberOfStations = 2;  ///< minimum number of stations for a scan
        double minFlux = 0.001;                ///< minimum flux density required for this source in jansky
        unsigned int minRepeat = 1800;         ///< minimum time between two observations of this source in seconds
        unsigned int maxScan = 9999;           ///< maximum allowed scan time in seconds
        unsigned int minScan = 0;              ///< minimum required scan time in seconds
        unsigned int maxNumberOfScans = 9999;  ///< maximum number of scans
        double minElevation = 0;               ///< minimum elevation in radians
        double minSunDistance = 4 * deg2rad;   ///< minimum sun distance in radians

        bool tryToFocusIfObservedOnce = false;     ///< flag if this source should be focused after observed once
        boost::optional<double> tryToFocusFactor;  ///< increase weight if scheduled once factor
        boost::optional<TryToFocusOccurrency> tryToFocusOccurrency;  ///< weight factor change occurrency
        boost::optional<TryToFocusType> tryToFocusType;              ///< weight factor change type

        boost::optional<unsigned int>
            tryToObserveXTimesEvenlyDistributed;  ///< tries to observe a source X times over the timespan in which the
                                                  ///< source is scanable. Overwrites maxScan and
                                                  ///< tryToFocusIfObservedOnce.
        boost::optional<unsigned int> tryToObserveXTimesMinRepeat;  ///< backup minimum repeat time

        boost::optional<unsigned int> fixedScanDuration;  ///< optional fixed scan duration

        std::vector<unsigned long> ignoreStations;   ///< list of all stations ids which should be ignored
        std::vector<unsigned long> ignoreBaselines;  ///< list of all baseline ids which should be ignored
        std::vector<unsigned long>
            requiredStations;  ///< list of station ids which are required for a scan to this source

        /**
         * @brief setter for available
         * @author Matthias Schartner
         *
         * @param flag true if source is available
         */
        void setAvailable( bool flag ) { Parameters::available = flag; }

        /**
         * @brief set global availability
         * @author Matthias Schartner
         *
         * @param flag flag
         */
        void setGlobalAvailable( bool flag ) {
            Parameters::globalAvailable = flag;
            if ( !flag ) {
                Parameters::available = flag;
            }
        }

        /**
         * @brief output of the curren parameters to out stream
         * @author Matthias Schartner
         *
         * @param of out stream object
         */
        void output( std::ofstream &of ) const {
            if ( globalAvailable ) {
                if ( available ) {
                    of << "    available: TRUE\n";
                } else {
                    of << "    available: FALSE\n";
                }
            } else {
                of << "    not available due to optimization\n";
            }

            of << "    minNumOfSta:      " << minNumberOfStations << "\n";
            of << "    minFlux:          " << minFlux << "\n";
            of << "    minRepeat:        " << minRepeat << "\n";
            of << "    maxScan:          " << maxScan << "\n";
            of << "    minScan:          " << minScan << "\n";
            of << "    weight:           " << weight << "\n";
            of << "    minElevation      " << minElevation << "\n";
            of << "    maxNumberOfScans: " << maxNumberOfScans << "\n";
            if ( fixedScanDuration.is_initialized() ) {
                of << "    fixedScanDuration " << *fixedScanDuration << "\n";
            }
            if ( tryToFocusIfObservedOnce ) {
                of << "    tryToFocusIfObservedOnce: TRUE\n";
            } else {
                of << "    tryToFocusIfObservedOnce: FALSE\n";
            }
            if ( tryToObserveXTimesEvenlyDistributed.is_initialized() ) {
                of << "    tryToObserveXTimesEvenlyDistributed: " << *tryToObserveXTimesEvenlyDistributed << "\n";
            }

            for ( const auto &it : minSNR ) {
                of << "    minSNR: " << it.first << " " << it.second << "\n";
            }

            if ( !ignoreStations.empty() ) {
                of << "    ignoreStations:";
                for ( unsigned long ignoreStation : ignoreStations ) {
                    of << " " << ignoreStation;
                }
                of << "\n";
            }
            if ( !requiredStations.empty() ) {
                of << "    requiredStations:";
                for ( unsigned long requiredStation : requiredStations ) {
                    of << " " << requiredStation;
                }
                of << "\n";
            }
            if ( !ignoreBaselines.empty() ) {
                of << "    ignoreBaselines:";
                for ( unsigned long ignoreBaseline : ignoreBaselines ) {
                    of << " " << ignoreBaseline;
                }
                of << "\n";
            }
        }
    };

    /**
     * @brief optimization conditions
     * @author Matthias Schartner
     */
    struct Optimization {
        unsigned int minNumScans = 0;  ///< minimum required number of scans
        unsigned int minNumObs = 0;    ///< minimum required number of observations
    };

    /**
     * @brief pre calculated parameters
     * @author Matthias Schartner
     */
    struct PreCalculated {
        std::vector<double> sourceInCrs;  ///< source vector in celestrial reference frame
    };

    /**
     * @brief changes in parameters
     * @author Matthias Schartner
     */
    struct Event {
        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param time event time
         * @param smoothTransition transition type
         * @param PARA parameter
         */
        Event( unsigned int time, bool smoothTransition, Parameters PARA )
            : time{time}, smoothTransition{smoothTransition}, PARA{std::move( PARA )} {}

        unsigned int time;      ///< time when new parameters should be used in seconds since start
        bool smoothTransition;  ///< transition type
        Parameters PARA;        ///< new parameters
    };

    /**
     * @brief statistics
     * @author Matthias Schartner
     */
    struct Statistics {
        std::vector<unsigned int> scanStartTimes;  ///< scan start times
        int totalObservingTime{0};                 ///< integrated observing time
    };

    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param src_name name of the source
     * @param src_name2 alternative name of source
     * @param src_ra_deg right ascension in degrees
     * @param src_de_deg declination in degrees
     * @param src_flux flux information per band
     */
    Source( const std::string &src_name, const std::string &src_name2, double src_ra_deg, double src_de_deg,
            std::unordered_map<std::string, std::unique_ptr<AbstractFlux>> &src_flux );

    /**
     * @brief getter of parameter object
     * @author Matthias Schartner
     *
     * @return parameter object
     */
    const Parameters &getPARA() const { return parameters_; }

    /**
     * @brief reference of parameter object
     * @author Matthias Schartner
     *
     * @return reference to parameter object
     */
    Parameters &referencePARA() { return parameters_; }

    /**
     * @brief reference for optimization conditions
     * @author Matthias Schartner
     *
     * @return optimization conditons
     */
    Optimization &referenceCondition() { return *condition_; }

    /**
     * @brief get source position in CRS
     * @author Matthias Schartner
     *
     * @return source position vector
     */
    const std::vector<double> &getSourceInCrs() const { return preCalculated_->sourceInCrs; }

    /**
     * @brief getter for right ascension
     * @author Matthias Schartner
     *
     * @return right ascension of the source in radians
     */
    double getRa() const noexcept { return ra_; }

    /**
     * @brief getter for right ascension string
     * @author Matthias Schartner
     *
     * @return right ascension string of the source
     */
    std::string getRaString() const noexcept;

    /**
     * @brief getter for declination string
     * @author Matthias Schartner
     *
     * @return declination string of the source
     */
    std::string getDeString() const noexcept;

    /**
     * @brief getter for declination
     * @author Matthias Schartner
     *
     * @return declination of the source in radians
     */
    double getDe() const noexcept { return de_; }

    /**
     * @brief getter for number of observed baselines
     * @author Matthias Schartner
     *
     * @return number of baselines already observed for this source
     */
    unsigned long getNObs() const noexcept { return nObs_; }

    /**
     * @brief geter for number of already scheduled scans to this source
     * @author Matthias Schartner
     *
     * @return number of already scheduled scans that influence schedule
     */
    unsigned int getNscans() const { return nScans_; }

    /**
     * @brief geter for number of already scheduled scans to this source
     * @author Matthias Schartner
     *
     * @return number of already scheduled scans in total
     */
    unsigned int getNTotalScans() const { return nTotalScans_; }

    /**
     * @brief get optimization conditons
     * @author Matthias Schartner
     *
     * @return optimization condition
     */
    const Optimization &getOptimization() const { return *condition_; }

    /**
     * @brief looks for last scan time
     * @author Matthias Schartner
     *
     * @return last observation time in seconds since session start
     */
    unsigned int lastScanTime() const noexcept { return lastScan_; }

    /**
     * @brief sets all events to this source
     * @author Matthias Schartner
     *
     * @param EVENTS list of all events
     */
    void setEVENTS( std::vector<Event> &EVENTS ) noexcept {
        events_ = std::make_shared<std::vector<Event>>( move( EVENTS ) );
        nextEvent_ = 0;
    }

    /**
     * @brief set next event index
     * @author Matthias Schartner
     *
     * @param nextEvent index
     */
    void setNextEvent( unsigned int nextEvent ) { Source::nextEvent_ = nextEvent; }

    /**
     * @brief get maxium possible flux density
     * @author Matthias Schartner
     *
     * @return maxium possible flux density
     */
    double getMaxFlux() const noexcept;

    /**
     * @brief get sun distance
     * @author Matthias Schartner
     *
     * @return sun distance
     */
    double getSunDistance() const noexcept;

    /**
     * @brief observed flux density per band
     * @author Matthias Schartner
     *
     * @param band observed band
     * @param gmst greenwhich meridian sedirial time
     * @param dxyz coordinate difference of participating stations
     * @return observed flux density per band
     */
    double observedFlux( const std::string &band, double gmst, const std::vector<double> &dxyz ) const noexcept;

    /**
     * @brief calc projection of baseline in uv plane
     * @author Matthias Schartner
     *
     * @param gmst greenwich mean sidereal time
     * @param dxyz baseline vector
     * @return projection of baseline vector in uv plane
     */
    std::pair<double, double> calcUV( double gmst, const std::vector<double> &dxyz ) const noexcept;

    /**
     * @brief this function checks if it is time to change the parameters
     * @author Matthias Schartner
     *
     * @param time current time
     * @param hardBreak flags this to true if a hard break was found
     * @return true if a new event was found
     */
    bool checkForNewEvent( unsigned int time, bool &hardBreak ) noexcept;

    /**
     * @brief updates scan to this source
     * @author Matthias Schartner
     *
     * @param nbl number of baselines observed in scan to this source
     * @param time scan end time in seconds since start
     * @param addToStatistics flag if scan should have an influence on the further scheduling process
     */
    void update( unsigned long nbl, unsigned int time, bool addToStatistics ) noexcept;

    /**
     * @brief clear all observations
     * @author Matthias Schartner
     */
    void clearObservations();

    /**
     * @brief set source statistics
     * @author Matthias Schartner
     *
     * @param stat source statistics
     */
    void setStatistics( const Statistics &stat ) { statistics_ = stat; }

    /**
     * @brief get source statistics
     * @author Matthias Schartner
     *
     * @return source statistics
     */
    const Statistics &getStatistics() const { return statistics_; }

   private:
    static unsigned long nextId;  ///< next id for this object type

    std::shared_ptr<std::unordered_map<std::string, std::unique_ptr<AbstractFlux>>>
        flux_;                                      ///< source flux information per band
    std::shared_ptr<std::vector<Event>> events_;    ///< list of all events
    std::shared_ptr<PreCalculated> preCalculated_;  ///< pre calculated values
    std::shared_ptr<Optimization> condition_;       ///< optimization conditions
    Statistics statistics_;                         ///< statistics

    double ra_;     ///< source right ascension
    double de_;     ///< source declination
    double sinDe_;  ///< sine of declination
    double cosDe_;  ///< cosine of declination

    Parameters parameters_;  ///< parameters

    unsigned int nextEvent_{0};    ///< index of next event
    unsigned int lastScan_{0};     ///< last scan to this source
    unsigned int nScans_{0};       ///< number of scans to this source that have influence on scheduling algorithms
    unsigned int nTotalScans_{0};  ///< number of total scans
    unsigned long nObs_{0};        ///< number of observed baselines to this source
};

}  // namespace VieVS
#endif /* SOURCE_H */
