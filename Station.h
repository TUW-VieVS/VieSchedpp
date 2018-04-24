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
#include "Nutation.h"
#include "Earth.h"
#include "TimeSystem.h"

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
            static int nextId;
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

            std::vector<int> ignoreSources; ///< list of all source ids which should be ignored

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
                    for (int ignoreSource : ignoreSources) {
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


        /**
         * @brief pre calculated values
         */
        struct PreCalculated{
            PreCalculated(std::vector<double> distance, std::vector<double> dx, std::vector<double> dy,
                          std::vector<double> dz):distance{std::move(distance)}, dx{std::move(dx)},
                                                  dy{std::move(dy)}, dz{std::move(dz)}{
                g2l.resize(3);
                g2l[0].resize(3);
                g2l[1].resize(3);
                g2l[2].resize(3);
            }

            std::vector<double> distance; ///< distance between stations
            std::vector<double> dx; ///< delta x of station coordinates
            std::vector<double> dy; ///< delta y of station coordinates
            std::vector<double> dz; ///< delta z of station coordinates
            std::vector<std::vector<double> > g2l; ///< geocentric to local transformation matrix
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
        Station(std::string sta_name, std::shared_ptr<Antenna> sta_antenna, std::shared_ptr<CableWrap> sta_cableWrap,
                std::shared_ptr<Position> sta_position, std::shared_ptr<Equipment> sta_equip,
                std::shared_ptr<HorizonMask> sta_mask);

        /**
         * @brief getter for station id
         *
         * @return station id
         */
//        int getId() const {
//            return VieVS_Object.;
//        }

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
        * @brief getter for delta x coordinates between two stations
        *
        * @param id second station id
        * @return delta x coordinate between this two stations
        */
        double dx(int id) const noexcept {
            return preCalculated_->dx[id];
        }

        /**
        * @brief getter for delta y coordinates between two stations
        *
        * @param id second station id
        * @return delta y coordinate between this two stations
         */
        double dy(int id) const noexcept {
            return preCalculated_->dy[id];
        }

        /**
        * @brief getter for delta z coordinates between two stations
        *
        * @param id second station id
        * @return delta z coordinate between this two stations
         */
        double dz(int id) const noexcept {
            return preCalculated_->dz[id];
        }

        /**
         * @brief getter for number of baselines which were already observed with this station
         *
         * @return number of already observed baselines
         */
        int getNbls() const noexcept {
            return nBaselines_;
        }

        int getNScans() const noexcept  {
            return nScans_;
        }

        int getNTotalScans() const noexcept{
            return nTotalScans_;
        }

        /**
         * @brief getter for sky coverage id
         *
         * @return sky coverage id
         */
        int getSkyCoverageID() const noexcept {
            return skyCoverageId_;
        }

        /**
         * @brief set the sky coverage id
         *
         * @param id new sky coverage id
         */
        void setSkyCoverageId(int id) noexcept {
            skyCoverageId_ = id;
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
        bool isVisible(const PointingVector &p, double minElevationSource) const noexcept;

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
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param sta station information that should be printed to stream
         * @return stream object
         */
        friend std::ostream &operator<<(std::ostream &out, const Station &sta) noexcept;

        /**
         * pre calculates some parameters
         *
         * @param distance distance vector between stations
         * @param dx delta x between stations
         * @param dy delta y between stations
         * @param dz delta z between stations
         */
        void preCalc(const std::vector<double> &distance, const std::vector<double> &dx, const std::vector<double> &dy,
                     const std::vector<double> &dz) noexcept;

        /**
         * @brief sets all upcoming events
         * @param EVENTS all upcoming events
         */
        void setEVENTS(std::vector<Event> &EVENTS) noexcept {
            Station::events_ = std::make_shared<std::vector<Event>>(move(EVENTS));
            Station::nextEvent_ = 0;
        }


        /**
         * @brief this function applies all changes of parameters at the start of session
         *
         */
        void checkForNewEvent() noexcept;

        bool checkForTagalongMode(unsigned int time) noexcept;

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
        void checkForNewEvent(unsigned int time, bool &hardBreak, bool output, std::ofstream &out) noexcept;

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
        void update(unsigned long nbl, const PointingVector &start, const PointingVector &end, bool addToStatistics) noexcept;


        /**
         * @brief returns all pointing vectors which were observed
         *
         * This function is usually at the end of the schedule to check if everything is right.
         *
         * @return first elements are start pointing vectors, second elements are end pointing vectors
         */
        std::pair<const std::vector<PointingVector> &, const std::vector<PointingVector> &> getAllScans() const noexcept {
            return {pointingVectorsStart_,pointingVectorsEnd_};
        };

        void addPointingVectorStart(const PointingVector &pv){
            pointingVectorsStart_.push_back(pv);
        }

        void addPointingVectorEnd(const PointingVector &pv){
            pointingVectorsEnd_.push_back(pv);
        }

        void clearObservations();

        void sortPointingVectors();

    private:
        static int nextId;

        std::shared_ptr<Antenna> antenna_; ///< station antenna
        std::shared_ptr<CableWrap> cableWrap_; ///< station cable wrap
        std::shared_ptr<Position> position_; ///< station position
        std::shared_ptr<Equipment> equip_; ///< station equipment
        std::shared_ptr<HorizonMask> mask_; ///< station horizon mask
        std::shared_ptr<WaitTimes> waitTimes_; ///< station wait times
        std::shared_ptr<PreCalculated> preCalculated_; ///< precalculated values
        std::shared_ptr<std::vector<Event>> events_; ///< list of all events


        int skyCoverageId_{-1}; ///< station sky coverage id
        Parameters parameters_; ///< station parameters
        unsigned int nextEvent_{0}; ///< index of next event
        PointingVector currentPositionVector_; ///< current pointing vector
        std::vector<PointingVector> pointingVectorsStart_; ///< all observed pointing vectors at scan start
        std::vector<PointingVector> pointingVectorsEnd_; ///< all observed pointing vectors at scan end
        int nScans_{0}; ///< number of participated scans
        int nTotalScans_{0}; ///< number of total scans
        int nBaselines_{0}; ///< number of observed baselines
    };
}
#endif /* STATION_H */

