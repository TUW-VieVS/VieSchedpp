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

    return (boost::format("%02dh%02dm%06.3fs") %d %m %sf).str();

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

    return (boost::format("%+03dd%02dm%05.2fs") %h %m %sf).str();

}

double util::wrapToPi(double angle){
    angle = fmod(angle,twopi);
    if(angle<0){
        angle += twopi;
    }
    return angle;

}