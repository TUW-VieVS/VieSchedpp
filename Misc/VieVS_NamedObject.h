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
 * @file VieVS_NamedObject.h
 * @brief class VieVS_NamedObject
 *
 *
 * @author Matthias Schartner
 * @date 15.03.2018
 */

#ifndef VLBI_SCHEDULER_VIEVS_NAMEDOBJECT_H
#define VLBI_SCHEDULER_VIEVS_NAMEDOBJECT_H

#include <string>
#include <utility>
#include "VieVS_Object.h"

namespace VieVS {

/**
 * @class VieVS_NamedObject
 * @brief VieVS base object with name
 *
 * @author Matthias Schartner
 * @date 15.03.2018
 */

class VieVS_NamedObject : public VieVS_Object {
   public:
    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param name object name
     * @param id object id
     */
    VieVS_NamedObject( std::string name, unsigned long id ) : VieVS_Object( id ), name_{std::move( name )} {};

    /**
     * @brief constructor
     * @author Matthias Schartner
     *
     * @param name object name
     * @param alternativeName object alternative name
     * @param id object id
     */
    VieVS_NamedObject( std::string name, std::string alternativeName, unsigned long id )
        : VieVS_Object( id ), name_{std::move( name )}, alternativeName_{std::move( alternativeName )} {};

    /**
     * @brief get object name
     * @author Matthias Schartner
     *
     * @return object name
     */
    const std::string &getName() const { return name_; }

    /**
     * @brief get object alternative name
     * @author Matthias Schartner
     *
     * @return object alternative name
     */
    const std::string &getAlternativeName() const { return alternativeName_; }

    /**
     * @brief check if object has alternative name
     * @author Matthias Schartner
     *
     * @return flag if alternative name exists
     */
    bool hasAlternativeName() const { return !alternativeName_.empty(); }

    /**
     * @brief check if object has name
     * @author Matthias Schartner
     *
     * checks object name and alternative object name
     *
     * @param name target name
     * @return true if object has target name
     */
    bool hasName( const std::string &name ) const { return name_ == name || alternativeName_ == name; }

    /**
     * @brief change name of object
     * @author Matthias Schartner
     *
     * Typically Objects should have an distinct name which should not change.
     * Try to avoid this function in the VieSchedpp Project!
     * It is implemented mainly for GUI interactions through VieSchedppGUI.
     *
     * @param newName new name
     */
    void changeName( const std::string &newName ) { name_ = newName; }

   private:
    std::string name_ = "";             ///< object name
    std::string alternativeName_ = "";  ///< object alternative name
};

}  // namespace VieVS

#endif  // VLBI_SCHEDULER_VIEVS_NAMEDOBJECT_H
