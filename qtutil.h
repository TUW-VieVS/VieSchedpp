#ifndef QTUTIL_H
#define QTUTIL_H

#include <QList>
#include <tuple>
#include "../VieSchedpp/Scheduler.h"

namespace qtUtil {

    QList<std::tuple<int, double, double, int>> pointingVectors2Lists(const std::vector<VieVS::PointingVector> &pvs);

}

#endif // QTUTIL_H
