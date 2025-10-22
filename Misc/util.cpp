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

#include "util.h"


using namespace std;
using namespace VieVS;
namespace fs = std::filesystem;


string util::ra2dms( double angle ) {
    double af = angle * rad2deg / 15;
    double d = floor( af );
    double mf = ( af - d ) * 60;
    double m = floor( mf );
    double sf = ( mf - m ) * 60;

    return ( boost::format( "%02dh %02dm %05.2fs" ) % d % m % sf ).str();
}


std::string util::ra2dms_astFormat( double angle ) {
    double af = angle * rad2deg / 15;
    double d = floor( af );
    double mf = ( af - d ) * 60;
    double m = floor( mf );
    double sf = ( mf - m ) * 60;

    return ( boost::format( "%02d:%02d:%09.6f" ) % d % m % sf ).str();
}


string util::dc2hms( double angle ) {
    double af = angle * rad2deg;
    bool positive = true;
    if ( af < 0 ) {
        positive = false;
    }
    af = abs( af );
    double h = floor( af );
    double mf = ( af - h ) * 60;
    double m = floor( mf );
    double sf = ( mf - m ) * 60;
    if ( !positive ) {
        h *= -1;
    }

    return ( boost::format( "%+03d° %02d' %05.2f\"" ) % h % m % sf ).str();
}


string util::dc2hms_astFormat( double angle ) {
    double af = angle * rad2deg;
    bool positive = true;
    if ( af < 0 ) {
        positive = false;
    }
    af = abs( af );
    double h = floor( af );
    double mf = ( af - h ) * 60;
    double m = floor( mf );
    double sf = ( mf - m ) * 60;
    if ( !positive ) {
        h *= -1;
    }

    return ( boost::format( "%+03d:%02d:%9.6f\"" ) % h % m % sf ).str();
}


double util::wrap2twoPi( double angle ) {
    angle = fmod( angle, twopi );
    if ( angle < 0 ) {
        angle += twopi;
    }
    return angle;
}


double util::wrap2pi( double angle ) {
    angle = fmod( angle, pi );
    return angle;
}


int util::duration( const boost::posix_time::ptime &start, const boost::posix_time::ptime &end ) {
    boost::posix_time::time_duration a = end - start;
    return static_cast<int>( a.total_seconds() );
}


void util::outputObjectList( const std::string &title, const std::vector<std::string> &names, std::ofstream &of,
                             unsigned long indents ) {
    string indent = string( indents, ' ' );

    if ( !names.empty() ) {
        int longest = 0;
        for ( const auto &any : names ) {
            if ( any.size() > longest ) {
                longest = static_cast<int>( any.size() );
            }
        }

        unsigned long n = 0;
        if ( longest != 0 ) {
            n = ( 100 - indents ) / longest;
        }
        string format = ( boost::format( "%%-%ds " ) % longest ).str();

        of << title << ": (" << names.size() << ")\n" << indent;
        for ( int i = 0; i < names.size(); ++i ) {
            if ( i % n == 0 && i != 0 ) {
                of << "\n" << indent;
            }
            of << boost::format( format ) % names[i];
        }
        of << "\n";
    }
}


string util::version() {
    string v;
    v = string( VIESCHEDPP_VERSION );

    return v;
}


double util::freqency2wavelenth( double frequency ) { return speedOfLight / frequency; }


double util::wavelength2frequency( double wavelength ) { return speedOfLight / wavelength; }


unsigned long util::getNumberOfStations( const boost::property_tree::ptree &xml ) {
    unsigned long nsta = 0;

    auto ptree_stations = xml.get_child_optional( "VieSchedpp.general.stations" );
    if ( ptree_stations.is_initialized() ) {
        nsta = distance( ptree_stations->begin(), ptree_stations->end() );
    }

    return nsta;
}


std::vector<std::string> util::getStationNames( const boost::property_tree::ptree &xml ) {
    vector<string> names;
    auto ptree_stations = xml.get_child_optional( "VieSchedpp.general.stations" );
    if ( ptree_stations.is_initialized() ) {
        auto it = ptree_stations->begin();
        while ( it != ptree_stations->end() ) {
            auto item = it->second.data();
            names.push_back( item );
            ++it;
        }
    }
    sort(names.begin(), names.end());

    return names;
}


std::string util::weekDay2string( int weekday ) {
    string wd;
    switch ( weekday ) {
        case 0: {
            wd = "SUN";
            break;
        }
        case 1: {
            wd = "MON";
            break;
        }
        case 2: {
            wd = "TUE";
            break;
        }
        case 3: {
            wd = "WED";
            break;
        }
        case 4: {
            wd = "THU";
            break;
        }
        case 5: {
            wd = "FRI";
            break;
        }
        case 6: {
            wd = "SAT";
            break;
        }
        default: {
            wd = "   ";
            break;
        };
    }
    return wd;
}


std::string util::month2string( int month ) {
    string monthStr;
    switch ( month ) {
        case 1: {
            monthStr = "JAN";
            break;
        }
        case 2: {
            monthStr = "FEB";
            break;
        }
        case 3: {
            monthStr = "MAR";
            break;
        }
        case 4: {
            monthStr = "APR";
            break;
        }
        case 5: {
            monthStr = "MAY";
            break;
        }
        case 6: {
            monthStr = "JUN";
            break;
        }
        case 7: {
            monthStr = "JUL";
            break;
        }
        case 8: {
            monthStr = "AUG";
            break;
        }
        case 9: {
            monthStr = "SEP";
            break;
        }
        case 10: {
            monthStr = "OCT";
            break;
        }
        case 11: {
            monthStr = "NOV";
            break;
        }
        case 12: {
            monthStr = "DEC";
            break;
        }
        default: {
            monthStr = "   ";
            break;
        };
    }
    return monthStr;
}
char util::numberOfScans2char( long n ) {
    if ( n == 0 ) {
        return ' ';
    } else if ( n < 10 ) {
        return to_string( n )[0];
    } else if ( n == 10 ) {
        return '0';
    } else if ( n < 37 ) {
        return static_cast<char>( 'A' + n - 11 );
    } else if ( n < 63 ) {
        return static_cast<char>( 'a' + n - 37 );
    } else {
        return '#';
    }
}
std::string util::numberOfScans2char_header() {
    std::string header = "#scans -> char: 1-9 -> '1'-'9'; 10 -> '0'; 11-36 -> 'A'-'Z'; 37-62 -> 'a'-'z'; 63+ -> '#'";
    return header;
}

void util::simplify_inline( string &str ) {
    std::string::iterator new_end =
        std::unique( str.begin(), str.end(), []( char lhs, char rhs ) { return ( lhs == rhs ) && ( lhs == ' ' ); } );
    str.erase( new_end, str.end() );

    if ( str[0] == ' ' ) {
        str.erase( 0 );
    }

    if ( str[str.size() - 1] == ' ' ) {
        str.erase( str.size() - 1 );
    }
}
std::string util::simplify( const string &str ) {
    string copy = str;
    std::string::iterator new_end =
        std::unique( copy.begin(), copy.end(), []( char lhs, char rhs ) { return ( lhs == rhs ) && ( lhs == ' ' ); } );
    copy.erase( new_end, copy.end() );

    if ( copy[0] == ' ' ) {
        copy.erase( 0 );
    }

    if ( copy[copy.size() - 1] == ' ' ) {
        copy.erase( copy.size() - 1 );
    }

    return copy;
}

std::string util::milliseconds2string( long long int usec, bool forceSeconds ) {
    if ( forceSeconds ) {
        auto seconds = static_cast<double>( usec ) / 1000. / 1000.;
        return ( boost::format( "%.3f sec" ) % seconds ).str();
    }

    auto milliseconds = usec / 1000 % 1000;
    auto seconds = usec / 1000 / 1000 % 60;
    auto minutes = usec / 1000 / 1000 / 60 % 60;
    auto hours = usec / 1000 / 1000 / 60 / 60;
    std::stringstream t;
    if ( hours > 0 ) {
        t << hours << "h ";
    }
    if ( minutes > 0 || hours > 0 ) {
        t << minutes << "min ";
    }
    if ( seconds > 0 || minutes > 0 || hours > 0 ) {
        t << seconds << "sec ";
    }
    t << milliseconds << "msec";

    return t.str();
}

std::string util::version2prefix(int version) {
    if (version > 0) {
        return (boost::format("version %d: ") % version).str();
    } else {
        return "";
    }
}

#ifdef COMPRESSION_ENABLED
void util::compress( const string& path, const string &fname ) {
    // Construct version pattern for filtering and archive naming
    std::string versionPattern;

    std::string zipName = "schedule_" + fname;
    zipName += ".zip";

    fs::path zipPath = fs::path(path) / zipName;

    mz_zip_archive zipArchive;
    memset(&zipArchive, 0, sizeof(zipArchive));

    if (!mz_zip_writer_init_file(&zipArchive, zipPath.string().c_str(), 0)) {
        return;
    }

    std::vector<fs::path> filesToRemove;

    for (const auto& entry : fs::directory_iterator(path)) {
        if (!entry.is_regular_file()) continue;

        const auto& filepath = entry.path();
        std::string filename = filepath.filename().string();

        if (filename.find(fname) != 0) continue;

        if (mz_zip_writer_add_file(&zipArchive, filename.c_str(), filepath.string().c_str(), nullptr, 0, MZ_BEST_COMPRESSION)) {
            filesToRemove.push_back(filepath);  // mark for removal
        }
    }

    if (mz_zip_writer_finalize_archive(&zipArchive)) {
        for (const auto& file : filesToRemove) {
            std::error_code ec;
            if (!fs::remove(file, ec)) {
                std::cerr << "Failed to remove file: " << file << " (" << ec.message() << ")\n";
            } else {
                std::cout << "Removed original file: " << file << "\n";
            }
        }
    }
    mz_zip_writer_end(&zipArchive);
}
#endif
