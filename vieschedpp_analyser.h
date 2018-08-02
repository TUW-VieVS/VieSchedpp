#ifndef VIESCHEDPP_ANALYSER_H
#define VIESCHEDPP_ANALYSER_H


#include <QMainWindow>
#include <QDateTime>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QStandardItemModel>
#include <QtMath>
#include <QGraphicsLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QPolarChart>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QAreaSeries>
#include <QSignalMapper>
#include <QSortFilterProxyModel>

#include "../VieSchedpp/Scheduler.h"
#include "qtutil.h"
#include "callout.h"

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class VieSchedpp_Analyser;
}

class VieSchedpp_Analyser : public QMainWindow
{
    Q_OBJECT

public:
    explicit VieSchedpp_Analyser(VieVS::Scheduler schedule, QDateTime start, QDateTime end, QWidget *parent = 0);
    ~VieSchedpp_Analyser();

    void setup();

private slots:
    void on_pushButton_skyCoverageLegend_clicked();

    void on_actiongeneral_triggered();

    void on_actionsky_coverage_triggered();

    void on_actionper_station_triggered();

    void on_actionper_source_triggered();

    void on_actionper_baseline_triggered();

    void on_actionworld_map_triggered();

    void on_actionsky_map_triggered();

    void on_actionuv_coverage_triggered();

    void on_horizontalSlider_start_valueChanged(int value);

    void on_horizontalSlider_end_valueChanged(int value);

    void on_dateTimeEdit_start_dateTimeChanged(const QDateTime &dateTime);

    void on_dateTimeEdit_end_dateTimeChanged(const QDateTime &dateTime);

    void on_spinBox_duration_valueChanged(int arg1);

    void updateDuration();

    void on_doubleSpinBox_hours_valueChanged(double arg1);

    void setSkyCoverageLayout(int rows, int columns);

    void on_pushButton_skyCoverageLayout_clicked();

    void updateSkyCoverage(QString name);

    void updateSkyCoverage(int idx, QString name);

    void updateSkyCoverageTimes();

    void updateSkyCoverageTimes(int idx);

    void skyCoverageHovered(QPointF point, bool flag);

    void on_lineEdit_skyCoverageSourceFilter_textChanged(const QString &arg1);

    void on_treeView_skyCoverage_sources_clicked(const QModelIndex &index);

    void setupWorldmap();

    void worldmap_hovered(QPointF point, bool state);

    void setupSkymap();

    void skymap_hovered(QPointF point, bool state);

private:
    Ui::VieSchedpp_Analyser *ui;

    VieVS::Scheduler schedule_;
    QDateTime sessionStart_;
    QDateTime sessionEnd_;

    QStandardItemModel *srcModel;
    QStandardItemModel *staModel;


//    QSignalMapper *comboBox2skyCoverage;

};

class QScatterSeriesExtended: public QScatterSeries{
public:

    QScatterSeriesExtended(QWidget *parent = 0) :
        QScatterSeries(parent){
    }

    void append(double x, double y, int startTime, int endTime,
                VieVS::CableWrap::CableWrapFlag cableWrapFlag, int srcid, int nsta){
        QScatterSeries::append(x,y);
        startTime_.append(startTime);
        endTime_.append(endTime);
        cableWrap_.append(cableWrapFlag);
        srcid_.append(srcid);
        nsta_.append(nsta);
    }

    int getStartTime(int idx){
        return startTime_.at(idx);
    }
    int getEndTime(int idx){
        return endTime_.at(idx);
    }

    int getSrcid(int idx){
        return srcid_.at(idx);
    }
    int getNSta(int idx){
        return nsta_.at(idx);
    }

    VieVS::CableWrap::CableWrapFlag getCableWrapFlag(int idx){
        return cableWrap_.at(idx);
    }

private:
    QVector<int> startTime_;
    QVector<int> endTime_;
    QVector<int> nsta_;
    QVector<int> srcid_;
    QVector<VieVS::CableWrap::CableWrapFlag> cableWrap_;
};

#endif // VIESCHEDPP_ANALYSER_H
