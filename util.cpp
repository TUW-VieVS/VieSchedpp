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

string util::ra2dms(double angle){
    double af = angle*rad2deg/15;
    double d = floor(af);
    double mf = (af - d)*60;
    double m = floor(mf);
    double sf = (mf - m)*60;

    return (boost::format("%02dh %02dm %05.2fs") %d %m %sf).str();

}

string util::dc2hms(double angle){
    double af = angle*rad2deg;
    bool positive = true;
    if( af < 0){
        positive = false;
    }
    af = abs(af);
    double h = floor(af);
    double mf = (af - h)*60;
    double m = floor(mf);
    double sf = (mf - m)*60;
    if(!positive){
        h*=-1;
    }

    return (boost::format("%+03d° %02d' %05.2f\"") %h %m %sf).str();

}

double util::wrapToPi(double angle){
    angle = fmod(angle,twopi);
    if(angle<0){
        angle += twopi;
    }
    return angle;

}

int util::duration(const boost::posix_time::ptime &start, const boost::posix_time::ptime &end) {
    boost::posix_time::time_duration a = end - start;
    return static_cast<int>(a.total_seconds());
}

void util::outputObjectList(const std::string &title, const std::vector<std::string> &names, std::ofstream &of) {

    if(!names.empty()){

        int longest = 0;
        for(const auto&any:names){
            if(any.size()>longest){
                longest = static_cast<int>(any.size());
            }
        }

        int n=0;
        if(longest != 0){
            n = 100/longest;
        }
        string format = (boost::format("%%%ds ")%longest).str();

        of << title << ": (" << names.size() << ")\n    ";
        for(int i=0; i<names.size(); ++i){
            if(i%n==0 && i!=0 ){
                of << "\n    ";
            }
            of << boost::format(format)%names[i];
        }
        of << "\n\n";
    }

}


string util::version() {
    string v;
    v = string(GIT_COMMIT_HASH);

    return v;
}


double util::freqency2wavelenth( double frequency ){
    return speedOfLight/frequency;
}

double util::wavelength2frequency( double wavelength ){
    return speedOfLight/wavelength;
}