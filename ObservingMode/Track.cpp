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

#include "Track.h"

using namespace VieVS;
using namespace std;

unsigned long VieVS::Track::nextId = 0;
unsigned long VieVS::Track::Fanout_definition::nextId = 0;

Track::Track( std::string name ) : VieVS_NamedObject{std::move( name ), nextId++} {}

Track::Track( const boost::property_tree::ptree &tree )
    : VieVS_NamedObject{tree.get<std::string>( "<xmlattr>.name" ), nextId++} {
    for ( const auto &any : tree ) {
        if ( any.first == "fanout_def" ) {
            fanout_definitions_.emplace_back( any.second );
        }
    }
}

void Track::addFanout( std::string subpass, std::string trksId, Bitstream bitstream, int headstack_number,
                       int first_multiplex_track, int second_multiplex_track, int third_multiplex_track,
                       int fourth_multiplex_track ) {
    fanout_definitions_.emplace_back( subpass, trksId, bitstream, headstack_number, first_multiplex_track,
                                      second_multiplex_track, third_multiplex_track, fourth_multiplex_track );
}

boost::property_tree::ptree Track::toPropertytree() const {
    boost::property_tree::ptree p;
    p.add( "<xmlattr>.name", getName() );
    for ( const auto &any : fanout_definitions_ ) {
        p.add_child( "fanout_def", any.toPropertytree() );
    }
    return p;
}

void Track::toVexTracksDefinition( std::ofstream &of, const std::string &comment ) const {
    of << "    def " << getName() << ";    " << comment << "\n";
    int n = 1;
    if ( fanout_definitions_[0].second_multiplex_track_ != -999 ) {
        n = 2;
        if ( fanout_definitions_[0].third_multiplex_track_ != -999 ) {
            n = 3;
            if ( fanout_definitions_[0].fourth_multiplex_track_ != -999 ) {
                n = 4;
            }
        }
    }

    of << "*                    sub-   trksId  sign/  hdstk ";
    for ( int i = 0; i < n; ++i ) {
        of << boost::format( " trk%d " ) % i;
    }
    of << "\n";
    of << "*                    pass            mag \n";

    for ( const auto &any : fanout_definitions_ ) {
        of << boost::format( "        fanout_def = %4s : %5s : %4s : %4s :  %02d" ) % any.subpass_ % any.trksid_ %
                  toString( any.bitstream_ ) % any.headstack_number_ % any.first_multiplex_track_;

        if ( any.second_multiplex_track_ != -999 ) {
            of << boost::format( " :  %02d" ) % any.second_multiplex_track_;
            if ( any.third_multiplex_track_ != -999 ) {
                of << boost::format( " :  %02d" ) % any.third_multiplex_track_;
                if ( any.fourth_multiplex_track_ != -999 ) {
                    of << boost::format( " :  %02d" ) % any.fourth_multiplex_track_;
                }
            }
        }
        of << ";\n";
    }
    of << "    enddef;\n";
}

map<string, int> Track::numberOfBitsPerChannel( const std::shared_ptr<const Track> &other ) const {
    map<string, int> v;
    for ( const auto &fanout1 : fanout_definitions_ ) {
        for ( const auto &fanout2 : other->fanout_definitions_ ) {
            if ( fanout1.trksid_ == fanout2.trksid_ && fanout1.bitstream_ == fanout2.bitstream_ ) {
                if ( v.find( fanout1.trksid_ ) != v.end() ) {
                    ++v[fanout1.trksid_];
                    break;
                } else {
                    v[fanout1.trksid_] = 1;
                    break;
                }
            }
        }
    }
    return v;
}

map<string, int> Track::numberOfBitsPerChannel() const {
    map<string, int> v;
    for ( const auto &fanout1 : fanout_definitions_ ) {
        if ( v.find( fanout1.trksid_ ) != v.end() ) {
            ++v[fanout1.trksid_];
        } else {
            v[fanout1.trksid_] = 1;
        }
    }

    return v;
}

Track::Fanout_definition::Fanout_definition( const boost::property_tree::ptree &tree )
    : VieVS_Object( ++Fanout_definition::nextId ) {
    subpass_ = tree.get<std::string>( "subpass" );
    trksid_ = tree.get<std::string>( "trksid" );
    bitstream_ = bitstreamFromString( tree.get<std::string>( "bitstream" ) );
    headstack_number_ = tree.get<int>( "headstack_number" );
    first_multiplex_track_ = tree.get<int>( "first_multiplex_track" );
    second_multiplex_track_ = tree.get( "second_multiplex_track", -999 );
    third_multiplex_track_ = tree.get( "third_multiplex_track", -999 );
    fourth_multiplex_track_ = tree.get( "fourth_multiplex_track", -999 );
}

Track::Fanout_definition::Fanout_definition( std::string subpass, std::string trksId, Track::Bitstream bitstream,
                                             int headstack_number, int first_multiplex_track,
                                             int second_multiplex_track, int third_multiplex_track,
                                             int fourth_multiplex_track )
    : VieVS_Object{Fanout_definition::nextId++},
      subpass_{std::move( subpass )},
      trksid_{std::move( trksId )},
      bitstream_{bitstream},
      headstack_number_{headstack_number},
      first_multiplex_track_{first_multiplex_track},
      second_multiplex_track_{second_multiplex_track},
      third_multiplex_track_{third_multiplex_track},
      fourth_multiplex_track_{fourth_multiplex_track} {}

boost::property_tree::ptree Track::Fanout_definition::toPropertytree() const {
    boost::property_tree::ptree p;

    p.add( "subpass", subpass_ );
    p.add( "trksid", trksid_ );
    p.add( "bitstream", toString( bitstream_ ) );
    p.add( "headstack_number", headstack_number_ );
    p.add( "first_multiplex_track", first_multiplex_track_ );
    if ( second_multiplex_track_ > 0 ) {
        p.add( "second_multiplex_track", second_multiplex_track_ );
    }
    if ( third_multiplex_track_ > 0 ) {
        p.add( "third_multiplex_track", third_multiplex_track_ );
    }
    if ( fourth_multiplex_track_ > 0 ) {
        p.add( "fourth_multiplex_track", fourth_multiplex_track_ );
    }

    return p;
}
