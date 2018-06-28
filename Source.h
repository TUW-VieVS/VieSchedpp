/**
 * @file Source.h
 * @brief class Source
 *
 *
 * @author Matthias Schartner
 * @date 28.06.2017
 */

#ifndef SOURCE_H
#define SOURCE_H
#include <iostream>
#include <fstream>

#include <boost/format.hpp>
#include <cmath>
#include <utility>
#include <boost/optional.hpp>
#include <unordered_map>
#include <memory>

#include "Flux.h"
#include "Constants.h"
#include "TimeSystem.h"
#include "VieVS_NamedObject.h"
#include "AstronomicalParameters.h"

namespace VieVS{
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
    class Source: public VieVS_NamedObject {
    public:

        static unsigned long nextId;

        /**
         * @brief source parameters
         */
        enum class TryToFocusOccurrency {
            once,
            perScan
        };

        enum class TryToFocusType {
            additive,
            multiplicative
        };

        class Parameters: public VieVS_NamedObject {
        private:
            static unsigned long nextId;
        public:
            explicit Parameters(const std::string &name): VieVS_NamedObject(name,nextId++){}

            void setParameters(const Parameters &other);

            bool available = true; ///< flag is source is available
            bool globalAvailable = true;
            bool availableForFillinmode = true;

            double weight = 1; ///< multiplicative factor of score for scans to this source

            std::unordered_map<std::string, double> minSNR; ///< minimum required signal to noise ration for each band

            unsigned int minNumberOfStations = 2; ///< minimum number of stations for a scan
            double minFlux = 0.001; ///< minimum flux density required for this source in jansky
            unsigned int minRepeat = 1800; ///< minimum time between two observations of this source in seconds
            unsigned int maxScan = 20; ///< maximum allowed scan time in seconds
            unsigned int minScan = 600; ///< minimum required scan time in seconds
            unsigned int maxNumberOfScans = 9999; ///< maximum number of scans
            double minElevation = 0;
            double minSunDistance = 4*deg2rad;

            bool tryToFocusIfObservedOnce = false; ///< flag if this source should be focused after observed once
            boost::optional<double> tryToFocusFactor;
            boost::optional<TryToFocusOccurrency> tryToFocusOccurrency;
            boost::optional<TryToFocusType> tryToFocusType;

            boost::optional<unsigned int> tryToObserveXTimesEvenlyDistributed; ///< tries to observe a source X times over the timespan in which the source is scanable. Overwrites maxScan and tryToFocusIfObservedOnce.
            boost::optional<unsigned int> tryToObserveXTimesMinRepeat;

            boost::optional<unsigned int> fixedScanDuration; ///< optional fixed scan duration

            std::vector<unsigned long> ignoreStations; ///< list of all stations ids which should be ignored
            std::vector<unsigned long> ignoreBaselines; ///< list of all baseline ids which should be ignored
            std::vector<unsigned long> requiredStations; ///< list of station ids which are required for a scan to this source

            /**
             * @brief setter for available
             *
             * @param flag true if source is available
             */
            void setAvailable(bool flag) {
                Parameters::available = flag;
            }

            void setGlobalAvailable(bool flag) {
                Parameters::globalAvailable = flag;
                if(!flag){
                    Parameters::available = flag;
                }
            }

            /**
             * @brief output of the curren parameters to out stream
             *
             * @param of out stream object
             */
            void output(std::ofstream &of) const {
                if(globalAvailable){
                    if (available) {
                        of << "    available: TRUE\n";
                    } else {
                        of << "    available: FALSE\n";
                    }
                }else{
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
                if(fixedScanDuration.is_initialized()){
                    of << "    fixedScanDuration " << *fixedScanDuration << "\n";
                }
                if (tryToFocusIfObservedOnce) {
                    of << "    tryToFocusIfObservedOnce: TRUE\n";
                } else {
                    of << "    tryToFocusIfObservedOnce: FALSE\n";
                }
                if (tryToObserveXTimesEvenlyDistributed.is_initialized()) {
                    of << "    tryToObserveXTimesEvenlyDistributed: "<< *tryToObserveXTimesEvenlyDistributed <<"\n";
                }


                for (const auto &it:minSNR) {
                    of << "    minSNR: " << it.first << " " << it.second << "\n";
                }

                if (!ignoreStations.empty()) {
                    of << "    ignoreStations:";
                    for (unsigned long ignoreStation : ignoreStations) {
                        of << " " << ignoreStation;
                    }
                    of << "\n";
                }
                if (!requiredStations.empty()) {
                    of << "    requiredStations:";
                    for (unsigned long requiredStation : requiredStations) {
                        of << " " << requiredStation;
                    }
                    of << "\n";
                }
                if (!ignoreBaselines.empty()) {
                    of << "    ignoreBaselines:";
                    for (unsigned long ignoreBaseline : ignoreBaselines) {
                        of << " " << ignoreBaseline;
                    }
                    of << "\n";
                }
            }
        };

        struct Optimization{
            unsigned int minNumScans = 0;
            unsigned int minNumBaselines = 0;
        };

        /**
         * @brief pre calculated parameters
         */
        struct PreCalculated{
            std::vector<double> sourceInCrs; ///< source vector in celestrial reference frame
        };


        /**
         * @brief changes in parameters
         */
        struct Event {
            Event(unsigned int time, bool softTransition, Parameters PARA): time{time},
                                                                            softTransition{softTransition},
                                                                            PARA{std::move(PARA)}{}

            unsigned int time; ///< time when new parameters should be used in seconds since start
            bool softTransition; ///< transition type
            Parameters PARA; ///< new parameters
        };

        struct Statistics{
            std::vector<unsigned int> scanStartTimes;
            int totalObservingTime{0};
        };

        /**
         * @brief constructor
         *
         * @param src_name name of the source
         * @param src_name2 alternative name of source
         * @param src_ra_deg right ascension in degrees
         * @param src_de_deg declination in degrees
         * @param src_flux flux information per band
         */
        Source(const std::string &src_name, const std::string &src_name2, double src_ra_deg, double src_de_deg,
               std::unordered_map<std::string, std::unique_ptr<Flux>> &src_flux);


        /**
         * @brief getter of parameter object
         * @return parameter object
         */
        const Parameters &getPARA() const {
            return parameters_;
        }

        /**
         * @brief reference of parameter object
         * @return reference to parameter object
         */
        Parameters &referencePARA() {
            return parameters_;
        }

        Optimization &referenceCondition() {
            return *condition_;
        }


        /**
         * @brief get source position in CRS
         *
         * @return source position vector
         */
        const std::vector<double> &getSourceInCrs() const {
            return preCalculated_->sourceInCrs;
        }

        /**
         * @brief getter for right ascension
         *
         * @return right ascension of the source in radians
         */
        double getRa() const noexcept {
            return ra_;
        }

        /**
         * @brief getter for right ascension string
         *
         * @return right ascension string of the source
         */
        std::string getRaString() const noexcept;

        /**
         * @brief getter for declination string
         *
         * @return declination string of the source
         */
        std::string getDeString() const noexcept;

        /**
         * @brief getter for declination
         *
         * @return declination of the source in radians
         */
        double getDe() const noexcept {
            return de_;
        }

        /**
         * @brief getter for number of observed baselines
         *
         * @return number of baselines already observed for this source
         */
        unsigned long getNbls() const noexcept {
            return nBaselines_;
        }

        /**
         * @brief geter for number of already scheduled scans to this source
         *
         * @return number of already scheduled scans that influence schedule
         */
        unsigned int getNscans() const {
            return nScans_;
        }

        /**
         * @brief geter for number of already scheduled scans to this source
         *
         * @return number of already scheduled scans in total
         */
        unsigned int getNTotalScans() const {
            return nTotalScans_;
        }

        const Optimization &getOptimization() const {
            return *condition_;
        }

        /**
         * @brief looks for last scan time
         *
         * @return last observation time in seconds since session start
         */
        unsigned int lastScanTime() const noexcept {
            return lastScan_;
        }

        /**
         * @brief sets all events to this source
         *
         * @param EVENTS list of all events
         */
        void setEVENTS(std::vector<Event> &EVENTS) noexcept {
            events_ = std::make_shared<std::vector<Event>>(move(EVENTS));
            nextEvent_ = 0;
        }

        void setNextEvent(unsigned int nextEvent) {
            Source::nextEvent_ = nextEvent;
        }

        /**
         * checks if source is strong enough
         *
         * !!! this function changes maxFlux !!!
         *
         * @param maxFlux maximum flux density of this source (will be calculated)
         * @return true if source is strong enough, otherwise false
         */
        double getMaxFlux() const noexcept;

        double getSunDistance() const noexcept;

        /**
         * @brief observed flux density per band
         *
         * @param gmst greenwhich meridian sedirial time
         * @param dx baseline delta x
         * @param dy baseline delta y
         * @param dz baseline delta z
         * @return observed flux density per band
         */
        double observedFlux(const std::string &band, double gmst, const std::vector<double> &dxyz) const noexcept;

        /**
         * @brief this function checks if it is time to change the parameters
         *
         * !!! This function changes hardBreak !!!
         *
         * @param time current time
         * @param hardBreak flags this to true if a hard break was found
         * @param output displays output (default is false)
         * @param bodyLog out stream object
         * @return true if a new event was found
         */
        bool checkForNewEvent(unsigned int time, bool &hardBreak) noexcept;


        /**
         * @brief updates scan to this source
         *
         * @param nbl number of baselines observed in scan to this source
         * @param time scan end time in seconds since start
         * @param addToStatistics flag if scan should have an influence on the further scheduling process
         */
        void update(unsigned long nbl, unsigned int time, bool addToStatistics) noexcept;

        void clearObservations();

        /**
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param src source information that should be printed to stream
         * @return stream object
         */
        friend std::ostream &operator<<(std::ostream &out, const Source &src) noexcept;

        void setStatistics(const Statistics &stat){
            statistics_ = stat;
        }

        const Statistics &getStatistics() const {
            return statistics_;
        }


    private:
        std::shared_ptr<std::unordered_map<std::string, std::unique_ptr<Flux>>> flux_; ///< source flux information per band
        std::shared_ptr<std::vector<Event>> events_; ///< list of all events
        std::shared_ptr<PreCalculated> preCalculated_; ///< pre calculated values
        std::shared_ptr<Optimization> condition_;
        Statistics statistics_;

        double ra_; ///< source right ascension
        double de_; ///< source declination

        Parameters parameters_; ///< parameters

        unsigned int nextEvent_{0}; ///< index of next event
        unsigned int lastScan_{0}; ///< last scan to this source
        unsigned int nScans_{0}; ///< number of scans to this source that have influence on scheduling algorithms
        unsigned int nTotalScans_{0}; ///< number of total scans
        unsigned long nBaselines_{0}; ///< number of observed baselines to this source
    };
    
    
}
#endif /* SOURCE_H */

