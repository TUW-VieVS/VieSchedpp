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

#include "Satellite.h"

using namespace std;
using namespace VieVS;

Satellite::Satellite( const std::string& hdr, const std::string& l1, const std::string& l2,
                      unordered_map<std::string, std::unique_ptr<AbstractFlux>>& src_flux )
    : AbstractSource( hdr, util::simplify( hdr ), src_flux ),
      header_{ hdr },
      line1_{ l1 },
      line2_{ l2 } {
    auto epoch = extractReferenceEpoch(l1);
    pSGP4Data_.emplace_back( make_tuple( epoch, SGP4( Tle( hdr, l1, l2 ) ), hdr + "\n" + l1 + "\n" + l2 ) );
    //    startDateTime_ = internalTime2sgpt4Time(0);
    //    string tmp = TimeSystem::time2string(epoch);
    //    std::cout << tmp;
}

std::pair<std::pair<double, double>, std::vector<double>> Satellite::getSourceInCrs(
    unsigned int time, const std::shared_ptr<const Position>& sta_pos ) const {
    auto srcRaDe = getRaDe( time, sta_pos );
    double cosDe = cos( srcRaDe.second );

    return { srcRaDe, { cosDe * cos( srcRaDe.first ), cosDe * sin( srcRaDe.first ), sin( srcRaDe.second ) } };
}

std::tuple<double, double, double, double> Satellite::calcRaDeDistTime(
    unsigned int time, const std::shared_ptr<const Position>& sta_pos ) const noexcept {
    DateTime currentTime = TimeSystem::startSgp4.AddSeconds( time );
    const auto& tmp = getSGP4( time );
    boost::posix_time::ptime reftime = get<0>( tmp );
    const SGP4& sgp4 = get<1>( tmp );

    Eci eci = sgp4.FindPosition( currentTime );
    double dt = abs( ( reftime - TimeSystem::internalTime2PosixTime( time ) ).total_seconds() / 86400.0 );
    CoordGeodetic station;
    if ( sta_pos != nullptr ) {
        station = CoordGeodetic( sta_pos->getLat(), sta_pos->getLon(), sta_pos->getAltitude() / 1000., true );
    }

    Eci stat( currentTime, station );

    // difference vector between station and satellite
    Vector x_sat = eci.Position();
    Vector x_stat = stat.Position();
    Vector xd = x_sat - x_stat;
    Vector vd = eci.Velocity() - stat.Velocity();

    // calculation of right ascension and declination for satellite
    double r = sqrt( xd.x * xd.x + xd.y * xd.y + xd.z * xd.z );
    double de = asin( xd.z / r );
    double ra;

    if ( sqrt( xd.x * xd.x + xd.y * xd.y ) < 0.00000001 ) {
        ra = atan2( vd.y, vd.x );
    } else {
        ra = atan2( xd.y, xd.x );
    }
    return make_tuple( ra, de, r * 1e3, dt );
}

pair<double, double> Satellite::calcRaDe( unsigned int time, const std::shared_ptr<const Position>& sta_pos ) const {
    DateTime currentTime = TimeSystem::startSgp4.AddSeconds( time );
    const SGP4& sgp4 = get<1>( getSGP4( time ) );

    Eci eci = sgp4.FindPosition( currentTime );
    CoordGeodetic station;
    if ( sta_pos != nullptr ) {
        station = CoordGeodetic( sta_pos->getLat(), sta_pos->getLon(), sta_pos->getAltitude()/1000., true );
    }

    Eci stat( currentTime, station );

    // difference vector between station and satellite
    Vector x_sat = eci.Position();
    Vector x_stat = stat.Position();
    Vector xd = x_sat - x_stat;
    Vector vd = eci.Velocity() - stat.Velocity();

    // calculation of right ascension and declination for satellite
    double r = sqrt( xd.x * xd.x + xd.y * xd.y + xd.z * xd.z );
    double de = asin( xd.z / r );
    double ra;

    if(sqrt(xd.x*xd.x + xd.y*xd.y) < 0.00000001) {
        ra = atan2(vd.y,vd.x);
    }
    else {
        ra = atan2(xd.y,xd.x);
    }
    return make_pair( ra, de );
}
boost::posix_time::ptime Satellite::extractReferenceEpoch( const std::string& l1 ) {
    string datestr = l1.substr(18,14);
    int year = boost::lexical_cast<int>(datestr.substr(0,2));
    if (year < 80){
        year += 2000;
    }else{
        year += 1900;
    }
    auto frac_day = boost::lexical_cast<double>(datestr.substr(2));
    double frac_hours = fmod(frac_day,1)*24;
    double frac_min = fmod(frac_hours,1)*60;
    double frac_sec = fmod(frac_min,1)*60;

    boost::posix_time::ptime epoch(boost::gregorian::date( year, 1, 1), boost::posix_time::time_duration( int(frac_hours), int(frac_min), int(frac_sec) ));
    boost::gregorian::days off(int(frac_day)-1);
    epoch += off;

    return epoch;
}
void Satellite::toVex( std::ofstream& of ) const {
    string eol = ";\n";
    of << "    def " << getName() << eol;
    of << "        source_type = tle" << eol;
    of << "        tle0 = '" << header_ << "'" << eol;
    of << "        tle1 = '" << line1_ << "'" << eol;
    of << "        tle2 = '" << line2_ << "'" << eol;
    of << "    enddef;\n";
}

void Satellite::toVex( ofstream& of, const vector<unsigned int>& times,
                       const shared_ptr<const Position>& sta_pos ) const {
    string eol = ";\n";
    for ( unsigned int t : times ) {
        string name = getNameTime( t );
        auto rade = getRaDe( t, sta_pos );

        of << "    def " << name << eol;
        of << "        source_type = star" << eol;
        of << "        source_name = " << name << eol;
        of << "        ra = " << getRaString( rade.first ) << eol;
        of << "        dec = " << getDeString( rade.second ) << eol;
        of << "        ref_coord_frame = J2000" << eol;
        of << "        ra_rate = 0 asec/yr" << eol;
        of << "        dec_rate = 0 asec/yr" << eol;
        of << "    enddef;\n";
    }
}


void Satellite::toNgsHeader( ofstream& of ) const {
    string name = getName();
    std::replace( name.begin(), name.end(), ' ', '_' );

    of << "satellite " << name << "\n";
    of << "    " << header_ << "\n";
    of << "    " << line1_ << "\n";
    of << "    " << line2_ << "\n";
}

const std::tuple<boost::posix_time::ptime, SGP4, std::string>& Satellite::getSGP4( unsigned int epoch ) const {
    if ( pSGP4Data_.size() > 1 ) {
        boost::posix_time::ptime ref = TimeSystem::internalTime2PosixTime( epoch );
        long dt = numeric_limits<long>::max();
        int idx = 0;
        for ( int i = 0; i < pSGP4Data_.size(); ++i ) {
            const auto& any = pSGP4Data_[i];
            //            string x = TimeSystem::time2string(any.first);
            //            string y = TimeSystem::time2string(ref);

            long n = ( get<0>( any ) - ref ).total_seconds();
            n = abs( n );
            if ( n < dt ) {
                idx = i;
                dt = n;
            }
        }
        return pSGP4Data_[idx];
    } else {
        return pSGP4Data_[0];
    }
}
