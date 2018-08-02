#ifndef QTUTIL_H
#define QTUTIL_H

#include <QList>
#include <tuple>
#include <QtCharts/QLineSeries>

#include "../VieSchedpp/Scheduler.h"
#include "chartview.h"

namespace qtUtil {

    struct ObsData{
        int srcid;
        double az;
        double el;
        int startTime;
        int endTime;
        int nsta;
    };

    QList<ObsData> getObsData(unsigned long staid, const std::vector<VieVS::Scan> &scans);

    QList<std::tuple<int, double, double, int>> pointingVectors2Lists(const std::vector<VieVS::PointingVector> &pvs);

    void worldMap(ChartView *worldmap);

}

#endif // QTUTIL_H
