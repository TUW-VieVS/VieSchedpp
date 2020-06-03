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

#include "ParameterSetup.h"


using namespace std;
using namespace VieVS;


ParameterSetup::ParameterSetup() : start_{ 0 }, end_{ 0 }, transition_{ Transition::smooth } {}


ParameterSetup::ParameterSetup( unsigned int start, unsigned int end )
    : start_{ start }, end_{ end }, transition_{ Transition::smooth } {}


ParameterSetup::ParameterSetup( const std::string &parameterName, const std::string &memberName, unsigned int start,
                                unsigned int end, Transition transition )
    : parameterName_{ parameterName },
      memberName_{ memberName },
      start_{ start },
      end_{ end },
      transition_{ transition } {
    ParameterSetup::members_.push_back( memberName );
}


ParameterSetup::ParameterSetup( const std::string &parameterName, const std::string &groupName,
                                const std::vector<std::string> &groupMembers, unsigned int start, unsigned int end,
                                Transition transition )
    : parameterName_{ parameterName },
      memberName_{ groupName },
      start_{ start },
      end_{ end },
      transition_{ transition } {
    ParameterSetup::members_.insert( members_.end(), groupMembers.begin(), groupMembers.end() );
}


int ParameterSetup::isValidChild( const ParameterSetup &other ) const {
    const std::vector<std::string> &otherMembers = other.getMembers();

    unsigned int otherStart = other.start_;
    unsigned int otherEnd = other.end_;

    if ( other.getMemberName() == "__all__" && memberName_ != "__all__" ) {
        return 1;
    }

    bool valid = otherStart >= start_ && otherEnd <= end_;
    if ( !valid ) {
        return 2;
    }

    for ( const auto &any : otherMembers ) {
        if ( memberName_ != "__all__" && find( members_.begin(), members_.end(), any ) == members_.end() ) {
            return 3;
        }
    }
    return 0;
}


int ParameterSetup::isValidSibling( const ParameterSetup &other ) const {
    unsigned int otherStart = other.start_;
    unsigned int otherEnd = other.end_;
    bool seperate = otherEnd <= start_ || otherStart >= end_;
    if ( seperate ) {
        return 0;
    }

    if ( memberName_ == "__all__" || other.getMemberName() == "__all__" ) {
        return 4;
    }

    const std::vector<std::string> &otherMembers = other.getMembers();

    if ( members_.empty() || otherMembers.empty() ) {
        return 5;
    }

    for ( const auto &any : members_ ) {
        if ( find( otherMembers.begin(), otherMembers.end(), any ) != otherMembers.end() ) {
            return 6;
        }
    }

    return 0;
}


int ParameterSetup::addChild( ParameterSetup c ) {
    // do not add children if it is equal to this
    if ( this->isEqual( c.getParameterName(), c.getMemberName(), c.getMembers(), c.getTransition(), c.getStart(),
                        c.getEnd() ) ) {
        return 0;
    }

    // check if c is a valid child of this
    int errorCode = isValidChild( c );
    if ( errorCode != 0 ) {
        return errorCode;
    }

    // check if there is any already existing children who can serve as valid parent for c
    for ( auto &any : childrens_ ) {
        if ( any.isValidChild( c ) == 0 ) {
            errorCode = any.addChild( c );
            return errorCode;
        }
    }

    // add c here and properly handly existing children (they can become child of c)
    vector<ParameterSetup> newChildren;
    int i = 0;
    while ( i < childrens_.size() ) {
        const auto &any = childrens_[i];
        errorCode = any.isValidSibling( c );
        if ( errorCode != 0 ) {
            int errorSibling = c.isValidChild( any );

            if ( errorSibling == 0 ) {
                newChildren.push_back( any );
                childrens_.erase( childrens_.begin() + i );
                --i;
                errorCode = 0;
            } else {
                return errorCode;
            }
        }
        ++i;
    }

    for ( const auto &any : newChildren ) {
        c.addChild( any );
    }

    childrens_.push_back( c );


    return errorCode;
}


boost::optional<ParameterSetup &> ParameterSetup::search( int thisLevel, int level, const std::string &parameterName,
                                                          const std::string &memberName,
                                                          const std::vector<std::string> &members,
                                                          ParameterSetup::Transition transition, unsigned int start,
                                                          unsigned int end ) {
    if ( thisLevel == level && this->isEqual( parameterName, memberName, members, transition, start, end ) ) {
        return *this;
    } else {
        for ( auto &any : childrens_ ) {
            auto ans = any.search( thisLevel + 1, level, parameterName, memberName, members, transition, start, end );
            if ( ans.is_initialized() ) {
                return ans;
            }
        }
    }
    return boost::none;
}


bool ParameterSetup::deleteChild( int thisLevel, int level, const string &parameterName, const string &memberName,
                                  const std::vector<string> &members, ParameterSetup::Transition transition,
                                  unsigned int start, unsigned int end ) {
    int i = 0;
    for ( auto &any : childrens_ ) {
        if ( thisLevel + 1 == level && any.isEqual( parameterName, memberName, members, transition, start, end ) ) {
            childrens_.erase( childrens_.begin() + i );
            return true;
        }
        ++i;
    }
    bool found = false;
    for ( auto &any : childrens_ ) {
        found = any.deleteChild( thisLevel + 1, level, parameterName, memberName, members, transition, start, end );
        if ( found == true ) {
            return found;
        }
    }
}


bool ParameterSetup::isEqual( std::string parameterName, std::string memberName, std::vector<std::string> members,
                              ParameterSetup::Transition transition, unsigned int start, unsigned int end ) {
    if ( parameterName_ != parameterName ) {
        return false;
    }
    if ( memberName_ != memberName ) {
        return false;
    }
    if ( members_ != members ) {
        return false;
    }
    if ( transition_ != transition ) {
        return false;
    }
    if ( start_ != start ) {
        return false;
    }
    if ( end_ != end ) {
        return false;
    }

    return true;
}
