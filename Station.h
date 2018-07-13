/**
 * @file Station.h
 * @brief class Station
 *
 *
 * @author Matthias Schartner
 * @date 21.06.2017
 */

#ifndef STATION_H
#define STATION_H
#include <iostream>
#include <fstream>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <utility>
#include <memory>

#include "Position.h"
#include "Antenna.h"
#include "CableWrap.h"
#include "Equipment.h"
#include "HorizonMask.h"
#include "Source.h"
#include "PointingVector.h"
#include "Constants.h"
#include "TimeSystem.h"
#include "AstronomicalParameters.h"

#include "sofa.h"
#include "VieVS_NamedObject.h"


namespace VieVS{
    /**
     * @class Station
     * @brief representation of a VLBI station
     *
     * First all station objects must be created, usually via the VLBI_initializer::createStationsFromCatalogs()
     * Afterwards stations need to be initialized via VLBI_initializer::initializeStations()
     *
     * @author Matthias Schartner
     * @date 21.06.2017
    */
    class Station: public VieVS_NamedObject {
    public:

        /**
         * @brief azimuth elevation calculation model
         */
        enum class AzelModel {
            simple, ///< simple model without nutation
            rigorous ///< rigorous model
        };

        /**
         * @brief station parameters
         */
        class Parameters: public VieVS_NamedObject{
        private:
            static unsigned long nextId;
        public:
            explicit Parameters(const std::string &name):VieVS_NamedObject(name,nextId++){}

            void setParameters(const Parameters &other);

            bool firstScan = false; ///< if set to true: no time is spend for setup, source, tape, calibration, and slewing
            bool available = true;  ///< if set to true: this station is available for a scan
            bool tagalong = false;  ///< if set to true: station is in tagalong mode
            bool availableForFillinmode = true;

            double weight = 1; ///< multiplicative factor of score for scans with this station
            double minElevation = 5*deg2rad; /// minimum elevation

            std::unordered_map<std::string, double> minSNR; ///< minimum required signal to noise ration for each band

            unsigned int maxSlewtime = 600; ///< maximum allowed slewtime
            double maxSlewDistance = 175 * deg2rad;
            double minSlewDistance = 0;
            unsigned int maxWait = 600; ///< maximum allowed wait time for slow antennas
            unsigned int maxScan = 600; ///< maximum allowed scan time
            unsigned int minScan = 20; ///< minimum required scan time
            unsigned int maxNumberOfScans = 9999;

            std::vector<unsigned long> ignoreSources; ///< list of all source ids which should be ignored

            /**
             * @brief output of the curren parameters to out stream
             *
             * @param of out stream object
             */
            void output(std::ofstream &of) const {
                if (available) {
                    of << "    available: TRUE\n";
                } else {
                    of << "    available: FALSE\n";
                }

                of << "    maxSlewtime: " << maxSlewtime << "\n";
                of << "    maxWait:     " << maxWait << "\n";
                of << "    maxScan:     " << maxScan << "\n";
                of << "    minScan:     " << minScan << "\n";
                of << "    weight:      " << weight << "\n";
                of << "    minElevation " << minElevation << "\n";

                for (const auto &it:minSNR) {
                    of << "    minSNR: " << it.first << " " << it.second << "\n";
                }

                if (!ignoreSources.empty()) {
                    of << "    ignoreSources:";
                    for (unsigned long ignoreSource : ignoreSources) {
                        of << " " << ignoreSource;
                    }
                    of << "\n";
                }
            }
        };

        /**
         * @brief wait times for field system and correlator synchronization
         */
        struct WaitTimes {
            unsigned int fieldSystem = 6; ///< time required for setup
            unsigned int preob = 10; ///< time required for source
            unsigned int midob = 3; ///< time required for tape
            unsigned int postob = 0; ///< calibration time
        };

        /**
         * @brief setter for wait times
         * @param waittimes new wait times
         */
        void setWaitTimes(WaitTimes &waittimes) {
            Station::waitTimes_ = std::make_shared<WaitTimes>(std::move(waittimes));
        }

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
            std::vector<unsigned int> scanStartTimes{};
            int totalObservingTime{0};
            int totalSlewTime{0};
            int totalIdleTime{0};
            int totalFieldSystemTime{0};
            int totalPreobTime{0};
        };

        /**
         * @brief constructor
         *
         * @param sta_name station name
         * @param id station id
         * @param sta_antenna station antenna
         * @param sta_cableWrap station cable wrap
         * @param sta_position station position
         * @param sta_equip station equipment
         * @param sta_mask station horizon mask
         * @param sta_axis station axis type
         */
        Station(std::string sta_name, std::string tlc, std::shared_ptr<Antenna> sta_antenna,
                std::shared_ptr<CableWrap> sta_cableWrap, std::shared_ptr<Position> sta_position,
                std::shared_ptr<Equipment> sta_equip, std::shared_ptr<HorizonMask> sta_mask);


        /**
         * @brief getter for parameters
         *
         * @return currently used parameters
         */
        const Parameters &getPARA() const {
            return parameters_;
        }

        /**
         * @brief reference to current parameters
         *
         * @return reference of current parameters
         */
        Parameters &referencePARA() {
            return parameters_;
        }

        /**
         * @brief getter for wait times
         *
         * @return station wait times
         */
        const WaitTimes &getWaittimes() const {
            return *waitTimes_;
        }


        /**
         * @brief getter for cable wrap
         * @return cable wrap of this station
         */
        const CableWrap &getCableWrap() const noexcept {
            return *cableWrap_;
        }

        /**
         * @brief reference to cable wrap
         * @return cable wrap of this station
         */
        CableWrap &referenceCableWrap() noexcept {
            return *cableWrap_;
        }

        /**
         * @brief getter for last time this antenna was mentioned in scheduling
         * @return last station usage time in seconds since session start
         */
        unsigned int getCurrentTime() const noexcept {
            return currentPositionVector_.getTime();
        }

        /**
         * @brief getter for equipment information
         * @return equipment objecct
         */
        const Equipment &getEquip() const noexcept {
            return *equip_;
        }

        bool hasHorizonMask() const{
            return mask_ !=  nullptr;
        }

        /**
         * @brief getter for horizon mask
         *
         * @return horizon mask object
         */
        const HorizonMask &getMask() const {
            return *mask_;
        }

        /**
         * @brief getter for antenna
         * @return antenna object
         */
        const Antenna &getAntenna() const noexcept {
            return *antenna_;
        }

        /**
         * @brief getter for position
         * @return position object
         */
        const Position &getPosition() const noexcept {
            return *position_;
        }

        /**
         * @brief getter for number of baselines which were already observed with this station
         *
         * @return number of already observed baselines
         */
        int getNObs() const noexcept {
            return nObs_;
        }

        int getNScans() const noexcept  {
            return nScans_;
        }

        int getNTotalScans() const noexcept{
            return nTotalScans_;
        }

        /**
         * @brief getter for current pointing vector
         * @return current pointing vector
         */
        const PointingVector &getCurrentPointingVector() const noexcept {
            return currentPositionVector_;
        }

        /**
         * @brief distance between two stations
         *
         * @param other other station
         * @return distance between this two stations
         */
        double distance(const Station &other) const noexcept;

        /**
         * @brief checks if a source is visible for this station
         *
         * @param p pointing vector which holds time information and will be filled with azimuth and elevation information
         * @return true if station is visible
         */
        bool isVisible(const PointingVector &p, double minElevationSource = 0) const noexcept;

        /**
         * @brief calculate slew time between current pointing vector and this pointing vector
         *
         * If this is the first scan of this station the slew time is zero.
         *
         * @param pointingVector slew end position
         * @return slew time in seconds
         */
        boost::optional<unsigned int> slewTime(const PointingVector &pointingVector) const noexcept;

        /**
         * @brief calculate azimuth and elevation of source at a given time
         *
         * !!! This function changes p !!!
         *
         * @param source observed source
         * @param p pointing vector where azimuth and elevation is saved
         * @param model model used for calculation (default is simple model)
         */
        void
        calcAzEl(const Source &source, PointingVector &p,
                 AzelModel model = AzelModel::rigorous) const noexcept;

        /**
         * @brief change current pointing vector
         *
         * @param pointingVector new current pointing vector
         */
        void setCurrentPointingVector(const PointingVector &pointingVector) noexcept;
        
        /**
         * @brief sets all upcoming events
         * @param EVENTS all upcoming events
         */
        void setEVENTS(std::vector<Event> &EVENTS) noexcept {
            Station::events_ = std::make_shared<std::vector<Event>>(move(EVENTS));
            Station::nextEvent_ = 0;
        }


        bool checkForTagalongMode(unsigned int time) const noexcept;

        /**
         * @brief this function checks if it is time to change the parameters
         *
         * !!! This function changes hardBreak and tagalong !!!
         *
         * @param time current time in seconds since start
         * @param hardBreak flag if a hard break was found
         * @param bodyLog output stream object
         * @param tagalong flag if station should be scheduled in tagalong mode
         */
        bool checkForNewEvent(unsigned int time, bool &hardBreak) noexcept;

        unsigned int maximumAllowedObservingTime(Timestamp ts) const noexcept;

        /**
         * @brief changes parameters to next setup
         *
         * @param out output stream object
         */
        void applyNextEvent(std::ofstream & out) noexcept;

        void setNextEvent(unsigned int idx) noexcept{
            nextEvent_ = idx;
        }

        /**
         * @brief update station if used for a scan
         *
         * @param nbl number of observed baselines
         * @param start pointing vector at start time
         * @param end pointing vector at end time
         * @param addToStatistics flag if scan should have an influence on the further scheduling process
         */
        void update(unsigned long nbl, const PointingVector &end, bool addToStatistics) noexcept;

        void clearObservations();

        void setStatistics(const Statistics &stat){
            statistics_ = stat;
        }

        const Statistics &getStatistics() const {
            return statistics_;
        }

    private:
        static unsigned long nextId;

        std::shared_ptr<Antenna> antenna_; ///< station antenna
        std::shared_ptr<CableWrap> cableWrap_; ///< station cable wrap
        std::shared_ptr<Position> position_; ///< station position
        std::shared_ptr<Equipment> equip_; ///< station equipment
        std::shared_ptr<HorizonMask> mask_; ///< station horizon mask
        std::shared_ptr<WaitTimes> waitTimes_; ///< station wait times
        std::shared_ptr<std::vector<Event>> events_; ///< list of all events

        Statistics statistics_;

        Parameters parameters_; ///< station parameters
        PointingVector currentPositionVector_; ///< current pointing vector
        unsigned int nextEvent_{0}; ///< index of next event
        int nScans_{0}; ///< number of participated scans
        int nTotalScans_{0}; ///< number of total scans
        int nObs_{0}; ///< number of observed baselines
    };
}
#endif /* STATION_H */

