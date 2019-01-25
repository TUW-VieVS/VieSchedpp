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
* @file If.h
* @brief class If
*
* @author Matthias Schartner
* @date 17.09.2018
*/

#ifndef VIESCHEDPP_IF_H
#define VIESCHEDPP_IF_H

#include <utility>
#include <vector>
#include <fstream>
#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include "../Misc/VieVS_NamedObject.h"

namespace VieVS{

    /**
     * @class If
     * @brief IF section of observing mode
     *
     * following vex standard
     * The $IF block defines the characteristics of the IF bands used in the observations and is linked to the $BBC
     * block (which specifies the detailed BBC-to-IF connections). An if_def statement must be defined for each of the
     * IF 'links' specified in the selected $BBC 'def'.
     *
     * @author Matthias Schartner
     * @date 17.09.2018
     */
    class If: public VieVS_NamedObject {
    private:
        class If_def;
    public:

        /**
         * @brief polarization types
         * @author Matthias Schartner
         */
        enum class Polarization{
            R, ///< right
            L, ///< left
            X, ///< x-axis
            Y, ///< y-axis
            H, ///< horizontal
            V, ///< vertical
        };

        /**
         * @brief net sideband types
         * @author Matthias Schartner
         */
        enum class Net_sidband{
            U, ///< upper
            L, ///< lower
            D, ///< D?
        };

        /**
         * @brief converts polarization to vex format string
         * @author Matthias Schartner
         *
         * @param p polarization type
         * @return string in vex format
         */
        static std::string toString(Polarization p) {
            switch(p){
                case Polarization::R: return "R";
                case Polarization::L: return "L";
                case Polarization::X: return "X";
                case Polarization::Y: return "Y";
                case Polarization::H: return "H";
                case Polarization::V: return "V";
            }
        }

        /**
         * @brief convert vex format string to polarization type
         * @author Matthias Schartner
         *
         * @param s name in vex format
         * @return polarization type
         */
        static Polarization polarizationFromString(const std::string &s){
            if(s == "R"){
                return Polarization::R;
            }
            if(s == "L"){
                return Polarization::L;
            }
            if(s == "X"){
                return Polarization::X;
            }
            if(s == "Y"){
                return Polarization::Y;
            }
            if(s == "H"){
                return Polarization::H;
            }
            if(s == "V"){
                return Polarization::V;
            }
        }


        /**
         * @brief converts net sideband type to vex format string
         * @author Matthias Schartner
         *
         * @param n net sideband type
         * @return string in vex format
         */
        static std::string toString(Net_sidband n) {
            switch(n){
                case Net_sidband::U: return "U";
                case Net_sidband::L: return "L";
                case Net_sidband::D: return "D";
            }
        }

        /**
         * @brief convert vex format string to net sideband type
         * @author Matthias Schartner
         *
         * @param s net sideband name in vex format
         * @return net sideband type
         */
        static Net_sidband netSidebandFromString(const std::string &s){
            if(s == "U"){
                return Net_sidband::U;
            }
            if(s == "L"){
                return Net_sidband::L;
            }
            if(s == "D"){
                return Net_sidband::D;
            }
        }


        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param name IF block name
         */
        explicit If(std::string name);

        /**
         * @brief constructor
         * @author Matthias Schartner
         *
         * @param tree input property tree from xml file
         */
        explicit If(const boost::property_tree::ptree &tree);


        /**
         * @brief define new IF
         * @author Matthias Schartner
         *
         * @param name 'IF_ID' link word
         * @param physical_name Physical IF name
         * @param polarization Polarization
         * @param total_lo Total effective LO of IF (just before signal enters BBC or sampler)
         * @param net_sidband Net sideband of IF
         * @param phase_cal_freq_spacing Phase-cal frequency interval
         * @param phase_cal_base_freqency Phase-cal base frequency
         */
        void addIf(std::string name, std::string physical_name, Polarization polarization,  double total_lo,
                   Net_sidband net_sidband, double phase_cal_freq_spacing, double phase_cal_base_freqency);

        /**
         * @brief converts object to property tree
         * @author Matthias Schartner
         *
         * @return property tree
         */
        boost::property_tree::ptree toPropertytree() const;


        /**
         * @brief writes If block in vex format
         * @author Matthias Schartner
         *
         * @param of vex file stream
         * @param comment optional comment
         */
        void toVecIfDefinition( std::ofstream &of, const std::string &comment = "") const;


        /**
          * @brief get IF definitions
          * @author Matthias schartner
          *
          * necessary for custom observing mode in GUI
          *
          * @return all IF definitions
          */
        const std::vector<If_def> &getIf_defs() const{
            return if_defs_;
        }

        /**
          * @brief reference IF definitions
          * @author Matthias schartner
          *
          * necessary for custom observing mode in GUI
          *
          * @return reference to all IF definitions
          */
        std::vector<If_def> &refIf_defs(){
            return if_defs_;
        }


    private:
        static unsigned long nextId; ///< next id for this object type


        /**
         * @class If_def
         * @brief IF definition
         *
         * @author Matthias Schartner
         * @date 17.09.2018
         */
        class If_def: public VieVS_NamedObject{
        public:

            /**
             * @brief constructor
             * @author Matthias Schartner
             *
             * @param name 'IF_ID' link word
             * @param physical_name Physical IF name
             * @param polarization Polarization
             * @param total_lo Total effective LO of IF (just before signal enters BBC or sampler)
             * @param net_sidband Net sideband of IF
             * @param phase_cal_freq_spacing Phase-cal frequency interval
             * @param phase_cal_base_freqency Phase-cal base frequency
             */
            If_def(std::string name,
                   std::string physical_name,
                   Polarization polarization,
                   double total_lo,
                   Net_sidband net_sidband,
                   double phase_cal_freq_spacing,
                   double phase_cal_base_freqency);


            /**
             * @brief constructor
             * @author Matthias Schartner
             *
             * @param tree input property tree from xml file
             */
            explicit If_def(const boost::property_tree::ptree &tree);

            /**
             * @brief converts object to property tree
             * @author Matthias Schartner
             *
             * @return property tree
             */
            boost::property_tree::ptree toPropertytree() const;


            std::string physical_name_; ///< Physical IF name
            Polarization polarization_; ///< Polarization
            double total_lo_; ///< Total effective LO of IF (just before signal enters BBC or sampler)
            Net_sidband net_sidband_; ///< Net sideband of IF
            double phase_cal_freq_spacing_; ///< Phase-cal frequency interval
            double phase_cal_base_frequency_; ///< Phase-cal base frequency

        private:
            static unsigned long nextId; ///< next id for this object type

        };

        std::vector<If_def> if_defs_; ///< list of IF definitions
    };
}


#endif //VIESCHEDPP_IF_H
