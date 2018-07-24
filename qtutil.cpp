#include "qtutil.h"


QList<std::tuple<int, double, double, int>> qtUtil::pointingVectors2Lists(const std::vector<VieVS::PointingVector> &pvs)
{
    QList<std::tuple<int, double, double, int>> l;

    for(const VieVS::PointingVector &p : pvs){
        l.append({p.getTime(), p.getAz(), p.getEl(), p.getSrcid()});
    }

    return l;
}

QList<qtUtil::ObsData> qtUtil::getObsData(unsigned long staid, const std::vector<VieVS::Scan> &scans)
{
    QList<ObsData> list;
    for(const VieVS::Scan &scan: scans){
        auto oidx = scan.findIdxOfStationId(staid);
        if(oidx.is_initialized()){
            int idx = oidx.get();
            ObsData obs;
            obs.srcid = scan.getSourceId();
            obs.az = scan.getPointingVector(idx).getAz();
            obs.el = scan.getPointingVector(idx).getEl();
            obs.startTime = scan.getPointingVector(idx).getTime();
            obs.endTime = scan.getPointingVector(idx,VieVS::Timestamp::end).getTime();
            obs.nsta = scan.getNSta();
            list.append(obs);
        }
    }
    return list;
}
