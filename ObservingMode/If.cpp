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

#include "If.h"


using namespace VieVS;
using namespace std;

unsigned long VieVS::If::nextId = 0;
unsigned long VieVS::If::If_def::nextId = 0;


If::If( std::string name ) : VieVS_NamedObject{std::move( name ), nextId++} {}


If::If( const boost::property_tree::ptree &tree )
    : VieVS_NamedObject{tree.get<std::string>( "<xmlattr>.name" ), nextId++} {
    for ( const auto &any : tree ) {
        if ( any.first == "if_def" ) {
            if_defs_.emplace_back( any.second );
        }
    }
}


void If::addIf( std::string name, std::string physical_name, If::Polarization polarization, double total_lo,
                If::Net_sidband net_sidband, double phase_cal_freq_spacing, double phase_cal_base_freqency ) {
    if_defs_.emplace_back( name, physical_name, polarization, total_lo, net_sidband, phase_cal_freq_spacing,
                           phase_cal_base_freqency );
}


boost::property_tree::ptree If::toPropertytree() const {
    boost::property_tree::ptree p;
    p.add( "<xmlattr>.name", getName() );
    for ( const auto &any : if_defs_ ) {
        p.add_child( "if_def", any.toPropertytree() );
    }
    return p;
}


void If::toVecIfDefinition( std::ofstream &of, const std::string &comment ) const {
    of << "    def " << getName() << ";\n*   " << comment << "\n";
    of << "*                  IF   Physical Pol    Total      Net     Phase-cal   P-cal base  \n"
          "*                  ID     Name            IO        SB   freq spacing     freq\n";
    for ( const auto &any : if_defs_ ) {
        of << boost::format( "        if_def = %6s :   %2s : %2s : %7.2f MHz : %2s : %7.2f MHz : % 7.2f Hz;" ) %
              any.getName() % any.physical_name_ % toString( any.polarization_ ) % any.total_lo_ %
              toString( any.net_sidband_ ) % any.phase_cal_freq_spacing_ % any.phase_cal_base_frequency_;
        if(getName() == "VG_2GB"){
            if(any.physical_name_ == "A"){
                of << " *    7900.00   3100.00  13cm     0 NA\n";
            }else if(any.physical_name_ == "B"){
                of << " *    7900.00   3100.00  4cm     0 NA\n";
            }else{
                of << "\n";
            };
        }else{
            of << "\n";
        }
    }

    of << "    enddef;\n";
}


If::If_def::If_def( std::string name, std::string physical_name, If::Polarization polarization, double total_lo,
                    If::Net_sidband net_sidband, double phase_cal_freq_spacing, double phase_cal_base_freqency )
    : VieVS_NamedObject{std::move( name ), If_def::nextId++},
      physical_name_{std::move( physical_name )},
      polarization_{polarization},
      total_lo_{total_lo},
      net_sidband_{net_sidband},
      phase_cal_base_frequency_{phase_cal_base_freqency},
      phase_cal_freq_spacing_{phase_cal_freq_spacing} {}


If::If_def::If_def( const boost::property_tree::ptree &tree )
    : VieVS_NamedObject{tree.get<std::string>( "IF_ID" ), If_def::nextId++} {
    physical_name_ = tree.get<std::string>( "physical_name" );
    polarization_ = polarizationFromString( tree.get<std::string>( "polarization" ) );
    total_lo_ = tree.get<double>( "total_lo" );
    net_sidband_ = netSidebandFromString( tree.get<std::string>( "net_sidband" ) );
    phase_cal_freq_spacing_ = tree.get<double>( "phase_cal_freq_spacing" );
    phase_cal_base_frequency_ = tree.get<double>( "phase_cal_base_frequency" );
}


boost::property_tree::ptree If::If_def::toPropertytree() const {
    boost::property_tree::ptree p;

    p.add( "IF_ID", getName() );
    p.add( "physical_name", physical_name_ );
    p.add( "polarization", toString( polarization_ ) );
    p.add( "total_lo", total_lo_ );
    p.add( "net_sidband", toString( net_sidband_ ) );
    p.add( "phase_cal_freq_spacing", phase_cal_freq_spacing_ );
    p.add( "phase_cal_base_frequency", phase_cal_base_frequency_ );

    return p;
};
