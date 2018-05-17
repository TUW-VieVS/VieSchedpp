//
// Created by mschartn on 25.04.18.
//

#include "Miscellaneous.h"
using namespace VieVS;
using namespace std;

std::string Miscellaneous::ra2dms(double angle) noexcept{
    double af = angle*rad2deg/15;
    double d = floor(af);
    double mf = (af - d)*60;
    double m = floor(mf);
    double sf = (mf - m)*60;

    return (boost::format("%02dd%02dm%06.3fs") %d %m %sf).str();
}

std::string Miscellaneous::dc2hms(double angle) noexcept{
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

    return (boost::format("%+03dh%02dm%05.2fs") %h %m %sf).str();
}

double Miscellaneous::wrapToPi(double angle) noexcept{
    angle = fmod(angle,twopi);
    if(angle<0){
        angle += twopi;
    }
    return angle;
}
