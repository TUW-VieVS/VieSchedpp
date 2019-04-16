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
    std::string name;                  ///< group name
    std::vector<std::string> members;  ///< group members

    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param name group name
     * @param members vector of group members
     */
    ParameterGroup( const std::string &name, const std::vector<std::string> &members );
};
}  // namespace VieVS

#endif  // PARAMETERGROUP_H
