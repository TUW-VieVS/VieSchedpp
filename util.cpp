//
// Created by matth on 06.05.2018.
//
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
    return a.total_seconds();
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
            n = 80/longest;
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
