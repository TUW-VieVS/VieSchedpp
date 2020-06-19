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

#include "Bbc.h"


using namespace VieVS;
using namespace std;

unsigned long VieVS::Bbc::nextId = 0;
unsigned long VieVS::Bbc::Bbc_assign::nextId = 0;


Bbc::Bbc( std::string name ) : VieVS_NamedObject{ std::move( name ), nextId++ } {}


Bbc::Bbc( const boost::property_tree::ptree &tree )
    : VieVS_NamedObject{ tree.get<std::string>( "<xmlattr>.name" ), nextId++ } {
    for ( const auto &any : tree ) {
        if ( any.first == "BBC_assign" ) {
            bbc_assigns_.emplace_back( any.second );
        }
    }
}


void Bbc::addBbc( std::string name, unsigned int physical_bbc_number, std::string if_name ) {
    bbc_assigns_.emplace_back( name, physical_bbc_number, if_name );
}


boost::property_tree::ptree Bbc::toPropertytree() const {
    boost::property_tree::ptree p;
    p.add( "<xmlattr>.name", getName() );
    for ( const auto &any : bbc_assigns_ ) {
        p.add_child( "BBC_assign", any.toPropertytree() );
    }
    return p;
}


void Bbc::toVexBbcDefinition( std::ofstream &of, const std::string &comment ) const {
    of << "    def " << getName() << ";    " << comment << "\n";
    of << "*                     BBC    Physical   IF\n"
          "*                      ID      BBC#     ID\n";
    for ( const auto &any : bbc_assigns_ ) {
        of << boost::format( "        BBC_assign = %6s :    %02d : %6s;\n" ) % any.getName() %
                  any.physical_bbc_number_ % any.if_name_;
    }

    of << "    enddef;\n";
}


int Bbc::numberOfBBCs() const {
    set<string> names;
    for ( const auto &any : bbc_assigns_ ) {
        names.insert( any.getName() );
    }
    return static_cast<int>( names.size() );
}


Bbc::Bbc_assign::Bbc_assign( std::string name, unsigned int physical_bbc_number, std::string if_name )
    : VieVS_NamedObject{ std::move( name ), Bbc_assign::nextId++ },
      physical_bbc_number_{ physical_bbc_number },
      if_name_{ std::move( if_name ) } {}


Bbc::Bbc_assign::Bbc_assign( const boost::property_tree::ptree &tree )
    : VieVS_NamedObject{ tree.get<std::string>( "BBC_ID" ), Bbc_assign::nextId++ } {
    physical_bbc_number_ = tree.get<unsigned int>( "physical_bbc_number" );
    ;
    if_name_ = tree.get<std::string>( "IF_ID" );
}


boost::property_tree::ptree Bbc::Bbc_assign::toPropertytree() const {
    boost::property_tree::ptree p;

    p.add( "BBC_ID", getName() );
    p.add( "physical_bbc_number", physical_bbc_number_ );
    p.add( "IF_ID", if_name_ );

    return p;
};
