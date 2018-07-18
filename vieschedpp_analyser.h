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

#include "VieSchedpp/Scheduler.h"
#include "qtutil.h"

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

    void append(int time, double x, double y, VieVS::CableWrap::CableWrapFlag cableWrapFlag, int srcid){
        QScatterSeries::append(x,y);
        time_.append(time);
        srcid_.append(srcid);
        cableWrap_.append(cableWrapFlag);
    }

    int getTime(int idx){
        return time_.at(idx);
    }

    int getSrcid(int idx){
        return srcid_.at(idx);
    }

    VieVS::CableWrap::CableWrapFlag getCableWrapFlag(int idx){
        return cableWrap_.at(idx);
    }

private:
    QVector<int> time_;
    QVector<int> srcid_;
    QVector<VieVS::CableWrap::CableWrapFlag> cableWrap_;
};

#endif // VIESCHEDPP_ANALYSER_H
