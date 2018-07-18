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
#include <QtCharts/QAreaSeries>
#include <QSignalMapper>

#include "VieSchedpp/Scheduler.h"
#include "qtutil.h"

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

    void skyCoverageChanged(QString name);

    void updateSkyCoverage(int idx, QString name);

private:
    Ui::VieSchedpp_Analyser *ui;

    VieVS::Scheduler schedule_;
    QDateTime sessionStart_;
    QDateTime sessionEnd_;

    QStandardItemModel *srcModel;
    QStandardItemModel *staModel;

//    QSignalMapper *comboBox2skyCoverage;

};

#endif // VIESCHEDPP_ANALYSER_H
