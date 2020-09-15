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
 * @file Freq.h
 * @brief class Freq
 *
 * @author Matthias Schartner
 * @date 17.09.2018
 */

#ifndef VIESCHEDPP_FREQ_H
#define VIESCHEDPP_FREQ_H


#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../Misc/VieVS_NamedObject.h"
#include "../Misc/util.h"


namespace VieVS {

/**
 * @class Freq
 * @brief FREQ section of observing mode
 *
 * following vex standard
 * The $FREQ block describes the RF sky Frequency, net sideband, and bandwidth characteristics of the channels recorded,
 * where a 'channel' is defined as a single USB or LSB output from a BBC. The $FREQ block does not attempt to
 * describe the recording mode (including format, sample rate, bits per sample, and sample type), since the same set
 * of channels may be recorded in different recording modes (or on different equipment) at different stations. Each
 * frequency channel is defined by a chan_def statement with at least seven fields in each statement.
 *
 * @author Matthias Schartner
 * @date 17.09.2018
 */
class Freq : public VieVS_NamedObject {
   private:
    class Chan_def;


   public:
    /**
     * @brief net sideband types
     * @author Matthias Schartner
     */
    enum class Net_sideband {
        U,   ///< upper side band
        L,   ///< lower side band
        UC,  ///< upper side band I/Q encoding
        LC,  ///< lower side band I/Q encoding
    };


    /**
     * @brief converts Net_sidband to string
     * @author Matthias Schartner
     *
     * @param n Net_sideband type
     * @return name in vex format
     */
    static std::string toString( Net_sideband n ) {
        switch ( n ) {
            case Net_sideband::U:
                return "U";
            case Net_sideband::L:
                return "L";
            case Net_sideband::UC:
                return "UC";
            case Net_sideband::LC:
                return "LC";
            default:
                return "";
        }
    }


    /**
     * @brief convert vex format string to net sideband type
     * @author Matthias Schartner
     *
     * @param s net sideband name in vex format
     * @return net sideband type
     */
    static Net_sideband netSidebandFromString( const std::string &s ) {
        if ( s == "U" ) {
            return Net_sideband::U;
        }
        if ( s == "L" ) {
            return Net_sideband::L;
        }
        if ( s == "UC" ) {
            return Net_sideband::UC;
        }
        if ( s == "LC" ) {
            return Net_sideband::LC;
        }
    }


    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param name FREQ name
     */
    explicit Freq( std::string name );


    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param tree input property tree from xml file
     */
    explicit Freq( const boost::property_tree::ptree &tree );


    /**
     * @brief add new channel
     * @author Matthias Schartner
     *
     * @param bandId 'Band_ID': RF band name
     * @param sky_freq RF sky frequency at 0Hz in the BBC output
     * @param net_sideband Net sideband of this BBC channel
     * @param chan_bandwidth BBC Channel bandwidth
     * @param chan_id 'Chan_ID': Logical channel name
     * @param bbc_id 'BBC_ID': Logical BBC name
     * @param phase_cal_id 'Phase-cal_ID': Logical phase-cal name
     */
    void addChannel( std::string bandId, double sky_freq, Net_sideband net_sideband, double chan_bandwidth,
                     std::string chan_id, std::string bbc_id, std::string phase_cal_id );


    /**
     * @brief converts object to property tree
     * @author Matthias Schartner
     *
     * @return property tree
     */
    boost::property_tree::ptree toPropertytree() const;


    /**
     * @brief get all frequencies for a specific band
     * @author Matthias Schartner
     *
     * @param band target band
     * @return list of all frequencies in this band
     */
    std::vector<double> getFrequencies( const std::string &band ) const;


    /**
     * @brief set sample rate
     * @author Matthias Schartner
     *
     * @param sample_rate Sample frequency
     */
    void setSampleRate( double sample_rate ) { sample_rate_ = sample_rate; }


    /**
     * @brief get list of all bands
     * @author Matthias Schartner
     *
     * @return list of all bands
     */
    const std::set<std::string> &getBands() const { return bands_; }


    /**
     * @brief calculates observing rates for each band between two stations with potentially different FREQ blocks
     * @author Matthias Schartner
     *
     * @param other 2nd FREQ block
     * @param bitsPerChannel number of sampled bits per channel
     * @return total mutual observing rate per band
     */
    std::unordered_map<std::string, double> observingRate( const std::shared_ptr<const Freq> &other,
                                                           const std::map<std::string, int> &bitsPerChannel ) const;


    /**
     * @brief writes FREQ block in vex format
     * @author Matthias Schartner
     *
     * @param of vex file stream
     * @param comment optional comment
     */
    void toVexFreqDefinition( std::ofstream &of, const std::string &comment = "" ) const;


    /**
     * @brief get channel definitions
     * @author Matthias schartner
     *
     * necessary for custom observing mode in GUI
     *
     * @return all channel definitions
     */
    const std::vector<Chan_def> &getChan_defs() const { return chan_defs_; }


    /**
     * @brief reference channel definitions
     * @author Matthias schartner
     *
     * necessary for custom observing mode in GUI
     *
     * @return reference to all channel definitions
     */
    std::vector<Chan_def> &refChan_defs() { return chan_defs_; }


    /**
     * @brief calculate total bandwidth for all channels
     * @author Matthias Schartner
     *
     * @return total bandwidth in MHz
     */
    double totalBandwidth() const;


    /**
     * @brief calculate total observing rate
     * @author Matthias Schartner
     *
     * @param bitsPerChannel number of sampled bits per channel
     * @return total observing rate
     */
    double totalRate( const std::map<std::string, int> &bitsPerChannel ) const;


   private:
    static unsigned long nextId;  ///< next id for this object type

    /**
     * @class Chan_def
     * @brief Channel definition for observing mode
     *
     * @author Matthias Schartner
     * @date 17.09.2018
     */
    class Chan_def : public VieVS_Object {
       public:
        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param bandId 'Band_ID': RF band name
         * @param sky_freq RF sky frequency at 0Hz in the BBC output
         * @param net_sideband Net sideband of this BBC channel
         * @param chan_bandwidth BBC Channel bandwidth
         * @param chan_id 'Chan_ID': Logical channel name
         * @param bbc_id 'BBC_ID': Logical BBC name
         * @param phase_cal_id 'Phase-cal_ID': Logical phase-cal name
         */
        Chan_def( std::string bandId, double sky_freq, Net_sideband net_sideband, double chan_bandwidth,
                  std::string chan_id, std::string bbc_id, std::string phase_cal_id );


        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param tree input property tree from xml file
         */
        explicit Chan_def( const boost::property_tree::ptree &tree );


        /**
         * @brief converts object to property tree
         * @author Matthias Schartner
         *
         * @return property tree
         */
        boost::property_tree::ptree toPropertytree() const;


        std::string bandId_;         ///< 'Band_ID': RF band name
        double sky_freq_;            ///< RF sky frequency at 0Hz in the BBC output
        double wavelength_;          ///< corresponding wavelength of sky frequency
        Net_sideband net_sideband_;  ///< Net sideband of this BBC channel
        double chan_bandwidth_;      ///< BBC Channel bandwidth
        std::string chan_id_;        ///< 'Chan_ID': Logical channel name
        std::string bbc_id_;         ///< 'BBC_ID': Logical BBC name
        std::string phase_cal_id_;   ///< 'Phase-cal_ID': Logical phase-cal name

       private:
        static unsigned long nextId;  ///< next id for this object type
    };


    double sample_rate_;               ///< sample frequency in MHz
    std::vector<Chan_def> chan_defs_;  ///< list of channels
    std::set<std::string> bands_;      ///< list of bands

    /**
     * @brief calculate lower and upper frequency limit of particular channel
     * @author Matthias Schartner
     *
     * @param skyFreq sky frequency in MHz
     * @param bandwidth bandwidth in MHz
     * @param net_sideband net sideband
     * @return lower and upper limit
     */
    std::pair<double, double> lower_upper_bound( double skyFreq, double bandwidth, Net_sideband net_sideband ) const;


    /**
     * @brief calculate overlapping bandwidth
     * @author Matthias Schartner
     *
     * @param low1 lower limit of first channel in MHz
     * @param up1 upper limit of first channel in MHz
     * @param low2 lower limit of second channel in MHz
     * @param up2 upper limit of second channel in MHz
     * @return overlapping bandwidth in MHz
     */
    double overlappingBandwidth( double low1, double up1, double low2, double up2 ) const;
};
}  // namespace VieVS

#endif  // VIESCHEDPP_FREQ_H
