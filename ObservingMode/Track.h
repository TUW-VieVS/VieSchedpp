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
 * @file Track.h
 * @brief class Track
 *
 * @author Matthias Schartner
 * @date 17.09.2018
 */

#ifndef VIESCHEDPP_TRACKS_H
#define VIESCHEDPP_TRACKS_H


#include <algorithm>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <fstream>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "../Misc/VieVS_NamedObject.h"


namespace VieVS {

/**
 * @class Track
 * @brief track section of observing mode
 *
 * following vex standard
 * The $TRACKS block defines the various multiplex (fan-in and fan-out) modes that can be used to record data with Mark
 * 5A recorders. For purposes of multiplex definitions, the sample data from each channel are separated into a 'sign'
 * bitstream and (for 2-bit sampling) a 'magnitude' bitstream. The fan-out modes (single bitstream to 1, 2 or 4 tracks)
 * are defined with a set of fanout_def statements, one such statement for each bitstream, which defines the destination
 * tracks and bit ordering among the tracks. In this way a complete definition of the multiplex format is specified. The
 * 'ChanID' linkword in each fanout_def statement connects a particular bitstream to the selected 'def' in the $FREQ
 * block.
 *
 * @author Matthias Schartner
 * @date 17.09.2018
 */
class Track : public VieVS_NamedObject {
   private:
    class Fanout_definition;


   public:
    /**
     * @brief bitstream types
     * @author Matthias Schartner
     */
    enum class Bitstream {
        sign,  ///< sign
        mag,   ///< magnitude
    };


    /**
     * @brief convert bitstream type to vex format string
     * @author Matthias Schartner
     *
     * @param b bitsream type
     * @return name in vex format
     */
    static std::string toString( Bitstream b ) {
        switch ( b ) {
            case Bitstream::sign:
                return "sign";
            case Bitstream::mag:
                return "mag";
        }
    }


    /**
     * @brief convert vex format string to bitstream type
     * @author Matthias Schartner
     *
     * @param name name in vex format
     * @return bitsream type
     */
    static Bitstream bitstreamFromString( const std::string &name ) {
        if ( name == "mag" ) {
            return Bitstream::mag;
        } else {
            return Bitstream::sign;
        }
    }


    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param name tracks name
     */
    explicit Track( std::string name );


    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param tree input property tree from xml file
     */
    explicit Track( const boost::property_tree::ptree &tree );


    /**
     * @brief get number of recording bits between two TRACKS blocks
     * @author Matthias Schartner
     *
     * @param other other TRACKS block
     * @return number of recording bits per channel
     */
    std::map<std::string, int> numberOfBitsPerChannel( const std::shared_ptr<const Track> &other ) const;


    /**
     * @brief get number of recording bits
     * @author Matthias Schartner
     *
     * @return number of recording bits per channel
     */
    std::map<std::string, int> numberOfBitsPerChannel() const;


    /**
     * @brief number of tracks
     * @author Matthias Schartner
     *
     * @return number of track definitions
     */
    int numberOfTracks() const { return static_cast<int>( fanout_definitions_.size() ); }


    /**
     * @brief add new fanout
     * @author Matthias Schartner
     *
     * @param subpass Sub-pass ID
     * @param trksId 'Chan_ID' linkword
     * @param bitstream Sign or magnitude bitstream
     * @param headstack_number Headstack number
     * @param first_multiplex_track First multiplex track
     * @param second_multiplex_track Second multiplex track
     * @param third_multiplex_track Third multiplex track
     * @param fourth_multiplex_track Fourth multiplex track
     */
    void addFanout( std::string subpass, std::string trksId, Bitstream bitstream, int headstack_number,
                    int first_multiplex_track, int second_multiplex_track = -999, int third_multiplex_track = -999,
                    int fourth_multiplex_track = -999 );


    /**
     * @brief writes TRACKS block in vex format
     * @author Matthias Schartner
     *
     * @param of vex file stream
     * @param comment optional comment
     */
    void toVexTracksDefinition( std::ofstream &of, const std::string &comment = "" ) const;


    /**
     * @brief converts object to property tree
     * @author Matthias Schartner
     *
     * @return property tree
     */
    boost::property_tree::ptree toPropertytree() const;


    /**
     * @brief get fanout definitions
     * @author Matthias schartner
     *
     * necessary for custom observing mode in GUI
     *
     * @return all fanout definitions
     */
    const std::vector<Fanout_definition> &getFanout_defs() const { return fanout_definitions_; }


    /**
     * @brief reference fanout definitions
     * @author Matthias schartner
     *
     * necessary for custom observing mode in GUI
     *
     * @return reference to all fanout definitions
     */
    std::vector<Fanout_definition> &refFanout_defs() { return fanout_definitions_; }


   private:
    static unsigned long nextId;  ///< next id for this object type

    /**
     * @class Fanout_definition
     * @brief Fanout definition
     *
     * @author Matthias Schartner
     * @date 17.09.2018
     */
    class Fanout_definition : public VieVS_Object {
       public:
        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param subpass Sub-pass ID
         * @param trksId 'Chan_ID' linkword
         * @param bitstream Sign or magnitude bitstream
         * @param headstack_number Headstack number
         * @param first_multiplex_track First multiplex track
         * @param second_multiplex_track Second multiplex track
         * @param third_multiplex_track Third multiplex track
         * @param fourth_multiplex_track Fourth multiplex track
         */
        Fanout_definition( std::string subpass, std::string trksId, Bitstream bitstream, int headstack_number,
                           int first_multiplex_track, int second_multiplex_track = -999,
                           int third_multiplex_track = -999, int fourth_multiplex_track = -999 );


        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param tree input property tree from xml file
         */
        explicit Fanout_definition( const boost::property_tree::ptree &tree );


        /**
         * @brief converts object to property tree
         * @author Matthias Schartner
         *
         * @return property tree
         */
        boost::property_tree::ptree toPropertytree() const;


        std::string subpass_;   ///< Sub-pass ID
        std::string trksid_;    ///< 'Chan_ID' linkword
        Bitstream bitstream_;   ///< Sign or magnitude bitstream
        int headstack_number_;  ///< Headstack number

        int first_multiplex_track_;   ///< First multiplex track
        int second_multiplex_track_;  ///< Second multiplex track
        int third_multiplex_track_;   ///< Third multiplex track
        int fourth_multiplex_track_;  ///< Fourth multiplex track

       private:
        static unsigned long nextId;  ///< next id for this object type
    };


    std::vector<Fanout_definition> fanout_definitions_;  ///< list of fanout definitions
};
}  // namespace VieVS

#endif  // VIESCHEDPP_TRACKS_H
