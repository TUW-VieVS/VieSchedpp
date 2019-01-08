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
 * @file ParameterSetup.h
 * @brief class ParameterSetup
 *
 *
 * @author Matthias Schartner
 * @date 24.08.17.
 */

#ifndef PARAMETERSETUP_H
#define PARAMETERSETUP_H

#include <string>
#include <boost/optional.hpp>
#include <utility>
#include <vector>
#include <memory>

namespace VieVS {
    /**
     * @class ParameterSetup
     * @brief representation of a setup in parameter.xml file
     *
     * @author Matthias Schartner
     * @date 24.08.17.
     */
    class ParameterSetup {
    public:
        /**
         * @brief list of all possible transition types
         * @author Matthias Schartner
         */
        enum class Transition {
            smooth, ///< soft transition
            hard ///< hard transition
        };


        /**
         * @brief empty default constructor
         * @author Matthias Schartner
         */
        ParameterSetup();

        /**
         * @brief constructor for root object
         * @author Matthias Schartner
         *
         * @param start start ot this setup in seconds since start
         * @param end end of this setup in seconds since start
         */
        ParameterSetup(unsigned int start, unsigned int end);

        /**
         * @brief constructor for setup with single member
         * @author Matthias Schartner
         *
         * @param parameterName name of the parameter object
         * @param memberName name of the member
         * @param start start ot this setup in seconds since start
         * @param end end of this setup in seconds since start
         * @param transition transition type
         */
        ParameterSetup(const std::string &parameterName, const std::string &memberName,
                   unsigned int start, unsigned int end, Transition transition = Transition::smooth);


        /**
         * @brief constructor for setup with group member
         * @author Matthias Schartner
         *
         * @param parameterName name of the parameter object
         * @param groupName name of the group
         * @param groupMembers name of the group members
         * @param start start ot this setup in seconds since start
         * @param end end of this setup in seconds since start
         * @param transition transition type
         */
        ParameterSetup(const std::string &parameterName, const std::string &groupName,
                   const std::vector<std::string> &groupMembers, unsigned int start, unsigned int end,
                   Transition transition = Transition::smooth);

        /**
         * @brief getter for parameter name
         * @author Matthias Schartner
         *
         * @return parameter name
         */
        const std::string &getParameterName() const {
            return parameterName_;
        }

        /**
         * @brief getter for member name
         * @author Matthias Schartner
         *
         * @return name of group member
         */
        const std::string &getMemberName() const {
            return memberName_;
        }

        /**
         * @brief getter for start time
         * @author Matthias Schartner
         *
         * @return start time in seconds since start
         */
        unsigned int getStart() const {
            return start_;
        }

        /**
         * @brief getter for end time
         * @author Matthias Schartner
         *
         * @return end time in seconds since start
         */
        unsigned int getEnd() const {
            return end_;
        }

        /**
         * @brief getter for member
         * @author Matthias Schartner
         *
         * @return list of all members
         */
        const std::vector<std::string> &getMembers() const {
            return members_;
        }

        /**
         * @brief getter for setup childrens
         * @author Matthias Schartner
         *
         * @return all setup childrens of this setup
         */
        const std::vector<ParameterSetup> &getChildren() const {
            return childrens_;
        }

        /**
         * @brief getter for transition type
         * @author Matthias Schartner
         *
         * @return transition type
         */
        Transition getTransition() const {
            return transition_;
        }

        /**
         * @brief checks if this setup is a valid child of other setup
         * @author Matthias Schartner
         *
         * @param other other setup
         * @return error code or 0 if everything is ok
         */
        int isValidChild(const ParameterSetup &other) const;

        /**
         * @brief checks if this setup is a valid siblig of other setup
         * @author Matthias Schartner
         *
         * @param other other setup
         * @return error code or 0 if everything is ok
         */
        int isValidSibling(const ParameterSetup &other) const;

        /**
         * @brief add a child setup to current setup
         * @author Matthias Schartner
         *
         * @param child child setup object
         * @return error code or 0 if everything is ok
         */
        int addChild(const ParameterSetup &child);

        /**
         * @brief search parameter child
         * @author Matthias Schartner
         *
         * @param thisLevel current parameter level
         * @param level target level
         * @param parameterName target parameter name
         * @param memberName target member name
         * @param members target members
         * @param transition target transition type
         * @param start target start time
         * @param end target end time
         * @return parameter setup if found
         */
        boost::optional<ParameterSetup &> search(int thisLevel, int level, const std::string &parameterName, const std::string &memberName,
                               const std::vector<std::string> &members, Transition transition,
                               unsigned int start, unsigned int end);

        /**
         * @brief delete parameter setup
         * @author Matthias Schartner
         *
         * @param thisLevel current parameter level
         * @param level target level
         * @param parameterName target parameter name
         * @param memberName target member name
         * @param members target members
         * @param transition target transition type
         * @param start target start time
         * @param end target end time
         * @return if deleted, otherwise false
         */
        bool deleteChild(int thisLevel, int level, const std::string &parameterName, const std::string &memberName,
                         const std::vector<std::string> &members, Transition transition,
                         unsigned int start, unsigned int end);

    private:
        std::string parameterName_; ///< parameter name
        std::string memberName_; ///< member name

        std::vector<std::string> members_; ///< list of all members

        Transition transition_; ///< transition type

        unsigned int start_; ///< start time in seconds since session start
        unsigned int end_; ///< end time in seconds since session start

        std::vector<ParameterSetup> childrens_; ///< list of all setup children

        /**
         * @brief check if parameters are equal
         * @author Matthias Schartner
         *
         * @param parameterName target parameter name
         * @param memberName target member name
         * @param members target members
         * @param transition target transition type
         * @param start target start time
         * @param end target end time
         * @return true if equal, otherwise false
         */
        bool isEqual(std::string parameterName, std::string memberName, std::vector<std::string> members,
                     ParameterSetup::Transition transition, unsigned int start, unsigned int end);
    };
}


#endif //PARAMETERSETUP_H
