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

#include "Quasar.h"
using namespace std;
using namespace VieVS;

Quasar::Quasar( const string& src_name, const string& src_name2, double src_ra_deg, double src_de_deg,
                unordered_map<std::string, std::unique_ptr<AbstractFlux>>& src_flux )
    : AbstractSource( src_name, src_name2, src_flux ), ra_{ src_ra_deg * deg2rad }, de_{ src_de_deg * deg2rad } {
    PreCalculated preCalculated = PreCalculated();
    preCalculated.sourceInCrs.resize( 3 );
    double sinDe = sin( de_ );
    double cosDe = cos( de_ );
    preCalculated.sourceInCrs[0] = cosDe * cos( ra_ );
    preCalculated.sourceInCrs[1] = cosDe * sin( ra_ );
    preCalculated.sourceInCrs[2] = sinDe;

    preCalculated_ = make_shared<PreCalculated>( move( preCalculated ) );
}

bool Quasar::checkForNewEvent( unsigned int time, bool& hardBreak ) noexcept {
    bool b = AbstractSource::checkForNewEvent( time, hardBreak );

    if ( getSunDistance( time, nullptr ) < getPARA().minSunDistance ) {
        referencePARA().available = false;
    }

    return b;
}

void Quasar::toVex( std::ofstream &of ) const {
    string eol = "\n";
    of << "    def " << getName() << eol;
    of << "        source_type = star" << eol;
    of << "        source_name = " << getName() << eol;
    if ( hasAlternativeName() ) {
        of << "        IAU_name = " << getAlternativeName() << eol;
    } else {
        of << "        IAU_name = " << getName() << eol;
    }
    of << "        ra = " << getRaString( getRa() ) << eol;
    of << "        dec = " << getDeString( getDe() ) << eol;
    of << "        ref_coord_frame = J2000" << eol;
    of << "        ra_rate = 0 asec/yr" << eol;
    of << "        dec_rate = 0 asec/yr" << eol;
    of << "    enddef;\n";
}

void Quasar::toNgsHeader( ofstream& of ) const {
    string strRa;
    {
        double h = rad2deg * ra_ / 15;
        auto m = fmod( h, 1. );
        m *= 60;
        auto s = fmod( m, 1. );
        s *= 60;
        strRa =
            ( boost::format( "%02d %02d   %08.5f " ) % static_cast<int>( h ) % static_cast<int>( m ) % s ).str();
    }

    string strDe;
    {
        double d = rad2deg * de_;
        auto m = abs( fmod( d, 1. ) );
        m *= 60;
        auto s = fmod( m, 1. );
        s *= 60;
        strDe =
            ( boost::format( "%+03d %02d   %08.5f" ) % static_cast<int>( d ) % static_cast<int>( m ) % s ).str();
    }

    of << boost::format( "%-8s  %s  %s\n" ) % getName() % strRa % strDe;
}
