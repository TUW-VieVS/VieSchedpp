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
 * @file AbstractSource.h
 * @brief class Source
 *
 * @author Matthias Schartner
 * @date 28.06.2017
 */

#ifndef SOURCE_H
#define SOURCE_H


#include <boost/format.hpp>
#include <boost/optional.hpp>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <utility>

#include "../Misc/AstronomicalParameters.h"
#include "../Misc/Constants.h"
#include "../Misc/Flags.h"
#include "../Misc/TimeSystem.h"
#include "../Misc/VieVS_NamedObject.h"
#include "../Station/Position.h"
#include "Flux/AbstractFlux.h"
#ifdef VIESCHEDPP_LOG
#include <boost/log/trivial.hpp>
#endif


namespace VieVS {
/**
 * @class AbstractSource
 * @brief representation of an abstract source which can be scheduled
 *
 * First all source objects must be created, usually via the VLBI_initializer::createSourcesFromCatalogs()
 * Afterwards sources need to be initialized via VLBI_initializer::initializeSources()
 *
 * @author Matthias Schartner
 * @date 28.06.2017
 */
class AbstractSource : public VieVS_NamedObject {
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

        unsigned int minNumberOfStations = 3;  ///< minimum number of stations for a scan
        double minFlux = 0.001;                ///< minimum flux density required for this source in jansky
        unsigned int minRepeat = 1800;         ///< minimum time between two observations of this source in seconds
        unsigned int maxScan = 9999;           ///< maximum allowed scan time in seconds
        unsigned int minScan = 0;              ///< minimum required scan time in seconds
        unsigned int maxNumberOfScans = 9999;  ///< maximum number of scans
        double minElevation = 0;               ///< minimum elevation in radians
        double minSunDistance = 4 * deg2rad;   ///< minimum sun distance in radians
        boost::optional<double> jetAngleBuffer;  ///< avoid observations along jet angle +- buffer
        boost::optional<double> jetAngleFactor;  ///< avoid observations along jet angle +- factor*std

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
            if ( jetAngleBuffer.is_initialized() ) {
                of << "    jetAngleBuffer " << *jetAngleBuffer << "\n";
            }
            if ( jetAngleFactor.is_initialized() ) {
                of << "    jetAngleFactor " << *jetAngleFactor << "\n";
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
            : time{ time }, smoothTransition{ smoothTransition }, PARA{ std::move( PARA ) } {}


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
        int totalObservingTime{ 0 };               ///< integrated observing time
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
    AbstractSource( const std::string &src_name, const std::string &src_name2,
                    std::unordered_map<std::string, std::unique_ptr<AbstractFlux>> &src_flux );

    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param src_name name of the source
     * @param src_name2 alternative name of source
     * @param src_ra_deg right ascension in degrees
     * @param src_de_deg declination in degrees
     * @param src_flux flux information per band
     * @param jet_angle jet angle in radians (-90,90]
     * @param jet_angle_std standard deviation of jet angle estimation in radians
     */
    AbstractSource( const std::string &src_name, const std::string &src_name2,
                    std::unordered_map<std::string, std::unique_ptr<AbstractFlux>> &src_flux, double jet_angle,
                    double jet_angle_std );

    virtual ~AbstractSource() = default;

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
    virtual std::vector<double> getSourceInCrs( unsigned int time,
                                                const std::shared_ptr<const Position> &sta_pos ) const = 0;


    virtual std::pair<double, double> getRaDe( unsigned int time,
                                               const std::shared_ptr<const Position> &sta_pos ) const noexcept = 0;

    virtual void toVex(std::ofstream &of) const =0;

    virtual void toNgsHeader(std::ofstream &of) const =0;

    /**
     * @brief getter for right ascension string
     * @author Matthias Schartner
     *
     * @return right ascension string of the source
     */
    std::string getRaString( double ang ) const noexcept;


    /**
     * @brief getter for declination string
     * @author Matthias Schartner
     *
     * @return declination string of the source
     */
    std::string getDeString( double ang ) const noexcept;


    /**
     * @brief getter for number of observed baselines
     * @author Matthias Schartner
     *
     * @return number of baselines already observed for this source
     */
    unsigned long getNObs() const noexcept { return nObs_; }


    /**
     * @brief getter for number of independent closure phases and amplitudes
     * @author Matthias Schartner
     *
     * @return number of independent closure phases and amplitudes that are already observed for this source
     */
    unsigned long getNClosures() const noexcept { return nClosures_; }


    /**
     * @brief increase number of observations by one
     * @author Matthias Schartner
     */
    void increaseNObs() noexcept { ++nObs_; }


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
        events_ = move( EVENTS );
        nextEvent_ = 0;
    }


    /**
     * @brief reference parameters to add multi-scheduling setup
     * @author Matthias Schartner
     *
     * @return reference to parameters object
     */
    Parameters &refParaForMultiScheduling(){
        return events_[1].PARA;
    }


    /**
     * @brief set next event index
     * @author Matthias Schartner
     *
     * @param nextEvent index
     */
    void setNextEvent( unsigned int nextEvent ) { AbstractSource::nextEvent_ = nextEvent; }


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
    double getSunDistance( unsigned int time, const std::shared_ptr<const Position> &sta_pos ) const noexcept;


    /**
     * @brief observed flux density per band
     * @author Matthias Schartner
     *
     * @param band observed band
     * @param gmst greenwhich meridian sedirial time
     * @param dxyz coordinate difference of participating stations
     * @return observed flux density per band
     */
    double observedFlux( const std::string &band, unsigned int time, double gmst,
                         const std::vector<double> &dxyz ) const noexcept;


    /**
     * @brief calc projection of baseline in uv plane
     * @author Matthias Schartner
     *
     * @param gmst greenwich mean sidereal time
     * @param dxyz baseline vector
     * @return projection of baseline vector in uv plane
     */
    std::pair<double, double> calcUV( unsigned int time, double gmst, const std::vector<double> &dxyz ) const noexcept;

    /**
     * @brief check if observations along jet angle should be investigated
     * @author Matthias Schartner
     *
     * @return true if jet angle is relevant otherwise false
     */
    bool checkJetAngle() const{
        return jet_angle_.is_initialized() &&
               (parameters_.jetAngleBuffer.is_initialized() || parameters_.jetAngleFactor.is_initialized());
    }

    /**
     * @brief check if observations along jet angle should be investigated
     * @author Matthias Schartner
     *
     * @param time observation start time in seconds since session start
     * @param gmst greenwhich meridian sedirial time
     * @param dxyz coordinate difference of participating stations
     * @return true if observation is valid, otherwise false
     */
    bool jet_angle_valid(unsigned int time, double gmst, const std::vector<double> &dxyz ) const;


    /**
     * @brief this function checks if it is time to change the parameters
     * @author Matthias Schartner
     *
     * @param time current time
     * @param hardBreak flags this to true if a hard break was found
     * @return true if a new event was found
     */
    virtual bool checkForNewEvent( unsigned int time, bool &hardBreak ) noexcept;


    /**
     * @brief updates scan to this source
     * @author Matthias Schartner
     *
     * @param nsta number of stations
     * @param nbl number of baselines observed in scan to this source
     * @param time scan end time in seconds since start
     * @param addToStatistics flag if scan should have an influence on the further scheduling process
     */
    void update( unsigned long nsta, unsigned long nbl, unsigned int time, bool addToStatistics ) noexcept;


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

    /**
     * @brief calculate flux density of any band based on available flux densities
     * @author Matthias Schartner
     *
     * @param wavelength target wavelength
     * @param gmst greenwhich meridian sedirial time
     * @param dxyz coordinate difference of participating stations
     * @return observed flux density for this wavelength
     */
    double observedFlux_model( double wavelength, unsigned int time, double gmst,
                               const std::vector<double> &dxyz ) const;

    /**
     * @brief checks if flux information is available
     * @author Matthias Schartner
     *
     * @param band band name
     * @return true if flux information is available, otherwise false
     */
    bool hasFluxInformation( const std::string &band ) const { return flux_->find( band ) != flux_->end(); }

   private:
    static unsigned long nextId;  ///< next id for this object type

    std::shared_ptr<std::unordered_map<std::string, std::unique_ptr<AbstractFlux>>>
        flux_;                                      ///< source flux information per band
    std::vector<Event> events_;    ///< list of all events
    std::shared_ptr<Optimization> condition_;       ///< optimization conditions
    Statistics statistics_;                         ///< statistics

    Parameters parameters_;  ///< parameters

    boost::optional<double> jet_angle_;    ///< jet angle in uv-plane
    double jet_angle_std_ = 10*deg2rad;    ///< uncertainty of jet angle

    unsigned int nextEvent_{ 0 };    ///< index of next event
    unsigned int lastScan_{ 0 };     ///< last scan to this source
    unsigned int nScans_{ 0 };       ///< number of scans to this source that have influence on scheduling algorithms
    unsigned int nTotalScans_{ 0 };  ///< number of total scans
    unsigned long nObs_{ 0 };        ///< number of observed baselines to this source
    unsigned long nClosures_{ 0 };   ///< number of independent closure phases and amplitudes
};

}  // namespace VieVS
#endif /* SOURCE_H */
