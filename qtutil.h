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
