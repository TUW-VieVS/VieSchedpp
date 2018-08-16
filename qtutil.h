#ifndef QTUTIL_H
#define QTUTIL_H

#include <QList>
#include <tuple>
#include <QtCharts/QLineSeries>
#include <QTextStream>
#include <QtMath>

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

    void skyMap(ChartView *skymap);

    std::pair<double,double> radec2xy(double ra, double dc);

    QList<QLineSeries *> baselineSeries(double lat1, double lon1, QString name1, double lat2, double lon2, QString name2);

}

#endif // QTUTIL_H
