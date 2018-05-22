/**
 * @file ParameterGroup.h
 * @brief class ParameterGroup
 *
 * @author Matthias Schartner
 * @date 10.09.2017
 */

#ifndef PARAMETERGROUP_H
#define PARAMETERGROUP_H

#include <string>
#include <vector>

namespace VieVS {
    /**
     * @class ParameterGroup
     * @brief objects represents a group of any type (station, source, baseline) used only for creating and reading
     * of the parameter.xlm file
     *
     * @author Matthias Schartner
     * @date 10.09.2017
     */
    class ParameterGroup {
    public:
        std::string name; ///< group name
        std::vector<std::string> members; ///< group members

        /**
         * @brief constructor
         *
         * @param name group name
         * @param members vector of group members
         */
        ParameterGroup(const std::string &name, const std::vector<std::string> &members);
    };
}


#endif //PARAMETERGROUP_H
