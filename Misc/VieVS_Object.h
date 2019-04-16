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
 * @file VieVS_Object.h
 * @brief class VieVS_Object
 *
 *
 * @author Matthias Schartner
 * @date 14.08.2017
 */

#ifndef VLBI_SCHEDULER_VIEVS_OBJECT_H
#define VLBI_SCHEDULER_VIEVS_OBJECT_H

#include <string>

namespace VieVS {

/**
 * @class VieVS_Object
 * @brief VieVS base object
 *
 * @author Matthias Schartner
 * @date 15.03.2018
 */

class VieVS_Object {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param id object id
     */
    explicit VieVS_Object( unsigned long id ) : id_{id} {};

    /**
     * @brief get object id
     * @author Matthias Schartner
     *
     * @return object id
     */
    const unsigned long getId() const { return id_; }

    /**
     * @brief check if id is valid
     * @author Matthias Schartner
     *
     * @return flag if id is valid
     */
    bool hasValidId() const { return id_ > 0; }

    /**
     * @brief check if object has specific id
     * @author Matthias Schartner
     *
     * @param id target id
     * @return true if target it is equal to object id, otherwise false
     */
    bool hasId( unsigned long id ) const { return id_ == id; }

    /**
     * @brief set object id
     * @author Matthias Schartner
     *
     * @param id new id
     */
    void setId( unsigned long id ) { id_ = id; }

    /**
     * @brief object id string
     * @author Matthias Schartner
     *
     * @return formatted string with object id
     */
    std::string printId() const { return std::string( "(id: " ).append( std::to_string( id_ ) ).append( ")" ); }

   private:
    unsigned long id_;  ///< object id
};
}  // namespace VieVS

#endif  // VLBI_SCHEDULER_VIEVS_OBJECT_H
