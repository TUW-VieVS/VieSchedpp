/**
 * @file VLBI_station.h
 * @brief class VLBI_station
 *
 *
 * @author Matthias Schartner
 * @date 21.06.2017
 */

#ifndef VLBI_STATION_H
#define VLBI_STATION_H
#include <iostream>
#include <fstream>

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <utility>

#include "VLBI_position.h"
#include "VLBI_antenna.h"
#include "VLBI_cableWrap.h"
#include "VLBI_equip.h"
#include "VLBI_mask.h"
#include "VLBI_source.h"
#include "VLBI_pointingVector.h"
#include "VieVS_constants.h"
#include "VieVS_nutation.h"
#include "VieVS_earth.h"
#include "VieVS_time.h"

#include "sofa.h"

using namespace std;

namespace VieVS{
    
    class VLBI_station {
    public:
        /**
         * @brief antenna type
         */
        enum class axisType {
            AZEL, ///< azimuth elevation antenna
            HADC, ///< hour angle declination antenna
            XYNS, ///< x-y north south antenna
            XYEW, ///< x-y east west antenna
            RICH, ///< keine ahnung
            SEST, ///< keine ahnung
            ALGO, ///< keine ahnung
            undefined ///< undefined antenna type
        };

        /**
         * @brief azimuth elevation calculation model
         */
        enum class azelModel {
            simple, ///< simple model without nutation
            rigorous ///< rigorous model
        };

        /**
         * @brief station parameters
         */
        struct PARAMETERS{
            boost::optional<bool> firstScan = false; ///< if true no time is spend for setup, source, tape, calibration, and slewing
            boost::optional<bool> available = true; ///< if true this station is available for a scan

            boost::optional<double> axis1_low_offset; ///< safety margin for lower limit for first axis in degrees
            boost::optional<double> axis1_up_offset; ///< safety margin for upper limit for first axis in degrees
            boost::optional<double> axis2_low_offset; ///< safety margin for lower limit for second axis in degrees
            boost::optional<double> axis2_up_offset; ///< safety margin for upper limit for second axis in degrees

            unordered_map<string, double> minSNR; ///< minimum required signal to noise ration for each band

            boost::optional<unsigned int> wait_setup; ///< time required for setup
            boost::optional<unsigned int> wait_source; ///< time required for source
            boost::optional<unsigned int> wait_tape; ///< time required for tape
            boost::optional<unsigned int> wait_calibration; ///< calibration time
            boost::optional<unsigned int> wait_corsynch; ///< additional scan time vor correlator synchronization

            boost::optional<unsigned int> maxSlewtime; ///< maximum allowed slewtime
            boost::optional<unsigned int> maxWait; ///< maximum allowed wait time for slow antennas
            boost::optional<unsigned int> maxScan; ///< maximum allowed scan time
            boost::optional<unsigned int> minScan; ///< minimum required scan time

            boost::optional<double> weight; ///< multiplicative factor of score for scans with this station

            vector<int> ignoreSources;
            vector<string> ignoreSources_str;

            void output(std::ofstream &of) const {
                if (*available) {
                    of << "    available: TRUE\n";
                } else {
                    of << "    available: FALSE\n";
                }

                of << "    maxSlewtime: " << *maxSlewtime << "\n";
                of << "    maxWait:     " << *maxWait << "\n";
                of << "    maxScan:     " << *maxScan << "\n";
                of << "    minScan:     " << *minScan << "\n";
                of << "    weight:      " << *weight << "\n";

                for (const auto it:minSNR) {
                    of << "    minSNR: " << it.first << " " << it.second << "\n";
                }

                if (!ignoreSources.empty()) {
                    of << "    ignoreSources:";
                    for (int i = 0; i < ignoreSources.size(); ++i) {
                        of << " " << ignoreSources[i];
                    }
                    of << "\n";
                }
            }
        };


        /**
         * @brief changes in parameters
         */
        struct EVENT {
            unsigned int time;
            bool softTransition;
            PARAMETERS PARA;
        };


        /**
     * @brief pre calculated values
     */
        struct PRECALCULATED{
            vector<double> distance; ///< distance between stations
            vector<double> dx; ///< delta x of station coordinates
            vector<double> dy; ///< delta y of station coordinates
            vector<double> dz; ///< delta z of station coordinates
            vector<vector<double> > g2l; ///< geocentric to local transformation matrix
        };

        /**
         * @brief empty default constructor
         */
        VLBI_station();

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
        VLBI_station(const string &sta_name,
                int id,
                const VLBI_antenna &sta_antenna,
                const VLBI_cableWrap &sta_cableWrap,
                const VLBI_position &sta_position,
                const VLBI_equip &sta_equip,
                const VLBI_mask &sta_mask,
                const string &sta_axis);

        /**
         * @brief default copy constructor
         *
         * @param other other station
         */
        VLBI_station(const VLBI_station &other) = default;

        /**
         * @brief default move constructor
         *
         * @param other other station
         */
        VLBI_station(VLBI_station &&other) = default;

        /**
         * @brief default copy assignment operator
         *
         * @param other other station
         * @return copy of other station
         */
        VLBI_station &operator=(const VLBI_station &other) = default;

        /**
         * @brief default move assignment operator
         *
         * @param other other station
         * @return moved other station
         */
        VLBI_station &operator=(VLBI_station &&other) = default;

        /**
         * @brief destuctor
         */
        virtual ~VLBI_station(){};

        const PARAMETERS &getPARA() const {
            return PARA;
        }

        PARAMETERS &referencePARA() {
            return PARA;
        }

        /**
         * @brief station availability
         * @return true if station is available
         */
        bool available() const noexcept {
            return *PARA.available;
        }

        /**
         * @brief sets the flag if a station is available or not
         * @param flag true if station is available, otherwise false
         */
        void setAvailable(bool flag) noexcept {
            PARA.available = flag;
        }

        /**
         * @brief first scan check
         * @return true if this is the first scan of the station after a break or session start
         */
        bool firstScan() const noexcept {
            return *PARA.firstScan;
        }

        /**
         * @brief sets the flag if this is the first scan
         * @param flag true if this is first scan, otherwise false
         */
        void setFirstScan(bool flag) noexcept {
            PARA.firstScan = flag;
        }


        /**
         * @brief getter for maximum allowed slew time
         * @return maximum allowed slew time
         */
        unsigned int getMaxSlewtime() const noexcept {
            return *PARA.maxSlewtime;
        }

        /**
         * @brief getter for maximum allowed idle time
         * @return maximum allowed idle time
         */
        unsigned int getMaxIdleTime() const noexcept {
            return *PARA.maxWait;
        }

        /**
         * @brief getter for minimum required scan time
         * @return minimum required scan time
         */
        unsigned int getMinScanTime() const noexcept {
            return *PARA.minScan;
        }

        /**
         * @brief getter for maximum allowed scan time
         * @return maximum required scan time
         */
        unsigned int getMaxScanTime() const noexcept {
            return *PARA.maxScan;
        }

        /**
         * @brief getter for cable wrap
         * @return cable wrap of this station
         */
        const VLBI_cableWrap &getCableWrap() const noexcept {
            return cableWrap;
        }

        /**
         * @brief getter for station name
         * @return station name
         */
        const string &getName() const noexcept {
            return name;
        }

        /**
         * @brief getter for last time this antenna was mentioned in scheduling
         * @return last station usage time in seconds since session start
         */
        unsigned int getCurrentTime() const noexcept {
            return current.getTime();
        }

        /**
         * @brief getter for equipment information
         * @return equipment objecct
         */
        const VLBI_equip &getEquip() const noexcept {
            return equip;
        }


        /**
         * @brief getter for minimum signal to noise ration of a band
         *
         * @param band requested information for this band
         * @return minimum signal to noise ration for this band
         */
        double getMinSNR(string band) const noexcept {
            return PARA.minSNR.at(band);
        }

        /**
         * @brief getter for distance between two stations
         *
         * @param other_staid other station id
         * @return distance between this two stations
         */
        double getDistance(int other_staid) const noexcept {
            return PRECALC.distance[other_staid];
        }

        const VLBI_mask &getMask() const {
            return mask;
        }


        /**
         * @brief getter for antenna
         * @return antenna object
         */
        const VLBI_antenna &getAntenna() const noexcept {
            return antenna;
        }

        /**
         * @brief getter for position
         * @return position object
         */
        const VLBI_position &getPosition() const noexcept {
            return position;
        }

        /**
        * @brief getter for delta x coordinates between two stations
        *
        * @param id second station id
        * @return delta x coordinate between this two stations
        */
        double dx(int id) const noexcept {
            return PRECALC.dx[id];
        }

        /**
        * @brief getter for delta y coordinates between two stations
        *
        * @param id second station id
        * @return delta y coordinate between this two stations
         */
        double dy(int id) const noexcept {
            return PRECALC.dy[id];
        }

        /**
        * @brief getter for delta z coordinates between two stations
        *
        * @param id second station id
        * @return delta z coordinate between this two stations
         */
        double dz(int id) const noexcept {
            return PRECALC.dz[id];
        }

        /**
         * @brief getter for number of baselines which were already observed with this station
         *
         * @return number of already observed baselines
         */
        int getNbls() const noexcept {
            return nbls;
        }

        /**
         * @brief getter for sky coverage id
         *
         * @return sky coverage id
         */
        int getSkyCoverageID() const noexcept {
            return skyCoverageID;
        }

        /**
         * @brief set the sky coverage id
         *
         * @param id new sky coverage id
         */
        void setSkyCoverageId(int id) noexcept {
            skyCoverageID = id;
        }

        /**
         * @brief getter for current pointing vector
         * @return current pointing vector
         */
        const VLBI_pointingVector &getCurrentPointingVector() const noexcept {
            return current;
        }

        /**
         * @brief distance between two stations
         *
         * @param other other station
         * @return distance between this two stations
         */
        double distance(const VLBI_station &other) const noexcept;

        /**
         * @brief checks if a source is visible for this station
         *
         *
         * @param source source which should be checked
         * @param p pointing vector which holds time information and will be filled with azimuth and elevation information
         * @param useTimeFromStation if true additional times will be added, if false time from parameter p will be taken
         * @return true if station is visible
         */
        bool isVisible(const VLBI_pointingVector &p) const noexcept;

        /**
         * @brief calculate slew time between current pointing vector and this pointing vector
         *
         * If this is the first scan of this station the slew time is zero.
         *
         * @param pointingVector slew end position
         * @return slewtime in seconds
         */
        unsigned int slewTime(const VLBI_pointingVector &pointingVector) const noexcept;

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
        calcAzEl(const VLBI_source &source, VLBI_pointingVector &p,
                 azelModel model = azelModel::simple) const noexcept;

        /**
         * @brief change current pointing vector
         *
         * @param pointingVector new current pointing vector
         */
        void setCurrentPointingVector(const VLBI_pointingVector &pointingVector) noexcept;

        /**
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param sta station information that should be printed to stream
         * @return stream object
         */
        friend ostream &operator<<(ostream &out, const VLBI_station &sta) noexcept;

        /**
         * pre calculates some parameters
         *
         * @param mjd modified julian date of session start
         * @param distance distance vector between stations
         * @param dx delta x between stations
         * @param dy delta y between stations
         * @param dz delta z between stations
         */
        void preCalc(const vector<double> &distance, const vector<double> &dx, const vector<double> &dy,
                     const vector<double> &dz) noexcept;

        /**
         * @brief sets all upcoming events
         * @param EVENTS all upcoming events
         */
        void setEVENTS(const vector<EVENT> &EVENTS) noexcept {
            VLBI_station::EVENTS = EVENTS;
            VLBI_station::nextEvent = EVENTS[0].time;
        }

        /**
         * @brief this function checks if it is time to change the parameters
         * @param output displays output (default is false)
         */
        void checkForNewEvent(unsigned int time, bool &hardBreak, bool output, ofstream &bodyLog) noexcept;

        /**
         * @brief update station if used for a scan
         *
         * @param nbl number of observed baselines
         * @param start pointing vector at start time
         * @param end pointing vector at end time
         * @param times time stamps of scan
         * @param srcName name of observed source
         */
        void update(unsigned long nbl, const VLBI_pointingVector &start, const VLBI_pointingVector &end,
                    const vector<unsigned int> &times, const string &srcName) noexcept;

        //todo: remove this function
        /**
         * @brief sets offsets of cable wrap axis limits
         */
        void setCableWrapMinimumOffsets() noexcept;

        /**
         * @brief get required time for setup
         * @return setup time in seconds
         */
        unsigned int getWaitSetup() const noexcept {
            return *PARA.wait_setup;
        }

        /**
        * @brief get required time for source
        * @return source time in seconds
        */
        unsigned int getWaitSource() const noexcept {
            return *PARA.wait_source;
        }

        /**
         * @brief get required time for tape
         * @return tape time in seconds
         */
        unsigned int getWaitTape() const noexcept {
            return *PARA.wait_tape;
        }

        /**
         * @brief get required time for calibration
         * @return calibration time in seconds
         */
        unsigned int getWaitCalibration() const noexcept {
            return *PARA.wait_calibration;
        }

        /**
         * @brief get required time for correlator synchronization
         * @return correlator synchronization time in seconds
         */
        unsigned int getWaitCorsynch() const noexcept {
            return *PARA.wait_corsynch;
        }

        /**
         * @brief returns all pointing vectors which were observed
         *
         * This function is usually at the end of the schedule to check if everything is right.
         *
         * @return first elements are start pointing vectors, second elements are end pointing vectors
         */
        pair<const vector<VLBI_pointingVector> &, const vector<VLBI_pointingVector> &> getAllScans() const noexcept {
            return pair<const vector<VLBI_pointingVector> &, const vector<VLBI_pointingVector> &>(pv_startScan,
                                                                                                  pv_endScan);
        };

    private:
        string name; ///< station name
        int id; ///< station id
        VLBI_antenna antenna; ///< station antenna
        VLBI_cableWrap cableWrap; ///< station cable wrap
        VLBI_position position; ///< station position
        VLBI_equip equip; ///< station equipment
        VLBI_mask mask; ///< station horizon mask
        axisType axis; ///< station axis type
        int skyCoverageID; ///< station sky coverage id

        PARAMETERS PARA; ///< station parameters
        PRECALCULATED PRECALC; ///< precalculated values

        vector<EVENT> EVENTS;
        unsigned int nextEvent;

        VLBI_pointingVector current; ///< current pointing vector

        vector<VLBI_pointingVector> pv_startScan; ///< all observed pointing vectors at scan start
        vector<VLBI_pointingVector> pv_endScan; ///< all observed pointing vectors at scan end

        int nscans; ///< number of participated scans
        int nbls; ///< number of observed baselines
    };
}
#endif /* VLBI_STATION_H */

