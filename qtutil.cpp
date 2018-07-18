#include "qtutil.h"


QList<std::tuple<int, double, double>> qtUtil::pointingVectors2Lists(const std::vector<VieVS::PointingVector> &pvs)
{
    QList<std::tuple<int, double, double>> l;

    for(const VieVS::PointingVector &p : pvs){
        l.append({p.getTime(), p.getAz(), p.getEl()});
    }

    return l;
}
