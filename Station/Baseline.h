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
 * @file Baseline.h
 * @brief class Baseline
 *
 * @author Matthias Schartner
 * @date 23.06.2018
 */

#ifndef BASELINE_H
#define BASELINE_H


#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "../Misc/VieVS_NamedObject.h"


namespace VieVS {

/**
 * @class Baseline
 * @brief representation of a VLBI baseline
 *
 * @author Matthias Schartner
 * @date 23.06.2018
 */
class Baseline : public VieVS_NamedObject {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param name baseline name (station 1 two letter code - station 2 two letter code)
     * @param alternativeName baseline alternative name (station 2 two letter code - station 1 two letter code)
     * @param staid1 station 1 id
     * @param staid2 station 2 id
     */
    Baseline( std::string name, std::string alternativeName, unsigned long staid1, unsigned long staid2 );


    /**
     * @brief checks if this baseline is between the two stations
     * @author Matthias Schartner
     *
     * @param staid1 station id 1
     * @param staid2 station id 2
     * @return true is baseline is between those stations, otherwise false
     */
    bool hasStationIds( unsigned long staid1, unsigned long staid2 ) const noexcept;


    /**
     * @brief checks if this baseline is between the two stations
     * @author Matthias Schartner
     *
     * @param staids station ids
     * @return true is baseline is between those stations, otherwise false
     */
    bool hasStationIds( const std::pair<unsigned long, unsigned long> &staids ) const noexcept;


    /**
     * @brief baseline parameters
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
         * @brief copies parameters
         * @author Matthias Schartner
         *
         * @param other parent parameters
         */
        void setParameters( const Parameters &other );


        bool ignore = false;                             ///< ignore this baseline in scheduling process
        double weight = 1;                               ///< weight of this baseline
        unsigned int minScan = 0;                        ///< minimum scan time in seconds
        unsigned int maxScan = 9999;                     ///< maximum scan time in seconds
        std::unordered_map<std::string, double> minSNR;  ///< minimum signal to noise ration for each band
    };


    /**
     * @brief event which changes parameters
     * @author Matthias Schartner
     */
    struct Event {
        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param time time when transition should occure
         * @param smoothTransition true if it is a soft transition, otherwise hard
         * @param PARA new parameters
         */
        Event( unsigned int time, bool smoothTransition, Parameters PARA )
            : time{ time }, smoothTransition{ smoothTransition }, PARA{ std::move( PARA ) } {}


        unsigned int time;      ///< time when new parameters should be used in seconds since start
        bool smoothTransition;  ///< transition type
        Parameters PARA;        ///< new parameters
    };


    /**
     * @brief baseline statistics
     * @author Matthias Schartner
     */
    struct Statistics {
        std::vector<unsigned int> scanStartTimes{};  ///< observation start times
        int totalObservingTime{ 0 };                 ///< total number of observations
    };


    /**
     * @brief sets all upcoming events
     * @author Matthias Schartner
     *
     * @param EVENTS all upcoming events
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
     * @brief getter method for first station id
     * @author Matthias Schartner
     *
     * @return first station id
     */
    unsigned long getStaid1() const noexcept { return staid1_; }


    /**
     * @brief getter method for second station id
     * @author Matthias Schartner
     *
     * @return second station id
     */
    unsigned long getStaid2() const noexcept { return staid2_; }


    /**
     * @brief getter method for station ids
     * @author Matthias Schartner
     *
     * @return pair of station ids
     */
    std::pair<unsigned long, unsigned long> getStaids() const noexcept { return { staid1_, staid2_ }; };


    /**
     * @brief getter for current parameters
     * @author Matthias Schartner
     *
     * @return current parameters
     */
    const Parameters &getParameters() const { return parameters_; }


    /**
     * @brief reference of current parameters
     * @author Matthias Schartner
     *
     * @return current parameters
     */
    Parameters &refParameters() { return parameters_; }


    /**
     * @brief checks for new events at certain time
     * @author Matthias Schartner
     *
     * @param time target time
     * @param hardBreak return which tells you if a hard break occured
     * @return true if a new event occured, otherwise false
     */
    bool checkForNewEvent( unsigned int time, bool &hardBreak ) noexcept;


    /**
     * @brief update new observation
     * @author Matthias Schartner
     *
     * @param influence true is observation should count for further scan selections, otherwise false
     */
    void update( bool influence ) noexcept;


    /**
     * @brief set next event index
     * @author Matthias Schartner
     *
     * @param idx new idx
     */
    void setNextEvent( unsigned int idx ) noexcept { nextEvent_ = idx; }


    /**
     * @brief set baseline statistics
     * @author Matthias Schartner
     *
     * @param stat baseline statistics
     */
    void setStatistics( const Statistics &stat ) { statistics_ = stat; }


    /**
     * @brief getter for baseline statistics
     * @author Matthias Schartner
     *
     * @return baseline statistics
     */
    const Statistics &getStatistics() const { return statistics_; }


    /**
     * @brief get number of observations
     * @author Matthias Schartner
     *
     * @return number of observations
     */
    unsigned long getNObs() const { return nObs_; }


   private:
    static unsigned long nextId;  ///< next id for this object type

    unsigned long staid1_;  ///< id of first antenna
    unsigned long staid2_;  ///< id of second antenna

    std::vector<Event> events_;  ///< list of all events

    Statistics statistics_;  ///< baseline statistics
    Parameters parameters_;  ///< station parameters

    unsigned int nextEvent_{ 0 };  ///< index of next event
    int nObs_{ 0 };                ///< number of observations
    int nTotalObs_{ 0 };           ///< number of total observations
};
}  // namespace VieVS
#endif /* BASELINE_H */
