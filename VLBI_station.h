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
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <utility>
#include <boost/container/flat_map.hpp>

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
            vector<string> parameterGroups; ///< name of .xml groups to which this station belongs
            bool firstScan = true; ///< if true no time is spend for setup, source, tape, calibration, and slewing
            bool available = true; ///< if true this station is available for a scan

            double axis1_low_offset = 5; ///< safety margin for lower limit for first axis in degrees
            double axis1_up_offset = 5; ///< safety margin for upper limit for first axis in degrees
            double axis2_low_offset = 1; ///< safety margin for lower limit for second axis in degrees
            double axis2_up_offset = 1; ///< safety margin for upper limit for second axis in degrees

            vector<pair<string, double> > minSNR; ///< minimum required signal to noise ration for each band

            unsigned int wait_setup = 10; ///< time required for setup
            unsigned int wait_source = 5; ///< time required for source
            unsigned int wait_tape = 1; ///< time required for tape
            unsigned int wait_calibration = 10; ///< calibration time
            unsigned int wait_corsynch = 3; ///< additional scan time vor correlator synchronization
            unsigned int maxSlewtime = 9999; ///< maximum allowed slewtime
            unsigned int maxWait = 9999; ///< maximum allowed wait time for slow antennas
            unsigned int maxScan = 600; ///< maximum allowed scan time
            unsigned int minScan = 30; ///< minimum required scan time

            double weight = 1; ///< multiplicative factor of score for scans with this station
        };

        /**
         * @brief pre calculated values
         */
        struct PRECALCULATED{
            double mjdStart; ///< modified julian date of session start
            vector<double> distance; ///< distance between stations
            vector<double> dx; ///< delta x of station coordinates
            vector<double> dy; ///< delta y of station coordinates
            vector<double> dz; ///< delta z of station coordinates
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
        VLBI_station(string sta_name, 
                int id,
                VLBI_antenna sta_antenna, 
                VLBI_cableWrap sta_cableWrap, 
                VLBI_position sta_position, 
                VLBI_equip sta_equip, 
                VLBI_mask sta_mask,
                string sta_axis);

        /**
         * @brief destuctor
         */
        virtual ~VLBI_station(){};

        /**
         * @brief station availability
         * @return true if station is available
         */
        bool available() {
            return PARA.available;
        }

        /**
         * @brief sets the flag if a station is available or not
         * @param flag true if station is available, otherwise false
         */
        void setAvailable(bool flag) {
            PARA.available = flag;
        }

        /**
         * @brief first scan check
         * @return true if this is the first scan of the station after a break or session start
         */
        bool firstScan() {
            return PARA.firstScan;
        }


        /**
         * @brief getter for maximum allowed slew time
         * @return maximum allowed slew time
         */
        unsigned int getMaxSlewtime(){
            return PARA.maxSlewtime;
        }

        /**
         * @brief getter for maximum allowed idle time
         * @return maximum allowed idle time
         */
        unsigned int getMaxIdleTime(){
            return PARA.maxWait;
        }

        /**
         * @brief getter for minimum required scan time
         * @return minimum required scan time
         */
        unsigned int getMinScanTime(){
            return PARA.minScan;
        }

        /**
         * @brief getter for maximum allowed scan time
         * @return maximum required scan time
         */
        unsigned int getMaxScanTime(){
            return PARA.maxScan;
        }

        /**
         * @brief getter for cable wrap
         * @return cable wrap of this station
         */
        const VLBI_cableWrap &getCableWrap() const {
            return cableWrap;
        }

        /**
         * @brief getter for station name
         * @return station name
         */
        string getName()const {
            return name;
        }

        /**
         * @brief getter for last time this antenna was mentioned in scheduling
         * @return last station usage time in seconds since session start
         */
        unsigned int getCurrentTime(){
            return current.getTime();
        }

        /**
         * @brief getter for equipment information
         * @return equipment objecct
         */
        const VLBI_equip &getEquip() const {
            return equip;
        }


        /**
         * @brief getter for minimum signal to noise ration of a band
         *
         * @param band requested information for this band
         * @return minimum signal to noise ration for this band
         */
        double getMinSNR(string band){
            for (auto &any:PARA.minSNR) {
                if (any.first == band) {
                    return any.second;
                }
            }
        }

        /**
         * @brief getter for distance between two stations
         *
         * @param other_staid other station id
         * @return distance between this two stations
         */
        double getDistance(int other_staid){
            return PRECALC.distance[other_staid];
        }

        /**
         * @brief getter for antenna
         * @return antenna object
         */
        const VLBI_antenna &getAntenna() const {
            return antenna;
        }

        /**
         * @brief getter for position
         * @return position object
         */
        const VLBI_position &getPosition() const {
            return position;
        }

        /**
        * @brief getter for delta x coordinates between two stations
        *
        * @param id second station id
        * @return delta x coordinate between this two stations
        */
        double dx(int id) const{
            return PRECALC.dx[id];
        }

        /**
        * @brief getter for delta y coordinates between two stations
        *
        * @param id second station id
        * @return delta y coordinate between this two stations
         */
        double dy(int id) const{
            return PRECALC.dy[id];
        }

        /**
        * @brief getter for delta z coordinates between two stations
        *
        * @param id second station id
        * @return delta z coordinate between this two stations
         */
        double dz(int id) const{
            return PRECALC.dz[id];
        }

        /**
         * @brief getter for number of baselines which were already observed with this station
         *
         * @return number of already observed baselines
         */
        int getNbls() const {
            return nbls;
        }

        /**
         * @brief getter for sky coverage id
         *
         * @return sky coverage id
         */
        int getSkyCoverageID() const {
            return skyCoverageID;
        }

        /**
         * @brief set the sky coverage id
         *
         * @param id new sky coverage id
         */
        void setSkyCoverageId(int id) {
            skyCoverageID = id;
        }

        /**
         * @brief getter for current pointing vector
         * @return current pointing vector
         */
        const VLBI_pointingVector &getCurrentPointingVector() const {
            return current;
        }

        /**
         * @brief distance between two stations
         *
         * @param other other station
         * @return distance between this two stations
         */
        double distance(VLBI_station other);

        /**
         * @brief checks if a source is visible for this station
         *
         *
         * @param source source which should be checked
         * @param p pointing vector which holds time information and will be filled with azimuth and elevation information
         * @param useTimeFromStation if true additional times will be added, if false time from parameter p will be taken
         * @return true if station is visible
         */
        bool isVisible(VLBI_pointingVector &p);

        /**
         * @brief calculate slew time between current pointing vector and this pointing vector
         *
         * If this is the first scan of this station the slew time is zero.
         *
         * @param pointingVector slew end position
         * @return slewtime in seconds
         */
        unsigned int slewTime(VLBI_pointingVector &pointingVector);

        /**
         * @brief calculate azimuth and elevation of source at a given time
         *
         * @param source observed source
         * @param p pointing vector where azimuth and elevation is saved
         * @param model model used for calculation (default is simple model)
         */
        void
        updateAzEl(VLBI_source &source, VLBI_pointingVector &p, azelModel model = azelModel::simple);

        /**
         * @brief change current pointing vector
         *
         * @param pointingVector new current pointing vector
         */
        void pushPointingVector(VLBI_pointingVector &pointingVector);

        /**
         * @brief sets all parameters from .xml group
         *
         * @param group group name
         * @param PARA_station .xml parameters
         */
        void setParameters(const string& group, boost::property_tree::ptree& PARA_station);

        /**
         * @brief overload of the << operator for output to stream
         *
         * @param out output stream object
         * @param sta station information that should be printed to stream
         * @return stream object
         */
        friend ostream& operator<<(ostream& out, const VLBI_station& sta);

        /**
         * pre calculates some parameters
         *
         * @param mjd modified julian date of session start
         * @param distance distance vector between stations
         * @param dx delta x between stations
         * @param dy delta y between stations
         * @param dz delta z between stations
         */
        void preCalc(double mjd, vector<double> distance, vector<double> dx, vector<double> dy, vector<double> dz);

        /**
         * @brief update station if used for a scan
         *
         * @param nbl number of observed baselines
         * @param start pointing vector at start time
         * @param end pointing vector at end time
         * @param times time stamps of scan
         * @param srcName name of observed source
         */
        void update(unsigned long nbl, VLBI_pointingVector start, VLBI_pointingVector end, vector<unsigned int> times,
                    string srcName);

        //todo: remove this function
        /**
         * @brief sets offsets of cable wrap axis limits
         */
        void setCableWrapMinimumOffsets();

        /**
         * @brief get required time for setup
         * @return setup time in seconds
         */
        unsigned int getWaitSetup()const {
            return PARA.wait_setup;
        }

        /**
        * @brief get required time for source
        * @return source time in seconds
        */
        unsigned int getWaitSource()const {
            return PARA.wait_source;
        }

        /**
         * @brief get required time for tape
         * @return tape time in seconds
         */
        unsigned int getWaitTape()const {
            return PARA.wait_tape;
        }

        /**
         * @brief get required time for calibration
         * @return calibration time in seconds
         */
        unsigned int getWaitCalibration()const {
            return PARA.wait_calibration;
        }

        /**
         * @brief get required time for correlator synchronization
         * @return correlator synchronization time in seconds
         */
        unsigned int getWaitCorsynch() {
            return PARA.wait_corsynch;
        }

        /**
         * @brief returns all pointing vectors which were observed
         *
         * This function is usually at the end of the schedule to check if everything is right.
         *
         * @return first elements are start pointing vectors, second elements are end pointing vectors
         */
        pair<const vector<VLBI_pointingVector>&, const vector<VLBI_pointingVector>& > getAllScans()const {
            return pair<const vector<VLBI_pointingVector>&, const vector<VLBI_pointingVector>& >(pv_startScan,pv_endScan);
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

        VLBI_pointingVector current; ///< current pointing vector

        vector<unsigned int> history_time; ///< history of all event times
        vector<string> history_events; ///< history of all events
        vector<VLBI_pointingVector> pv_startScan; ///< all observed pointing vectors at scan start
        vector<VLBI_pointingVector> pv_endScan; ///< all observed pointing vectors at scan end

        int nscans; ///< number of participated scans
        int nbls; ///< number of observed baselines
    };
}
#endif /* VLBI_STATION_H */

