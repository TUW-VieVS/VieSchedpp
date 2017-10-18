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
         */
        enum class Transition {
            soft, ///< soft transition
            hard ///< hard transition
        };


        /**
         * @brief empty default constructor
         */
        ParameterSetup();

        /**
         * @brief constructor for root object
         *
         * @param start start ot this setup in seconds since start
         * @param end end of this setup in seconds since start
         */
        ParameterSetup(unsigned int start, unsigned int end);

        /**
         * @brief constructor for setup with single member
         *
         * @param parameterName name of the parameter object
         * @param memberName name of the member
         * @param start start ot this setup in seconds since start
         * @param end end of this setup in seconds since start
         * @param transition transition type
         */
        ParameterSetup(const std::string &parameterName, const std::string &memberName,
                   unsigned int start, unsigned int end, Transition transition = Transition::soft);


        /**
         * @brief constructor for setup with group member
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
                   Transition transition = Transition::soft);

        /**
         * @brief getter for parameter name
         *
         * @return parameter name
         */
        const std::string &getParameterName() const {
            return parameterName_;
        }

        /**
         * @brief getter for member name
         *
         * @return name of group member
         */
        const std::string &getMemberName() const {
            return memberName_;
        }

        /**
         * @brief getter for start time
         *
         * @return start time in seconds since start
         */
        unsigned int getStart() const {
            return start_;
        }

        /**
         * @brief getter for end time
         *
         * @return end time in seconds since start
         */
        unsigned int getEnd() const {
            return end_;
        }

        /**
         * @brief getter for member
         *
         * @return list of all members
         */
        const std::vector<std::string> &getMembers() const {
            return members_;
        }

        /**
         * @brief getter for setup childrens
         *
         * @return all setup childrens of this setup
         */
        const std::vector<ParameterSetup> &getChildren() const {
            return childrens_;
        }

        /**
         * @brief getter for transition type
         *
         * @return transition type
         */
        Transition getTransition() const {
            return transition_;
        }

        /**
         * @brief checks if this setup is a valid child of other setup
         *
         * @param other other setup
         * @return true if it is a valid child
         */
        bool isValidChild(const ParameterSetup &other) const;

        /**
         * @brief checks if this setup is a valid siblig of other setup
         *
         * @param other other setup
         * @return true if it is a valid sibling
         */
        bool isValidSibling(const ParameterSetup &other) const;

        /**
         * @brief add a child setup to current setup
         *
         * @param child child setup object
         * @return true if the addition of the child was sucessfull
         */
        bool addChild(const ParameterSetup &child);

    private:
        std::string parameterName_; ///< parameter name
        std::string memberName_; ///< member name

        std::vector<std::string> members_; ///< list of all members

        Transition transition_; ///< transition type

        unsigned int start_; ///< start time in seconds since session start
        unsigned int end_; ///< end time in seconds since session start

        std::vector<ParameterSetup> childrens_; ///< list of all setup children
    };
}


#endif //PARAMETERSETUP_H
